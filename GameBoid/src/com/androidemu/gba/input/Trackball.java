package com.androidemu.gba.input;

import android.view.KeyEvent;
import android.view.View;
import com.androidemu.Emulator;

public class Trackball implements View.OnKeyListener {

	private GameKeyListener gameKeyListener;
	private Keyboard keyboard;
	private int keyStates;

	public Trackball(Keyboard kb, GameKeyListener listener) {
		keyboard = kb;
		gameKeyListener = listener;
	}

	public int getKeyStates() {
		return keyStates;
	}

	public void reset() {
		keyStates = 0;
	}

	public void setEnabled(boolean enabled) {
		if (enabled)
			keyboard.setOnKeyListener(this);
		else {
			keyboard.setOnKeyListener(null);
			keyStates = 0;
		}
	}

	public boolean onKey(View v, int keyCode, KeyEvent event) {
		if (event.getRepeatCount() > 0 ||
				event.getAction() != KeyEvent.ACTION_DOWN)
			return false;

		int states;

		switch (keyCode) {
		case KeyEvent.KEYCODE_DPAD_UP:
			states = Emulator.GAMEPAD_UP;
			break;
		case KeyEvent.KEYCODE_DPAD_DOWN:
			states = Emulator.GAMEPAD_DOWN;
			break;
		case KeyEvent.KEYCODE_DPAD_LEFT:
			states = Emulator.GAMEPAD_LEFT;
			break;
		case KeyEvent.KEYCODE_DPAD_RIGHT:
			states = Emulator.GAMEPAD_RIGHT;
			break;
		case KeyEvent.KEYCODE_DPAD_CENTER:
			states = 0;
			break;
		default:
			return false;
		}

		if (keyStates != states) {
			keyStates = states;
			gameKeyListener.onGameKeyChanged();
		}
		return true;
	}
}
