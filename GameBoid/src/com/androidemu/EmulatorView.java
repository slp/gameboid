package com.androidemu;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class EmulatorView extends SurfaceView
		implements SurfaceHolder.Callback {

	public static final int SCALING_ORIGINAL = 0;
	public static final int SCALING_PROPORTIONAL = 1;
	public static final int SCALING_STRETCH = 2;

	private static final String LOG_TAG = "EmulatorView";

	private Emulator emulator;
	private int scalingMode = SCALING_STRETCH;

	public EmulatorView(Context context, AttributeSet attrs) {
		super(context, attrs);

		final SurfaceHolder holder = getHolder();
		holder.setFixedSize(Emulator.VIDEO_W, Emulator.VIDEO_H);
		holder.setKeepScreenOn(true);
		holder.addCallback(this);

		setFocusableInTouchMode(true);
		requestFocus();
	}

	public void setEmulator(Emulator e) {
		emulator = e;
	}

	public void setScalingMode(int mode) {
		if (scalingMode != mode) {
			scalingMode = mode;
			requestLayout();
		}
	}

	public void onImageUpdate(int[] data) {
		SurfaceHolder holder = getHolder();
		Canvas canvas = holder.lockCanvas();
		canvas.drawBitmap(data, 0, Emulator.VIDEO_W, 0, 0,
				Emulator.VIDEO_W, Emulator.VIDEO_H, false, null);
		holder.unlockCanvasAndPost(canvas);
	}

	public void surfaceCreated(SurfaceHolder holder) {
	}

	public void surfaceDestroyed(SurfaceHolder holder) {
		emulator.setRenderSurface(null, 0, 0);
	}

	public void surfaceChanged(SurfaceHolder holder,
			int format, int width, int height) {
		emulator.setRenderSurface(this, width, height);
	}

	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		int specWidth = MeasureSpec.getSize(widthMeasureSpec);
		int specHeight = MeasureSpec.getSize(heightMeasureSpec);
		int w, h;

		switch (scalingMode) {
		case SCALING_ORIGINAL:
			w = Emulator.VIDEO_W;
			h = Emulator.VIDEO_H;
			break;
		case SCALING_STRETCH:
			if (specWidth >= specHeight) {
				w = specWidth;
				h = specHeight;
				break;
			}
			// fall through
		case SCALING_PROPORTIONAL:
			h = specHeight;
			w = h * Emulator.VIDEO_W / Emulator.VIDEO_H;
			if (w > specWidth) {
				w = specWidth;
				h = w * Emulator.VIDEO_H / Emulator.VIDEO_W;
			}
			break;
		default:
			super.onMeasure(widthMeasureSpec, heightMeasureSpec);
			return;
		}

		setMeasuredDimension(w, h);
	}
}
