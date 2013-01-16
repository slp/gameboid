package com.androidemu;

import java.io.File;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;

import com.androidemu.FileChooser;

import com.androidemu.gba.R;
import com.androidemu.gba.Emulator;
import com.androidemu.gba.EmulatorView;
import com.androidemu.gba.GamePreferences;
import com.androidemu.gba.UserPrefs;
import com.androidemu.gba.input.GameKeyListener;
import com.androidemu.gba.input.Keyboard;
import com.androidemu.gba.input.Keycodes;
import com.androidemu.gba.input.VirtualKeypad;
import com.androidemu.gba.input.Trackball;

import com.androidemu.wrapper.Wrapper;

public class EmulatorActivity extends GameActivity implements GameKeyListener
{
	private static final int REQUEST_BROWSE_ROM = 1;
	private static final int REQUEST_BROWSE_BIOS = 2;
	private static final int REQUEST_SETTINGS = 3;

	private static final int DIALOG_LOAD_STATE = 2;
	private static final int DIALOG_SAVE_STATE = 3;

	private static final int GAMEPAD_LEFT_RIGHT = (Keycodes.GAMEPAD_LEFT | Keycodes.GAMEPAD_RIGHT);
	private static final int GAMEPAD_UP_DOWN = (Keycodes.GAMEPAD_UP | Keycodes.GAMEPAD_DOWN);
	private static final int GAMEPAD_DIRECTION = (GAMEPAD_UP_DOWN | GAMEPAD_LEFT_RIGHT);

	private static Emulator emulator;

	private EmulatorView emulatorView;
	private View placeholder;
	private Keyboard keyboard;
	private VirtualKeypad keypad;
	private Trackball trackball;

	private String currentGame;
	private String lastPickedGame;
	private int quickLoadKey;
	private int quickSaveKey;
	
	private UserPrefs cfg;

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		cfg = new UserPrefs(getApplication().getApplicationContext());
		
		File datadir = getDir("data", MODE_PRIVATE);
		emulator = Emulator.createInstance(this, datadir);
		
		if (emulator == null)
		{
			finish();
			return;
		}
		setContentView(R.layout.main);

		emulatorView = (EmulatorView) findViewById(R.id.emulator);
		placeholder = findViewById(R.id.empty);

		emulatorView.setEmulator(emulator);

		// create physical keyboard and trackball
		keyboard = new Keyboard(emulatorView, this);
		trackball = new Trackball(keyboard, this);

		keypad = (VirtualKeypad) findViewById(R.id.keypad);
		keypad.setGameKeyListener(this);

		// copy preset files
		extractAsset(new File(datadir, "game_config.txt"));

		// load settings
		lastPickedGame = cfg.lastPickedGame;
		loadGlobalSettings();
		
		currentGame = cfg.lastRunningGame;

		// restore state if any
		if (savedInstanceState != null)
		{
			currentGame = savedInstanceState.getString("currentGame");
		}
		
		// TODO: find way to set view as default for sure 
		showPlaceholder();
		
		if (currentGame != null)
		{
			debug("Last running game: " + currentGame);

			if (loadBIOS(cfg.bios))
			{
				String last = currentGame;
				
				if (new File(getGameStateFile(last, 0)).exists() && loadROM(last, false))
				{
					quickLoad();
				}
				
				hidePlaceholder();
			}
		}
	}

	protected void initResources()
	{
		super.initResources();
		
		cfg = new UserPrefs(getApplicationContext());
	}
	
	@Override
	protected void onPause()
	{
		cfg.setLastRunningGame(currentGame);

		super.onPause();
	}
	
	@Override
	protected void onDestroy()
	{
		super.onDestroy();

		if (isFinishing())
		{
			emulator.cleanUp();
			emulator = null;
		}
	}
	
	@Override
	protected void onSaveInstanceState(Bundle outState)
	{
		debug("onSaveInstanceState");
		
		super.onSaveInstanceState(outState);

		outState.putString("currentGame", currentGame);
	}

	@Override
	protected Dialog onCreateDialog(int id)
	{
		switch (id)
		{
			case DIALOG_EXIT_PROMPT:
				return createQuitGameDialog();
			case DIALOG_LOAD_STATE:
				return createLoadStateDialog();
			case DIALOG_SAVE_STATE:
				return createSaveStateDialog();
		}
		return super.onCreateDialog(id);
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event)
	{
		if (event.getKeyCode() == KeyEvent.KEYCODE_BACK
				&& Wrapper.KeyEvent_isLongPress(event))
		{
			if (Wrapper.SDK_INT < 11)
			{
				openOptionsMenu();
			}
			else
			{
				uiHider.show();
			}
			return true;
		}
		
		return keyboard.onKey(null, event.getKeyCode(), event) || super.dispatchKeyEvent(event);
	}

	@SuppressLint("NewApi")
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		if (keyCode == quickLoadKey)
		{
			quickLoad();
			return true;
		}
		else if (keyCode == quickSaveKey)
		{
			quickSave();
			return true;
		}
		else
			return super.onKeyDown(keyCode, event);
	}

	@SuppressWarnings("deprecation")
	@Override
	public void onBackPressed()
	{
		if (currentGame != null)
		{
			showDialog(DIALOG_EXIT_PROMPT);
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		super.onCreateOptionsMenu(menu);

		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu)
	{
		menu.setGroupVisible(R.id.GAME_MENU, currentGame != null);
		
		return super.onPrepareOptionsMenu(menu);
	}

	@SuppressWarnings("deprecation")
	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		if (super.onOptionsItemSelected(item))
		{
			return true;
		}
		
		switch (item.getItemId())
		{
			case R.id.menu_open:
				onLoadROM();
				return true;

			case R.id.menu_settings:
				startActivityForResult(new Intent(this, GamePreferences.class),
						REQUEST_SETTINGS);
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
		
		return false;
	}

	@Override
	protected void onActivityResult(int request, int result, Intent data)
	{
		super.onActivityResult(request, result, data);
		
		switch (request)
		{
			case REQUEST_BROWSE_ROM:
				if (result == RESULT_OK)
				{
					lastPickedGame = data.getData().getPath();
					cfg.setLastPickedGame(lastPickedGame);
					loadROM(lastPickedGame);
				}
				break;

			case REQUEST_BROWSE_BIOS:
				loadBIOS(result == RESULT_OK ? data.getData().getPath() : null);
				break;

			case REQUEST_SETTINGS:
				loadGlobalSettings();
				break;
		}
	}

	public void onGameKeyChanged()
	{
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
		if ((states & GAMEPAD_UP_DOWN) == GAMEPAD_UP_DOWN) states &= ~GAMEPAD_UP_DOWN;

		emulator.setKeyStates(states);
	}

	@Override
	protected void resumeGame()
	{
		super.resumeGame();

		keyboard.reset();
		keypad.reset();
		trackball.reset();
		onGameKeyChanged();
		
		emulator.resume();
	}

	@Override
	protected void pauseGame()
	{
		super.pauseGame();

		emulator.pause();
	}

	private static int getScalingMode(String mode)
	{
		if (mode.equals("original")) return EmulatorView.SCALING_ORIGINAL;
		if (mode.equals("proportional")) return EmulatorView.SCALING_PROPORTIONAL;
		return EmulatorView.SCALING_STRETCH;
	}

	private void loadGlobalSettings()
	{
		emulator.setOption("autoFrameSkip", cfg.autoFrameSkip);
		emulator.setOption("maxFrameSkips", cfg.maxFrameSkips);
		
		emulator.setOption("soundEnabled", cfg.soundEnabled);

		trackball.setEnabled(cfg.enableTrackball);
		
		keypad.setVisibility(cfg.enableVirtualKeypad ? View.VISIBLE : View.GONE);

		emulatorView.setScalingMode(getScalingMode(cfg.scalingMode));

		// key bindings
		final int[] gameKeys = GamePreferences.gameKeys;

		keyboard.clearKeyMap();
		for (int i = 0; i < gameKeys.length; i++)
		{
			keyboard.mapKey(gameKeys[i], cfg.keysMap[i]);
		}

		// shortcut keys
		quickLoadKey = cfg.quickLoad;
		quickSaveKey = cfg.quickSave;
	}

	private void showPlaceholder()
	{
		emulatorView.setVisibility(View.INVISIBLE);
		placeholder.setVisibility(View.VISIBLE);
	}

	private void hidePlaceholder()
	{
		placeholder.setVisibility(View.GONE);
		emulatorView.setVisibility(View.VISIBLE);
	}

	private Dialog createLoadStateDialog()
	{
		DialogInterface.OnClickListener l = new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				loadGameState(which);
			}
		};

		return new AlertDialog.Builder(this)
			.setTitle(R.string.load_state_title)
			.setItems(R.array.game_state_slots, l)
			.create();
	}

	private Dialog createSaveStateDialog()
	{
		DialogInterface.OnClickListener l = new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				saveGameState(which);
			}
		};

		return new AlertDialog.Builder(this)
			.setTitle(R.string.save_state_title)
			.setItems(R.array.game_state_slots, l)
			.create();
	}
	
	private Dialog createQuitGameDialog()
	{
		DialogInterface.OnClickListener l = new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				switch (which)
				{
					case 0:
						// game is resumed after dismissing
						break;
					case 1:
						onLoadROM();
						break;
					case 2:
						quickSave();
						// fall through
					case 3:
						finish();
						break;
				}
			}
		};

		return new AlertDialog.Builder(this)
			.setTitle(R.string.quit_game_title)
			.setItems(R.array.exit_game_options, l)
			.create();
	}

	private void browseBIOS(String initial)
	{
		Intent intent = new Intent(this, FileChooser.class);
		intent.putExtra(FileChooser.EXTRA_TITLE,
				getResources().getString(R.string.title_select_bios));
		intent.setData(initial == null ? null : Uri.fromFile(new File(initial)));
		intent.putExtra(FileChooser.EXTRA_FILTERS, new String[] { ".bin" });
		startActivityForResult(intent, REQUEST_BROWSE_BIOS);
	}

	private boolean loadBIOS(String name)
	{
		if (name != null && emulator.loadBIOS(name))
		{
			cfg.setBIOS(name);
			return true;
		}

		final String biosFileName = name;
		DialogInterface.OnClickListener l = new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				switch (which)
				{
					case DialogInterface.BUTTON_POSITIVE:
						browseBIOS(biosFileName);
						break;
					case DialogInterface.BUTTON_NEGATIVE:
						finish();
						break;
				}
			}
		};

		new AlertDialog.Builder(this)
				.setCancelable(false)
				.setTitle(R.string.load_bios_title)
				.setMessage(
						name == null ? R.string.bios_not_found
								: R.string.load_bios_failed)
				.setPositiveButton(R.string.browse_bios, l)
				.setNegativeButton(R.string.quit, l).show();

		return false;
	}

	private boolean loadROM(String fname)
	{
		return loadROM(fname, true);
	}

	private boolean loadROM(String fname, boolean failPrompt)
	{
		cfg.setLastRunningGame(null);

		unloadROM();
		if (!emulator.loadROM(fname) && failPrompt)
		{
			Toast.makeText(this, R.string.load_rom_failed, Toast.LENGTH_SHORT).show();
			return false;
		}
		currentGame = fname;
		hidePlaceholder();

		return true;
	}

	private void unloadROM()
	{
		if (currentGame != null)
		{
			emulator.unloadROM();
			currentGame = null;
			showPlaceholder();
		}
	}

	private void onLoadROM()
	{
		Intent intent = new Intent(this, FileChooser.class);
		intent.putExtra(FileChooser.EXTRA_TITLE,
				getResources().getString(R.string.title_select_rom));
		intent.setData(lastPickedGame == null ? null : Uri.fromFile(new File(
				lastPickedGame)));
		intent.putExtra(FileChooser.EXTRA_FILTERS,
				new String[] { ".gba", ".bin", ".zip" });
		startActivityForResult(intent, REQUEST_BROWSE_ROM);
	}

	private void saveGameState(int slot)
	{
		String fname = getGameStateFile(currentGame, slot);
		emulator.saveState(fname);
	}

	private void loadGameState(int slot)
	{
		String fname = getGameStateFile(currentGame, slot);
		if (new File(fname).exists()) emulator.loadState(fname);
	}

	private void quickSave()
	{
		saveGameState(0);
	}

	private void quickLoad()
	{
		loadGameState(0);
	}

	private static String getGameStateFile(String name, int slot)
	{
		int i = name.lastIndexOf('.');
		if (i >= 0) name = name.substring(0, i);
		name += ".ss" + slot;
		return name;
	}
}
