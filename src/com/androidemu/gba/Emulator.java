package com.androidemu.gba;

import java.io.File;
import java.io.IOException;

import android.content.Context;

public class Emulator
{
	public static final int VIDEO_W = 240;
	public static final int VIDEO_H = 160;
	
	private static Emulator emulator;

	private Thread emuThread;
	
	static
	{
		System.loadLibrary("gba");
	}
	
	public static Emulator createInstance(Context context, File datadir)
	{
		if (emulator == null)
		{
			emulator = new Emulator("/data/data/" + context.getPackageName() + "/lib", datadir.getAbsolutePath());
		}
		
		return emulator;
	}
	
	private Emulator(String libDir, String datadir)
	{
		initialize(libDir, datadir);

		if (emuThread == null)
		{
			emuThread = new Thread()
			{
				public void run()
				{
					nativeRun();
				}
			};
			emuThread.start();
		}
	}
	
	public void setOption(String name, boolean value)
	{
		setOption(name, String.valueOf(value));
	}
	
	public void setOption(String name, int value)
	{
		setOption(name, Integer.toString(value));
	}
	
	public void saveGameState(String currentGame, int slot)
	{
		String fname = getGameStateFile(currentGame, slot);
		saveState(fname);
	}

	public void loadGameState(String currentGame, int slot)
	{
		String fname = getGameStateFile(currentGame, slot);
		if (new File(fname).exists()) loadState(fname);
	}

	public static String getGameStateFile(String name, int slot)
	{
		int i = name.lastIndexOf('.');
		if (i >= 0) name = name.substring(0, i);
		name += ".ss" + slot;
		return name;
	}
	
	public native void setRenderSurface(EmulatorView surface, int width, int height);

	public native void setKeyStates(int states);

	public native void setOption(String name, String value);

	public native boolean initialize(String libdir, String datadir);

	private native void cleanUp();

	public native void reset();

	public native void power();

	public native boolean loadBIOS(String file);

	public native boolean loadROM(String file);

	public native void unloadROM();

	public native void pause();

	public native void resume();

	protected native void nativeRun();

	public native boolean saveState(String file);

	public native boolean loadState(String file);
}
