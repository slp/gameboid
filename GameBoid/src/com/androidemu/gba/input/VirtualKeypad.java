package com.androidemu.gba.input;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.RelativeLayout;

import com.androidemu.Emulator;
import com.androidemu.gba.R;

public class VirtualKeypad extends RelativeLayout
		implements View.OnTouchListener {

	private static final String LOG_TAG = "VirtualKeypad";
	private static final int DEAD_ZONE = 20;

	private View dpadView;
	private GameKeyListener gameKeyListener;
	private int keyStates;

	public VirtualKeypad(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public void setGameKeyListener(GameKeyListener listener) {
		gameKeyListener = listener;
	}

	public int getKeyStates() {
		return keyStates;
	}

	public void reset() {
		keyStates = 0;
	}

	@Override
	protected void onFinishInflate() {
		dpadView = findViewById(R.id.dpad);
		dpadView.setOnTouchListener(this);

		findViewById(R.id.select).setOnTouchListener(this);
		findViewById(R.id.start).setOnTouchListener(this);
	}

	private void setKeyStates(int newStates) {
		if (keyStates != newStates) {
			keyStates = newStates;
			gameKeyListener.onGameKeyChanged();
		}
	}

	private int getGameKey(float fx, float fy) {
		final int x = (int) (fx + 0.5f);
		final int y = (int) (fy + 0.5f);
		final int cx = dpadView.getWidth() / 2;
		final int cy = dpadView.getHeight() / 2;

		int key = 0;
		if (x < cx - DEAD_ZONE)
			key |= Emulator.GAMEPAD_LEFT;
		else if (x > cx + DEAD_ZONE)
			key |= Emulator.GAMEPAD_RIGHT;
		if (y < cy - DEAD_ZONE)
			key |= Emulator.GAMEPAD_UP;
		else if (y > cy + DEAD_ZONE)
			key |= Emulator.GAMEPAD_DOWN;

		return key;
	}

	private boolean onDpadTouch(MotionEvent event) {
		switch (event.getAction()) {
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_MOVE:
			setKeyStates(getGameKey(event.getX(), event.getY()));
			break;
		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_OUTSIDE:
		case MotionEvent.ACTION_CANCEL:
			setKeyStates(0);
			break;
		default:
			return false;
		}
		return true;
	}

	private boolean onButtonTouch(int id, MotionEvent event) {
		int gameKey;
		switch (id) {
		case R.id.select:
			gameKey = Emulator.GAMEPAD_SELECT;
			break;
		case R.id.start:
			gameKey = Emulator.GAMEPAD_START;
			break;
		default:
			return false;
		}

		switch (event.getAction()) {
		case MotionEvent.ACTION_DOWN:
			keyStates |= gameKey;
			break;
		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_OUTSIDE:
		case MotionEvent.ACTION_CANCEL:
			keyStates &= ~gameKey;
			break;
		default:
			return false;
		}
		gameKeyListener.onGameKeyChanged();
		return true;
	}

	public boolean onTouch(View v, MotionEvent event) {
		int id = v.getId();
		if (id == R.id.dpad)
			return onDpadTouch(event);
		return onButtonTouch(id, event);
	}
}
