#include <stdlib.h>
#include "emumedia.h"

class EmuMediaImpl : public EmuMedia {
public:
	virtual bool init(JNIEnv *env);
	virtual void destroy(JNIEnv *env);

	virtual void setSurface(JNIEnv *env, jobject holder);
	virtual void setSurfaceRegion(JNIEnv *env, int x, int y, int w, int h);
	virtual bool lockSurface(JNIEnv *env,
			EmuEngine::Surface *info, bool flip);
	virtual void unlockSurface(JNIEnv *env);

	virtual bool audioCreate(JNIEnv *env,
			unsigned int rate, int bits, int channels);
	virtual void audioSetVolume(JNIEnv *env, int volume);
	virtual void audioStart(JNIEnv *env);
	virtual void audioStop(JNIEnv *env);
	virtual void audioPause(JNIEnv *env);
	virtual void audioPlay(JNIEnv *env, void *data, int size);

private:
	void releaseVideoBuffer(JNIEnv *env);

	jclass jPeerClass;
	jmethodID midDestroy;
	jmethodID midSetSurface;
	jmethodID midSetSurfaceRegion;
	jmethodID midBitBlt;
	jmethodID midAudioCreate;
	jmethodID midAudioSetVolume;
	jmethodID midAudioStart;
	jmethodID midAudioStop;
	jmethodID midAudioPause;
	jmethodID midAudioPlay;

	jintArray jVideoBuffer;
	jbyteArray jAudioBuffer;
	unsigned short *screen;
	int screenWidth;
	int screenHeight;
	bool flipScreen;
};


void EmuMediaImpl::releaseVideoBuffer(JNIEnv *env)
{
	if (jVideoBuffer != NULL) {
		env->DeleteGlobalRef(jVideoBuffer);
		jVideoBuffer = NULL;
	}
	if (screen != NULL) {
		free(screen);
		screen = NULL;
	}
}

bool EmuMediaImpl::init(JNIEnv *env)
{
	jVideoBuffer = NULL;
	screen = NULL;
	flipScreen = false;

	jPeerClass = env->FindClass("com/androidemu/EmuMedia");
	jPeerClass = (jclass) env->NewGlobalRef(jPeerClass);

	jAudioBuffer = env->NewByteArray(8192);
	jAudioBuffer = (jbyteArray) env->NewGlobalRef(jAudioBuffer);

	midDestroy = env->GetStaticMethodID(jPeerClass, "destroy", "()V");
	midAudioCreate = env->GetStaticMethodID(jPeerClass,
			"audioCreate", "(III)Z");
	midAudioSetVolume = env->GetStaticMethodID(jPeerClass,
			"audioSetVolume", "(I)V");
	midAudioStart = env->GetStaticMethodID(jPeerClass, "audioStart", "()V");
	midAudioStop = env->GetStaticMethodID(jPeerClass, "audioStop", "()V");
	midAudioPause = env->GetStaticMethodID(jPeerClass, "audioPause", "()V");
	midAudioPlay = env->GetStaticMethodID(jPeerClass, "audioPlay", "([BI)V");

	return true;
}

void EmuMediaImpl::destroy(JNIEnv *env)
{
	env->CallStaticVoidMethod(jPeerClass, midDestroy);
	env->DeleteGlobalRef(jAudioBuffer);
	env->DeleteGlobalRef(jPeerClass);

	releaseVideoBuffer(env);
	delete this;
}

void EmuMediaImpl::setSurface(JNIEnv *env, jobject holder)
{
	env->CallStaticVoidMethod(jPeerClass, midSetSurface, holder);

	if (holder == NULL)
		releaseVideoBuffer(env);
}

void EmuMediaImpl::setSurfaceRegion(JNIEnv *env,
		int x, int y, int w, int h)
{
	releaseVideoBuffer(env);

	screenWidth = w;
	screenHeight = h;

	env->CallStaticIntMethod(jPeerClass, midSetSurfaceRegion, x, y, w, h);

	screen = new unsigned short[w * h];
	jVideoBuffer = env->NewIntArray(w * h);
	jVideoBuffer = (jintArray) env->NewGlobalRef(jVideoBuffer);
}

bool EmuMediaImpl::lockSurface(JNIEnv *env,
		EmuEngine::Surface *info, bool flip)
{
	flipScreen = flip;

	info->bits = screen;
	info->bpr = screenWidth * 2;
	info->w = screenWidth;
	info->h = screenHeight;
	return true;
}

void EmuMediaImpl::unlockSurface(JNIEnv *env)
{
	jsize size = env->GetArrayLength(jVideoBuffer);
	jint *image = env->GetIntArrayElements(jVideoBuffer, 0);
	for (int i = 0; i < size; i++) {
		unsigned short pix = screen[i];
		image[i] = (pix & 0xf800) << 8 |
			    (pix & 0x07e0) << 5 |
				(pix & 0x1f) << 3;
	}
	env->ReleaseIntArrayElements(jVideoBuffer, image, 0);

	env->CallStaticVoidMethod(jPeerClass, midBitBlt,
			jVideoBuffer, flipScreen);
}

bool EmuMediaImpl::audioCreate(JNIEnv *env,
		unsigned int rate, int bits, int channels)
{
	jboolean rv = env->CallStaticBooleanMethod(jPeerClass,
			midAudioCreate, rate, bits, channels);
	return (rv != JNI_FALSE);
}

void EmuMediaImpl::audioSetVolume(JNIEnv *env, int volume)
{
	env->CallStaticVoidMethod(jPeerClass, midAudioSetVolume, volume);
}

void EmuMediaImpl::audioStart(JNIEnv *env)
{
	env->CallStaticVoidMethod(jPeerClass, midAudioStart);
}

void EmuMediaImpl::audioStop(JNIEnv *env)
{
	env->CallStaticVoidMethod(jPeerClass, midAudioStop);
}

void EmuMediaImpl::audioPause(JNIEnv *env)
{
	env->CallStaticVoidMethod(jPeerClass, midAudioPause);
}

void EmuMediaImpl::audioPlay(JNIEnv *env, void *src, int size)
{
	env->SetByteArrayRegion(jAudioBuffer, 0, size, (jbyte *) src);
	env->CallStaticVoidMethod(jPeerClass, midAudioPlay, jAudioBuffer, size);
}


EmuMedia *createEmuMedia()
{
	return new EmuMediaImpl;
}
