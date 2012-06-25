#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

class AudioPlayer {
public:
	virtual bool init(unsigned int rate, int bits, int channels) = 0;
	virtual void destroy() = 0;

	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void pause() = 0;
	virtual void play(void *data, int size) = 0;
};

#endif

