#include <media/AudioTrack.h>
#include "audioplayer.h"

using namespace android;

class AudioPlayerImpl : public AudioPlayer {
public:
	AudioPlayerImpl();
	virtual ~AudioPlayerImpl();

	virtual bool init(unsigned int rate, int bits, int channels);
	virtual void destroy();
	virtual void start();
	virtual void stop();
	virtual void pause();
	virtual void play(void *data, int size);

private:
	AudioTrack *track;
};

AudioPlayerImpl::AudioPlayerImpl()
		: track(NULL)
{
}

AudioPlayerImpl::~AudioPlayerImpl()
{
	if (track != NULL)
		delete track;
}

bool AudioPlayerImpl::init(unsigned int rate, int bits, int channels)
{
	int format = (bits == 16 ?
			AudioSystem::PCM_16_BIT : AudioSystem::PCM_8_BIT);

	if (track != NULL) {
		if (track->getSampleRate() == rate &&
			track->format() == format &&
			track->channelCount() == channels)
			return true;
		delete track;
	}

	track = new AudioTrack(AudioSystem::MUSIC, rate, format, channels, 0);
	if (track->initCheck() < 0) {
		delete track;
		track = NULL;
		return false;
	}
	return true;
}

void AudioPlayerImpl::destroy()
{
	delete this;
}

void AudioPlayerImpl::start()
{
	if (track != NULL)
		track->start();
}

void AudioPlayerImpl::stop()
{
	if (track != NULL) {
		track->stop();
		track->flush();
	}
}

void AudioPlayerImpl::pause()
{
	if (track != NULL)
		track->pause();
}

void AudioPlayerImpl::play(void *data, int size)
{
	if (track != NULL)
		track->write(data, size);
}


extern "C" __attribute__((visibility("default")))
AudioPlayer *createPlayer()
{
	return new AudioPlayerImpl;
}

