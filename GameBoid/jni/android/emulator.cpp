#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>

#define LOG_TAG "libgba"
#include <utils/Log.h>
#include <nativehelper/JNIHelp.h>
#include <nativehelper/jni.h>
#include "audioplayer.h"
extern "C" {
#include "common.h"
#include "main.h"
}

#define SCREEN_W		240
#define SCREEN_H		160
#define SCREEN_PITCH	(SCREEN_W * 2)

enum {
	EMUSTATE_RUNNING,
	EMUSTATE_PAUSED,
	EMUSTATE_REQUEST_PAUSE,
	EMUSTATE_REQUEST_RUN,
	EMUSTATE_QUIT
};

extern u16 *screen;

static pthread_mutex_t emuStateMutex;
static pthread_cond_t emuStateCond;
static int emuState = EMUSTATE_PAUSED;
static int resumeRestart;

static bool resumeRequested;
static unsigned int keyStates;
static bool initialized;
static bool romLoaded;
static AudioPlayer *audioPlayer;

static JNIEnv *jEnv;
static jobject renderSurface;
static int surfaceWidth, surfaceHeight;
static jintArray jImage;
static jmethodID jSendImageMethod;

static AudioPlayer *loadAudioPlayer(const char *libdir)
{
	static const char *const so_names[] = {
		"emusound",
		"emusound2",
	};

	void *handle = NULL;
	for (int i = 0; i < NELEM(so_names); i++) {
		char path[1024];
		snprintf(path, sizeof(path), "%s/lib%s.so", libdir, so_names[i]);
		handle = dlopen(path, RTLD_NOW);
		if (handle != NULL)
			break;

		LOGD("Cannot load %s: %s", path, dlerror());
	}
	if (handle == NULL)
		return NULL;

	AudioPlayer *(*createPlayer)() = (AudioPlayer *(*)())
			dlsym(handle, "createPlayer");
	if (createPlayer == NULL) {
		dlclose(handle);
		return NULL;
	}
	return createPlayer();
}

static void pauseEmulator()
{
	pthread_mutex_lock(&emuStateMutex);
	if (emuState == EMUSTATE_RUNNING) {
		emuState = EMUSTATE_REQUEST_PAUSE;
		while (emuState == EMUSTATE_REQUEST_PAUSE)
			pthread_cond_wait(&emuStateCond, &emuStateMutex);
	}
	pthread_mutex_unlock(&emuStateMutex);

	if (global_enable_audio && audioPlayer != NULL)
		audioPlayer->pause();
}

static void resumeEmulator(int restart = 0)
{
	resumeRestart |= restart;

	if (!resumeRequested || renderSurface == NULL || !romLoaded)
		return;

	pthread_mutex_lock(&emuStateMutex);
	if (emuState == EMUSTATE_PAUSED) {
		emuState = EMUSTATE_REQUEST_RUN;
		pthread_cond_signal(&emuStateCond);

		while (emuState == EMUSTATE_REQUEST_RUN)
			pthread_cond_wait(&emuStateCond, &emuStateMutex);
	}
	pthread_mutex_unlock(&emuStateMutex);
}

static void unloadROM()
{
	if (!romLoaded)
		return;

	pauseEmulator();

	if (audioPlayer != NULL)
		audioPlayer->stop();

	romLoaded = false;
}

static int waitForStart()
{
	pthread_mutex_lock(&emuStateMutex);

	while (emuState == EMUSTATE_PAUSED)
		pthread_cond_wait(&emuStateCond, &emuStateMutex);

	if (emuState == EMUSTATE_REQUEST_RUN) {
		emuState = EMUSTATE_RUNNING;
		pthread_cond_signal(&emuStateCond);
	}
	pthread_mutex_unlock(&emuStateMutex);

	if (global_enable_audio && audioPlayer != NULL)
		audioPlayer->start();

	int rv = resumeRestart;
	resumeRestart = 0;
	return rv;
}

static bool gpspInitialize()
{
	screen = (u16 *) malloc(SCREEN_PITCH * SCREEN_H);

	init_gamepak_buffer();

	init_main();
	init_sound();
	init_input();

	get_ticks_us(&frame_count_initial_timestamp);
	return true;
}

extern "C" int check_paused()
{
	if (emuState == EMUSTATE_RUNNING)
		return 0;

	pthread_mutex_lock(&emuStateMutex);
	if (emuState == EMUSTATE_REQUEST_PAUSE) {
		emuState = EMUSTATE_PAUSED;
		pthread_cond_signal(&emuStateCond);
	}
	pthread_mutex_unlock(&emuStateMutex);

	return waitForStart();
}

extern "C" u32 get_key_states()
{
	return keyStates;
}

extern "C" void flip_screen()
{
	jsize size = jEnv->GetArrayLength(jImage);
	jint *image = jEnv->GetIntArrayElements(jImage, 0);
	for (int i = 0; i < size; i++) {
		unsigned short pix = screen[i];
		image[i] = (pix & 0xf800) << 8 |
				(pix & 0x07e0) << 5 |
				(pix & 0x1f) << 3;
	}
	jEnv->ReleaseIntArrayElements(jImage, image, 0);
	jEnv->CallVoidMethod(renderSurface, jSendImageMethod, jImage);
}

extern "C" void render_audio(s16 *data, u32 size)
{
	if (audioPlayer != NULL)
		audioPlayer->play(data, size * 2);
}

static jboolean
Emulator_initialize(JNIEnv *env, jobject self,
		jstring jlibdir, jstring jdatadir)
{
	resumeRequested = false;

	const char *libdir = env->GetStringUTFChars(jlibdir, NULL);
	audioPlayer = loadAudioPlayer(libdir);
	env->ReleaseStringUTFChars(jlibdir, libdir);

	const char *datadir = env->GetStringUTFChars(jdatadir, NULL);
	strcpy((char *) main_path, datadir);
	env->ReleaseStringUTFChars(jdatadir, datadir);

	LOGW_IF(audioPlayer == NULL, "Cannot initialize sound module");
	if (audioPlayer != NULL)
		audioPlayer->init(44100, 16, 2);

	if (initialized)
		return JNI_TRUE;

	gpspInitialize();

	initialized = true;
	return JNI_TRUE;
}

static void Emulator_cleanUp(JNIEnv *env, jobject self)
{
	unloadROM();

	if (audioPlayer != NULL) {
		audioPlayer->destroy();
		audioPlayer = NULL;
	}
}

static void
Emulator_setRenderSurface(JNIEnv *env, jobject self,
		jobject surface, int width, int height)
{
	pauseEmulator();

	if (renderSurface != NULL) {
		env->DeleteGlobalRef(jImage);
		jImage = NULL;
		env->DeleteGlobalRef(renderSurface);
		renderSurface = NULL;
	}

	if (surface != NULL) {
		surfaceWidth = width;
		surfaceHeight = height;

		jImage = env->NewIntArray(width * height);
		jImage = (jintArray) env->NewGlobalRef(jImage);

		renderSurface = env->NewGlobalRef(surface);
		jclass cls = env->GetObjectClass(surface);
		jSendImageMethod = env->GetMethodID(cls, "onImageUpdate", "([I)V");

		resumeEmulator();
	}
}

static void
Emulator_setKeyStates(JNIEnv *env, jobject self, jint states)
{
	keyStates = states;
}

static void
Emulator_setOption(JNIEnv *env, jobject self, jstring jname, jstring jvalue)
{
	const char *name = env->GetStringUTFChars(jname, NULL);
	const char *value = env->GetStringUTFChars(jvalue, NULL);

	if (strcmp(name, "autoFrameSkip") == 0) {
		current_frameskip_type = (strcmp(value, "false") != 0 ?
			auto_frameskip : manual_frameskip);

	} else if (strcmp(name, "maxFrameSkips") == 0) {
		frameskip_value = atoi(value);
		if (frameskip_value < 2)
			frameskip_value = 2;
		else if (frameskip_value > 99)
			frameskip_value = 99;

	} else if (strcmp(name, "soundEnabled") == 0) {
		global_enable_audio = (strcmp(value, "false") != 0);
	}

	env->ReleaseStringUTFChars(jname, name);
	env->ReleaseStringUTFChars(jvalue, value);
}

static void Emulator_reset(JNIEnv *env, jobject self)
{
	pauseEmulator();
	reset_gba();
	reg[CHANGED_PC_STATUS] = 1;
	resumeEmulator(1);
}

static void Emulator_power(JNIEnv *env, jobject self)
{
	Emulator_reset(env, self);
}

static jboolean Emulator_loadBIOS(JNIEnv *env, jobject self, jstring jfile)
{
	const char *file = env->GetStringUTFChars(jfile, NULL);
	int rv = load_bios(file);
	env->ReleaseStringUTFChars(jfile, file);

	if (rv == -1) {
		LOGE("cannot load BIOS: %s", file);
		return JNI_FALSE;
	}
	if (bios_rom[0] != 0x18) {
		LOGE("BIOS is incorrect: %s", file);
		return JNI_FALSE;
	}
	return JNI_TRUE;
}

static jboolean Emulator_loadROM(JNIEnv *env, jobject self, jstring jfile)
{
	unloadROM();

	const char *file = env->GetStringUTFChars(jfile, NULL);
	jboolean rv = JNI_FALSE;

	if (load_gamepak(file) == -1)
		goto error;

	reset_gba();
	reg[CHANGED_PC_STATUS] = 1;
	romLoaded = true;

	resumeEmulator(1);
	rv = JNI_TRUE;
error:
	env->ReleaseStringUTFChars(jfile, file);
	return rv;
}

static void Emulator_unloadROM(JNIEnv *env, jobject self)
{
	unloadROM();
}

static void Emulator_pause(JNIEnv *env, jobject self)
{
	resumeRequested = false;
	pauseEmulator();
}

static void Emulator_resume(JNIEnv *env, jobject self)
{
	resumeRequested = true;
	resumeEmulator();
}

static jboolean Emulator_saveState(JNIEnv *env, jobject self, jstring jfile)
{
	const char *file = env->GetStringUTFChars(jfile, NULL);

	pauseEmulator();
	save_state(file);
	resumeEmulator();

	env->ReleaseStringUTFChars(jfile, file);
	return JNI_TRUE;
}

static jboolean Emulator_loadState(JNIEnv *env, jobject self, jstring jfile)
{
	const char *file = env->GetStringUTFChars(jfile, NULL);

	pauseEmulator();
	load_state(file);
	resumeEmulator(1);

	env->ReleaseStringUTFChars(jfile, file);
	return JNI_TRUE;
}

static void Emulator_run(JNIEnv *env, jobject self)
{
	jEnv = env;

	pthread_mutex_init(&emuStateMutex, NULL);
	pthread_cond_init(&emuStateCond, NULL);

	waitForStart();
	execute_arm_translate(execute_cycles);
}

int register_Emulator(JNIEnv *env)
{
	static const JNINativeMethod methods[] = {
		{ "setRenderSurface", "(Lcom/androidemu/EmulatorView;II)V",
				(void *) Emulator_setRenderSurface },
		{ "setKeyStates", "(I)V",
				(void *) Emulator_setKeyStates },
		{ "setOption", "(Ljava/lang/String;Ljava/lang/String;)V",
				(void *) Emulator_setOption },

		{ "initialize", "(Ljava/lang/String;Ljava/lang/String;)Z",
				(void *) Emulator_initialize },
		{ "cleanUp", "()V", (void *) Emulator_cleanUp },
		{ "reset", "()V", (void *) Emulator_reset },
		{ "power", "()V", (void *) Emulator_power },
		{ "loadBIOS", "(Ljava/lang/String;)Z", (void *) Emulator_loadBIOS },
		{ "loadROM", "(Ljava/lang/String;)Z", (void *) Emulator_loadROM },
		{ "unloadROM", "()V", (void *) Emulator_unloadROM },
		{ "pause", "()V", (void *) Emulator_pause },
		{ "resume", "()V", (void *) Emulator_resume },
		{ "run", "()V", (void *) Emulator_run },
		{ "saveState", "(Ljava/lang/String;)Z", (void *) Emulator_saveState },
		{ "loadState", "(Ljava/lang/String;)Z", (void *) Emulator_loadState },
	};

	return jniRegisterNativeMethods(env, "com/androidemu/Emulator",
			methods, NELEM(methods));
}
