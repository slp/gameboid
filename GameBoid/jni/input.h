/* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef INPUT_H
#define INPUT_H

typedef enum
{
  BUTTON_L = 0x200,
  BUTTON_R = 0x100,
  BUTTON_DOWN = 0x80,
  BUTTON_UP = 0x40,
  BUTTON_LEFT = 0x20,
  BUTTON_RIGHT = 0x10,
  BUTTON_START = 0x08,
  BUTTON_SELECT = 0x04,
  BUTTON_B = 0x02,
  BUTTON_A = 0x01,
  BUTTON_NONE = 0x00
} input_buttons_type;

typedef enum
{
  BUTTON_ID_UP,
  BUTTON_ID_DOWN,
  BUTTON_ID_LEFT,
  BUTTON_ID_RIGHT,
  BUTTON_ID_A,
  BUTTON_ID_B,
  BUTTON_ID_L,
  BUTTON_ID_R,
  BUTTON_ID_START,
  BUTTON_ID_SELECT,
  BUTTON_ID_MENU,
  BUTTON_ID_FASTFORWARD,
  BUTTON_ID_LOADSTATE,
  BUTTON_ID_SAVESTATE,
  BUTTON_ID_RAPIDFIRE_A,
  BUTTON_ID_RAPIDFIRE_B,
  BUTTON_ID_RAPIDFIRE_L,
  BUTTON_ID_RAPIDFIRE_R,
  BUTTON_ID_VOLUP,
  BUTTON_ID_VOLDOWN,
  BUTTON_ID_FPS,
  BUTTON_ID_NONE
} input_buttons_id_type;

typedef enum
{
  CURSOR_UP,
  CURSOR_DOWN,
  CURSOR_LEFT,
  CURSOR_RIGHT,
  CURSOR_SELECT,
  CURSOR_BACK,
  CURSOR_EXIT,
  CURSOR_NONE
} gui_action_type;

void init_input();
void update_input();
gui_action_type get_gui_input();
gui_action_type get_gui_input_fs_hold(u32 button_id);
void input_write_mem_savestate(file_tag_type savestate_file);
void input_read_savestate(file_tag_type savestate_file);

extern u32 gamepad_config_map[16];
extern u32 global_enable_analog;
extern u32 analog_sensitivity_level;

#endif

