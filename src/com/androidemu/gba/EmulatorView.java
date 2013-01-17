package com.androidemu.gba;

import com.androidemu.gba.UserPrefs.Scaling;

import android.content.Context;

import android.graphics.Canvas;
import android.graphics.PixelFormat;

import android.util.AttributeSet;
import android.util.Log;

import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.ImageView.ScaleType;

public class EmulatorView extends SurfaceView implements SurfaceHolder.Callback
{
	public static final int SCALING_ORIGINAL = 0;
	public static final int SCALING_PROPORTIONAL = 1;
	public static final int SCALING_STRETCH = 2;
	public static final int SCALING_2X = 3;

	private Emulator emulator;
	private Scaling scalingMode = Scaling.proportional;
	
	private int actualWidth;
	private int actualHeight;
	private float aspectRatio;

	public EmulatorView(Context context, AttributeSet attrs)
	{
		super(context, attrs);

		final SurfaceHolder holder = getHolder();
		holder.setFormat(PixelFormat.RGB_565);
		holder.setFixedSize(Emulator.VIDEO_W, Emulator.VIDEO_H);
		holder.setKeepScreenOn(true);
		holder.addCallback(this);

		setFocusableInTouchMode(true);
		requestFocus();
	}

	public void setEmulator(Emulator e)
	{
		emulator = e;
	}

	public void setScalingMode(Scaling mode)
	{
		scalingMode = mode;
		requestLayout();
		updateSurfaceSize();
	}

	public void onImageUpdate(int[] data)
	{
		SurfaceHolder holder = getHolder();
		Canvas canvas = holder.lockCanvas();
		canvas.drawBitmap(data, 0, Emulator.VIDEO_W, 0, 0, Emulator.VIDEO_W,
				Emulator.VIDEO_H, false, null);
		holder.unlockCanvasAndPost(canvas);
	}

	public void surfaceCreated(SurfaceHolder holder)
	{
	}

	public void surfaceDestroyed(SurfaceHolder holder)
	{
		emulator.setRenderSurface(null, 0, 0);
	}

	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
	{
		emulator.setRenderSurface(this, width, height);
	}

	public void setActualSize(int w, int h)
	{
		if (actualWidth != w || actualHeight != h)
		{
			actualWidth = w;
			actualHeight = h;
			updateSurfaceSize();
		}
	}
	
	public void setAspectRatio(float ratio)
	{
		if (aspectRatio != ratio)
		{
			aspectRatio = ratio;
			updateSurfaceSize();
		}
	}
	
	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh)
	{
		updateSurfaceSize();
	}
	
	private void updateSurfaceSize()
	{
		int viewWidth = getWidth();
		int viewHeight = getHeight();
		if (viewWidth == 0 || viewHeight == 0 || actualWidth == 0 || actualHeight == 0)
			return;

		int w = 0;
		int h = 0;

		if (scalingMode != Scaling.stretch && aspectRatio != 0)
		{
			float ratio = aspectRatio * actualHeight / actualWidth;
			viewWidth = (int) (viewWidth / ratio);
		}

		switch (scalingMode)
		{
			case original:
				w = viewWidth;
				h = viewHeight;
				break;

			case x2:
				w = viewWidth / 2;
				h = viewHeight / 2;
				break;

			case stretch:
				if (viewWidth * actualHeight >= viewHeight * actualWidth)
				{
					w = actualWidth;
					h = actualHeight;
				}
				break;
		}

		if (w < actualWidth || h < actualHeight)
		{
			h = actualHeight;
			w = h * viewWidth / viewHeight;
			if (w < actualWidth)
			{
				w = actualWidth;
				h = w * viewHeight / viewWidth;
			}
		}

		w = (w + 3) & ~3;
		h = (h + 3) & ~3;
		getHolder().setFixedSize(w, h);
	}
}
