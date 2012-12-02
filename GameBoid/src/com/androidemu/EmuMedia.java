package com.androidemu;

import android.graphics.Rect;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.view.SurfaceHolder;

public class EmuMedia
{
	private static SurfaceHolder holder;
	private static Rect region = new Rect();
	private static AudioTrack track;
	private static float volume = AudioTrack.getMaxVolume();

	static void destroy()
	{
		if (track != null)
		{
			track.stop();
			track = null;
		}
	}

	static void setSurface(SurfaceHolder h)
	{
		holder = h;
	}

	static void setSurfaceRegion(int x, int y, int w, int h)
	{
		region.set(x, y, x + w, y + h);
	}

	static boolean audioCreate(int rate, int bits, int channels)
	{
		int format = (bits == 16 ? AudioFormat.ENCODING_PCM_16BIT
				: AudioFormat.ENCODING_PCM_8BIT);
		int channelConfig = (channels == 2 ? AudioFormat.CHANNEL_CONFIGURATION_STEREO
				: AudioFormat.CHANNEL_CONFIGURATION_MONO);

		// avoid recreation if no parameters change
		if (track != null && track.getSampleRate() == rate
				&& track.getAudioFormat() == format
				&& track.getChannelCount() == channels) return true;

		int bufferSize = AudioTrack.getMinBufferSize(rate, channelConfig, format) * 2;
		if (bufferSize < 1500) bufferSize = 1500;

		try
		{
			track = new AudioTrack(AudioManager.STREAM_MUSIC, rate, channelConfig,
					format, bufferSize, AudioTrack.MODE_STREAM);

			if (track.getState() == AudioTrack.STATE_UNINITIALIZED) track = null;

		}
		catch (IllegalArgumentException e)
		{
			track = null;
		}
		if (track == null) return false;

		track.setStereoVolume(volume, volume);
		return true;
	}

	static void audioSetVolume(int vol)
	{
		final float min = AudioTrack.getMinVolume();
		final float max = AudioTrack.getMaxVolume();
		volume = min + (max - min) * vol / 100;

		if (track != null) track.setStereoVolume(volume, volume);
	}

	static void audioDestroy()
	{
		if (track != null)
		{
			track.stop();
			track = null;
		}
	}

	static void audioStart()
	{
		if (track != null) track.play();
	}

	static void audioStop()
	{
		if (track != null)
		{
			track.stop();
			track.flush();
		}
	}

	static void audioPause()
	{
		if (track != null) track.pause();
	}

	static void audioPlay(byte[] data, int size)
	{
		if (track != null) track.write(data, 0, size);
	}
}
