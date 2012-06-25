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

#include "common.h"

u32 load_file_zip(u8 *filename);

// This table is configured for sequential access on system defaults

u32 waitstate_cycles_sequential[16][3] =
{
  { 1, 1, 1 }, // BIOS
  { 1, 1, 1 }, // Invalid
  { 3, 3, 6 }, // EWRAM (default settings)
  { 1, 1, 1 }, // IWRAM
  { 1, 1, 1 }, // IO Registers
  { 1, 1, 2 }, // Palette RAM
  { 1, 1, 2 }, // VRAM
  { 1, 1, 2 }, // OAM
  { 3, 3, 6 }, // Gamepak (wait 0)
  { 3, 3, 6 }, // Gamepak (wait 0)
  { 5, 5, 9 }, // Gamepak (wait 1)
  { 5, 5, 9 }, // Gamepak (wait 1)
  { 9, 9, 17 }, // Gamepak (wait 2)
  { 9, 9, 17 }, // Gamepak (wait 2)
};

// Different settings for gamepak ws0-2 sequential (2nd) access

u32 gamepak_waitstate_sequential[2][3][3] =
{
  {
    { 3, 3, 6 },
    { 5, 5, 9 },
    { 9, 9, 17 }
  },
  {
    { 2, 2, 3 },
    { 2, 2, 3 },
    { 2, 2, 3 }
  }
};

u16 palette_ram[512] __attribute__ ((aligned(4)));
u16 oam_ram[512] __attribute__ ((aligned(4)));
u16 palette_ram_converted[512] __attribute__ ((aligned(4)));
u16 io_registers[1024 * 16] __attribute__ ((aligned(4)));
u8 ewram[1024 * 256 * 2] __attribute__ ((aligned(4)));
u8 iwram[1024 * 32 * 2] __attribute__ ((aligned(4)));
u8 vram[1024 * 96 * 2] __attribute__ ((aligned(4)));

u8 bios_rom[1024 * 32] __attribute__ ((aligned(4)));
u32 bios_read_protect;

// Up to 128kb, store SRAM, flash ROM, or EEPROM here.
u8 gamepak_backup[1024 * 128] __attribute__ ((aligned(4)));

// Keeps us knowing how much we have left.
u8 *gamepak_rom;
u32 gamepak_size;

dma_transfer_type dma[4];

u8 *memory_regions[16];
u32 memory_limits[16];

typedef struct
{
  u32 page_timestamp;
  u32 physical_index;
} gamepak_swap_entry_type;

u32 gamepak_ram_buffer_size;
u32 gamepak_ram_pages;

// Enough to map the gamepak RAM space.
gamepak_swap_entry_type *gamepak_memory_map;

// This is global so that it can be kept open for large ROMs to swap
// pages from, so there's no slowdown with opening and closing the file
// a lot.
file_tag_type gamepak_file_large = NULL;

u32 direct_map_vram = 0;

// Writes to these respective locations should trigger an update
// so the related subsystem may react to it.

// If OAM is written to:
u32 oam_update = 1;

// If GBC audio is written to:
u32 gbc_sound_update = 0;

// If the GBC audio waveform is modified:
u32 gbc_sound_wave_update = 0;

// If the backup space is written (only update once this hits 0)
u32 backup_update = 0;

// Write out backup file this many cycles after the most recent
// backup write.
const u32 write_backup_delay = 10;


typedef enum
{
  BACKUP_SRAM,
  BACKUP_FLASH,
  BACKUP_EEPROM,
  BACKUP_NONE
} backup_type_type;

typedef enum
{
  SRAM_SIZE_32KB,
  SRAM_SIZE_64KB
} sram_size_type;

// Keep it 32KB until the upper 64KB is accessed, then make it 64KB.

backup_type_type backup_type = BACKUP_NONE;
sram_size_type sram_size = SRAM_SIZE_32KB;

typedef enum
{
  FLASH_BASE_MODE,
  FLASH_ERASE_MODE,
  FLASH_ID_MODE,
  FLASH_WRITE_MODE,
  FLASH_BANKSWITCH_MODE
} flash_mode_type;

typedef enum
{
  FLASH_SIZE_64KB,
  FLASH_SIZE_128KB
} flash_size_type;

flash_mode_type flash_mode = FLASH_BASE_MODE;
u32 flash_command_position = 0;
u8 *flash_bank_ptr = gamepak_backup;

flash_device_id_type flash_device_id = FLASH_DEVICE_MACRONIX_64KB;
flash_manufacturer_id_type flash_manufacturer_id =
 FLASH_MANUFACTURER_MACRONIX;
flash_size_type flash_size = FLASH_SIZE_64KB;

u8 read_backup(u32 address)
{
  u8 value;

  if(backup_type == BACKUP_NONE)
    backup_type = BACKUP_SRAM;

  if(backup_type == BACKUP_SRAM)
  {
    value = gamepak_backup[address];
  }
  else

  if(flash_mode == FLASH_ID_MODE)
  {
    /* ID manufacturer type */
    if(address == 0x0000)
      value = flash_manufacturer_id;
    else

    /* ID device type */
    if(address == 0x0001)
      value = flash_device_id;
  }
  else
  {
    value = flash_bank_ptr[address];
  }

  return value;
}

#define read_backup8()                                                        \
  value = read_backup(address & 0xFFFF)                                       \

#define read_backup16()                                                       \
  value = 0                                                                   \

#define read_backup32()                                                       \
  value = 0                                                                   \


// EEPROM is 512 bytes by default; it is autodetecte as 8KB if
// 14bit address DMAs are made (this is done in the DMA handler).

typedef enum
{
  EEPROM_512_BYTE,
  EEPROM_8_KBYTE
} eeprom_size_type;

typedef enum
{
  EEPROM_BASE_MODE,
  EEPROM_READ_MODE,
  EEPROM_READ_HEADER_MODE,
  EEPROM_ADDRESS_MODE,
  EEPROM_WRITE_MODE,
  EEPROM_WRITE_ADDRESS_MODE,
  EEPROM_ADDRESS_FOOTER_MODE,
  EEPROM_WRITE_FOOTER_MODE
} eeprom_mode_type;


eeprom_size_type eeprom_size = EEPROM_512_BYTE;
eeprom_mode_type eeprom_mode = EEPROM_BASE_MODE;
u32 eeprom_address_length;
u32 eeprom_address = 0;
s32 eeprom_counter = 0;
u8 eeprom_buffer[8] __attribute__ ((aligned(4)));


void function_cc write_eeprom(u32 address, u32 value)
{
  switch(eeprom_mode)
  {
    case EEPROM_BASE_MODE:
      backup_type = BACKUP_EEPROM;
      eeprom_buffer[0] |= (value & 0x01) << (1 - eeprom_counter);
      eeprom_counter++;
      if(eeprom_counter == 2)
      {
        if(eeprom_size == EEPROM_512_BYTE)
          eeprom_address_length = 6;
        else
          eeprom_address_length = 14;

        eeprom_counter = 0;

        switch(eeprom_buffer[0] & 0x03)
        {
          case 0x02:
            eeprom_mode = EEPROM_WRITE_ADDRESS_MODE;
            break;

          case 0x03:
            eeprom_mode = EEPROM_ADDRESS_MODE;
            break;
        }
        address16(eeprom_buffer, 0) = 0;
      }
      break;

    case EEPROM_ADDRESS_MODE:
    case EEPROM_WRITE_ADDRESS_MODE:
      eeprom_buffer[eeprom_counter / 8]
       |= (value & 0x01) << (7 - (eeprom_counter % 8));
      eeprom_counter++;
      if(eeprom_counter == eeprom_address_length)
      {
        if(eeprom_size == EEPROM_512_BYTE)
        {
          eeprom_address =
           (address16(eeprom_buffer, 0) >> 2) * 8;
        }
        else
        {
          eeprom_address = (((u32)eeprom_buffer[1] >> 2) |
           ((u32)eeprom_buffer[0] << 6)) * 8;
        }

        address16(eeprom_buffer, 0) = 0;
        eeprom_counter = 0;

        if(eeprom_mode == EEPROM_ADDRESS_MODE)
        {
          eeprom_mode = EEPROM_ADDRESS_FOOTER_MODE;
        }
        else
        {
          eeprom_mode = EEPROM_WRITE_MODE;
          memset(gamepak_backup + eeprom_address, 0, 8);
        }
      }
      break;

    case EEPROM_WRITE_MODE:
      gamepak_backup[eeprom_address + (eeprom_counter / 8)] |=
       (value & 0x01) << (7 - (eeprom_counter % 8));
      eeprom_counter++;
      if(eeprom_counter == 64)
      {
        backup_update = write_backup_delay;
        eeprom_counter = 0;
        eeprom_mode = EEPROM_WRITE_FOOTER_MODE;
      }
      break;

    case EEPROM_ADDRESS_FOOTER_MODE:
    case EEPROM_WRITE_FOOTER_MODE:
      eeprom_counter = 0;
      if(eeprom_mode == EEPROM_ADDRESS_FOOTER_MODE)
      {
        eeprom_mode = EEPROM_READ_HEADER_MODE;
      }
      else
      {
        eeprom_mode = EEPROM_BASE_MODE;
      }
      break;
  }
}

#define read_memory_gamepak(type)                                             \
  u32 gamepak_index = address >> 15;                                          \
  u8 *map = memory_map_read[gamepak_index];                                   \
                                                                              \
  if(map == NULL)                                                             \
    map = load_gamepak_page(gamepak_index & 0x3FF);                           \
                                                                              \
  value = address##type(map, address & 0x7FFF)                                \

#define read_open8()                                                          \
  if(!(reg[REG_CPSR] & 0x20))                                                 \
    value = read_memory8(reg[REG_PC] + 4 + (address & 0x03));                 \
  else                                                                        \
    value = read_memory8(reg[REG_PC] + 2 + (address & 0x01))                  \

#define read_open16()                                                         \
  if(!(reg[REG_CPSR] & 0x20))                                                 \
    value = read_memory16(reg[REG_PC] + 4 + (address & 0x02));                \
  else                                                                        \
    value = read_memory16(reg[REG_PC] + 2)                                    \

#define read_open32()                                                         \
  if(!(reg[REG_CPSR] & 0x20))                                                 \
  {                                                                           \
    value = read_memory32(reg[REG_PC] + 4);                                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    u32 current_instruction = read_memory16(reg[REG_PC] + 2);                 \
    value = current_instruction | (current_instruction << 16);                \
  }                                                                           \

u32 function_cc read_eeprom()
{
  u32 value;

  switch(eeprom_mode)
  {
    case EEPROM_BASE_MODE:
      value = 1;
      break;

    case EEPROM_READ_MODE:
      value = (gamepak_backup[eeprom_address + (eeprom_counter / 8)] >>
       (7 - (eeprom_counter % 8))) & 0x01;
      eeprom_counter++;
      if(eeprom_counter == 64)
      {
        eeprom_counter = 0;
        eeprom_mode = EEPROM_BASE_MODE;
      }
      break;

    case EEPROM_READ_HEADER_MODE:
      value = 0;
      eeprom_counter++;
      if(eeprom_counter == 4)
      {
        eeprom_mode = EEPROM_READ_MODE;
        eeprom_counter = 0;
      }
      break;

    default:
      value = 0;
      break;
  }

  return value;
}


#define read_memory(type)                                                     \
  switch(address >> 24)                                                       \
  {                                                                           \
    case 0x00:                                                                \
      /* BIOS */                                                              \
      if(reg[REG_PC] >= 0x4000)                                               \
        value = address##type(&bios_read_protect, address & 0x03);            \
      else                                                                    \
        value = address##type(bios_rom, address & 0x3FFF);                    \
      break;                                                                  \
                                                                              \
    case 0x02:                                                                \
      /* external work RAM */                                                 \
      address = (address & 0x7FFF) + ((address & 0x38000) * 2) + 0x8000;      \
      value = address##type(ewram, address);                                  \
      break;                                                                  \
                                                                              \
    case 0x03:                                                                \
      /* internal work RAM */                                                 \
      value = address##type(iwram, (address & 0x7FFF) + 0x8000);              \
      break;                                                                  \
                                                                              \
    case 0x04:                                                                \
      /* I/O registers */                                                     \
      value = address##type(io_registers, address & 0x3FF);                   \
      break;                                                                  \
                                                                              \
    case 0x05:                                                                \
      /* palette RAM */                                                       \
      value = address##type(palette_ram, address & 0x3FF);                    \
      break;                                                                  \
                                                                              \
    case 0x06:                                                                \
      /* VRAM */                                                              \
      address &= 0x1FFFF;                                                     \
      if(address > 0x18000)                                                   \
        address -= 0x8000;                                                    \
                                                                              \
      value = address##type(vram, address);                                   \
      break;                                                                  \
                                                                              \
    case 0x07:                                                                \
      /* OAM RAM */                                                           \
      value = address##type(oam_ram, address & 0x3FF);                        \
      break;                                                                  \
                                                                              \
    case 0x08:                                                                \
    case 0x09:                                                                \
    case 0x0A:                                                                \
    case 0x0B:                                                                \
    case 0x0C:                                                                \
      /* gamepak ROM */                                                       \
      if((address & 0x1FFFFFF) >= gamepak_size)                               \
      {                                                                       \
        value = 0;                                                            \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        read_memory_gamepak(type);                                            \
      }                                                                       \
      break;                                                                  \
                                                                              \
    case 0x0D:                                                                \
      if((address & 0x1FFFFFF) < gamepak_size)                                \
      {                                                                       \
        read_memory_gamepak(type);                                            \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        value = read_eeprom();                                                \
      }                                                                       \
                                                                              \
      break;                                                                  \
                                                                              \
    case 0x0E:                                                                \
    case 0x0F:                                                                \
      read_backup##type();                                                    \
      break;                                                                  \
                                                                              \
    default:                                                                  \
      read_open##type();                                                      \
      break;                                                                  \
  }                                                                           \

#define trigger_dma(dma_number)                                               \
  if(value & 0x8000)                                                          \
  {                                                                           \
    if(dma[dma_number].start_type == DMA_INACTIVE)                            \
    {                                                                         \
      u32 start_type = (value >> 12) & 0x03;                                  \
      u32 dest_address = address32(io_registers, (dma_number * 12) + 0xB4) &  \
       0xFFFFFFF;                                                             \
                                                                              \
      dma[dma_number].dma_channel = dma_number;                               \
      dma[dma_number].source_address =                                        \
       address32(io_registers, (dma_number * 12) + 0xB0) & 0xFFFFFFF;         \
      dma[dma_number].dest_address = dest_address;                            \
      dma[dma_number].source_direction = (value >>  7) & 0x03;                \
      dma[dma_number].repeat_type = (value >> 9) & 0x01;                      \
      dma[dma_number].start_type = start_type;                                \
      dma[dma_number].irq = (value >> 14) & 0x01;                             \
                                                                              \
      /* If it is sound FIFO DMA make sure the settings are a certain way */  \
      if((dma_number >= 1) && (dma_number <= 2) &&                            \
       (start_type == DMA_START_SPECIAL))                                     \
      {                                                                       \
        dma[dma_number].length_type = DMA_32BIT;                              \
        dma[dma_number].length = 4;                                           \
        dma[dma_number].dest_direction = DMA_FIXED;                           \
        if(dest_address == 0x40000A4)                                         \
          dma[dma_number].direct_sound_channel = DMA_DIRECT_SOUND_B;          \
        else                                                                  \
          dma[dma_number].direct_sound_channel = DMA_DIRECT_SOUND_A;          \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        u32 length =                                                          \
         address16(io_registers, (dma_number * 12) + 0xB8);                   \
                                                                              \
        if((dma_number == 3) && ((dest_address >> 24) == 0x0D) &&             \
         ((length & 0x1F) == 17))                                             \
        {                                                                     \
          eeprom_size = EEPROM_8_KBYTE;                                       \
        }                                                                     \
                                                                              \
        if(dma_number < 3)                                                    \
          length &= 0x3FFF;                                                   \
                                                                              \
        if(length == 0)                                                       \
        {                                                                     \
          if(dma_number == 3)                                                 \
            length = 0x10000;                                                 \
          else                                                                \
            length = 0x04000;                                                 \
        }                                                                     \
                                                                              \
        dma[dma_number].length = length;                                      \
        dma[dma_number].length_type = (value >> 10) & 0x01;                   \
        dma[dma_number].dest_direction = (value >> 5) & 0x03;                 \
      }                                                                       \
                                                                              \
      address16(io_registers, (dma_number * 12) + 0xBA) = value;              \
      if(start_type == DMA_START_IMMEDIATELY)                                 \
        return dma_transfer(dma + dma_number);                                \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    dma[dma_number].start_type = DMA_INACTIVE;                                \
    dma[dma_number].direct_sound_channel = DMA_NO_DIRECT_SOUND;               \
    address16(io_registers, (dma_number * 12) + 0xBA) = value;                \
  }                                                                           \


#define access_register8_high(address)                                        \
  value = (value << 8) | (address8(io_registers, address))                    \

#define access_register8_low(address)                                         \
  value = ((address8(io_registers, address + 1)) << 8) | value                \

#define access_register16_high(address)                                       \
  value = (value << 16) | (address16(io_registers, address))                  \

#define access_register16_low(address)                                        \
  value = ((address16(io_registers, address + 2)) << 16) | value              \

cpu_alert_type function_cc write_io_register8(u32 address, u32 value)
{
  value &= 0xFF;

  switch(address)
  {
    case 0x00:
    {
      u32 dispcnt = io_registers[REG_DISPCNT];

      if((value & 0x07) != (dispcnt & 0x07))
        oam_update = 1;

      address8(io_registers, 0x00) = value;
      break;
    }

    // DISPSTAT (lower byte)
    case 0x04:
      address8(io_registers, 0x04) =
       (address8(io_registers, 0x04) & 0x07) | (value & ~0x07);
      break;

    // VCOUNT
    case 0x06:
    case 0x07:
      break;

    // BG2 reference X
    case 0x28:
      access_register8_low(0x28);
      access_register16_low(0x28);
      affine_reference_x[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x28) = value;
      break;

    case 0x29:
      access_register8_high(0x28);
      access_register16_low(0x28);
      affine_reference_x[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x28) = value;
      break;

    case 0x2A:
      access_register8_low(0x2A);
      access_register16_high(0x28);
      affine_reference_x[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x28) = value;
      break;

    case 0x2B:
      access_register8_high(0x2A);
      access_register16_high(0x28);
      affine_reference_x[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x28) = value;
      break;

    // BG2 reference Y
    case 0x2C:
      access_register8_low(0x2C);
      access_register16_low(0x2C);
      affine_reference_y[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x2C) = value;
      break;

    case 0x2D:
      access_register8_high(0x2C);
      access_register16_low(0x2C);
      affine_reference_y[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x2C) = value;
      break;

    case 0x2E:
      access_register8_low(0x2E);
      access_register16_high(0x2C);
      affine_reference_y[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x2C) = value;
      break;

    case 0x2F:
      access_register8_high(0x2E);
      access_register16_high(0x2C);
      affine_reference_y[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x2C) = value;
      break;

    // BG3 reference X
    case 0x38:
      access_register8_low(0x38);
      access_register16_low(0x38);
      affine_reference_x[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x38) = value;
      break;

    case 0x39:
      access_register8_high(0x38);
      access_register16_low(0x38);
      affine_reference_x[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x38) = value;
      break;

    case 0x3A:
      access_register8_low(0x3A);
      access_register16_high(0x38);
      affine_reference_x[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x38) = value;
      break;

    case 0x3B:
      access_register8_high(0x3A);
      access_register16_high(0x38);
      affine_reference_x[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x38) = value;
      break;

    // BG3 reference Y
    case 0x3C:
      access_register8_low(0x3C);
      access_register16_low(0x3C);
      affine_reference_y[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x3C) = value;
      break;

    case 0x3D:
      access_register8_high(0x3C);
      access_register16_low(0x3C);
      affine_reference_y[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x3C) = value;
      break;

    case 0x3E:
      access_register8_low(0x3E);
      access_register16_high(0x3C);
      affine_reference_y[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x3C) = value;
      break;

    case 0x3F:
      access_register8_high(0x3E);
      access_register16_high(0x3C);
      affine_reference_y[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x3C) = value;
      break;

    // Sound 1 control sweep
    case 0x60:
      access_register8_low(0x60);
      gbc_sound_tone_control_sweep();
      break;

    case 0x61:
      access_register8_low(0x60);
      gbc_sound_tone_control_sweep();
      break;

    // Sound 1 control duty/length/envelope
    case 0x62:
      access_register8_low(0x62);
      gbc_sound_tone_control_low(0, 0x62);
      break;

    case 0x63:
      access_register8_high(0x62);
      gbc_sound_tone_control_low(0, 0x62);
      break;

    // Sound 1 control frequency
    case 0x64:
      access_register8_low(0x64);
      gbc_sound_tone_control_high(0, 0x64);
      break;

    case 0x65:
      access_register8_high(0x64);
      gbc_sound_tone_control_high(0, 0x64);
      break;

    // Sound 2 control duty/length/envelope
    case 0x68:
      access_register8_low(0x68);
      gbc_sound_tone_control_low(1, 0x68);
      break;

    case 0x69:
      access_register8_high(0x68);
      gbc_sound_tone_control_low(1, 0x68);
      break;

    // Sound 2 control frequency
    case 0x6C:
      access_register8_low(0x6C);
      gbc_sound_tone_control_high(1, 0x6C);
      break;

    case 0x6D:
      access_register8_high(0x6C);
      gbc_sound_tone_control_high(1, 0x6C);
      break;

    // Sound 3 control wave
    case 0x70:
      access_register8_low(0x70);
      gbc_sound_wave_control();
      break;

    case 0x71:
      access_register8_high(0x70);
      gbc_sound_wave_control();
      break;

    // Sound 3 control length/volume
    case 0x72:
      access_register8_low(0x72);
      gbc_sound_tone_control_low_wave();
      break;

    case 0x73:
      access_register8_high(0x72);
      gbc_sound_tone_control_low_wave();
      break;

    // Sound 3 control frequency
    case 0x74:
      access_register8_low(0x74);
      gbc_sound_tone_control_high_wave();
      break;

    case 0x75:
      access_register8_high(0x74);
      gbc_sound_tone_control_high_wave();
      break;

    // Sound 4 control length/envelope
    case 0x78:
      access_register8_low(0x78);
      gbc_sound_tone_control_low(3, 0x78);
      break;

    case 0x79:
      access_register8_high(0x78);
      gbc_sound_tone_control_low(3, 0x78);
      break;

    // Sound 4 control frequency
    case 0x7C:
      access_register8_low(0x7C);
      gbc_sound_noise_control();
      break;

    case 0x7D:
      access_register8_high(0x7C);
      gbc_sound_noise_control();
      break;

    // Sound control L
    case 0x80:
      access_register8_low(0x80);
      gbc_trigger_sound();
      break;

    case 0x81:
      access_register8_high(0x80);
      gbc_trigger_sound();
      break;

    // Sound control H
    case 0x82:
      access_register8_low(0x82);
      trigger_sound();
      break;

    case 0x83:
      access_register8_high(0x82);
      trigger_sound();
      break;

    // Sound control X
    case 0x84:
      sound_on();
      break;

    // Sound wave RAM
    case 0x90 ... 0x9F:
      gbc_sound_wave_update = 1;
      address8(io_registers, address) = value;
      break;

    // Sound FIFO A
    case 0xA0:
      sound_timer_queue8(0, value);
      break;

    // Sound FIFO B
    case 0xA4:
      sound_timer_queue8(1, value);
      break;

    // DMA control (trigger byte)
    case 0xBB:
      access_register8_low(0xBA);
      trigger_dma(0);
      break;

    case 0xC7:
      access_register8_low(0xC6);
      trigger_dma(1);
      break;

    case 0xD3:
      access_register8_low(0xD2);
      trigger_dma(2);
      break;

    case 0xDF:
      access_register8_low(0xDE);
      trigger_dma(3);
      break;

    // Timer counts
    case 0x100:
      access_register8_low(0x100);
      count_timer(0);
      break;

    case 0x101:
      access_register8_high(0x100);
      count_timer(0);
      break;

    case 0x104:
      access_register8_low(0x104);
      count_timer(1);
      break;

    case 0x105:
      access_register8_high(0x104);
      count_timer(1);
      break;

    case 0x108:
      access_register8_low(0x108);
      count_timer(2);
      break;

    case 0x109:
      access_register8_high(0x108);
      count_timer(2);
      break;

    case 0x10C:
      access_register8_low(0x10C);
      count_timer(3);
      break;

    case 0x10D:
      access_register8_high(0x10C);
      count_timer(3);
      break;

    // Timer control (trigger byte)
    case 0x103:
      access_register8_low(0x102);
      trigger_timer(0);
      break;

    case 0x107:
      access_register8_low(0x106);
      trigger_timer(1);
      break;

    case 0x10B:
      access_register8_low(0x10A);
      trigger_timer(2);
      break;

    case 0x10F:
      access_register8_low(0x10E);
      trigger_timer(3);
      break;

    // IF
    case 0x202:
      address8(io_registers, 0x202) &= ~value;
      break;

    case 0x203:
      address8(io_registers, 0x203) &= ~value;
      break;

    // Halt
    case 0x301:
      if((value & 0x01) == 0)
        reg[CPU_HALT_STATE] = CPU_HALT;
      else
        reg[CPU_HALT_STATE] = CPU_STOP;

      return CPU_ALERT_HALT;
      break;

    default:
      address8(io_registers, address) = value;
      break;
  }

  return CPU_ALERT_NONE;
}

cpu_alert_type function_cc write_io_register16(u32 address, u32 value)
{
  value &= 0xFFFF;

  switch(address)
  {
    case 0x00:
    {
      u32 dispcnt = io_registers[REG_DISPCNT];
      if((value & 0x07) != (dispcnt & 0x07))
        oam_update = 1;

      address16(io_registers, 0x00) = value;
      break;
    }

    // DISPSTAT
    case 0x04:
      address16(io_registers, 0x04) =
       (address16(io_registers, 0x04) & 0x07) | (value & ~0x07);
      break;

    // VCOUNT
    case 0x06:
      break;

    // BG2 reference X
    case 0x28:
      access_register16_low(0x28);
      affine_reference_x[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x28) = value;
      break;

    case 0x2A:
      access_register16_high(0x28);
      affine_reference_x[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x28) = value;
      break;

    // BG2 reference Y
    case 0x2C:
      access_register16_low(0x2C);
      affine_reference_y[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x2C) = value;
      break;

    case 0x2E:
      access_register16_high(0x2C);
      affine_reference_y[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x2C) = value;
      break;

    // BG3 reference X

    case 0x38:
      access_register16_low(0x38);
      affine_reference_x[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x38) = value;
      break;

    case 0x3A:
      access_register16_high(0x38);
      affine_reference_x[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x38) = value;
      break;

    // BG3 reference Y
    case 0x3C:
      access_register16_low(0x3C);
      affine_reference_y[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x3C) = value;
      break;

    case 0x3E:
      access_register16_high(0x3C);
      affine_reference_y[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x3C) = value;
      break;

    // Sound 1 control sweep
    case 0x60:
      gbc_sound_tone_control_sweep();
      break;

    // Sound 1 control duty/length/envelope
    case 0x62:
      gbc_sound_tone_control_low(0, 0x62);
      break;

    // Sound 1 control frequency
    case 0x64:
      gbc_sound_tone_control_high(0, 0x64);
      break;

    // Sound 2 control duty/length/envelope
    case 0x68:
      gbc_sound_tone_control_low(1, 0x68);
      break;

    // Sound 2 control frequency
    case 0x6C:
      gbc_sound_tone_control_high(1, 0x6C);
      break;

    // Sound 3 control wave
    case 0x70:
      gbc_sound_wave_control();
      break;

    // Sound 3 control length/volume
    case 0x72:
      gbc_sound_tone_control_low_wave();
      break;

    // Sound 3 control frequency
    case 0x74:
      gbc_sound_tone_control_high_wave();
      break;

    // Sound 4 control length/envelope
    case 0x78:
      gbc_sound_tone_control_low(3, 0x78);
      break;

    // Sound 4 control frequency
    case 0x7C:
      gbc_sound_noise_control();
      break;

    // Sound control L
    case 0x80:
      gbc_trigger_sound();
      break;

    // Sound control H
    case 0x82:
      trigger_sound();
      break;

    // Sound control X
    case 0x84:
      sound_on();
      break;

    // Sound wave RAM
    case 0x90 ... 0x9E:
      gbc_sound_wave_update = 1;
      address16(io_registers, address) = value;
      break;

    // Sound FIFO A
    case 0xA0:
      sound_timer_queue16(0, value);
      break;

    // Sound FIFO B
    case 0xA4:
      sound_timer_queue16(1, value);
      break;

    // DMA control
    case 0xBA:
      trigger_dma(0);
      break;

    case 0xC6:
      trigger_dma(1);
      break;

    case 0xD2:
      trigger_dma(2);
      break;

    case 0xDE:
      trigger_dma(3);
      break;

    // Timer counts
    case 0x100:
      count_timer(0);
      break;

    case 0x104:
      count_timer(1);
      break;

    case 0x108:
      count_timer(2);
      break;

    case 0x10C:
      count_timer(3);
      break;

    // Timer control
    case 0x102:
      trigger_timer(0);
      break;

    case 0x106:
      trigger_timer(1);
      break;

    case 0x10A:
      trigger_timer(2);
      break;

    case 0x10E:
      trigger_timer(3);
      break;

    // P1
    case 0x130:
      break;

    // Interrupt flag
    case 0x202:
      address16(io_registers, 0x202) &= ~value;
      break;

    // WAITCNT
    case 0x204:
      break;

    // Halt
    case 0x300:
      if(((value >> 8) & 0x01) == 0)
        reg[CPU_HALT_STATE] = CPU_HALT;
      else
        reg[CPU_HALT_STATE] = CPU_STOP;

      return CPU_ALERT_HALT;

    default:
      address16(io_registers, address) = value;
      break;
  }

  return CPU_ALERT_NONE;
}


cpu_alert_type function_cc write_io_register32(u32 address, u32 value)
{
  switch(address)
  {
    // BG2 reference X
    case 0x28:
      affine_reference_x[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x28) = value;
      break;

    // BG2 reference Y
    case 0x2C:
      affine_reference_y[0] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x2C) = value;
      break;

    // BG3 reference X
    case 0x38:
      affine_reference_x[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x38) = value;
      break;

    // BG3 reference Y
    case 0x3C:
      affine_reference_y[1] = (s32)(value << 4) >> 4;
      address32(io_registers, 0x3C) = value;
      break;

    // Sound FIFO A
    case 0xA0:
      sound_timer_queue32(0, value);
      break;

    // Sound FIFO B
    case 0xA4:
      sound_timer_queue32(1, value);
      break;

    default:
    {
      cpu_alert_type alert_low =
        write_io_register16(address, value & 0xFFFF);

      cpu_alert_type alert_high =
        write_io_register16(address + 2, value >> 16);

      if(alert_high)
        return alert_high;

      return alert_low;
    }
  }

  return CPU_ALERT_NONE;
}

#define write_palette8(address, value)                                        \

#define write_palette16(address, value)                                       \
{                                                                             \
  u32 palette_address = address;                                              \
  address16(palette_ram, palette_address) = value;                            \
  convert_palette(value);                                                     \
  address16(palette_ram_converted, palette_address) = value;                  \
}                                                                             \

#define write_palette32(address, value)                                       \
{                                                                             \
  u32 palette_address = address;                                              \
  u32 value_high = value >> 16;                                               \
  u32 value_low = value & 0xFFFF;                                             \
  address32(palette_ram, palette_address) = value;                            \
  convert_palette(value_high);                                                \
  convert_palette(value_low);                                                 \
  value = (value_high << 16) | value_low;                                     \
  address32(palette_ram_converted, palette_address) = value;                  \
}                                                                             \


void function_cc write_backup(u32 address, u32 value)
{
  value &= 0xFF;

  if(backup_type == BACKUP_NONE)
    backup_type = BACKUP_SRAM;


  // gamepak SRAM or Flash ROM
  if((address == 0x5555) && (flash_mode != FLASH_WRITE_MODE))
  {
    if((flash_command_position == 0) && (value == 0xAA))
    {
      backup_type = BACKUP_FLASH;
      flash_command_position = 1;
    }

    if(flash_command_position == 2)
    {
      switch(value)
      {
        case 0x90:
          // Enter ID mode, this also tells the emulator that we're using
          // flash, not SRAM

          if(flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_ID_MODE;

          break;

        case 0x80:
          // Enter erase mode
          if(flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_ERASE_MODE;
          break;

        case 0xF0:
          // Terminate ID mode
          if(flash_mode == FLASH_ID_MODE)
            flash_mode = FLASH_BASE_MODE;
          break;

        case 0xA0:
          // Write mode
          if(flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_WRITE_MODE;
          break;

        case 0xB0:
          // Bank switch
          // Here the chip is now officially 128KB.
          flash_size = FLASH_SIZE_128KB;
          if(flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_BANKSWITCH_MODE;
          break;

        case 0x10:
          // Erase chip
          if(flash_mode == FLASH_ERASE_MODE)
          {
            if(flash_size == FLASH_SIZE_64KB)
              memset(gamepak_backup, 0xFF, 1024 * 64);
            else
              memset(gamepak_backup, 0xFF, 1024 * 128);
            backup_update = write_backup_delay;
            flash_mode = FLASH_BASE_MODE;
          }
          break;

        default:
          break;
      }
      flash_command_position = 0;
    }
    if(backup_type == BACKUP_SRAM)
      gamepak_backup[0x5555] = value;
  }
  else

  if((address == 0x2AAA) && (value == 0x55) &&
   (flash_command_position == 1))
  {
    flash_command_position = 2;
  }
  else
  {
    if((flash_command_position == 2) &&
     (flash_mode == FLASH_ERASE_MODE) && (value == 0x30))
    {
      // Erase sector
      memset(flash_bank_ptr + (address & 0xF000), 0xFF, 1024 * 4);
      backup_update = write_backup_delay;
      flash_mode = FLASH_BASE_MODE;
      flash_command_position = 0;
    }
    else

    if((flash_command_position == 0) &&
     (flash_mode == FLASH_BANKSWITCH_MODE) && (address == 0x0000) &&
     (flash_size == FLASH_SIZE_128KB))
    {
      flash_bank_ptr = gamepak_backup + ((value & 0x01) * (1024 * 64));
      flash_mode = FLASH_BASE_MODE;
    }
    else

    if((flash_command_position == 0) && (flash_mode == FLASH_WRITE_MODE))
    {
      // Write value to flash ROM
      backup_update = write_backup_delay;
      flash_bank_ptr[address] = value;
      flash_mode = FLASH_BASE_MODE;
    }
    else

    if(backup_type == BACKUP_SRAM)
    {
      // Write value to SRAM
      backup_update = write_backup_delay;
      // Hit 64KB territory?
      if(address >= 0x8000)
        sram_size = SRAM_SIZE_64KB;
      gamepak_backup[address] = value;
    }
  }
}

#define write_backup8()                                                       \
  write_backup(address & 0xFFFF, value)                                       \

#define write_backup16()                                                      \

#define write_backup32()                                                      \

#define write_vram8()                                                         \
  address &= ~0x01;                                                           \
  address16(vram, address) = ((value << 8) | value)                           \

#define write_vram16()                                                        \
  address16(vram, address) = value                                            \

#define write_vram32()                                                        \
  address32(vram, address) = value                                            \

// RTC code derived from VBA's (due to lack of any real publically available
// documentation...)

typedef enum
{
  RTC_DISABLED,
  RTC_IDLE,
  RTC_COMMAND,
  RTC_OUTPUT_DATA,
  RTC_INPUT_DATA
} rtc_state_type;

typedef enum
{
  RTC_COMMAND_RESET            = 0x60,
  RTC_COMMAND_WRITE_STATUS     = 0x62,
  RTC_COMMAND_READ_STATUS      = 0x63,
  RTC_COMMAND_OUTPUT_TIME_FULL = 0x65,
  RTC_COMMAND_OUTPUT_TIME      = 0x67
} rtc_command_type;

typedef enum
{
  RTC_WRITE_TIME,
  RTC_WRITE_TIME_FULL,
  RTC_WRITE_STATUS
} rtc_write_mode_type;

rtc_state_type rtc_state = RTC_DISABLED;
rtc_write_mode_type rtc_write_mode;
u8 rtc_registers[3];
u32 rtc_command;
u32 rtc_data[12];
u32 rtc_status = 0x40;
u32 rtc_data_bytes;
s32 rtc_bit_count;

u32 encode_bcd(u8 value)
{
  return ((value / 10) << 4) | (value % 10);
}

#define write_rtc_register(index, _value)                                     \
  update_address = 0x80000C4 + (index * 2);                                   \
  rtc_registers[index] = _value;                                              \
  rtc_page_index = update_address >> 15;                                      \
  map = memory_map_read[rtc_page_index];                                      \
                                                                              \
  if(map == NULL)                                                             \
    map = load_gamepak_page(rtc_page_index & 0x3FF);                          \
                                                                              \
  address16(map, update_address & 0x7FFF) = _value                            \

void function_cc write_rtc(u32 address, u32 value)
{
  u32 rtc_page_index;
  u32 update_address;
  u8 *map;

  value &= 0xFFFF;

  switch(address)
  {
    // RTC command
    // Bit 0: SCHK, perform action
    // Bit 1: IO, input/output command data
    // Bit 2: CS, select input/output? If high make I/O write only
    case 0xC4:
      if(rtc_state == RTC_DISABLED)
        rtc_state = RTC_IDLE;
      if(!(rtc_registers[0] & 0x04))
        value = (rtc_registers[0] & 0x02) | (value & ~0x02);
      if(rtc_registers[2] & 0x01)
      {
        // To begin writing a command 1, 5 must be written to the command
        // registers.
        if((rtc_state == RTC_IDLE) && (rtc_registers[0] == 0x01) &&
         (value == 0x05))
        {
          // We're now ready to begin receiving a command.
          write_rtc_register(0, value);
          rtc_state = RTC_COMMAND;
          rtc_command = 0;
          rtc_bit_count = 7;
        }
        else
        {
          write_rtc_register(0, value);
          switch(rtc_state)
          {
            // Accumulate RTC command by receiving the next bit, and if we
            // have accumulated enough bits to form a complete command
            // execute it.
            case RTC_COMMAND:
              if(rtc_registers[0] & 0x01)
              {
                rtc_command |= ((value & 0x02) >> 1) << rtc_bit_count;
                rtc_bit_count--;
              }

              // Have we received a full RTC command? If so execute it.
              if(rtc_bit_count < 0)
              {
                switch(rtc_command)
                {
                  // Resets RTC
                  case RTC_COMMAND_RESET:
                    rtc_state = RTC_IDLE;
                    memset(rtc_registers, 0, sizeof(rtc_registers));
                    break;

                  // Sets status of RTC
                  case RTC_COMMAND_WRITE_STATUS:
                    rtc_state = RTC_INPUT_DATA;
                    rtc_data_bytes = 1;
                    rtc_write_mode = RTC_WRITE_STATUS;
                    break;

                  // Outputs current status of RTC
                  case RTC_COMMAND_READ_STATUS:
                    rtc_state = RTC_OUTPUT_DATA;
                    rtc_data_bytes = 1;
                    rtc_data[0] = rtc_status;
                    break;

                  // Actually outputs the time, all of it
                  case RTC_COMMAND_OUTPUT_TIME_FULL:
                  {
                    struct tm *current_time;
                    time_t current_time_flat;
                    u32 day_of_week;

                    time(&current_time_flat);
                    current_time = localtime(&current_time_flat);

                    day_of_week = current_time->tm_wday;
                    if(day_of_week == 0)
                      day_of_week = 6;
                    else
                      day_of_week--;

                    rtc_state = RTC_OUTPUT_DATA;
                    rtc_data_bytes = 7;
                    rtc_data[0] = encode_bcd(current_time->tm_year % 100);
                    rtc_data[1] = encode_bcd(current_time->tm_mon + 1);
                    rtc_data[2] = encode_bcd(current_time->tm_mday);
                    rtc_data[3] = encode_bcd(day_of_week);
                    rtc_data[4] = encode_bcd(current_time->tm_hour);
                    rtc_data[5] = encode_bcd(current_time->tm_min);
                    rtc_data[6] = encode_bcd(current_time->tm_sec);

                    break;
                  }

                  // Only outputs the current time of day.
                  case RTC_COMMAND_OUTPUT_TIME:
                  {
                    struct tm *current_time;
                    time_t current_time_flat;

                    time(&current_time_flat);
                    current_time = localtime(&current_time_flat);

                    rtc_state = RTC_OUTPUT_DATA;
                    rtc_data_bytes = 3;
                    rtc_data[0] = encode_bcd(current_time->tm_hour);
                    rtc_data[1] = encode_bcd(current_time->tm_min);
                    rtc_data[2] = encode_bcd(current_time->tm_sec);
                    break;
                  }
                }
                rtc_bit_count = 0;
              }
              break;

            // Receive parameters from the game as input to the RTC
            // for a given command. Read one bit at a time.
            case RTC_INPUT_DATA:
              // Bit 1 of parameter A must be high for input
              if(rtc_registers[1] & 0x02)
              {
                // Read next bit for input
                if(!(value & 0x01))
                {
                  rtc_data[rtc_bit_count >> 3] |=
                   ((value & 0x01) << (7 - (rtc_bit_count & 0x07)));
                }
                else
                {
                  rtc_bit_count++;

                  if(rtc_bit_count == (rtc_data_bytes * 8))
                  {
                    rtc_state = RTC_IDLE;
                    switch(rtc_write_mode)
                    {
                      case RTC_WRITE_STATUS:
                        rtc_status = rtc_data[0];
                        break;
                    }
                  }
                }
              }
              break;

            case RTC_OUTPUT_DATA:
              // Bit 1 of parameter A must be low for output
              if(!(rtc_registers[1] & 0x02))
              {
                // Write next bit to output, on bit 1 of parameter B
                if(!(value & 0x01))
                {
                  u8 current_output_byte = rtc_registers[2];

                  current_output_byte =
                   (current_output_byte & ~0x02) |
                   (((rtc_data[rtc_bit_count >> 3] >>
                   (rtc_bit_count & 0x07)) & 0x01) << 1);

                  write_rtc_register(0, current_output_byte);

                }
                else
                {
                  rtc_bit_count++;

                  if(rtc_bit_count == (rtc_data_bytes * 8))
                  {
                    rtc_state = RTC_IDLE;
                    memset(rtc_registers, 0, sizeof(rtc_registers));
                  }
                }
              }
              break;
          }
        }
      }
      else
      {
        write_rtc_register(2, value);
      }
      break;

    // Write parameter A
    case 0xC6:
      write_rtc_register(1, value);
      break;

    // Write parameter B
    case 0xC8:
      write_rtc_register(2, value);
      break;
  }
}

#define write_rtc8()                                                          \

#define write_rtc16()                                                         \
  write_rtc(address & 0xFF, value)                                            \

#define write_rtc32()                                                         \

#define write_memory(type)                                                    \
  switch(address >> 24)                                                       \
  {                                                                           \
    case 0x02:                                                                \
      /* external work RAM */                                                 \
      address = (address & 0x7FFF) + ((address & 0x38000) * 2) + 0x8000;      \
      address##type(ewram, address) = value;                                  \
      break;                                                                  \
                                                                              \
    case 0x03:                                                                \
      /* internal work RAM */                                                 \
      address##type(iwram, (address & 0x7FFF) + 0x8000) = value;              \
      break;                                                                  \
                                                                              \
    case 0x04:                                                                \
      /* I/O registers */                                                     \
      return write_io_register##type(address & 0x3FF, value);                 \
                                                                              \
    case 0x05:                                                                \
      /* palette RAM */                                                       \
      write_palette##type(address & 0x3FF, value);                            \
      break;                                                                  \
                                                                              \
    case 0x06:                                                                \
      /* VRAM */                                                              \
      address &= 0x1FFFF;                                                     \
      if(address >= 0x18000)                                                  \
        address -= 0x8000;                                                    \
                                                                              \
      write_vram##type();                                                     \
      break;                                                                  \
                                                                              \
    case 0x07:                                                                \
      /* OAM RAM */                                                           \
      oam_update = 1;                                                         \
      address##type(oam_ram, address & 0x3FF) = value;                        \
      break;                                                                  \
                                                                              \
    case 0x08:                                                                \
      /* gamepak ROM or RTC */                                                \
      write_rtc##type();                                                      \
      break;                                                                  \
                                                                              \
    case 0x09 ... 0x0C:                                                       \
      /* gamepak ROM space */                                                 \
      break;                                                                  \
                                                                              \
    case 0x0D:                                                                \
      write_eeprom(address, value);                                           \
      break;                                                                  \
                                                                              \
    case 0x0E:                                                                \
      write_backup##type();                                                   \
      break;                                                                  \
  }                                                                           \

u8 function_cc read_memory8(u32 address)
{
  u8 value;
  read_memory(8);
  return value;
}

u16 function_cc read_memory16_signed(u32 address)
{
  u16 value;

  if(address & 0x01)
  {
    return (s8)read_memory8(address);
  }
  else
  {
    read_memory(16);
  }

  return value;
}

// unaligned reads are actually 32bit

u32 function_cc read_memory16(u32 address)
{
  u32 value;

  if(address & 0x01)
  {
    address &= ~0x01;
    read_memory(16);
    ror(value, value, 8);
  }
  else
  {
    read_memory(16);
  }

  return value;
}


u32 function_cc read_memory32(u32 address)
{
  u32 value;
  if(address & 0x03)
  {
    u32 rotate = (address & 0x03) * 8;
    address &= ~0x03;
    read_memory(32);
    ror(value, value, rotate);
  }
  else
  {
    read_memory(32);
  }

  return value;
}

cpu_alert_type function_cc write_memory8(u32 address, u8 value)
{
  write_memory(8);
  return CPU_ALERT_NONE;
}

cpu_alert_type function_cc write_memory16(u32 address, u16 value)
{
  write_memory(16);
  return CPU_ALERT_NONE;
}

cpu_alert_type function_cc write_memory32(u32 address, u32 value)
{
  write_memory(32);
  return CPU_ALERT_NONE;
}

char backup_filename[512];

u32 load_backup(const char *name)
{
  file_open(backup_file, name, read);

  if(file_check_valid(backup_file))
  {
    u32 backup_size = file_length(name, backup_file);

    file_read(backup_file, gamepak_backup, backup_size);

    file_close(backup_file);

    // The size might give away what kind of backup it is.
    switch(backup_size)
    {
      case 0x200:
        backup_type = BACKUP_EEPROM;
        eeprom_size = EEPROM_512_BYTE;
        break;

      case 0x2000:
        backup_type = BACKUP_EEPROM;
        eeprom_size = EEPROM_8_KBYTE;
        break;

      case 0x8000:
        backup_type = BACKUP_SRAM;
        sram_size = SRAM_SIZE_32KB;
        break;

      // Could be either flash or SRAM, go with flash
      case 0x10000:
        backup_type = BACKUP_FLASH;
        sram_size = FLASH_SIZE_64KB;
        break;

      case 0x20000:
        backup_type = BACKUP_FLASH;
        flash_size = FLASH_SIZE_128KB;
        break;
    }
    return 1;
  }
  else
  {
    backup_type = BACKUP_NONE;
    memset(gamepak_backup, 0xFF, 1024 * 128);
  }

  return 0;
}

u32 save_backup(char *name)
{
  if(backup_type != BACKUP_NONE)
  {
    file_open(backup_file, name, write);

    if(file_check_valid(backup_file))
    {
      u32 backup_size;

      switch(backup_type)
      {
        case BACKUP_SRAM:
          if(sram_size == SRAM_SIZE_32KB)
            backup_size = 0x8000;
          else
            backup_size = 0x10000;
          break;

        case BACKUP_FLASH:
          if(flash_size == FLASH_SIZE_64KB)
            backup_size = 0x10000;
          else
            backup_size = 0x20000;
          break;

        case BACKUP_EEPROM:
          if(eeprom_size == EEPROM_512_BYTE)
            backup_size = 0x200;
          else
            backup_size = 0x2000;
          break;
      }

      file_write(backup_file, gamepak_backup, backup_size);

      file_close(backup_file);
      return 1;
    }
  }

  return 0;
}

void update_backup()
{
  if(backup_update != (write_backup_delay + 1))
    backup_update--;

  if(backup_update == 0)
  {
    save_backup(backup_filename);
    backup_update = write_backup_delay + 1;
  }
}

void update_backup_force()
{
  save_backup(backup_filename);
}

#define CONFIG_FILENAME "game_config.txt"

u8 *skip_spaces(u8 *line_ptr)
{
  while(*line_ptr == ' ')
    line_ptr++;

  return line_ptr;
}

s32 parse_config_line(u8 *current_line, u8 *current_variable, u8 *current_value)
{
  u8 *line_ptr = current_line;
  u8 *line_ptr_new;

  if((current_line[0] == 0) || (current_line[0] == '#'))
    return -1;

  line_ptr_new = strchr(line_ptr, ' ');
  if(line_ptr_new == NULL)
    return -1;

  *line_ptr_new = 0;
  strcpy(current_variable, line_ptr);
  line_ptr_new = skip_spaces(line_ptr_new + 1);

  if(*line_ptr_new != '=')
    return -1;

  line_ptr_new = skip_spaces(line_ptr_new + 1);
  strcpy(current_value, line_ptr_new);
  line_ptr_new = current_value + strlen(current_value) - 1;
  if(*line_ptr_new == '\n')
  {
    line_ptr_new--;
    *line_ptr_new = 0;
  }

  if(*line_ptr_new == '\r')
    *line_ptr_new = 0;

  return 0;
}

s32 load_game_config(u8 *gamepak_title, u8 *gamepak_code, u8 *gamepak_maker)
{
  u8 current_line[256];
  u8 current_variable[256];
  u8 current_value[256];
  u8 config_path[512];
  u8 *line_ptr;
  u32 fgets_value;
  FILE *config_file;

  idle_loop_target_pc = 0xFFFFFFFF;
  iwram_stack_optimize = 1;
  bios_rom[0x39] = 0x00;
  bios_rom[0x2C] = 0x00;
  translation_gate_targets = 0;
  flash_device_id = FLASH_DEVICE_MACRONIX_64KB;

  sprintf(config_path, "%s/%s", main_path, CONFIG_FILENAME);

  config_file = fopen(config_path, "rb");

  if(config_file)
  {
    while(fgets(current_line, 256, config_file))
    {
      if(parse_config_line(current_line, current_variable, current_value)
       != -1)
      {
        if(strcmp(current_variable, "game_name") ||
         strcmp(current_value, gamepak_title))
          continue;

        if(!fgets(current_line, 256, config_file) ||
         (parse_config_line(current_line, current_variable,
           current_value) == -1) ||
         strcmp(current_variable, "game_code") ||
         strcmp(current_value, gamepak_code))
          continue;

        if(!fgets(current_line, 256, config_file) ||
         (parse_config_line(current_line, current_variable,
           current_value) == -1) ||
         strcmp(current_variable, "vender_code") ||
          strcmp(current_value, gamepak_maker))
          continue;

        while(fgets(current_line, 256, config_file))
        {
          if(parse_config_line(current_line, current_variable, current_value)
           != -1)
          {
            if(!strcmp(current_variable, "game_name"))
            {
              fclose(config_file);
              return 0;
            }

            if(!strcmp(current_variable, "idle_loop_eliminate_target"))
              idle_loop_target_pc = strtol(current_value, NULL, 16);

            if(!strcmp(current_variable, "translation_gate_target"))
            {
              if(translation_gate_targets < MAX_TRANSLATION_GATES)
              {
                translation_gate_target_pc[translation_gate_targets] =
                 strtol(current_value, NULL, 16);
                translation_gate_targets++;
              }
            }

            if(!strcmp(current_variable, "iwram_stack_optimize") &&
              !strcmp(current_value, "no"))
            {
                iwram_stack_optimize = 0;
            }

            if(!strcmp(current_variable, "flash_rom_type") &&
              !strcmp(current_value, "128KB"))
            {
              flash_device_id = FLASH_DEVICE_MACRONIX_128KB;
            }

            if(!strcmp(current_variable, "bios_rom_hack_39") &&
              !strcmp(current_value, "yes"))
            {
              bios_rom[0x39] = 0xC0;
            }

            if(!strcmp(current_variable, "bios_rom_hack_2C") &&
              !strcmp(current_value, "yes"))
            {
               bios_rom[0x2C] = 0x02;
            }
          }
        }

        fclose(config_file);
        return 0;
      }
    }

    fclose(config_file);
  }

  return -1;
}

s32 load_gamepak_raw(const char *name)
{
  file_open(gamepak_file, name, read);

  if(file_check_valid(gamepak_file))
  {
    u32 file_size = file_length(name, gamepak_file);

    // First, close the last one if it was open, we won't
    // be needing it anymore.
    if(file_check_valid(gamepak_file_large))
      file_close(gamepak_file_large);

    // If it's a big file size keep it, don't close it, we'll
    // probably want to load from it more later.
    if(file_size <= gamepak_ram_buffer_size)
    {
      file_read(gamepak_file, gamepak_rom, file_size);

      file_close(gamepak_file);

      gamepak_file_large = NULL;
    }
    else
    {
      // Read in just enough for the header
      file_read(gamepak_file, gamepak_rom, 0x100);
      gamepak_file_large = gamepak_file;
    }

    return file_size;
  }

  return -1;
}

u8 gamepak_title[13];
u8 gamepak_code[5];
u8 gamepak_maker[3];
u8 gamepak_filename[512];

u32 load_gamepak(const char *name)
{
  char *dot_position = strrchr(name, '.');
  s32 file_size;
  u8 cheats_filename[256];

  if(!strcmp(dot_position, ".zip"))
    file_size = load_file_zip(name);
  else
    file_size = load_gamepak_raw(name);

  // A dumb April fool's joke was here once :o

  if(file_size != -1)
  {
    gamepak_size = (file_size + 0x7FFF) & ~0x7FFF;

    strcpy(backup_filename, name);
    strncpy(gamepak_filename, name, 512);
    change_ext(gamepak_filename, backup_filename, ".sav");

    load_backup(backup_filename);

    memcpy(gamepak_title, gamepak_rom + 0xA0, 12);
    memcpy(gamepak_code, gamepak_rom + 0xAC, 4);
    memcpy(gamepak_maker, gamepak_rom + 0xB0, 2);
    gamepak_title[12] = 0;
    gamepak_code[4] = 0;
    gamepak_maker[2] = 0;

    load_game_config(gamepak_title, gamepak_code, gamepak_maker);
    load_game_config_file();

    change_ext(gamepak_filename, cheats_filename, ".cht");
    add_cheats(cheats_filename);

    return 0;
  }

  return -1;
}

s32 load_bios(const char *name)
{
  file_open(bios_file, name, read);

  if(file_check_valid(bios_file))
  {
    file_read(bios_file, bios_rom, 0x4000);

    // This is a hack to get Zelda working, because emulating
    // the proper memory read behavior here is much too expensive.
    file_close(bios_file);
    return 0;
  }

  return -1;
}

// DMA memory regions can be one of the following:
// IWRAM - 32kb offset from the contiguous iwram region.
// EWRAM - like segmented but with self modifying code check.
// VRAM - 96kb offset from the contiguous vram region, should take care
// Palette RAM - Converts palette entries when written to.
// OAM RAM - Sets OAM modified flag to true.
// I/O registers - Uses the I/O register function.
// of mirroring properly.
// Segmented RAM/ROM - a region >= 32kb, the translated address has to
//  be reloaded if it wraps around the limit (cartride ROM)
// Ext - should be handled by the memory read/write function.

// The following map determines the region of each (assumes DMA access
// is not done out of bounds)

typedef enum
{
  DMA_REGION_IWRAM,
  DMA_REGION_EWRAM,
  DMA_REGION_VRAM,
  DMA_REGION_PALETTE_RAM,
  DMA_REGION_OAM_RAM,
  DMA_REGION_IO,
  DMA_REGION_GAMEPAK,
  DMA_REGION_EXT,
  DMA_REGION_BIOS,
  DMA_REGION_NULL
} dma_region_type;

dma_region_type dma_region_map[16] =
{
  DMA_REGION_BIOS,          // 0x00 - BIOS
  DMA_REGION_NULL,          // 0x01 - Nothing
  DMA_REGION_EWRAM,         // 0x02 - EWRAM
  DMA_REGION_IWRAM,         // 0x03 - IWRAM
  DMA_REGION_IO,            // 0x04 - I/O registers
  DMA_REGION_PALETTE_RAM,   // 0x05 - palette RAM
  DMA_REGION_VRAM,          // 0x06 - VRAM
  DMA_REGION_OAM_RAM,       // 0x07 - OAM RAM
  DMA_REGION_GAMEPAK,       // 0x08 - gamepak ROM
  DMA_REGION_GAMEPAK,       // 0x09 - gamepak ROM
  DMA_REGION_GAMEPAK,       // 0x0A - gamepak ROM
  DMA_REGION_GAMEPAK,       // 0x0B - gamepak ROM
  DMA_REGION_GAMEPAK,       // 0x0C - gamepak ROM
  DMA_REGION_EXT,           // 0x0D - EEPROM
  DMA_REGION_EXT,           // 0x0E - gamepak SRAM/flash ROM
  DMA_REGION_EXT            // 0x0F - gamepak SRAM/flash ROM
};

#define dma_adjust_ptr_inc(ptr, size)                                         \
  ptr += (size / 8)                                                           \

#define dma_adjust_ptr_dec(ptr, size)                                         \
  ptr -= (size / 8)                                                           \

#define dma_adjust_ptr_fix(ptr, size)                                         \

#define dma_adjust_ptr_writeback()                                            \
  dma->dest_address = dest_ptr                                                \

#define dma_adjust_ptr_reload()                                               \

#define dma_print(src_op, dest_op, transfer_size, wb)                         \
  printf("dma from %x (%s) to %x (%s) for %x (%s) (%s) (%d) (pc %x)\n",       \
   src_ptr, #src_op, dest_ptr, #dest_op, length, #transfer_size, #wb,         \
   dma->irq, reg[15]);                                                        \

#define dma_smc_vars_src()                                                    \

#define dma_smc_vars_dest()                                                   \
  u32 smc_trigger = 0                                                         \

#define dma_vars_iwram(type)                                                  \
  dma_smc_vars_##type()                                                       \

#define dma_vars_vram(type)                                                   \

#define dma_vars_palette_ram(type)                                            \

#define dma_oam_ram_src()                                                     \

#define dma_oam_ram_dest()                                                    \
  oam_update = 1                                                              \

#define dma_vars_oam_ram(type)                                                \
  dma_oam_ram_##type()                                                        \

#define dma_vars_io(type)                                                     \

#define dma_segmented_load_src()                                              \
  memory_map_read[src_current_region]                                         \

#define dma_segmented_load_dest()                                             \
  memory_map_write[dest_current_region]                                       \

#define dma_vars_gamepak(type)                                                \
  u32 type##_new_region;                                                      \
  u32 type##_current_region = type##_ptr >> 15;                               \
  u8 *type##_address_block = dma_segmented_load_##type();                     \
  if(type##_address_block == NULL)                                            \
  {                                                                           \
    if((type##_ptr & 0x1FFFFFF) >= gamepak_size)                              \
      break;                                                                  \
    type##_address_block = load_gamepak_page(type##_current_region & 0x3FF);  \
  }                                                                           \

#define dma_vars_ewram(type)                                                  \
  dma_smc_vars_##type();                                                      \
  u32 type##_new_region;                                                      \
  u32 type##_current_region = type##_ptr >> 15;                               \
  u8 *type##_address_block = dma_segmented_load_##type()                      \

#define dma_vars_bios(type)                                                   \

#define dma_vars_ext(type)                                                    \

#define dma_ewram_check_region(type)                                          \
  type##_new_region = (type##_ptr >> 15);                                     \
  if(type##_new_region != type##_current_region)                              \
  {                                                                           \
    type##_current_region = type##_new_region;                                \
    type##_address_block = dma_segmented_load_##type();                       \
  }                                                                           \

#define dma_gamepak_check_region(type)                                        \
  type##_new_region = (type##_ptr >> 15);                                     \
  if(type##_new_region != type##_current_region)                              \
  {                                                                           \
    type##_current_region = type##_new_region;                                \
    type##_address_block = dma_segmented_load_##type();                       \
    if(type##_address_block == NULL)                                          \
    {                                                                         \
      type##_address_block =                                                  \
       load_gamepak_page(type##_current_region & 0x3FF);                      \
    }                                                                         \
  }                                                                           \

#define dma_read_iwram(type, transfer_size)                                   \
  read_value = address##transfer_size(iwram + 0x8000, type##_ptr & 0x7FFF)    \

#define dma_read_vram(type, transfer_size)                                    \
  read_value = address##transfer_size(vram, type##_ptr & 0x1FFFF)             \

#define dma_read_io(type, transfer_size)                                      \
  read_value = address##transfer_size(io_registers, type##_ptr & 0x7FFF)      \

#define dma_read_oam_ram(type, transfer_size)                                 \
  read_value = address##transfer_size(oam_ram, type##_ptr & 0x3FF)            \

#define dma_read_palette_ram(type, transfer_size)                             \
  read_value = address##transfer_size(palette_ram, type##_ptr & 0x3FF)        \

#define dma_read_ewram(type, transfer_size)                                   \
  dma_ewram_check_region(type);                                               \
  read_value = address##transfer_size(type##_address_block,                   \
   type##_ptr & 0x7FFF)                                                       \

#define dma_read_gamepak(type, transfer_size)                                 \
  dma_gamepak_check_region(type);                                             \
  read_value = address##transfer_size(type##_address_block,                   \
   type##_ptr & 0x7FFF)                                                       \

// DMAing from the BIOS is funny, just returns 0..

#define dma_read_bios(type, transfer_size)                                    \
  read_value = 0                                                              \

#define dma_read_ext(type, transfer_size)                                     \
  read_value = read_memory##transfer_size(type##_ptr)                         \

#define dma_write_iwram(type, transfer_size)                                  \
  address##transfer_size(iwram + 0x8000, type##_ptr & 0x7FFF) = read_value;   \
  smc_trigger |= address##transfer_size(iwram, type##_ptr & 0x7FFF)           \

#define dma_write_vram(type, transfer_size)                                   \
  address##transfer_size(vram, type##_ptr & 0x1FFFF) = read_value             \

#define dma_write_io(type, transfer_size)                                     \
  write_io_register##transfer_size(type##_ptr & 0x3FF, read_value)            \

#define dma_write_oam_ram(type, transfer_size)                                \
  address##transfer_size(oam_ram, type##_ptr & 0x3FF) = read_value            \

#define dma_write_palette_ram(type, transfer_size)                            \
  write_palette##transfer_size(type##_ptr & 0x3FF, read_value)                \

#define dma_write_ext(type, transfer_size)                                    \
  write_memory##transfer_size(type##_ptr, read_value)                         \

#define dma_write_ewram(type, transfer_size)                                  \
  dma_ewram_check_region(type);                                               \
                                                                              \
  address##transfer_size(type##_address_block, type##_ptr & 0x7FFF) =         \
    read_value;                                                               \
  smc_trigger |= address##transfer_size(type##_address_block,                 \
   (type##_ptr & 0x7FFF) - 0x8000)                                            \

#define dma_epilogue_iwram()                                                  \
  if(smc_trigger)                                                             \
  {                                                                           \
    /* Special return code indicating to retranslate to the CPU code */       \
    return_value = CPU_ALERT_SMC;                                             \
  }                                                                           \

#define dma_epilogue_ewram()                                                  \
  if(smc_trigger)                                                             \
  {                                                                           \
    /* Special return code indicating to retranslate to the CPU code */       \
    return_value = CPU_ALERT_SMC;                                             \
  }                                                                           \

#define dma_epilogue_vram()                                                   \

#define dma_epilogue_io()                                                     \

#define dma_epilogue_oam_ram()                                                \

#define dma_epilogue_palette_ram()                                            \

#define dma_epilogue_GAMEPAK()                                                \

#define dma_epilogue_ext()                                                    \

#define print_line()                                                          \
  dma_print(src_op, dest_op, transfer_size, wb);                              \

#define dma_transfer_loop_region(src_region_type, dest_region_type, src_op,   \
 dest_op, transfer_size, wb)                                                  \
{                                                                             \
  dma_vars_##src_region_type(src);                                            \
  dma_vars_##dest_region_type(dest);                                          \
                                                                              \
  for(i = 0; i < length; i++)                                                 \
  {                                                                           \
    dma_read_##src_region_type(src, transfer_size);                           \
    dma_write_##dest_region_type(dest, transfer_size);                        \
    dma_adjust_ptr_##src_op(src_ptr, transfer_size);                          \
    dma_adjust_ptr_##dest_op(dest_ptr, transfer_size);                        \
  }                                                                           \
  dma->source_address = src_ptr;                                              \
  dma_adjust_ptr_##wb();                                                      \
  dma_epilogue_##dest_region_type();                                          \
  break;                                                                      \
}                                                                             \

#define dma_transfer_loop(src_op, dest_op, transfer_size, wb);                \
{                                                                             \
  u32 src_region = src_ptr >> 24;                                             \
  u32 dest_region = dest_ptr >> 24;                                           \
  dma_region_type src_region_type = dma_region_map[src_region];               \
  dma_region_type dest_region_type = dma_region_map[dest_region];             \
                                                                              \
  switch(src_region_type | (dest_region_type << 4))                           \
  {                                                                           \
    case (DMA_REGION_BIOS | (DMA_REGION_IWRAM << 4)):                         \
      dma_transfer_loop_region(bios, iwram, src_op, dest_op,                  \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IWRAM | (DMA_REGION_IWRAM << 4)):                        \
      dma_transfer_loop_region(iwram, iwram, src_op, dest_op,                 \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EWRAM | (DMA_REGION_IWRAM << 4)):                        \
      dma_transfer_loop_region(ewram, iwram, src_op, dest_op,                 \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_VRAM | (DMA_REGION_IWRAM << 4)):                         \
      dma_transfer_loop_region(vram, iwram, src_op, dest_op,                  \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_PALETTE_RAM | (DMA_REGION_IWRAM << 4)):                  \
      dma_transfer_loop_region(palette_ram, iwram, src_op, dest_op,           \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_OAM_RAM | (DMA_REGION_IWRAM << 4)):                      \
      dma_transfer_loop_region(oam_ram, iwram, src_op, dest_op,               \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IO | (DMA_REGION_IWRAM << 4)):                           \
      dma_transfer_loop_region(io, iwram, src_op, dest_op,                    \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_GAMEPAK | (DMA_REGION_IWRAM << 4)):                      \
      dma_transfer_loop_region(gamepak, iwram, src_op, dest_op,               \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EXT | (DMA_REGION_IWRAM << 4)):                          \
      dma_transfer_loop_region(ext, iwram, src_op, dest_op,                   \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_BIOS | (DMA_REGION_EWRAM << 4)):                         \
      dma_transfer_loop_region(bios, ewram, src_op, dest_op,                  \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IWRAM | (DMA_REGION_EWRAM << 4)):                        \
      dma_transfer_loop_region(iwram, ewram, src_op, dest_op,                 \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EWRAM | (DMA_REGION_EWRAM << 4)):                        \
      dma_transfer_loop_region(ewram, ewram, src_op, dest_op,                 \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_VRAM | (DMA_REGION_EWRAM << 4)):                         \
      dma_transfer_loop_region(vram, ewram, src_op, dest_op,                  \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_PALETTE_RAM | (DMA_REGION_EWRAM << 4)):                  \
      dma_transfer_loop_region(palette_ram, ewram, src_op, dest_op,           \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_OAM_RAM | (DMA_REGION_EWRAM << 4)):                      \
      dma_transfer_loop_region(oam_ram, ewram, src_op, dest_op,               \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IO | (DMA_REGION_EWRAM << 4)):                           \
      dma_transfer_loop_region(io, ewram, src_op, dest_op,                    \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_GAMEPAK | (DMA_REGION_EWRAM << 4)):                      \
      dma_transfer_loop_region(gamepak, ewram, src_op, dest_op,               \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EXT | (DMA_REGION_EWRAM << 4)):                          \
      dma_transfer_loop_region(ext, ewram, src_op, dest_op,                   \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_BIOS | (DMA_REGION_VRAM << 4)):                          \
      dma_transfer_loop_region(bios, vram, src_op, dest_op,                   \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IWRAM | (DMA_REGION_VRAM << 4)):                         \
      dma_transfer_loop_region(iwram, vram, src_op, dest_op,                  \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EWRAM | (DMA_REGION_VRAM << 4)):                         \
      dma_transfer_loop_region(ewram, vram, src_op, dest_op,                  \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_VRAM | (DMA_REGION_VRAM << 4)):                          \
      dma_transfer_loop_region(vram, vram, src_op, dest_op,                   \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_PALETTE_RAM | (DMA_REGION_VRAM << 4)):                   \
      dma_transfer_loop_region(palette_ram, vram, src_op, dest_op,            \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_OAM_RAM | (DMA_REGION_VRAM << 4)):                       \
      dma_transfer_loop_region(oam_ram, vram, src_op, dest_op,                \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IO | (DMA_REGION_VRAM << 4)):                            \
      dma_transfer_loop_region(io, vram, src_op, dest_op,                     \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_GAMEPAK | (DMA_REGION_VRAM << 4)):                       \
      dma_transfer_loop_region(gamepak, vram, src_op, dest_op,                \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EXT | (DMA_REGION_VRAM << 4)):                           \
      dma_transfer_loop_region(ext, vram, src_op, dest_op,                    \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_BIOS | (DMA_REGION_PALETTE_RAM << 4)):                   \
      dma_transfer_loop_region(bios, palette_ram, src_op, dest_op,            \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IWRAM | (DMA_REGION_PALETTE_RAM << 4)):                  \
      dma_transfer_loop_region(iwram, palette_ram, src_op, dest_op,           \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EWRAM | (DMA_REGION_PALETTE_RAM << 4)):                  \
      dma_transfer_loop_region(ewram, palette_ram, src_op, dest_op,           \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_VRAM | (DMA_REGION_PALETTE_RAM << 4)):                   \
      dma_transfer_loop_region(vram, palette_ram, src_op, dest_op,            \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_PALETTE_RAM | (DMA_REGION_PALETTE_RAM << 4)):            \
      dma_transfer_loop_region(palette_ram, palette_ram, src_op, dest_op,     \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_OAM_RAM | (DMA_REGION_PALETTE_RAM << 4)):                \
      dma_transfer_loop_region(oam_ram, palette_ram, src_op, dest_op,         \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IO | (DMA_REGION_PALETTE_RAM << 4)):                     \
      dma_transfer_loop_region(io, palette_ram, src_op, dest_op,              \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_GAMEPAK | (DMA_REGION_PALETTE_RAM << 4)):                \
      dma_transfer_loop_region(gamepak, palette_ram, src_op, dest_op,         \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EXT | (DMA_REGION_PALETTE_RAM << 4)):                    \
      dma_transfer_loop_region(ext, palette_ram, src_op, dest_op,             \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_BIOS | (DMA_REGION_OAM_RAM << 4)):                       \
      dma_transfer_loop_region(bios, oam_ram, src_op, dest_op,                \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IWRAM | (DMA_REGION_OAM_RAM << 4)):                      \
      dma_transfer_loop_region(iwram, oam_ram, src_op, dest_op,               \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EWRAM | (DMA_REGION_OAM_RAM << 4)):                      \
      dma_transfer_loop_region(ewram, oam_ram, src_op, dest_op,               \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_VRAM | (DMA_REGION_OAM_RAM << 4)):                       \
      dma_transfer_loop_region(vram, oam_ram, src_op, dest_op,                \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_PALETTE_RAM | (DMA_REGION_OAM_RAM << 4)):                \
      dma_transfer_loop_region(palette_ram, oam_ram, src_op, dest_op,         \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_OAM_RAM | (DMA_REGION_OAM_RAM << 4)):                    \
      dma_transfer_loop_region(oam_ram, oam_ram, src_op, dest_op,             \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IO | (DMA_REGION_OAM_RAM << 4)):                         \
      dma_transfer_loop_region(io, oam_ram, src_op, dest_op,                  \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_GAMEPAK | (DMA_REGION_OAM_RAM << 4)):                    \
      dma_transfer_loop_region(gamepak, oam_ram, src_op, dest_op,             \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EXT | (DMA_REGION_OAM_RAM << 4)):                        \
      dma_transfer_loop_region(ext, oam_ram, src_op, dest_op,                 \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_BIOS | (DMA_REGION_IO << 4)):                            \
      dma_transfer_loop_region(bios, io, src_op, dest_op,                     \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IWRAM | (DMA_REGION_IO << 4)):                           \
      dma_transfer_loop_region(iwram, io, src_op, dest_op,                    \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EWRAM | (DMA_REGION_IO << 4)):                           \
      dma_transfer_loop_region(ewram, io, src_op, dest_op,                    \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_VRAM | (DMA_REGION_IO << 4)):                            \
      dma_transfer_loop_region(vram, io, src_op, dest_op,                     \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_PALETTE_RAM | (DMA_REGION_IO << 4)):                     \
      dma_transfer_loop_region(palette_ram, io, src_op, dest_op,              \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_OAM_RAM | (DMA_REGION_IO << 4)):                         \
      dma_transfer_loop_region(oam_ram, io, src_op, dest_op,                  \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IO | (DMA_REGION_IO << 4)):                              \
      dma_transfer_loop_region(io, io, src_op, dest_op,                       \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_GAMEPAK | (DMA_REGION_IO << 4)):                         \
      dma_transfer_loop_region(gamepak, io, src_op, dest_op,                  \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EXT | (DMA_REGION_IO << 4)):                             \
      dma_transfer_loop_region(ext, io, src_op, dest_op,                      \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_BIOS | (DMA_REGION_EXT << 4)):                           \
      dma_transfer_loop_region(bios, ext, src_op, dest_op,                    \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IWRAM | (DMA_REGION_EXT << 4)):                          \
      dma_transfer_loop_region(iwram, ext, src_op, dest_op,                   \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EWRAM | (DMA_REGION_EXT << 4)):                          \
      dma_transfer_loop_region(ewram, ext, src_op, dest_op,                   \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_VRAM | (DMA_REGION_EXT << 4)):                           \
      dma_transfer_loop_region(vram, ext, src_op, dest_op,                    \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_PALETTE_RAM | (DMA_REGION_EXT << 4)):                    \
      dma_transfer_loop_region(palette_ram, ext, src_op, dest_op,             \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_OAM_RAM | (DMA_REGION_EXT << 4)):                        \
      dma_transfer_loop_region(oam_ram, ext, src_op, dest_op,                 \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_IO | (DMA_REGION_EXT << 4)):                             \
      dma_transfer_loop_region(io, ext, src_op, dest_op,                      \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_GAMEPAK | (DMA_REGION_EXT << 4)):                        \
      dma_transfer_loop_region(gamepak, ext, src_op, dest_op,                 \
       transfer_size, wb);                                                    \
                                                                              \
    case (DMA_REGION_EXT | (DMA_REGION_EXT << 3)):                            \
      dma_transfer_loop_region(ext, ext, src_op, dest_op,                     \
       transfer_size, wb);                                                    \
  }                                                                           \
  break;                                                                      \
}                                                                             \

#define dma_transfer_expand(transfer_size)                                    \
  switch((dma->dest_direction << 2) | dma->source_direction)                  \
  {                                                                           \
    case 0x00:                                                                \
      dma_transfer_loop(inc, inc, transfer_size, writeback);                  \
                                                                              \
    case 0x01:                                                                \
      dma_transfer_loop(dec, inc, transfer_size, writeback);                  \
                                                                              \
    case 0x02:                                                                \
      dma_transfer_loop(fix, inc, transfer_size, writeback);                  \
                                                                              \
    case 0x03:                                                                \
      break;                                                                  \
                                                                              \
    case 0x04:                                                                \
      dma_transfer_loop(inc, dec, transfer_size, writeback);                  \
                                                                              \
    case 0x05:                                                                \
      dma_transfer_loop(dec, dec, transfer_size, writeback);                  \
                                                                              \
    case 0x06:                                                                \
      dma_transfer_loop(fix, dec, transfer_size, writeback);                  \
                                                                              \
    case 0x07:                                                                \
      break;                                                                  \
                                                                              \
    case 0x08:                                                                \
      dma_transfer_loop(inc, fix, transfer_size, writeback);                  \
                                                                              \
    case 0x09:                                                                \
      dma_transfer_loop(dec, fix, transfer_size, writeback);                  \
                                                                              \
    case 0x0A:                                                                \
      dma_transfer_loop(fix, fix, transfer_size, writeback);                  \
                                                                              \
    case 0x0B:                                                                \
      break;                                                                  \
                                                                              \
    case 0x0C:                                                                \
      dma_transfer_loop(inc, inc, transfer_size, reload);                     \
                                                                              \
    case 0x0D:                                                                \
      dma_transfer_loop(dec, inc, transfer_size, reload);                     \
                                                                              \
    case 0x0E:                                                                \
      dma_transfer_loop(fix, inc, transfer_size, reload);                     \
                                                                              \
    case 0x0F:                                                                \
      break;                                                                  \
  }                                                                           \

cpu_alert_type dma_transfer(dma_transfer_type *dma)
{
  u32 i;
  u32 length = dma->length;
  u32 read_value;
  u32 src_ptr = dma->source_address;
  u32 dest_ptr = dma->dest_address;
  cpu_alert_type return_value = CPU_ALERT_NONE;

  // Technically this should be done for source and destination, but
  // chances are this is only ever used (probably mistakingly!) for dest.
  // The only game I know of that requires this is Lucky Luke.
  if((dest_ptr >> 24) != ((dest_ptr + length - 1) >> 24))
  {
    u32 first_length = ((dest_ptr & 0xFF000000) + 0x1000000) - dest_ptr;
    u32 second_length = length - first_length;
    dma->length = first_length;

    dma_transfer(dma);

    dma->length = length;

    length = second_length;
    dest_ptr += first_length;
    src_ptr += first_length;
  }

  if(dma->length_type == DMA_16BIT)
  {
    src_ptr &= ~0x01;
    dest_ptr &= ~0x01;
    cycle_dma16_words += length;
    dma_transfer_expand(16);
  }
  else
  {
    src_ptr &= ~0x03;
    dest_ptr &= ~0x03;
    cycle_dma32_words += length;
    dma_transfer_expand(32);
  }

  if((dma->repeat_type == DMA_NO_REPEAT) ||
   (dma->start_type == DMA_START_IMMEDIATELY))
  {
    dma->start_type = DMA_INACTIVE;
    address16(io_registers, (dma->dma_channel * 12) + 0xBA) &=
     (~0x8000);
  }

  if(dma->irq)
  {
    raise_interrupt(IRQ_DMA0 << dma->dma_channel);
    return_value = CPU_ALERT_IRQ;
  }

  return return_value;
}

// Be sure to do this after loading ROMs.

#define map_region(type, start, end, mirror_blocks, region)                   \
  for(map_offset = (start) / 0x8000; map_offset <                             \
   ((end) / 0x8000); map_offset++)                                            \
  {                                                                           \
    memory_map_##type[map_offset] =                                           \
     ((u8 *)region) + ((map_offset % mirror_blocks) * 0x8000);                \
  }                                                                           \

#define map_null(type, start, end)                                            \
  for(map_offset = start / 0x8000; map_offset < (end / 0x8000);               \
   map_offset++)                                                              \
  {                                                                           \
    memory_map_##type[map_offset] = NULL;                                     \
  }                                                                           \

#define map_ram_region(type, start, end, mirror_blocks, region)               \
  for(map_offset = (start) / 0x8000; map_offset <                             \
   ((end) / 0x8000); map_offset++)                                            \
  {                                                                           \
    memory_map_##type[map_offset] =                                           \
     ((u8 *)region) + ((map_offset % mirror_blocks) * 0x10000) + 0x8000;      \
  }                                                                           \

#define map_vram(type)                                                        \
  for(map_offset = 0x6000000 / 0x8000; map_offset < (0x7000000 / 0x8000);     \
   map_offset += 4)                                                           \
  {                                                                           \
    memory_map_##type[map_offset] = vram;                                     \
    memory_map_##type[map_offset + 1] = vram + 0x8000;                        \
    memory_map_##type[map_offset + 2] = vram + (0x8000 * 2);                  \
    memory_map_##type[map_offset + 3] = vram + (0x8000 * 2);                  \
  }                                                                           \

#define map_vram_firstpage(type)                                              \
  for(map_offset = 0x6000000 / 0x8000; map_offset < (0x7000000 / 0x8000);     \
   map_offset += 4)                                                           \
  {                                                                           \
    memory_map_##type[map_offset] = vram;                                     \
    memory_map_##type[map_offset + 1] = NULL;                                 \
    memory_map_##type[map_offset + 2] = NULL;                                 \
    memory_map_##type[map_offset + 3] = NULL;                                 \
  }                                                                           \


// Picks a page to evict
u32 page_time = 0;

u32 evict_gamepak_page()
{
  // Find the one with the smallest frame timestamp
  u32 page_index = 0;
  u32 physical_index;
  u32 smallest = gamepak_memory_map[0].page_timestamp;
  u32 i;

  for(i = 1; i < gamepak_ram_pages; i++)
  {
    if(gamepak_memory_map[i].page_timestamp <= smallest)
    {
      smallest = gamepak_memory_map[i].page_timestamp;
      page_index = i;
    }
  }

  physical_index = gamepak_memory_map[page_index].physical_index;

  memory_map_read[(0x8000000 / (32 * 1024)) + physical_index] = NULL;
  memory_map_read[(0xA000000 / (32 * 1024)) + physical_index] = NULL;
  memory_map_read[(0xC000000 / (32 * 1024)) + physical_index] = NULL;

  return page_index;
}

u8 *load_gamepak_page(u32 physical_index)
{
  if(physical_index >= (gamepak_size >> 15))
    return gamepak_rom;

  u32 page_index = evict_gamepak_page();
  u32 page_offset = page_index * (32 * 1024);
  u8 *swap_location = gamepak_rom + page_offset;

  gamepak_memory_map[page_index].page_timestamp = page_time;
  gamepak_memory_map[page_index].physical_index = physical_index;
  page_time++;

  file_seek(gamepak_file_large, physical_index * (32 * 1024), SEEK_SET);
  file_read(gamepak_file_large, swap_location, (32 * 1024));
  memory_map_read[(0x8000000 / (32 * 1024)) + physical_index] = swap_location;
  memory_map_read[(0xA000000 / (32 * 1024)) + physical_index] = swap_location;
  memory_map_read[(0xC000000 / (32 * 1024)) + physical_index] = swap_location;

  // If RTC is active page the RTC register bytes so they can be read
  if((rtc_state != RTC_DISABLED) && (physical_index == 0))
  {
    memcpy(swap_location + 0xC4, rtc_registers, sizeof(rtc_registers));
  }

  return swap_location;
}

void init_memory_gamepak()
{
  u32 map_offset = 0;

  if(gamepak_size > gamepak_ram_buffer_size)
  {
    // Large ROMs get special treatment because they
    // can't fit into the 16MB ROM buffer.
    u32 i;
    for(i = 0; i < gamepak_ram_pages; i++)
    {
      gamepak_memory_map[i].page_timestamp = 0;
      gamepak_memory_map[i].physical_index = 0;
    }

    map_null(read, 0x8000000, 0xD000000);
  }
  else
  {
    map_region(read, 0x8000000, 0x8000000 + gamepak_size, 1024, gamepak_rom);
    map_null(read, 0x8000000 + gamepak_size, 0xA000000);
    map_region(read, 0xA000000, 0xA000000 + gamepak_size, 1024, gamepak_rom);
    map_null(read, 0xA000000 + gamepak_size, 0xC000000);
    map_region(read, 0xC000000, 0xC000000 + gamepak_size, 1024, gamepak_rom);
    map_null(read, 0xC000000 + gamepak_size, 0xE000000);
  }
}

void init_gamepak_buffer()
{
  // Try to initialize 32MB (this is mainly for non-PSP platforms)
  gamepak_rom = NULL;

  gamepak_ram_buffer_size = 32 * 1024 * 1024;
  gamepak_rom = malloc(gamepak_ram_buffer_size);

  if(gamepak_rom == NULL)
  {
    // Try 16MB, for PSP, then lower in 2MB increments
    gamepak_ram_buffer_size = 16 * 1024 * 1024;
    gamepak_rom = malloc(gamepak_ram_buffer_size);

    while(gamepak_rom == NULL)
    {
      gamepak_ram_buffer_size -= (2 * 1024 * 1024);
      gamepak_rom = malloc(gamepak_ram_buffer_size);
    }
  }

  // Here's assuming we'll have enough memory left over for this,
  // and that the above succeeded (if not we're in trouble all around)
  gamepak_ram_pages = gamepak_ram_buffer_size / (32 * 1024);
  gamepak_memory_map = malloc(sizeof(gamepak_swap_entry_type) *
   gamepak_ram_pages);
}

void init_memory()
{
  u32 i;
  u32 map_offset = 0;

  memory_regions[0x00] = (u8 *)bios_rom;
  memory_regions[0x01] = (u8 *)bios_rom;
  memory_regions[0x02] = (u8 *)ewram;
  memory_regions[0x03] = (u8 *)iwram + 0x8000;
  memory_regions[0x04] = (u8 *)io_registers;
  memory_regions[0x05] = (u8 *)palette_ram;
  memory_regions[0x06] = (u8 *)vram;
  memory_regions[0x07] = (u8 *)oam_ram;
  memory_regions[0x08] = (u8 *)gamepak_rom;
  memory_regions[0x09] = (u8 *)(gamepak_rom + 0xFFFFFF);
  memory_regions[0x0A] = (u8 *)gamepak_rom;
  memory_regions[0x0B] = (u8 *)(gamepak_rom + 0xFFFFFF);
  memory_regions[0x0C] = (u8 *)gamepak_rom;
  memory_regions[0x0D] = (u8 *)(gamepak_rom + 0xFFFFFF);
  memory_regions[0x0E] = (u8 *)gamepak_backup;

  memory_limits[0x00] = 0x3FFF;
  memory_limits[0x01] = 0x3FFF;
  memory_limits[0x02] = 0x3FFFF;
  memory_limits[0x03] = 0x7FFF;
  memory_limits[0x04] = 0x7FFF;
  memory_limits[0x05] = 0x3FF;
  memory_limits[0x06] = 0x17FFF;
  memory_limits[0x07] = 0x3FF;
  memory_limits[0x08] = 0x1FFFFFF;
  memory_limits[0x09] = 0x1FFFFFF;
  memory_limits[0x0A] = 0x1FFFFFF;
  memory_limits[0x0B] = 0x1FFFFFF;
  memory_limits[0x0C] = 0x1FFFFFF;
  memory_limits[0x0D] = 0x1FFFFFF;
  memory_limits[0x0E] = 0xFFFF;

  // Fill memory map regions, areas marked as NULL must be checked directly
  map_region(read, 0x0000000, 0x1000000, 1, bios_rom);
  map_null(read, 0x1000000, 0x2000000);
  map_ram_region(read, 0x2000000, 0x3000000, 8, ewram);
  map_ram_region(read, 0x3000000, 0x4000000, 1, iwram);
  map_region(read, 0x4000000, 0x5000000, 1, io_registers);
  map_null(read, 0x5000000, 0x6000000);
  map_null(read, 0x6000000, 0x7000000);
  map_vram(read);
  map_null(read, 0x7000000, 0x8000000);
  init_memory_gamepak();
  map_null(read, 0xE000000, 0x10000000);

  // Fill memory map regions, areas marked as NULL must be checked directly
  map_null(write, 0x0000000, 0x2000000);
  map_ram_region(write, 0x2000000, 0x3000000, 8, ewram);
  map_ram_region(write, 0x3000000, 0x4000000, 1, iwram);
  map_null(write, 0x4000000, 0x5000000);
  map_null(write, 0x5000000, 0x6000000);

  // The problem here is that the current method of handling self-modifying code
  // requires writeable memory to be proceeded by 32KB SMC data areas or be
  // indirectly writeable. It's possible to get around this if you turn off the SMC
  // check altogether, but this will make a good number of ROMs crash (perhaps most
  // of the ones that actually need it? This has yet to be determined).

  // This is because VRAM cannot be efficiently made incontiguous, and still allow
  // the renderer to work as efficiently. It would, at the very least, require a
  // lot of hacking of the renderer which I'm not prepared to do.

  // However, it IS possible to directly map the first page no matter what because
  // there's 32kb of blank stuff sitting beneath it.
  if(direct_map_vram)
  {
    map_vram(write);
  }
  else
  {
    map_null(write, 0x6000000, 0x7000000);
  }

  map_null(write, 0x7000000, 0x8000000);
  map_null(write, 0x8000000, 0xE000000);
  map_null(write, 0xE000000, 0x10000000);

  memset(io_registers, 0, 0x8000);
  memset(oam_ram, 0, 0x400);
  memset(palette_ram, 0, 0x400);
  memset(iwram, 0, 0x10000);
  memset(ewram, 0, 0x80000);
  memset(vram, 0, 0x18000);

  io_registers[REG_DISPCNT] = 0x80;
  io_registers[REG_P1] = 0x3FF;
  io_registers[REG_BG2PA] = 0x100;
  io_registers[REG_BG2PD] = 0x100;
  io_registers[REG_BG3PA] = 0x100;
  io_registers[REG_BG3PD] = 0x100;
  io_registers[REG_RCNT] = 0x8000;

  backup_type = BACKUP_NONE;

  sram_size = SRAM_SIZE_32KB;
  flash_size = FLASH_SIZE_64KB;

  flash_bank_ptr = gamepak_backup;
  flash_command_position = 0;
  eeprom_size = EEPROM_512_BYTE;
  eeprom_mode = EEPROM_BASE_MODE;
  eeprom_address = 0;
  eeprom_counter = 0;

  flash_mode = FLASH_BASE_MODE;

  rtc_state = RTC_DISABLED;
  memset(rtc_registers, 0, sizeof(rtc_registers));
  bios_read_protect = 0xe129f000;
}

void bios_region_read_allow()
{
  memory_map_read[0] = bios_rom;
}

void bios_region_read_protect()
{
  memory_map_read[0] = NULL;
}


#define savestate_block(type)                                                 \
  cpu_##type##_savestate(savestate_file);                                     \
  input_##type##_savestate(savestate_file);                                   \
  main_##type##_savestate(savestate_file);                                    \
  memory_##type##_savestate(savestate_file);                                  \
  sound_##type##_savestate(savestate_file);                                   \
  video_##type##_savestate(savestate_file)                                    \

void load_state(const char *savestate_filename)
{
  file_open(savestate_file, savestate_filename, read);
  if(file_check_valid(savestate_file))
  {
    char current_gamepak_filename[512];
    char savestate_gamepak_filename[512];
    u32 i;
    u32 current_color;

    strcpy(current_gamepak_filename, gamepak_filename);

    savestate_block(read);

    file_close(savestate_file);

    flush_translation_cache_ram();
    flush_translation_cache_rom();
    flush_translation_cache_bios();

    oam_update = 1;
    gbc_sound_update = 1;
    if(strcmp(current_gamepak_filename, gamepak_filename))
    {
      u32 dot_position = strcspn(current_gamepak_filename, ".");

      // We'll let it slide if the filenames of the savestate and
      // the gamepak are similar enough.
      strcpy(gamepak_filename, current_gamepak_filename);
      if(strncmp(savestate_filename, current_gamepak_filename, dot_position))
      {
        if(load_gamepak(gamepak_filename) != -1)
        {
          reset_gba();
          // Okay, so this takes a while, but for now it works.
          load_state(savestate_filename);
        }
        else
        {
          quit();
        }

        return;
      }
    }

    for(i = 0; i < 512; i++)
    {
      current_color = palette_ram[i];
      palette_ram_converted[i] =
       convert_palette(current_color);
    }

    // Oops, these contain raw pointers
    for(i = 0; i < 4; i++)
    {
      gbc_sound_channel[i].sample_data = square_pattern_duty[2];
    }
    current_debug_state = STEP;
    instruction_count = 0;

    reg[CHANGED_PC_STATUS] = 1;
  }
}

u8 savestate_write_buffer[506947] __attribute__ ((aligned(4)));
u8 *write_mem_ptr;

void save_state(const char *savestate_filename)
{
  write_mem_ptr = savestate_write_buffer;
  file_open(savestate_file, savestate_filename, write);
  if(file_check_valid(savestate_file))
  {
    savestate_block(write_mem);
    file_write(savestate_file, savestate_write_buffer,
     sizeof(savestate_write_buffer));

    file_close(savestate_file);
  }
}


#define memory_savestate_builder(type)                                        \
void memory_##type##_savestate(file_tag_type savestate_file)                  \
{                                                                             \
  u32 i;                                                                      \
                                                                              \
  file_##type##_variable(savestate_file, backup_type);                        \
  file_##type##_variable(savestate_file, sram_size);                          \
  file_##type##_variable(savestate_file, flash_mode);                         \
  file_##type##_variable(savestate_file, flash_command_position);             \
  file_##type##_variable(savestate_file, flash_bank_ptr);                     \
  file_##type##_variable(savestate_file, flash_device_id);                    \
  file_##type##_variable(savestate_file, flash_manufacturer_id);              \
  file_##type##_variable(savestate_file, flash_size);                         \
  file_##type##_variable(savestate_file, eeprom_size);                        \
  file_##type##_variable(savestate_file, eeprom_mode);                        \
  file_##type##_variable(savestate_file, eeprom_address_length);              \
  file_##type##_variable(savestate_file, eeprom_address);                     \
  file_##type##_variable(savestate_file, eeprom_counter);                     \
  file_##type##_variable(savestate_file, rtc_state);                          \
  file_##type##_variable(savestate_file, rtc_write_mode);                     \
  file_##type##_array(savestate_file, rtc_registers);                         \
  file_##type##_variable(savestate_file, rtc_command);                        \
  file_##type##_array(savestate_file, rtc_data);                              \
  file_##type##_variable(savestate_file, rtc_status);                         \
  file_##type##_variable(savestate_file, rtc_data_bytes);                     \
  file_##type##_variable(savestate_file, rtc_bit_count);                      \
  file_##type##_array(savestate_file, eeprom_buffer);                         \
  file_##type##_array(savestate_file, gamepak_filename);                      \
  file_##type##_array(savestate_file, dma);                                   \
                                                                              \
  file_##type(savestate_file, iwram + 0x8000, 0x8000);                        \
  for(i = 0; i < 8; i++)                                                      \
  {                                                                           \
    file_##type(savestate_file, ewram + (i * 0x10000) + 0x8000, 0x8000);      \
  }                                                                           \
  file_##type(savestate_file, vram, 0x18000);                                 \
  file_##type(savestate_file, oam_ram, 0x400);                                \
  file_##type(savestate_file, palette_ram, 0x400);                            \
  file_##type(savestate_file, io_registers, 0x8000);                          \
                                                                              \
  /* This is a hack, for now. */                                              \
  if((flash_bank_ptr < gamepak_backup) ||                                     \
   (flash_bank_ptr > (gamepak_backup + (1024 * 64))))                         \
  {                                                                           \
    flash_bank_ptr = gamepak_backup;                                          \
  }                                                                           \
}                                                                             \

memory_savestate_builder(read);
memory_savestate_builder(write_mem);

