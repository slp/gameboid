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

#ifndef MEMORY_H
#define MEMORY_H

typedef enum
{
  DMA_START_IMMEDIATELY,
  DMA_START_VBLANK,
  DMA_START_HBLANK,
  DMA_START_SPECIAL,
  DMA_INACTIVE
} dma_start_type;

typedef enum
{
  DMA_16BIT,
  DMA_32BIT
} dma_length_type;

typedef enum
{
  DMA_NO_REPEAT,
  DMA_REPEAT
} dma_repeat_type;

typedef enum
{
  DMA_INCREMENT,
  DMA_DECREMENT,
  DMA_FIXED,
  DMA_RELOAD
} dma_increment_type;

typedef enum
{
  DMA_NO_IRQ,
  DMA_TRIGGER_IRQ
} dma_irq_type;

typedef enum
{
  DMA_DIRECT_SOUND_A,
  DMA_DIRECT_SOUND_B,
  DMA_NO_DIRECT_SOUND
} dma_ds_type;

typedef struct
{
  u32 dma_channel;
  u32 source_address;
  u32 dest_address;
  u32 length;
  dma_repeat_type repeat_type;
  dma_ds_type direct_sound_channel;
  dma_increment_type source_direction;
  dma_increment_type dest_direction;
  dma_length_type length_type;
  dma_start_type start_type;
  dma_irq_type irq;
} dma_transfer_type;

typedef enum
{
  REG_DISPCNT = 0x000,
  REG_DISPSTAT = 0x002,
  REG_VCOUNT = 0x003,
  REG_BG0CNT = 0x004,
  REG_BG1CNT = 0x005,
  REG_BG2CNT = 0x006,
  REG_BG3CNT = 0x007,
  REG_BG0HOFS = 0x08,
  REG_BG0VOFS = 0x09,
  REG_BG1HOFS = 0x0A,
  REG_BG1VOFS = 0x0B,
  REG_BG2HOFS = 0x0C,
  REG_BG2VOFS = 0x0D,
  REG_BG3HOFS = 0x0E,
  REG_BG3VOFS = 0x0F,
  REG_BG2PA = 0x10,
  REG_BG2PB = 0x11,
  REG_BG2PC = 0x12,
  REG_BG2PD = 0x13,
  REG_BG2X_L = 0x14,
  REG_BG2X_H = 0x15,
  REG_BG2Y_L = 0x16,
  REG_BG2Y_H = 0x17,
  REG_BG3PA = 0x18,
  REG_BG3PB = 0x19,
  REG_BG3PC = 0x1A,
  REG_BG3PD = 0x1B,
  REG_BG3X_L = 0x1C,
  REG_BG3X_H = 0x1D,
  REG_BG3Y_L = 0x1E,
  REG_BG3Y_H = 0x1F,
  REG_WIN0H = 0x20,
  REG_WIN1H = 0x21,
  REG_WIN0V = 0x22,
  REG_WIN1V = 0x23,
  REG_WININ = 0x24,
  REG_WINOUT = 0x25,
  REG_BLDCNT = 0x28,
  REG_BLDALPHA = 0x29,
  REG_BLDY = 0x2A,
  REG_TM0D = 0x80,
  REG_TM0CNT = 0x81,
  REG_TM1D = 0x82,
  REG_TM1CNT = 0x83,
  REG_TM2D = 0x84,
  REG_TM2CNT = 0x85,
  REG_TM3D = 0x86,
  REG_TM3CNT = 0x87,
  REG_P1 = 0x098,
  REG_P1CNT = 0x099,
  REG_RCNT = 0x9A,
  REG_IE = 0x100,
  REG_IF = 0x101,
  REG_IME = 0x104,
  REG_HALTCNT = 0x180
} hardware_register;

typedef enum
{
  FLASH_DEVICE_MACRONIX_64KB   = 0x1C,
  FLASH_DEVICE_AMTEL_64KB      = 0x3D,
  FLASH_DEVICE_SST_64K         = 0xD4,
  FLASH_DEVICE_PANASONIC_64KB  = 0x1B,
  FLASH_DEVICE_MACRONIX_128KB  = 0x09
} flash_device_id_type;

typedef enum
{
  FLASH_MANUFACTURER_MACRONIX  = 0xC2,
  FLASH_MANUFACTURER_AMTEL     = 0x1F,
  FLASH_MANUFACTURER_PANASONIC = 0x32,
  FLASH_MANUFACTURER_SST       = 0xBF
} flash_manufacturer_id_type;

u8 function_cc read_memory8(u32 address);
u32 function_cc read_memory16(u32 address);
u16 function_cc read_memory16_signed(u32 address);
u32 function_cc read_memory32(u32 address);
cpu_alert_type function_cc write_memory8(u32 address, u8 value);
cpu_alert_type function_cc write_memory16(u32 address, u16 value);
cpu_alert_type function_cc write_memory32(u32 address, u32 value);

extern u8 *memory_regions[16];
extern u32 memory_limits[16];

/* EDIT: Shouldn't this be extern ?! */
extern u32 waitstate_cycles_sequential[16][3];

extern u32 gamepak_size;
extern u8 gamepak_title[13];
extern u8 gamepak_code[5];
extern u8 gamepak_maker[3];
extern u8 gamepak_filename[512];

cpu_alert_type dma_transfer(dma_transfer_type *dma);
u8 *memory_region(u32 address, u32 *memory_limit);
u32 load_gamepak(const char *name);
u32 load_backup(const char *name);
s32 load_bios(const char *name);
void update_backup();
void update_backup_force();
void init_memory();
void init_gamepak_buffer();
void bios_region_read_allow();
void bios_region_read_protect();
u8 *load_gamepak_page(u32 physical_index);
void memory_write_mem_savestate(file_tag_type savestate_file);
void memory_read_savestate(file_tag_type savestate_file);
void load_state(const char *savestate_filename);
void save_state(const char *savestate_filename);

extern u8 *gamepak_rom;
extern u32 gamepak_ram_buffer_size;
extern u32 oam_update;
extern u32 gbc_sound_update;
extern u32 gbc_sound_wave_update;
extern dma_transfer_type dma[4];

extern u8 *write_mem_ptr;

extern u16 palette_ram[512];
extern u16 oam_ram[512];
extern u16 palette_ram_converted[512];
extern u16 io_registers[1024 * 16];
extern u8 ewram[1024 * 256 * 2];
extern u8 iwram[1024 * 32 * 2];
extern u8 vram[1024 * 96 * 2];

extern u8 bios_rom[1024 * 32];
extern u32 bios_read_protect;

extern u8 *memory_map_read[8 * 1024];
extern u32 *reg;
extern u8 *memory_map_write[8 * 1024];

extern flash_device_id_type flash_device_id;

#endif
