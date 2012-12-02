package com.androidemu.wrapper;

import android.annotation.TargetApi;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.view.MotionEvent;

@TargetApi(Build.VERSION_CODES.ECLAIR)
class Wrapper5
{

	public static final boolean isBluetoothPresent()
	{
		return (BluetoothAdapter.getDefaultAdapter() != null);
	}

	public static final boolean isBluetoothEnabled()
	{
		BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
		return (adapter != null ? adapter.isEnabled() : false);
	}

	public static final boolean isBluetoothDiscoverable()
	{
		BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
		if (adapter == null) return false;
		return (adapter.getScanMode() == BluetoothAdapter.SCAN_MODE_CONNECTABLE_DISCOVERABLE);
	}

	public static final boolean supportsMultitouch(Context context)
	{
		if (Wrapper.SDK_INT < 7) return true;

		return context.getPackageManager().hasSystemFeature(
				PackageManager.FEATURE_TOUCHSCREEN_MULTITOUCH);
	}

	public static final int MotionEvent_getPointerCount(MotionEvent event)
	{
		return event.getPointerCount();
	}

	public static final int MotionEvent_getPointerId(MotionEvent event, int pointerIndex)
	{
		return event.getPointerId(pointerIndex);
	}

	public static final int MotionEvent_findPointerIndex(MotionEvent event, int pointerId)
	{
		return event.findPointerIndex(pointerId);
	}

	public static final float MotionEvent_getX(MotionEvent event, int pointerIndex)
	{
		return event.getX(pointerIndex);
	}

	public static final float MotionEvent_getY(MotionEvent event, int pointerIndex)
	{
		return event.getY(pointerIndex);
	}

	public static final float MotionEvent_getSize(MotionEvent event, int pointerIndex)
	{
		return event.getSize(pointerIndex);
	}
}
