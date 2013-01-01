package com.androidemu.gba.input;

import java.util.ArrayList;

import android.content.Context;
import android.content.SharedPreferences;

import android.content.res.Resources;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;

import android.graphics.drawable.BitmapDrawable;

import android.os.Vibrator;

import android.preference.PreferenceManager;

import android.util.AttributeSet;
import android.util.Log;

import android.view.MotionEvent;
import android.view.View;

import com.androidemu.wrapper.Wrapper;

import com.androidemu.gba.R;

public class VirtualKeypad extends View
{
	private static final int DPAD_4WAY[] = { Keycodes.GAMEPAD_LEFT, Keycodes.GAMEPAD_UP,
			Keycodes.GAMEPAD_RIGHT, Keycodes.GAMEPAD_DOWN };

	private static final float DPAD_DEADZONE_VALUES[] = { 0.1f, 0.14f, 0.1667f, 0.2f,
			0.25f, };

	private Context context;
	private View view;
	private float scaleX;
	private float scaleY;
	private int transparency;

	private GameKeyListener gameKeyListener;
	private int keyStates;
	private Vibrator vibrator;
	private boolean vibratorEnabled;
	private boolean dpad4Way;
	private float dpadDeadZone = DPAD_DEADZONE_VALUES[2];
	private float pointSizeThreshold;
	private boolean inBetweenPress;

	private ArrayList<Control> controls = new ArrayList<Control>();
	private Control dpad;
	private Control buttons;
	private Control extraButtons;
	private Control selectStart;
	
	private Paint paint = new Paint();

	public VirtualKeypad(Context ctx)
	{
		super(ctx);
		
		createControls(ctx);
	}
	
	public VirtualKeypad(Context ctx, AttributeSet attrs)
	{
		super(ctx, attrs);
		
		createControls(ctx);
	}
	
	private void createControls(Context ctx)
	{
		view = this;
		context = ctx;

		vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);

		dpad = createControl(R.drawable.dpad);
		buttons = createControl(R.drawable.buttons);
		extraButtons = createControl(R.drawable.extra_buttons);
		selectStart = createControl(R.drawable.select_start_buttons);
		
		setFocusableInTouchMode(true);
	}
	
	public void setGameKeyListener(GameKeyListener l)
	{
		gameKeyListener = l;
	}
	
	public final int getKeyStates()
	{
		return keyStates;
	}

	public void reset()
	{
		keyStates = 0;
	}

	public final void destroy()
	{
	}
	
	@Override
	public void onDraw(Canvas canvas)
	{
		paint.setAlpha(transparency);
		
		for (Control c : controls)
			c.draw(canvas, paint);
	}
	
	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh)
	{
		resize(w, h);
	}
	
	@Override
	public void onWindowFocusChanged(boolean hasWindowFocus)
	{
		super.onWindowFocusChanged(hasWindowFocus);
		
		reset();
		
		if (isShown())
		{
			resize(getWidth(), getHeight());
		}
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
		return onTouch(event, false);
	}
	
	public void resize(int w, int h)
	{
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		vibratorEnabled = prefs.getBoolean("enableVibrator", true);
		dpad4Way = prefs.getBoolean("dpad4Way", false);

		int value = prefs.getInt("dpadDeadZone", 2);
		value = (value < 0 ? 0 : (value > 4 ? 4 : value));
		dpadDeadZone = DPAD_DEADZONE_VALUES[value];

		inBetweenPress = prefs.getBoolean("inBetweenPress", false);

		pointSizeThreshold = 1.0f;
		if (prefs.getBoolean("pointSizePress", false))
		{
			int threshold = prefs.getInt("pointSizePressThreshold", 7);
			pointSizeThreshold = (threshold / 10.0f) - 0.01f;
		}

		dpad.hide(prefs.getBoolean("hideDpad", false));
		buttons.hide(prefs.getBoolean("hideButtons", false));
		extraButtons.hide(prefs.getBoolean("hideExtraButtons", false));
		extraButtons.disable(prefs.getBoolean("disableExtraButtons", true));
		selectStart.hide(prefs.getBoolean("hideSelectStart", false));

		scaleX = (float) w / view.getWidth();
		scaleY = (float) h / view.getHeight();

		float controlScale = getControlScale(prefs);
		float sx = scaleX * controlScale;
		float sy = scaleY * controlScale;
		Resources res = context.getResources();
		for (Control c : controls)
			c.load(res, sx, sy);

		final int margin = prefs.getInt("layoutMargin", 2) * 10;
		int marginX = (int) (margin * scaleX);
		int marginY = (int) (margin * scaleY);
		reposition(w - marginX, h - marginY, prefs);

		transparency = prefs.getInt("vkeypadTransparency", 50);
	}

	private static float getControlScale(SharedPreferences prefs)
	{
		String value = prefs.getString("vkeypadSize", null);
		if ("small".equals(value)) return 1.0f;
		if ("large".equals(value)) return 1.33333f;
		return 1.2f;
	}

	private Control createControl(int resId)
	{
		Control c = new Control(resId);
		controls.add(c);
		return c;
	}

	private void makeBottomBottom(int w, int h)
	{
		if (dpad.getWidth() + buttons.getWidth() > w)
		{
			makeBottomTop(w, h);
			return;
		}

		dpad.move(0, h - dpad.getHeight());
		buttons.move(w - buttons.getWidth(), h - buttons.getHeight());
		if (extraButtons.isEnabled())
		{
			extraButtons.move(w - buttons.getWidth(), h - buttons.getHeight() * 7 / 3);
		}

		int x = (w + dpad.getWidth() - buttons.getWidth() - selectStart.getWidth()) / 2;
		if (x > dpad.getWidth())
			selectStart.move(x, h - selectStart.getHeight());
		else
		{
			x = (w - selectStart.getWidth()) / 2;
			selectStart.move(x, 0);
		}
	}

	private void makeTopTop(int w, int h)
	{
		if (dpad.getWidth() + buttons.getWidth() > w)
		{
			makeBottomTop(w, h);
			return;
		}

		dpad.move(0, 0);

		int y = 0;
		if (extraButtons.isEnabled())
		{
			extraButtons.move(w - extraButtons.getWidth(), y);
			y += buttons.getHeight() * 4 / 3;
		}
		buttons.move(w - buttons.getWidth(), y);

		selectStart.move((w - selectStart.getWidth()) / 2, h - selectStart.getHeight());
	}

	private void makeTopBottom(int w, int h)
	{
		dpad.move(0, 0);
		buttons.move(w - buttons.getWidth(), h - buttons.getHeight());
		if (extraButtons.isEnabled())
		{
			extraButtons.move(w - buttons.getWidth(), h - buttons.getHeight() * 7 / 3);
		}

		int x = (w - buttons.getWidth() - selectStart.getWidth()) / 2;
		selectStart.move(x, h - selectStart.getHeight());
	}

	private void makeBottomTop(int w, int h)
	{
		dpad.move(0, h - dpad.getHeight());

		int y = 0;
		if (extraButtons.isEnabled())
		{
			extraButtons.move(w - extraButtons.getWidth(), y);
			y += buttons.getHeight() * 4 / 3;
		}
		buttons.move(w - buttons.getWidth(), y);

		int x = (w + dpad.getWidth() - selectStart.getWidth()) / 2;
		selectStart.move(x, h - selectStart.getHeight());
	}

	private void reposition(int w, int h, SharedPreferences prefs)
	{
		String layout = prefs.getString("vkeypadLayout", "top_bottom");

		if ("top_bottom".equals(layout))
			makeTopBottom(w, h);
		else if ("bottom_top".equals(layout))
			makeBottomTop(w, h);
		else if ("top_top".equals(layout))
			makeTopTop(w, h);
		else
			makeBottomBottom(w, h);
	}

	private boolean shouldVibrate(int oldStates, int newStates)
	{
		return (((oldStates ^ newStates) & newStates) != 0);
	}

	private void setKeyStates(int newStates)
	{
		if (keyStates == newStates) return;

		if (vibratorEnabled && shouldVibrate(keyStates, newStates)) vibrator.vibrate(33);

		keyStates = newStates;
		gameKeyListener.onGameKeyChanged();
	}

	private int get4WayDirection(float x, float y)
	{
		x -= 0.5f;
		y -= 0.5f;

		if (Math.abs(x) >= Math.abs(y)) return (x < 0.0f ? 0 : 2);
		return (y < 0.0f ? 1 : 3);
	}

	private int getDpadStates(float x, float y)
	{
		if (dpad4Way) return DPAD_4WAY[get4WayDirection(x, y)];

		final float cx = 0.5f;
		final float cy = 0.5f;
		int states = 0;

		if (x < cx - dpadDeadZone)
			states |= Keycodes.GAMEPAD_LEFT;
		else if (x > cx + dpadDeadZone) states |= Keycodes.GAMEPAD_RIGHT;
		if (y < cy - dpadDeadZone)
			states |= Keycodes.GAMEPAD_UP;
		else if (y > cy + dpadDeadZone) states |= Keycodes.GAMEPAD_DOWN;

		return states;
	}

	private int getButtonsStates(int[] buttons, float x, float y, float size)
	{

		if (size > pointSizeThreshold) return (buttons[0] | buttons[1]);

		if (inBetweenPress)
		{
			int states = 0;
			if (x < 0.58f) states |= buttons[0];
			if (x > 0.42f) states |= buttons[1];
			return states;
		}
		return (x < 0.5f ? buttons[0] : buttons[1]);
	}

	private int getSelectStartStates(float x, float y)
	{
		return (x < 0.5f ? Keycodes.GAMEPAD_SELECT : Keycodes.GAMEPAD_START);
	}

	private float getEventX(MotionEvent event, int index, boolean flip)
	{
		float x = Wrapper.MotionEvent_getX(event, index);
		if (flip) x = view.getWidth() - x;
		return (x * scaleX);
	}

	private float getEventY(MotionEvent event, int index, boolean flip)
	{
		float y = Wrapper.MotionEvent_getY(event, index);
		if (flip) y = view.getHeight() - y;
		return y * scaleY;
	}

	private Control findControl(float x, float y)
	{
		for (Control c : controls)
		{
			if (c.hitTest(x, y)) return c;
		}
		return null;
	}

	private static final int[] BUTTONS = { Keycodes.GAMEPAD_B, Keycodes.GAMEPAD_A };

	private static final int[] EXTRA_BUTTONS = { Keycodes.GAMEPAD_B_TURBO,
			Keycodes.GAMEPAD_A_TURBO };

	private int getControlStates(Control c, float x, float y, float size)
	{
		x = (x - c.getX()) / c.getWidth();
		y = (y - c.getY()) / c.getHeight();

		if (c == dpad) return getDpadStates(x, y);
		if (c == buttons) return getButtonsStates(BUTTONS, x, y, size);
		if (c == extraButtons) return getButtonsStates(EXTRA_BUTTONS, x, y, size);
		if (c == selectStart) return getSelectStartStates(x, y);

		return 0;
	}
	
	public boolean onTouch(MotionEvent event, boolean flip)
	{
		int action = event.getAction();
		int id;
		int pointerUpId = -1;

		switch (action & MotionEvent.ACTION_MASK)
		{
			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_CANCEL:
				setKeyStates(0);
				return true;

			case MotionEvent.ACTION_DOWN:
			case MotionEvent.ACTION_POINTER_DOWN:
			case MotionEvent.ACTION_MOVE:
			case MotionEvent.ACTION_OUTSIDE:
				break;
			default:
				return false;
		}

		int states = 0;
		int n = Wrapper.MotionEvent_getPointerCount(event);
		for (int i = 0; i < n; i++)
		{
			float x = getEventX(event, i, flip);
			float y = getEventY(event, i, flip);
			Control c = findControl(x, y);
			if (c != null)
			{
				states |= getControlStates(c, x, y, Wrapper.MotionEvent_getSize(event, i));
			}
		}
		setKeyStates(states);
		return true;
	}

	private static class Control
	{
		private int resId;
		private boolean hidden;
		private boolean disabled;
		private Bitmap bitmap;
		private RectF bounds = new RectF();

		Control(int r)
		{
			resId = r;
		}

		final float getX()
		{
			return bounds.left;
		}

		final float getY()
		{
			return bounds.top;
		}

		final int getWidth()
		{
			return bitmap.getWidth();
		}

		final int getHeight()
		{
			return bitmap.getHeight();
		}

		final boolean isEnabled()
		{
			return !disabled;
		}

		final void hide(boolean b)
		{
			hidden = b;
		}

		final void disable(boolean b)
		{
			disabled = b;
		}

		final boolean hitTest(float x, float y)
		{
			return bounds.contains(x, y);
		}

		final void move(float x, float y)
		{
			bounds.set(x, y, x + bitmap.getWidth(), y + bitmap.getHeight());
		}

		final void load(Resources res, float sx, float sy)
		{
			bitmap = ((BitmapDrawable) res.getDrawable(resId)).getBitmap();
			bitmap = Bitmap.createScaledBitmap(bitmap, (int) (sx * bitmap.getWidth()),
					(int) (sy * bitmap.getHeight()), true);
		}

		final void reload(Resources res, int id)
		{
			int w = bitmap.getWidth();
			int h = bitmap.getHeight();
			bitmap = ((BitmapDrawable) res.getDrawable(id)).getBitmap();
			bitmap = Bitmap.createScaledBitmap(bitmap, w, h, true);
		}

		final void draw(Canvas canvas, Paint paint)
		{
			if (!hidden && !disabled)
				canvas.drawBitmap(bitmap, bounds.left, bounds.top, paint);
		}
	}
}
