package com.androidemu.gba;

import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;

public class GameKeyPreference extends DialogPreference
		implements DialogInterface.OnKeyListener {

	private static final String LOG_TAG = "GameKeyPreference";

	private Resources resources;
	private int oldValue;
	private int newValue;

	public GameKeyPreference(Context context) {
		this(context, null);
	}

	public GameKeyPreference(Context context, AttributeSet attrs) {
		super(context, attrs);

		resources = context.getResources();
		setPositiveButtonText(R.string.key_clear);
	}

	@Override
	protected void onPrepareDialogBuilder(Builder builder) {
		super.onPrepareDialogBuilder(builder);

		builder.setMessage(R.string.press_key_prompt).setOnKeyListener(this);
	}

	@Override
	public void onClick(DialogInterface dialog, int which) {
		// clear key binding
		if (which == DialogInterface.BUTTON_POSITIVE)
			newValue = 0;

		super.onClick(dialog, which);
	}

	@Override
	protected void onDialogClosed(boolean positiveResult) {
		super.onDialogClosed(positiveResult);

		if (!positiveResult)
			newValue = oldValue;
		else {
			oldValue = newValue;
			persistInt(newValue);
			updateSummary();
		}
	}

	@Override
	protected Object onGetDefaultValue(TypedArray a, int index) {
		return a.getInteger(index, 0);
	}

	@Override
	protected void onSetInitialValue(
			boolean restoreValue, Object defaultValue) {
		oldValue = (restoreValue ?
				getPersistedInt(0) : ((Integer) defaultValue).intValue());
		newValue = oldValue;
		updateSummary();
	}

	public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
		if (!isKeyConfigurable(keyCode))
			return false;

		newValue = keyCode;
		super.onClick(dialog, DialogInterface.BUTTON_POSITIVE);
		dialog.dismiss();
		return true;
	}

	private void updateSummary() {
		setSummary(getKeyName(newValue));
	}

	private static boolean isKeyConfigurable(int keyCode) {
		switch (keyCode) {
		case KeyEvent.KEYCODE_DPAD_UP:
		case KeyEvent.KEYCODE_DPAD_DOWN:
		case KeyEvent.KEYCODE_DPAD_LEFT:
		case KeyEvent.KEYCODE_DPAD_RIGHT:
		case KeyEvent.KEYCODE_HOME:
		case KeyEvent.KEYCODE_MENU:
		case KeyEvent.KEYCODE_POWER:
			return false;
		}
		return true;
	}

	private String getKeyName(int keyCode) {
		switch (keyCode) {
		case KeyEvent.KEYCODE_A: return "A";
		case KeyEvent.KEYCODE_B: return "B";
		case KeyEvent.KEYCODE_C: return "C";
		case KeyEvent.KEYCODE_D: return "D";
		case KeyEvent.KEYCODE_E: return "E";
		case KeyEvent.KEYCODE_F: return "F";
		case KeyEvent.KEYCODE_G: return "G";
		case KeyEvent.KEYCODE_H: return "H";
		case KeyEvent.KEYCODE_I: return "I";
		case KeyEvent.KEYCODE_J: return "J";
		case KeyEvent.KEYCODE_K: return "K";
		case KeyEvent.KEYCODE_L: return "L";
		case KeyEvent.KEYCODE_M: return "M";
		case KeyEvent.KEYCODE_N: return "N";
		case KeyEvent.KEYCODE_O: return "O";
		case KeyEvent.KEYCODE_P: return "P";
		case KeyEvent.KEYCODE_Q: return "Q";
		case KeyEvent.KEYCODE_R: return "R";
		case KeyEvent.KEYCODE_S: return "S";
		case KeyEvent.KEYCODE_T: return "T";
		case KeyEvent.KEYCODE_U: return "U";
		case KeyEvent.KEYCODE_V: return "V";
		case KeyEvent.KEYCODE_W: return "W";
		case KeyEvent.KEYCODE_X: return "X";
		case KeyEvent.KEYCODE_Y: return "Y";
		case KeyEvent.KEYCODE_Z: return "Z";

		case KeyEvent.KEYCODE_0: return "0";
		case KeyEvent.KEYCODE_1: return "1";
		case KeyEvent.KEYCODE_2: return "2";
		case KeyEvent.KEYCODE_3: return "3";
		case KeyEvent.KEYCODE_4: return "4";
		case KeyEvent.KEYCODE_5: return "5";
		case KeyEvent.KEYCODE_6: return "6";
		case KeyEvent.KEYCODE_7: return "7";
		case KeyEvent.KEYCODE_8: return "8";
		case KeyEvent.KEYCODE_9: return "9";

		case KeyEvent.KEYCODE_ALT_LEFT:		return "ALT (left)";
		case KeyEvent.KEYCODE_ALT_RIGHT:	return "ALT (right)";
		case KeyEvent.KEYCODE_SHIFT_LEFT:	return "SHIFT (left)";
		case KeyEvent.KEYCODE_SHIFT_RIGHT:	return "SHIFT (right)";
		case KeyEvent.KEYCODE_SPACE:		return "SPACE";
		case KeyEvent.KEYCODE_DEL:			return "DEL";
		case KeyEvent.KEYCODE_ENTER:		return "ENTER";
		case KeyEvent.KEYCODE_AT:			return "@";
		case KeyEvent.KEYCODE_PERIOD:		return ".";
		case KeyEvent.KEYCODE_COMMA:		return ",";

		case KeyEvent.KEYCODE_DPAD_CENTER:	return "DPAD CENTER";
		case KeyEvent.KEYCODE_BACK:			return "BACK";
		case KeyEvent.KEYCODE_CALL:			return "CALL";
		case KeyEvent.KEYCODE_CAMERA:		return "CAMERA";
		case KeyEvent.KEYCODE_FOCUS:		return "FOCUS";
		case KeyEvent.KEYCODE_SEARCH:		return "SEARCH";
		case KeyEvent.KEYCODE_VOLUME_UP:	return "Volume UP";
		case KeyEvent.KEYCODE_VOLUME_DOWN:	return "Volume DOWN";

		case 0:
			return resources.getString(R.string.key_none);
		default:
			return resources.getString(R.string.key_unknown);
		}
	}
}
