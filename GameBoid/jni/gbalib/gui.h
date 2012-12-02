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

#ifndef GUI_H
#define GUI_H

#define GPSP_CONFIG_FILENAME "gpsp.cfg"

s32 load_file(u8 **wildcards, u8 *result);
u32 adjust_frameskip(u32 button_id);
s32 load_game_config_file();
s32 load_config_file();
s32 save_game_config_file();
s32 save_config_file();
u32 menu(u16 *original_screen);

extern u32 savestate_slot;

void get_savestate_filename_noshot(u32 slot, u8 *name_buffer);
void get_savestate_filename(u32 slot, u8 *name_buffer);
void get_savestate_snapshot(u8 *savestate_filename);

#endif

