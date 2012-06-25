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

#ifndef SOUND_H
#define SOUND_H

#define BUFFER_SIZE 65536

#define SOUND_BUFFER_SIZE 4096

typedef enum
{
  DIRECT_SOUND_INACTIVE,
  DIRECT_SOUND_RIGHT,
  DIRECT_SOUND_LEFT,
  DIRECT_SOUND_LEFTRIGHT
} direct_sound_status_type;

typedef enum
{
  DIRECT_SOUND_VOLUME_50,
  DIRECT_SOUND_VOLUME_100
} direct_sound_volume_type;

typedef struct
{
  s8 fifo[32];
  u32 fifo_base;
  u32 fifo_top;
  fixed16_16 fifo_fractional;
  // The + 1 is to give some extra room for linear interpolation
  // when wrapping around.
  u32 buffer_index;
  direct_sound_status_type status;
  direct_sound_volume_type volume;
  u32 last_cpu_ticks;
} direct_sound_struct;

typedef enum
{
  GBC_SOUND_INACTIVE,
  GBC_SOUND_RIGHT,
  GBC_SOUND_LEFT,
  GBC_SOUND_LEFTRIGHT
} gbc_sound_status_type;


typedef struct
{
  u32 rate;
  fixed16_16 frequency_step;
  fixed16_16 sample_index;
  fixed16_16 tick_counter;
  u32 total_volume;
  u32 envelope_initial_volume;
  u32 envelope_volume;
  u32 envelope_direction;
  u32 envelope_status;
  u32 envelope_step;
  u32 envelope_ticks;
  u32 envelope_initial_ticks;
  u32 sweep_status;
  u32 sweep_direction;
  u32 sweep_ticks;
  u32 sweep_initial_ticks;
  u32 sweep_shift;
  u32 length_status;
  u32 length_ticks;
  u32 noise_type;
  u32 wave_type;
  u32 wave_bank;
  u32 wave_volume;
  gbc_sound_status_type status;
  u32 active_flag;
  u32 master_enable;
  s8 *sample_data;
} gbc_sound_struct;

extern direct_sound_struct direct_sound_channel[2];
extern gbc_sound_struct gbc_sound_channel[4];
extern s8 square_pattern_duty[4][8];
extern u32 gbc_sound_master_volume_left;
extern u32 gbc_sound_master_volume_right;
extern u32 gbc_sound_master_volume;

extern u32 sound_frequency;
extern u32 sound_on;

extern u32 global_enable_audio;
extern u32 enable_low_pass_filter;
extern u32 audio_buffer_size_number;

void sound_timer_queue8(u32 channel, u8 value);
void sound_timer_queue16(u32 channel, u16 value);
void sound_timer_queue32(u32 channel, u32 value);
void sound_timer(fixed16_16 frequency_step, u32 channel);
void sound_reset_fifo(u32 channel);
void update_gbc_sound(u32 cpu_ticks);
void init_sound();
void sound_write_mem_savestate(file_tag_type savestate_file);
void sound_read_savestate(file_tag_type savestate_file);

#define gbc_sound_tone_control_low(channel, address)                          \
{                                                                             \
  u32 initial_volume = (value >> 12) & 0x0F;                                  \
  u32 envelope_ticks = ((value >> 8) & 0x07) * 4;                             \
  gbc_sound_channel[channel].length_ticks = 64 - (value & 0x3F);              \
  gbc_sound_channel[channel].sample_data =                                    \
   square_pattern_duty[(value >> 6) & 0x03];                                  \
  gbc_sound_channel[channel].envelope_direction = (value >> 11) & 0x01;       \
  gbc_sound_channel[channel].envelope_initial_volume = initial_volume;        \
  gbc_sound_channel[channel].envelope_volume = initial_volume;                \
  gbc_sound_channel[channel].envelope_initial_ticks = envelope_ticks;         \
  gbc_sound_channel[channel].envelope_ticks = envelope_ticks;                 \
  gbc_sound_channel[channel].envelope_status = (envelope_ticks != 0);         \
  gbc_sound_channel[channel].envelope_volume = initial_volume;                \
  gbc_sound_update = 1;                                                       \
  address16(io_registers, address) = value;                                   \
}                                                                             \

#define gbc_sound_tone_control_high(channel, address)                         \
{                                                                             \
  u32 rate = value & 0x7FF;                                                   \
  gbc_sound_channel[channel].rate = rate;                                     \
  gbc_sound_channel[channel].frequency_step =                                 \
   float_to_fp16_16(((131072.0 / (2048 - rate)) * 8.0) / sound_frequency);    \
  gbc_sound_channel[channel].length_status = (value >> 14) & 0x01;            \
  if(value & 0x8000)                                                          \
  {                                                                           \
    gbc_sound_channel[channel].active_flag = 1;                               \
    gbc_sound_channel[channel].sample_index -= float_to_fp16_16(1.0 / 12.0);  \
    gbc_sound_channel[channel].envelope_ticks =                               \
     gbc_sound_channel[channel].envelope_initial_ticks;                       \
    gbc_sound_channel[channel].envelope_volume =                              \
     gbc_sound_channel[channel].envelope_initial_volume;                      \
  }                                                                           \
                                                                              \
  gbc_sound_update = 1;                                                       \
  address16(io_registers, address) = value;                                   \
}                                                                             \

#define gbc_sound_tone_control_sweep()                                        \
{                                                                             \
  u32 sweep_ticks = ((value >> 4) & 0x07) * 2;                                \
  gbc_sound_channel[0].sweep_shift = value & 0x07;                            \
  gbc_sound_channel[0].sweep_direction = (value >> 3) & 0x01;                 \
  gbc_sound_channel[0].sweep_status = (value != 8);                           \
  gbc_sound_channel[0].sweep_ticks = sweep_ticks;                             \
  gbc_sound_channel[0].sweep_initial_ticks = sweep_ticks;                     \
  gbc_sound_update = 1;                                                       \
  address16(io_registers, 0x60) = value;                                      \
}                                                                             \

#define gbc_sound_wave_control()                                              \
{                                                                             \
  gbc_sound_channel[2].wave_type = (value >> 5) & 0x01;                       \
  gbc_sound_channel[2].wave_bank = (value >> 6) & 0x01;                       \
  if(value & 0x80)                                                            \
  {                                                                           \
    gbc_sound_channel[2].master_enable = 1;                                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    gbc_sound_channel[2].master_enable = 0;                                   \
  }                                                                           \
                                                                              \
  gbc_sound_update = 1;                                                       \
  address16(io_registers, 0x70) = value;                                      \
}                                                                             \

static u32 gbc_sound_wave_volume[4] = { 0, 16384, 8192, 4096 };

#define gbc_sound_tone_control_low_wave()                                     \
{                                                                             \
  gbc_sound_channel[2].length_ticks = 256 - (value & 0xFF);                   \
  if((value >> 15) & 0x01)                                                    \
  {                                                                           \
    gbc_sound_channel[2].wave_volume = 12288;                                 \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    gbc_sound_channel[2].wave_volume =                                        \
     gbc_sound_wave_volume[(value >> 13) & 0x03];                             \
  }                                                                           \
  gbc_sound_update = 1;                                                       \
  address16(io_registers, 0x72) = value;                                      \
}                                                                             \

#define gbc_sound_tone_control_high_wave()                                    \
{                                                                             \
  u32 rate = value & 0x7FF;                                                   \
  gbc_sound_channel[2].rate = rate;                                           \
  gbc_sound_channel[2].frequency_step =                                       \
   float_to_fp16_16((2097152.0 / (2048 - rate)) / sound_frequency);           \
  gbc_sound_channel[2].length_status = (value >> 14) & 0x01;                  \
  if(value & 0x8000)                                                          \
  {                                                                           \
    gbc_sound_channel[2].sample_index = 0;                                    \
    gbc_sound_channel[2].active_flag = 1;                                     \
  }                                                                           \
  gbc_sound_update = 1;                                                       \
  address16(io_registers, 0x74) = value;                                      \
}                                                                             \

#define gbc_sound_noise_control()                                             \
{                                                                             \
  u32 dividing_ratio = value & 0x07;                                          \
  u32 frequency_shift = (value >> 4) & 0x0F;                                  \
  if(dividing_ratio == 0)                                                     \
  {                                                                           \
    gbc_sound_channel[3].frequency_step =                                     \
     float_to_fp16_16(1048576.0 / (1 << (frequency_shift + 1)) /              \
     sound_frequency);                                                        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    gbc_sound_channel[3].frequency_step =                                     \
     float_to_fp16_16(524288.0 / (dividing_ratio *                            \
     (1 << (frequency_shift + 1))) / sound_frequency);                        \
  }                                                                           \
  gbc_sound_channel[3].noise_type = (value >> 3) & 0x01;                      \
  gbc_sound_channel[3].length_status = (value >> 14) & 0x01;                  \
  if(value & 0x8000)                                                          \
  {                                                                           \
    gbc_sound_channel[3].sample_index = 0;                                    \
    gbc_sound_channel[3].active_flag = 1;                                     \
    gbc_sound_channel[3].envelope_ticks =                                     \
     gbc_sound_channel[3].envelope_initial_ticks;                             \
    gbc_sound_channel[3].envelope_volume =                                    \
     gbc_sound_channel[3].envelope_initial_volume;                            \
  }                                                                           \
  gbc_sound_update = 1;                                                       \
  address16(io_registers, 0x7C) = value;                                      \
}                                                                             \

#define gbc_trigger_sound_channel(channel)                                    \
  gbc_sound_master_volume_right = value & 0x07;                               \
  gbc_sound_master_volume_left = (value >> 4) & 0x07;                         \
  gbc_sound_channel[channel].status = ((value >> (channel + 8)) & 0x01) |     \
   ((value >> (channel + 11)) & 0x03)                                         \

#define gbc_trigger_sound()                                                   \
{                                                                             \
  gbc_trigger_sound_channel(0);                                               \
  gbc_trigger_sound_channel(1);                                               \
  gbc_trigger_sound_channel(2);                                               \
  gbc_trigger_sound_channel(3);                                               \
  address16(io_registers, 0x80) = value;                                      \
}                                                                             \

#define trigger_sound()                                                       \
{                                                                             \
  timer[0].direct_sound_channels = (((value >> 10) & 0x01) == 0) |            \
   ((((value >> 14) & 0x01) == 0) << 1);                                      \
  timer[1].direct_sound_channels = (((value >> 10) & 0x01) == 1) |            \
   ((((value >> 14) & 0x01) == 1) << 1);                                      \
  direct_sound_channel[0].volume = (value >> 2) & 0x01;                       \
  direct_sound_channel[0].status = (value >> 8) & 0x03;                       \
  direct_sound_channel[1].volume = (value >> 3) & 0x01;                       \
  direct_sound_channel[1].status = (value >> 12) & 0x03;                      \
  gbc_sound_master_volume = value & 0x03;                                     \
                                                                              \
  if((value >> 11) & 0x01)                                                    \
    sound_reset_fifo(0);                                                      \
  if((value >> 15) & 0x01)                                                    \
    sound_reset_fifo(1);                                                      \
  address16(io_registers, 0x82) = value;                                      \
}                                                                             \

#define sound_on()                                                            \
  if(value & 0x80)                                                            \
  {                                                                           \
    if(sound_on != 1)                                                         \
    {                                                                         \
      sound_on = 1;                                                           \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    u32 i;                                                                    \
    for(i = 0; i < 4; i++)                                                    \
    {                                                                         \
      gbc_sound_channel[i].active_flag = 0;                                   \
    }                                                                         \
    sound_on = 0;                                                             \
  }                                                                           \
  address16(io_registers, 0x84) =                                             \
    (address16(io_registers, 0x84) & 0x000F) | (value & 0xFFF0);              \

#define sound_update_frequency_step(timer_number)                             \
  timer[timer_number].frequency_step =                                        \
   float_to_fp16_16(16777216.0 / (timer_reload * sound_frequency))            \


void reset_sound();
void sound_exit();

#endif
