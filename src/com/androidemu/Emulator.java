package com.androidemu;

public class Emulator {

	public static final int VIDEO_W = 240;
	public static final int VIDEO_H = 160;

	public native void setRenderSurface(EmulatorView surface,
			int width, int height);
	public native void setKeyStates(int states);
	public native void setOption(String name, String value);

	public native boolean initialize(String libdir, String datadir);
	public native void cleanUp();
	public native void reset();
	public native void power();
	public native boolean loadBIOS(String file);
	public native boolean loadROM(String file);
	public native void unloadROM();
	public native void pause();
	public native void resume();
	public native void run();
	public native boolean saveState(String file);
	public native boolean loadState(String file);

	public void setOption(String name, boolean value) {
		setOption(name, value ? "true" : "false");
	}


	static {
		System.loadLibrary("gba");
	}
}
