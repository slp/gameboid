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

#ifndef MAIN_H
#define MAIN_H

typedef enum
{
  TIMER_INACTIVE,
  TIMER_PRESCALE,
  TIMER_CASCADE
} timer_status_type;

typedef enum
{
  TIMER_NO_IRQ,
  TIMER_TRIGGER_IRQ
} timer_irq_type;


typedef enum
{
  TIMER_DS_CHANNEL_NONE,
  TIMER_DS_CHANNEL_A,
  TIMER_DS_CHANNEL_B,
  TIMER_DS_CHANNEL_BOTH
} timer_ds_channel_type;

typedef struct
{
  s32 count;
  u32 reload;
  u32 prescale;
  u32 stop_cpu_ticks;
  fixed16_16 frequency_step;
  timer_ds_channel_type direct_sound_channels;
  timer_irq_type irq;
  timer_status_type status;
} timer_type;

typedef enum
{
  auto_frameskip,
  manual_frameskip,
  no_frameskip
} frameskip_type;

extern u32 cpu_ticks;
extern u32 frame_ticks;
extern u32 execute_cycles;
extern frameskip_type current_frameskip_type;
extern u32 frameskip_value;
extern u32 random_skip;
extern u32 global_cycles_per_instruction;
extern u32 synchronize_flag;
extern u32 skip_next_frame;

extern timer_type timer[4];
static u32 prescale_table[] = { 0, 6, 8, 10 };

extern u32 cycle_memory_access;
extern u32 cycle_pc_relative_access;
extern u32 cycle_sp_relative_access;
extern u32 cycle_block_memory_access;
extern u32 cycle_block_memory_sp_access;
extern u32 cycle_block_memory_words;
extern u32 cycle_dma16_words;
extern u32 cycle_dma32_words;
extern u32 flush_ram_count;

extern u64 base_timestamp;

extern u8 main_path[512];

extern u32 update_backup_flag;
extern u32 clock_speed;

void init_main();
u32 update_gba();
void reset_gba();
void synchronize();
void quit();
void delay_us(u32 us_count);
void get_ticks_us(u64 *tick_return);
void game_name_ext(u8 *src, u8 *buffer, u8 *extension);
void main_write_mem_savestate(file_tag_type savestate_file);
void main_read_savestate(file_tag_type savestate_file);


extern u64 frame_count_initial_timestamp;
extern u32 real_frame_count;
extern u32 virtual_frame_count;
extern u32 max_frameskip;
extern u32 num_skipped_frames;


#define count_timer(timer_number)                                             \
  timer[timer_number].reload = 0x10000 - value;                               \
  if(timer_number < 2)                                                        \
  {                                                                           \
    u32 timer_reload =                                                        \
     timer[timer_number].reload << timer[timer_number].prescale;              \
    sound_update_frequency_step(timer_number);                                \
  }                                                                           \

#define adjust_sound_buffer(timer_number, channel)                            \
  if(timer[timer_number].direct_sound_channels & (0x01 << channel))           \
  {                                                                           \
    direct_sound_channel[channel].buffer_index =                              \
     (direct_sound_channel[channel].buffer_index + buffer_adjust) %           \
     BUFFER_SIZE;                                                             \
  }                                                                           \

#define trigger_timer(timer_number)                                           \
  if(value & 0x80)                                                            \
  {                                                                           \
    if(timer[timer_number].status == TIMER_INACTIVE)                          \
    {                                                                         \
      u32 prescale = prescale_table[value & 0x03];                            \
      u32 timer_reload = timer[timer_number].reload;                          \
                                                                              \
      if((value >> 2) & 0x01)                                                 \
        timer[timer_number].status = TIMER_CASCADE;                           \
      else                                                                    \
        timer[timer_number].status = TIMER_PRESCALE;                          \
                                                                              \
      timer[timer_number].prescale = prescale;                                \
      timer[timer_number].irq = (value >> 6) & 0x01;                          \
                                                                              \
      address16(io_registers, 0x100 + (timer_number * 4)) =                   \
       -timer_reload;                                                         \
                                                                              \
      timer_reload <<= prescale;                                              \
      timer[timer_number].count = timer_reload;                               \
                                                                              \
      if(timer_reload < execute_cycles)                                       \
        execute_cycles = timer_reload;                                        \
                                                                              \
      if(timer_number < 2)                                                    \
      {                                                                       \
        u32 buffer_adjust =                                                   \
         (u32)(((float)(cpu_ticks - timer[timer_number].stop_cpu_ticks) *     \
         sound_frequency) / 16777216.0) * 2;                                  \
                                                                              \
        sound_update_frequency_step(timer_number);                            \
        adjust_sound_buffer(timer_number, 0);                                 \
        adjust_sound_buffer(timer_number, 1);                                 \
      }                                                                       \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    if(timer[timer_number].status != TIMER_INACTIVE)                          \
    {                                                                         \
      timer[timer_number].status = TIMER_INACTIVE;                            \
      timer[timer_number].stop_cpu_ticks = cpu_ticks;                         \
    }                                                                         \
  }                                                                           \
  address16(io_registers, 0x102 + (timer_number * 4)) = value;                \

void change_ext(u8 *src, u8 *buffer, u8 *extension);

#endif


