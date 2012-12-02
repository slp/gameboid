package com.androidemu.wrapper;

import java.io.File;

import android.content.Context;
import android.os.Build;
import android.os.Environment;

import android.view.MotionEvent;

@SuppressWarnings("deprecation")
public class Wrapper
{
	public static final int SDK_INT = Integer.parseInt(Build.VERSION.SDK);

	public static boolean isBluetoothPresent()
	{
		if (SDK_INT >= 5) return Wrapper5.isBluetoothPresent();
		return false;
	}

	public static boolean isBluetoothEnabled()
	{
		if (SDK_INT >= 5) return Wrapper5.isBluetoothEnabled();
		return false;
	}

	public static boolean isBluetoothDiscoverable()
	{
		if (SDK_INT >= 5) return Wrapper5.isBluetoothDiscoverable();
		return false;
	}

	public static boolean supportsMultitouch(Context context)
	{
		if (SDK_INT >= 5) return Wrapper5.supportsMultitouch(context);
		return false;
	}

	public static final int MotionEvent_getPointerCount(MotionEvent event)
	{
		if (SDK_INT >= 5) return Wrapper5.MotionEvent_getPointerCount(event);
		return 1;
	}

	public static final int MotionEvent_getPointerId(MotionEvent event, int pointerIndex)
	{
		if (SDK_INT >= 5) return Wrapper5.MotionEvent_getPointerId(event, pointerIndex);
		return 0;
	}

	public static final int MotionEvent_findPointerIndex(MotionEvent event, int pointerId)
	{
		if (SDK_INT >= 5) return Wrapper5.MotionEvent_findPointerIndex(event, pointerId);
		if (pointerId == 0) return 0;
		return -1;
	}

	public static final float MotionEvent_getX(MotionEvent event, int pointerIndex)
	{
		if (SDK_INT >= 5) return Wrapper5.MotionEvent_getX(event, pointerIndex);
		return event.getX();
	}

	public static final float MotionEvent_getY(MotionEvent event, int pointerIndex)
	{
		if (SDK_INT >= 5) return Wrapper5.MotionEvent_getY(event, pointerIndex);
		return event.getY();
	}

	public static final float MotionEvent_getSize(MotionEvent event, int pointerIndex)
	{
		if (SDK_INT >= 5) return Wrapper5.MotionEvent_getSize(event, pointerIndex);
		return event.getSize();
	}

	public static final File getExternalScreenshotDirectory()
	{
		File picturesDir;

		if (SDK_INT >= 8)
		{
			picturesDir = Wrapper8.getExternalPicturesDirectory();
		}
		else
		{
			picturesDir = new File(Environment.getExternalStorageDirectory(), "pictures");
		}

		return new File(picturesDir, "screenshots");
	}
}
