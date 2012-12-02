package com.androidemu.wrapper;

import java.io.File;

import android.annotation.TargetApi;

import android.os.Build;
import android.os.Environment;

@TargetApi(Build.VERSION_CODES.FROYO)
public class Wrapper8
{
	public static File getExternalPicturesDirectory()
	{
		return Environment
				.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
	}
}
