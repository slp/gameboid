package com.androidemu.gba;

import android.content.Context;
import android.content.SharedPreferences;

public class UserPrefs extends com.androidemu.persistent.UserPrefs
{
	public final String bios;
	public final String lastRunningGame;
	public final String lastPickedGame;
	public final boolean autoFrameSkip;
	public final String maxFrameSkips;
	public final boolean soundEnabled;
	public final String soundVolume;
	public final boolean enableTrackball;
	public final boolean enableVirtualKeypad;
	public final String scalingMode;
	public final int quickLoad;
	public final int quickSave;
	public int[] keysMap = new int[128];
	
	public UserPrefs(Context context)
	{
		super(context);
		
		bios = prefsData.getString("bios", null);

		lastRunningGame = prefsData.getString("lastRunningGame", null);

		lastPickedGame = prefsData.getString("lastPickedGame", null);
		
		autoFrameSkip = prefsData.getBoolean("autoFrameSkip", true);
		
		maxFrameSkips = Integer.toString(prefsData.getInt("maxFrameSkips", 2));
		
		soundEnabled = prefsData.getBoolean("soundEnabled", res.getBoolean(R.bool.def_soundEnabled));
		
		soundVolume = Integer.toString(prefsData.getInt("soundVolume", 50));
		
		enableTrackball = prefsData.getBoolean("enableTrackball", res.getBoolean(R.bool.def_hasTrackball));
		
		enableVirtualKeypad = prefsData.getBoolean("enableVirtualKeypad", res.getBoolean(R.bool.def_useTouch));
		
		scalingMode = prefsData.getString("scalingMode", res.getString(R.string.def_scalingMode));
		
		quickLoad = prefsData.getInt("quickLoad", 0);
		
		quickSave = prefsData.getInt("quickSave", 0);
		
		final String[] prefKeys = GamePreferences.keyPrefKeys;
		final int[] defaultKeys = GamePreferences.getDefaultKeys(context);
		
		for (int i = 0; i < prefKeys.length; i++)
		{
			keysMap[i] = prefsData.getInt(prefKeys[i], defaultKeys[i]);
		}
	}
	
	public void setLastPickedGame(String lastPickedGame)
	{
		SharedPreferences.Editor editor = prefsData.edit();
		editor.putString("lastPickedGame", lastPickedGame);
		editor.commit();
	}
	
	public void setLastRunningGame(String lastRunningGame)
	{
		SharedPreferences.Editor editor = prefsData.edit();
		editor.putString("lastRunningGame", lastRunningGame);
		editor.commit();
	}
	
	public void setBIOS(String bios)
	{
		SharedPreferences.Editor editor = prefsData.edit();
		editor.putString("bios", bios);
		editor.commit();
	}
}
