package com.androidemu.gba;

import android.app.SearchManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceGroup;
import android.preference.PreferenceManager;
import android.view.KeyEvent;
import android.widget.Toast;

import com.androidemu.Emulator;

public class GamePreferences extends PreferenceActivity
		implements Preference.OnPreferenceChangeListener {

	private static final String MARKET_SEARCH_URI =
		"http://market.android.com/search?q=pname:";

	public static final int[] gameKeys = {
		Emulator.GAMEPAD_UP,
		Emulator.GAMEPAD_DOWN,
		Emulator.GAMEPAD_LEFT,
		Emulator.GAMEPAD_RIGHT,
		Emulator.GAMEPAD_UP_LEFT,
		Emulator.GAMEPAD_UP_RIGHT,
		Emulator.GAMEPAD_DOWN_LEFT,
		Emulator.GAMEPAD_DOWN_RIGHT,
		Emulator.GAMEPAD_SELECT,
		Emulator.GAMEPAD_START,
		Emulator.GAMEPAD_A,
		Emulator.GAMEPAD_B,
		Emulator.GAMEPAD_A_TURBO,
		Emulator.GAMEPAD_B_TURBO,
		Emulator.GAMEPAD_TL,
		Emulator.GAMEPAD_TR,
	};

	public static final String[] keyPrefKeys = {
		"gamepad_up",
		"gamepad_down",
		"gamepad_left",
		"gamepad_right",
		"gamepad_up_left",
		"gamepad_up_right",
		"gamepad_down_left",
		"gamepad_down_right",
		"gamepad_select",
		"gamepad_start",
		"gamepad_A",
		"gamepad_B",
		"gamepad_A_turbo",
		"gamepad_B_turbo",
		"gamepad_TL",
		"gamepad_TR",
	};

	private static final int[] keyDisplayNames = {
		R.string.gamepad_up,
		R.string.gamepad_down,
		R.string.gamepad_left,
		R.string.gamepad_right,
		R.string.gamepad_up_left,
		R.string.gamepad_up_right,
		R.string.gamepad_down_left,
		R.string.gamepad_down_right,
		R.string.gamepad_select,
		R.string.gamepad_start,
		R.string.gamepad_A,
		R.string.gamepad_B,
		R.string.gamepad_A_turbo,
		R.string.gamepad_B_turbo,
		R.string.gamepad_TL,
		R.string.gamepad_TR,
	};

	private static final int defaultKeys_qwerty[] = {
		KeyEvent.KEYCODE_1,
		KeyEvent.KEYCODE_A,
		KeyEvent.KEYCODE_Q,
		KeyEvent.KEYCODE_W,
		0, 0, 0, 0,
		KeyEvent.KEYCODE_DEL,
		KeyEvent.KEYCODE_ENTER,
		KeyEvent.KEYCODE_P,
		KeyEvent.KEYCODE_O,
		KeyEvent.KEYCODE_0,
		KeyEvent.KEYCODE_9,
		KeyEvent.KEYCODE_K,
		KeyEvent.KEYCODE_L,
	};

	private static final int defaultKeys_non_qwerty[] = {
		0, 0, 0, 0,
		0, 0, 0, 0,
		KeyEvent.KEYCODE_VOLUME_UP,
		KeyEvent.KEYCODE_VOLUME_DOWN,
		KeyEvent.KEYCODE_SEARCH,
		KeyEvent.KEYCODE_BACK,
		0,
		0,
		KeyEvent.KEYCODE_CALL,
		KeyEvent.KEYCODE_CAMERA,
	};

	static {
		final int n = gameKeys.length;
		if (keyPrefKeys.length != n ||
				keyDisplayNames.length != n ||
				defaultKeys_qwerty.length != n ||
				defaultKeys_non_qwerty.length != n)
			throw new AssertionError("Key configurations are not consistent");
	}


	private static boolean isKeyboardQwerty(Context context) {
		return (context.getResources().getConfiguration().keyboard ==
				Configuration.KEYBOARD_QWERTY);
	}

	public static int[] getDefaultKeys(Context context) {
		return (isKeyboardQwerty(context) ?
				defaultKeys_qwerty : defaultKeys_non_qwerty);
	}

	public static boolean getDefaultVirtualKeypadEnabled(Context context) {
		return !isKeyboardQwerty(context);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setTitle(R.string.settings_title);
		addPreferencesFromResource(R.xml.preferences);

		findPreference("enableVirtualKeypad").
				setDefaultValue(getDefaultVirtualKeypadEnabled(this));

		PreferenceGroup group =
				(PreferenceGroup) findPreference("gameKeyBindings");

		int[] defaultKeys = getDefaultKeys(this);
		for (int i = 0; i < keyPrefKeys.length; i++) {
			GameKeyPreference pref = new GameKeyPreference(this);
			pref.setKey(keyPrefKeys[i]);
			pref.setTitle(keyDisplayNames[i]);
			pref.setDefaultValue(defaultKeys[i]);
			group.addPreference(pref);
		}

		Intent intent = new Intent(Intent.ACTION_VIEW,
				Uri.parse(MARKET_SEARCH_URI + getPackageName()));
		findPreference("appAbout").setIntent(intent);

		intent = new Intent(Intent.ACTION_WEB_SEARCH);
		intent.putExtra(SearchManager.QUERY,
				getResources().getString(R.string.search_roms_keyword));
		findPreference("searchRoms").setIntent(intent);

		setListSummary("scalingMode", "stretch");
	}

	private void setListSummary(String key, String def) {
		SharedPreferences sharedPref = PreferenceManager.
				getDefaultSharedPreferences(this);

		Preference pref = findPreference(key);
		pref.setOnPreferenceChangeListener(this);
		updateListSummary(pref, sharedPref.getString(key, def));
	}

	private void updateListSummary(Preference preference, Object newValue) {
		ListPreference list = (ListPreference) preference;
		CharSequence[] values = list.getEntryValues();
		int i;
		for (i = 0; i < values.length; i++) {
			if (values[i].equals(newValue))
				break;
		}
		if (i >= values.length)
			i = 0;

		list.setSummary(list.getEntries()[i]);
	}

	public boolean onPreferenceChange(Preference preference, Object newValue) {
		String key = preference.getKey();

		if (key.equals("scalingMode")) {
			updateListSummary(preference, newValue);
		}
		return true;
	}
}
