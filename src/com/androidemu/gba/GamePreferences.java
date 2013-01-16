package com.androidemu.gba;


import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceGroup;
import android.preference.PreferenceManager;
import android.view.KeyEvent;

import com.androidemu.gba.input.Keycodes;

public class GamePreferences extends com.androidemu.persistent.CommonPreferences implements
		Preference.OnPreferenceChangeListener
{
	public static final int[] gameKeys = { Keycodes.GAMEPAD_UP, Keycodes.GAMEPAD_DOWN,
			Keycodes.GAMEPAD_LEFT, Keycodes.GAMEPAD_RIGHT, Keycodes.GAMEPAD_UP_LEFT,
			Keycodes.GAMEPAD_UP_RIGHT, Keycodes.GAMEPAD_DOWN_LEFT,
			Keycodes.GAMEPAD_DOWN_RIGHT, Keycodes.GAMEPAD_SELECT, Keycodes.GAMEPAD_START,
			Keycodes.GAMEPAD_A, Keycodes.GAMEPAD_B, Keycodes.GAMEPAD_A_TURBO,
			Keycodes.GAMEPAD_B_TURBO, Keycodes.GAMEPAD_TL, Keycodes.GAMEPAD_TR, };

	public static final String[] keyPrefKeys = { "gamepad_up", "gamepad_down",
			"gamepad_left", "gamepad_right", "gamepad_up_left", "gamepad_up_right",
			"gamepad_down_left", "gamepad_down_right", "gamepad_select", "gamepad_start",
			"gamepad_A", "gamepad_B", "gamepad_A_turbo", "gamepad_B_turbo", "gamepad_TL",
			"gamepad_TR", };

	private static final int[] keyDisplayNames = { R.string.gamepad_up,
			R.string.gamepad_down, R.string.gamepad_left, R.string.gamepad_right,
			R.string.gamepad_up_left, R.string.gamepad_up_right,
			R.string.gamepad_down_left, R.string.gamepad_down_right,
			R.string.gamepad_select, R.string.gamepad_start, R.string.gamepad_A,
			R.string.gamepad_B, R.string.gamepad_A_turbo, R.string.gamepad_B_turbo,
			R.string.gamepad_TL, R.string.gamepad_TR, };

	private static final int defaultKeys_qwerty[] = { KeyEvent.KEYCODE_1,
			KeyEvent.KEYCODE_A, KeyEvent.KEYCODE_Q, KeyEvent.KEYCODE_W, 0, 0, 0, 0,
			KeyEvent.KEYCODE_DEL, KeyEvent.KEYCODE_ENTER, KeyEvent.KEYCODE_P,
			KeyEvent.KEYCODE_O, KeyEvent.KEYCODE_0, KeyEvent.KEYCODE_9,
			KeyEvent.KEYCODE_K, KeyEvent.KEYCODE_L, };

	private static final int defaultKeys_non_qwerty[] = { 0, 0, 0, 0, 0, 0, 0, 0,
			KeyEvent.KEYCODE_VOLUME_UP, KeyEvent.KEYCODE_VOLUME_DOWN,
			KeyEvent.KEYCODE_SEARCH, KeyEvent.KEYCODE_BACK, 0, 0, KeyEvent.KEYCODE_CALL,
			KeyEvent.KEYCODE_CAMERA, };

	static
	{
		final int n = gameKeys.length;
		if (keyPrefKeys.length != n || keyDisplayNames.length != n
				|| defaultKeys_qwerty.length != n || defaultKeys_non_qwerty.length != n)
			throw new AssertionError("Key configurations are not consistent");
	}

	private static boolean isKeyboardQwerty(Context context)
	{
		return (context.getResources().getConfiguration().keyboard == Configuration.KEYBOARD_QWERTY);
	}

	public static int[] getDefaultKeys(Context context)
	{
		return (isKeyboardQwerty(context) ? defaultKeys_qwerty : defaultKeys_non_qwerty);
	}

	public static boolean getDefaultVirtualKeypadEnabled(Context context)
	{
		return !isKeyboardQwerty(context);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		addPreferencesFromResource(R.xml.preferences);
		
		PreferenceGroup group = (PreferenceGroup) findPreference("gameKeyBindings");

		int[] defaultKeys = getDefaultKeys(this);
		for (int i = 0; i < keyPrefKeys.length; i++)
		{
			com.androidemu.persistent.GameKeyPreference pref = new com.androidemu.persistent.GameKeyPreference(this);
			pref.setKey(keyPrefKeys[i]);
			pref.setTitle(keyDisplayNames[i]);
			pref.setDefaultValue(defaultKeys[i]);
			group.addPreference(pref);
		}

		setListSummary("scalingMode", "stretch");
	}

	private void setListSummary(String key, String def)
	{
		SharedPreferences sharedPref = PreferenceManager
				.getDefaultSharedPreferences(this);

		Preference pref = findPreference(key);
		pref.setOnPreferenceChangeListener(this);
		updateListSummary(pref, sharedPref.getString(key, def));
	}

	private void updateListSummary(Preference preference, Object newValue)
	{
		ListPreference list = (ListPreference) preference;
		CharSequence[] values = list.getEntryValues();
		int i;
		for (i = 0; i < values.length; i++)
		{
			if (values[i].equals(newValue)) break;
		}
		if (i >= values.length) i = 0;

		list.setSummary(list.getEntries()[i]);
	}

	public boolean onPreferenceChange(Preference preference, Object newValue)
	{
		String key = preference.getKey();

		if (key.equals("scalingMode"))
		{
			updateListSummary(preference, newValue);
		}
		return true;
	}
}
