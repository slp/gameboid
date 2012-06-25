package com.androidemu.gba;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import com.androidemu.Emulator;
import com.androidemu.EmulatorView;
import com.androidemu.gba.input.GameKeyListener;
import com.androidemu.gba.input.Keyboard;
import com.androidemu.gba.input.VirtualKeypad;
import com.androidemu.gba.input.Trackball;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;

public class GameBoid extends Activity implements GameKeyListener,
		DialogInterface.OnCancelListener {

	private static final String LOG_TAG = "GameBoid";

	private static final int REQUEST_BROWSE_ROM = 1;
	private static final int REQUEST_BROWSE_BIOS = 2;
	private static final int REQUEST_SETTINGS = 3;

	private static final int DIALOG_QUIT_GAME = 1;
	private static final int DIALOG_LOAD_STATE = 2;
	private static final int DIALOG_SAVE_STATE = 3;

	private static final int GAMEPAD_LEFT_RIGHT =
			(Emulator.GAMEPAD_LEFT | Emulator.GAMEPAD_RIGHT);
	private static final int GAMEPAD_UP_DOWN =
			(Emulator.GAMEPAD_UP | Emulator.GAMEPAD_DOWN);
	private static final int GAMEPAD_DIRECTION =
			(GAMEPAD_UP_DOWN | GAMEPAD_LEFT_RIGHT);

	private static Emulator emulator;
	private static int resumeRequested;
	private static Thread emuThread;

	private EmulatorView emulatorView;
	private Keyboard keyboard;
	private VirtualKeypad keypad;
	private Trackball trackball;

	private String currentGame;
	private String lastPickedGame;
	private boolean isMenuShowing;
	private int quickLoadKey;
	private int quickSaveKey;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

		File datadir = getDir("data", MODE_PRIVATE);
		if (!initEmulator(datadir)) {
			finish();
			return;
		}
		setContentView(R.layout.main);
		emulatorView = (EmulatorView) findViewById(R.id.emulator);
		emulatorView.setEmulator(emulator);
		switchToView(R.id.empty);

		// create physical keyboard and trackball
		keyboard = new Keyboard(emulatorView, this);
		trackball = new Trackball(keyboard, this);

		// create virtual keypad
		keypad = (VirtualKeypad) findViewById(R.id.keypad);
		keypad.setGameKeyListener(this);

		// copy preset files
		copyAsset(new File(datadir, "game_config.txt"));

		// load settings
		SharedPreferences settings = getPreferences(MODE_PRIVATE);
		lastPickedGame = settings.getString("lastPickedGame", null);
		loadGlobalSettings();

		// restore state if any
		if (savedInstanceState != null)
			currentGame = savedInstanceState.getString("currentGame");
		switchToView(currentGame == null ? R.id.empty : R.id.game);

		// load BIOS
		if (loadBIOS(settings.getString("bios", null))) {
			// restore last running game
			String last = settings.getString("lastRunningGame", null);
			if (last != null) {
				saveLastRunningGame(null);
				if (new File(getGameStateFile(last, 0)).exists() &&
						loadROM(last, false))
					quickLoad();
			}
		}
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();

		if (isFinishing()) {
			resumeRequested = 0;
			emulator.cleanUp();
			emulator = null;
		}
	}

	@Override
	protected void onPause() {
		super.onPause();
		pauseEmulator();
	}

	@Override
	protected void onResume() {
		super.onResume();
		resumeEmulator();
	}

	@Override
	protected void onStop() {
		super.onStop();

		SharedPreferences.Editor editor = getPreferences(MODE_PRIVATE).edit();
		editor.putString("lastPickedGame", lastPickedGame);
		editor.commit();
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		super.onSaveInstanceState(outState);

		outState.putString("currentGame", currentGame);
	}

	@Override
	protected Dialog onCreateDialog(int id) {
		switch (id) {
		case DIALOG_QUIT_GAME:
			return createQuitGameDialog();
		case DIALOG_LOAD_STATE:
			return createLoadStateDialog();
		case DIALOG_SAVE_STATE:
			return createSaveStateDialog();
		}
		return super.onCreateDialog(id);
	}

	@Override
	protected void onPrepareDialog(int id, Dialog dialog) {
		super.onPrepareDialog(id, dialog);

		switch (id) {
		case DIALOG_QUIT_GAME:
		case DIALOG_LOAD_STATE:
		case DIALOG_SAVE_STATE:
			pauseEmulator();
			break;
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == quickLoadKey) {
			quickLoad();
			return true;
		}
		if (keyCode == quickSaveKey) {
			quickSave();
			return true;
		}
		if (keyCode == KeyEvent.KEYCODE_BACK && currentGame != null) {
			showDialog(DIALOG_QUIT_GAME);
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		super.onCreateOptionsMenu(menu);

		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		super.onPrepareOptionsMenu(menu);

		if (!isMenuShowing) {
			isMenuShowing = true;
			pauseEmulator();
		}
		menu.setGroupVisible(R.id.GAME_MENU, currentGame != null);
		return true;
	}

	@Override
	public void onOptionsMenuClosed(Menu menu) {
		super.onOptionsMenuClosed(menu);

		if (isMenuShowing) {
			isMenuShowing = false;
			resumeEmulator();
		}
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		if (isMenuShowing) {
			isMenuShowing = false;
			resumeEmulator();
		}

		switch (item.getItemId()) {
		case R.id.menu_open:
			onLoadROM();
			return true;

		case R.id.menu_settings:
			startActivityForResult(
				new Intent(this, GamePreferences.class), REQUEST_SETTINGS);
			return true;

		case R.id.menu_reset:
			emulator.reset();
			return true;

		case R.id.menu_save_state:
			showDialog(DIALOG_SAVE_STATE);
			return true;

		case R.id.menu_load_state:
			showDialog(DIALOG_LOAD_STATE);
			return true;

		case R.id.menu_close:
			unloadROM();
			return true;

		case R.id.menu_quit:
			finish();
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	@Override
	protected void onActivityResult(int request, int result, Intent data) {
		switch (request) {
		case REQUEST_BROWSE_ROM:
			if (result == RESULT_OK) {
				lastPickedGame = data.getStringExtra(
						FileChooser.EXTRA_FILEPATH);
				loadROM(lastPickedGame);
			}
			break;

		case REQUEST_BROWSE_BIOS:
			loadBIOS(result == RESULT_OK ?
				data.getStringExtra(FileChooser.EXTRA_FILEPATH) : null);
			break;

		case REQUEST_SETTINGS:
			loadGlobalSettings();
			break;
		}
	}

	public void onGameKeyChanged() {
		int states = 0;
		states |= keyboard.getKeyStates();
		states |= keypad.getKeyStates();

		if ((states & GAMEPAD_DIRECTION) != 0)
			trackball.reset();
		else
			states |= trackball.getKeyStates();

		// resolve conflict keys
		if ((states & GAMEPAD_LEFT_RIGHT) == GAMEPAD_LEFT_RIGHT)
			states &= ~GAMEPAD_LEFT_RIGHT;
		if ((states & GAMEPAD_UP_DOWN) == GAMEPAD_UP_DOWN)
			states &= ~GAMEPAD_UP_DOWN;

		emulator.setKeyStates(states);
	}

	public void onCancel(DialogInterface dialog) {
		resumeEmulator();
	}

	private boolean initEmulator(File datadir) {
		if (emulator != null)
			return true;

		// FIXME
		final String libdir = "/data/data/" + getPackageName() + "/lib";
		emulator = new Emulator();
		if (!emulator.initialize(libdir, datadir.getAbsolutePath()))
			return false;

		if (emuThread == null) { 
			emuThread = new Thread() {
				public void run() {
					emulator.run();
				}
			};
			emuThread.start();
		}
		return true;
	}

	private void resumeEmulator() {
		if (resumeRequested++ == 0) {
			keyboard.reset();
			keypad.reset();
			trackball.reset();
			onGameKeyChanged();

			emulator.resume();
		}
	}

	private void pauseEmulator() {
		if (--resumeRequested == 0)
			emulator.pause();
	}

	private boolean copyAsset(File file) {
		if (file.exists())
			return true;

		InputStream in = null;
		OutputStream out = null;

		try {
			in = getAssets().open(file.getName());
			out = new FileOutputStream(file);

			byte[] buf = new byte[8192];
			int len;
			while ((len = in.read(buf)) > 0)
				out.write(buf, 0, len);

		} catch (Exception e) {
			e.printStackTrace();
			return false;

		} finally {
			try {
				if (out != null)
					out.close();
				if (in != null)
					in.close();
			} catch (IOException e) {
			}
		}
		return true;
	}

	private static int getScalingMode(String mode) {
		if (mode.equals("original"))
			return EmulatorView.SCALING_ORIGINAL;
		if (mode.equals("proportional"))
			return EmulatorView.SCALING_PROPORTIONAL;
		return EmulatorView.SCALING_STRETCH;
	}

	private void saveLastRunningGame(String game) {
		SharedPreferences.Editor editor = getPreferences(MODE_PRIVATE).edit();
		editor.putString("lastRunningGame", game);
		editor.commit();
	}

	private void loadGlobalSettings() {
		pauseEmulator();

		SharedPreferences settings =
				PreferenceManager.getDefaultSharedPreferences(this);
		emulator.setOption("autoFrameSkip",
				settings.getBoolean("autoFrameSkip", true));
		emulator.setOption("maxFrameSkips",
				Integer.toString(settings.getInt("maxFrameSkips", 2)));
		emulator.setOption("soundEnabled", 
				settings.getBoolean("soundEnabled", true));

		trackball.setEnabled(settings.getBoolean("enableTrackball", false));
		keypad.setVisibility(settings.getBoolean("enableVirtualKeypad",
				GamePreferences.getDefaultVirtualKeypadEnabled(this)) ?
						View.VISIBLE : View.GONE);

		emulatorView.setScalingMode(getScalingMode(
				settings.getString("scalingMode", "stretch")));

		// key bindings
		final int[] gameKeys = GamePreferences.gameKeys;
		final String[] prefKeys = GamePreferences.keyPrefKeys;
		final int[] defaultKeys = GamePreferences.getDefaultKeys(this);

		keyboard.clearKeyMap();
		for (int i = 0; i < prefKeys.length; i++) {
			keyboard.mapKey(gameKeys[i],
					settings.getInt(prefKeys[i], defaultKeys[i]));
		}

		// shortcut keys
		quickLoadKey = settings.getInt("quickLoad", 0);
		quickSaveKey = settings.getInt("quickSave", 0);

		resumeEmulator();
	}

	private void switchToView(int id) {
		final int viewIds[] = {
			R.id.empty,
			R.id.game
		};
		for (int i = 0; i < viewIds.length; i++) {
			findViewById(viewIds[i]).setVisibility(
					viewIds[i] == id ? View.VISIBLE : View.INVISIBLE);
		}
	}

	private Dialog createLoadStateDialog() {
		DialogInterface.OnClickListener l =
			new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					loadGameState(which);
					resumeEmulator();
				}
			};

		return new AlertDialog.Builder(this).
			setTitle(R.string.load_state_title).
			setItems(R.array.game_state_slots, l).
			setOnCancelListener(this).create();
	}

	private Dialog createSaveStateDialog() {
		DialogInterface.OnClickListener l =
			new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					saveGameState(which);
					resumeEmulator();
				}
			};

		return new AlertDialog.Builder(this).
			setTitle(R.string.save_state_title).
			setItems(R.array.game_state_slots, l).
			setOnCancelListener(this).create();
	}

	private Dialog createQuitGameDialog() {
		DialogInterface.OnClickListener l =
			new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					switch (which) {
					case 0:
						resumeEmulator();
						onLoadROM();
						break;
					case 1:
						quickSave();
						saveLastRunningGame(currentGame);
						// fall through
					case 2:
						finish();
						break;
					}
				}
			};

		return new AlertDialog.Builder(this).
			setTitle(R.string.quit_game_title).
			setItems(R.array.exit_game_options, l).
			setOnCancelListener(this).create();
	}

	private void browseBIOS(String initial) {
		Intent intent = new Intent(this, FileChooser.class);
		intent.putExtra(FileChooser.EXTRA_TITLE,
				getResources().getString(R.string.title_select_bios));
		intent.putExtra(FileChooser.EXTRA_FILEPATH, initial);
		intent.putExtra(FileChooser.EXTRA_FILTERS, new String[] { ".bin" });
		startActivityForResult(intent, REQUEST_BROWSE_BIOS);
	}

	private boolean loadBIOS(String name) {
		if (name != null && emulator.loadBIOS(name)) {
			SharedPreferences.Editor editor =
					getPreferences(MODE_PRIVATE).edit();
			editor.putString("bios", name);
			editor.commit();
			return true;
		}

		final String biosFileName = name;
		DialogInterface.OnClickListener l =
			new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					switch (which) {
					case DialogInterface.BUTTON_POSITIVE:
						browseBIOS(biosFileName);
						break;
					case DialogInterface.BUTTON_NEGATIVE:
						finish();
						break;
					}
				}
			};

		new AlertDialog.Builder(this).
			setCancelable(false).
			setTitle(R.string.load_bios_title).
			setMessage(name == null ?
				R.string.bios_not_found : R.string.load_bios_failed).
			setPositiveButton(R.string.browse_bios, l).
			setNegativeButton(R.string.quit, l).
			show();

		return false;
	}

	private boolean loadROM(String fname) {
		return loadROM(fname, true);
	}

	private boolean loadROM(String fname, boolean failPrompt) {
		unloadROM();
		if (!emulator.loadROM(fname) && failPrompt) {
			Toast.makeText(this, R.string.load_rom_failed,
					Toast.LENGTH_SHORT).show();
			return false;
		}
		currentGame = fname;
		switchToView(R.id.game);
		return true;
	}

	private void unloadROM() {
		if (currentGame != null) {
			emulator.unloadROM();
			currentGame = null;
			switchToView(R.id.empty);
		}
	}

	private void onLoadROM() {
		Intent intent = new Intent(this, FileChooser.class);
		intent.putExtra(FileChooser.EXTRA_TITLE,
				getResources().getString(R.string.title_select_rom));
		intent.putExtra(FileChooser.EXTRA_FILEPATH, lastPickedGame);
		intent.putExtra(FileChooser.EXTRA_FILTERS,
				new String[] { ".gba", ".bin", ".zip" });
		startActivityForResult(intent, REQUEST_BROWSE_ROM);
	}

	private void saveGameState(int slot) {
		String fname = getGameStateFile(currentGame, slot);
		emulator.saveState(fname);
	}

	private void loadGameState(int slot) {
		String fname = getGameStateFile(currentGame, slot);
		if (new File(fname).exists())
			emulator.loadState(fname);
	}

	private void quickSave() {
		saveGameState(0);
	}

	private void quickLoad() {
		loadGameState(0);
	}

	private static String getGameStateFile(String name, int slot) {
		int i = name.lastIndexOf('.');
		if (i >= 0)
			name = name.substring(0, i);
		name += ".ss" + slot;
		return name;
	}
}
