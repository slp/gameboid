package com.androidemu.gba;

import android.app.ListActivity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;
import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class FileChooser extends ListActivity implements
		FileFilter, View.OnClickListener, View.OnKeyListener {

	public static final String EXTRA_TITLE = "title";
	public static final String EXTRA_FILEPATH = "filepath";
	public static final String EXTRA_FILTERS = "filters";

	private static final String LOG_TAG = "FileChooser";
	private final File sdcardDir = new File("/sdcard");
	private final File rootDir = new File("/");
	private File currentDir;
	private String[] filters;
	private EditText pathEdit;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.file_chooser);
		getListView().setEmptyView(findViewById(R.id.empty));

		pathEdit = (EditText) findViewById(R.id.path);
		pathEdit.setOnKeyListener(this);
		findViewById(R.id.goto_root).setOnClickListener(this);
		findViewById(R.id.goto_sdcard).setOnClickListener(this);
		findViewById(R.id.goto_parent).setOnClickListener(this);

		Intent intent = getIntent();
		if (intent != null) {
			String title = intent.getStringExtra(EXTRA_TITLE);
			if (title != null)
				setTitle(title);
			filters = intent.getStringArrayExtra(EXTRA_FILTERS);
		}

		String path = null;
		if (savedInstanceState != null)
			path = savedInstanceState.getString("currentDir");
		else if (intent != null)
			path = intent.getStringExtra(EXTRA_FILEPATH);

		File dir = null;
		if (path != null)
			dir = getDirectoryFromFile(path);
		if (dir == null)
			dir = sdcardDir;
		changeTo(dir);
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		if (currentDir != null)
			outState.putString("currentDir", currentDir.getAbsolutePath());
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		String name = l.getItemAtPosition(position).toString();
		File f = new File(currentDir, name);
		if (f.isDirectory())
			changeTo(f);
		else {
			Intent data = new Intent();
			data.putExtra(EXTRA_FILEPATH, f.getAbsolutePath());
			setResult(RESULT_OK, data);
			finish();
		}
	}

	public boolean onKey(View v, int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_ENTER) {
			String name = pathEdit.getText().toString().trim();
			if (name.length() > 0) {
				File dir = new File(name);
				if (dir.isDirectory())
					changeTo(dir);
				else {
					Toast.makeText(this, R.string.invalid_dir,
							Toast.LENGTH_SHORT).show();
				}
				return true;
			}
		}
		return false;
	}

	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.goto_root:
			changeTo(rootDir);
			break;
		case R.id.goto_sdcard:
			changeTo(sdcardDir);
			break;
		case R.id.goto_parent:
			File parent = currentDir.getParentFile();
			if (parent != null)
				changeTo(parent);
			break;
		}
	}

	public boolean accept(File pathname) {
		if (pathname.isDirectory())
			return true;

		String name = pathname.getName().toLowerCase();
		for (int i = 0; i < filters.length; i++) {
			if (name.endsWith(filters[i]))
				return true;
		}
		return false;
	}

	private File getDirectoryFromFile(String path) {
		File dir = new File(path);
		if (!dir.isDirectory()) {
			dir = dir.getParentFile();
			if (dir != null && !dir.isDirectory())
				dir = null;
		}
		return dir;
	}

	private void changeTo(File dir) {
		File[] files = dir.listFiles(filters == null ? null : this);
		if (files == null)
			return;

		currentDir = dir;
		pathEdit.setText(dir.getAbsolutePath());

		List<String> items = new ArrayList<String>(files.length);
		for (File f : files) {
			String name = f.getName();
			if (f.isDirectory())
				name += '/';
			items.add(name);
		}

		Collections.sort(items, String.CASE_INSENSITIVE_ORDER);
		setListAdapter(new ArrayAdapter(this,
				android.R.layout.simple_list_item_1, items));
	}
}
