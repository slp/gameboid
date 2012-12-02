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

cheat_type cheats[MAX_CHEATS];
u32 num_cheats;

void decrypt_gsa_code(u32 *address_ptr, u32 *value_ptr, cheat_variant_enum
 cheat_variant)
{
  u32 i, i2, code_position;
  u32 address = *address_ptr;
  u32 value = *value_ptr;
  u32 r = 0xc6ef3720;

  u32 seeds_v1[4] =
  {
    0x09f4fbbd, 0x9681884a, 0x352027e9, 0xf3dee5a7
  };
  u32 seeds_v3[4] =
  {
    0x7aa9648f, 0x7fae6994, 0xc0efaad5, 0x42712c57
  };
  u32 *seeds;

  if(cheat_variant == CHEAT_TYPE_GAMESHARK_V1)
    seeds = seeds_v1;
  else
    seeds = seeds_v3;

  for(i = 0; i < 32; i++)
  {
    value -= ((address << 4) + seeds[2]) ^ (address + r) ^
     ((address >> 5) + seeds[3]);
    address -= ((value << 4) + seeds[0]) ^ (value + r) ^
     ((value >> 5) + seeds[1]);
    r -= 0x9e3779b9;
  }

  *address_ptr = address;
  *value_ptr = value;
}

void add_cheats(u8 *cheats_filename)
{
  FILE *cheats_file;
  u8 current_line[256];
  u8 *name_ptr;
  u32 *cheat_code_ptr;
  u32 address, value;
  u32 num_cheat_lines;
  u32 cheat_name_length;
  cheat_variant_enum current_cheat_variant;

  num_cheats = 0;

  cheats_file = fopen(cheats_filename, "rb");

  if(cheats_file)
  {
    while(fgets(current_line, 256, cheats_file))
    {
      // Get the header line first
      name_ptr = strchr(current_line, ' ');
      if(name_ptr)
      {
        *name_ptr = 0;
        name_ptr++;
      }

      if(!strcasecmp(current_line, "gameshark_v1") ||
       !strcasecmp(current_line, "gameshark_v2") ||
       !strcasecmp(current_line, "PAR_v1") ||
       !strcasecmp(current_line, "PAR_v2"))
      {
        current_cheat_variant = CHEAT_TYPE_GAMESHARK_V1;
      }
      else

      if(!strcasecmp(current_line, "gameshark_v3") ||
       !strcasecmp(current_line, "PAR_v3"))
      {
        current_cheat_variant = CHEAT_TYPE_GAMESHARK_V3;
      }
      else
      {
        current_cheat_variant = CHEAT_TYPE_INVALID;
      }

      if(current_cheat_variant != CHEAT_TYPE_INVALID)
      {
        strncpy(cheats[num_cheats].cheat_name, name_ptr, CHEAT_NAME_LENGTH - 1);
        cheats[num_cheats].cheat_name[CHEAT_NAME_LENGTH - 1] = 0;
        cheat_name_length = strlen(cheats[num_cheats].cheat_name);
        if(cheat_name_length &&
         (cheats[num_cheats].cheat_name[cheat_name_length - 1] == '\n') ||
         (cheats[num_cheats].cheat_name[cheat_name_length - 1] == '\r'))
        {
          cheats[num_cheats].cheat_name[cheat_name_length - 1] = 0;
          cheat_name_length--;
        }

        if(cheat_name_length &&
         cheats[num_cheats].cheat_name[cheat_name_length - 1] == '\r')
        {
          cheats[num_cheats].cheat_name[cheat_name_length - 1] = 0;
        }

        cheats[num_cheats].cheat_variant = current_cheat_variant;
        cheat_code_ptr = cheats[num_cheats].cheat_codes;
        num_cheat_lines = 0;

        while(fgets(current_line, 256, cheats_file))
        {
          if(strlen(current_line) < 3)
            break;

          sscanf(current_line, "%08x %08x", &address, &value);

          decrypt_gsa_code(&address, &value, current_cheat_variant);

          cheat_code_ptr[0] = address;
          cheat_code_ptr[1] = value;

          cheat_code_ptr += 2;
          num_cheat_lines++;
        }

        cheats[num_cheats].num_cheat_lines = num_cheat_lines;

        num_cheats++;
      }
    }

    fclose(cheats_file);
  }
}

void process_cheat_gs1(cheat_type *cheat)
{
  u32 cheat_opcode;
  u32 *code_ptr = cheat->cheat_codes;
  u32 address, value;
  u32 i;

  for(i = 0; i < cheat->num_cheat_lines; i++)
  {
    address = code_ptr[0];
    value = code_ptr[1];

    code_ptr += 2;

    cheat_opcode = address >> 28;
    address &= 0xFFFFFFF;

    switch(cheat_opcode)
    {
      case 0x0:
        write_memory8(address, value);
        break;

      case 0x1:
        write_memory16(address, value);
        break;

      case 0x2:
        write_memory32(address, value);
        break;

      case 0x3:
      {
        u32 num_addresses = address & 0xFFFF;
        u32 address1, address2;
        u32 i2;

        for(i2 = 0; i2 < num_addresses; i2++)
        {
          address1 = code_ptr[0];
          address2 = code_ptr[1];
          code_ptr += 2;
          i++;

          write_memory32(address1, value);
          if(address2 != 0)
            write_memory32(address2, value);
        }
        break;
      }

      // ROM patch not supported yet
      case 0x6:
        break;

      // GS button down not supported yet
      case 0x8:
        break;

      // Reencryption (DEADFACE) not supported yet
      case 0xD:
        if(read_memory16(address) != (value & 0xFFFF))
        {
          code_ptr += 2;
          i++;
        }
        break;

      case 0xE:
        if(read_memory16(value & 0xFFFFFFF) != (address & 0xFFFF))
        {
          u32 skip = ((address >> 16) & 0x03);
          code_ptr += skip * 2;
          i += skip;
        }
        break;

      // Hook routine not supported yet (not important??)
      case 0x0F:
        break;
    }
  }
}

// These are especially incomplete.

void process_cheat_gs3(cheat_type *cheat)
{
  u32 cheat_opcode;
  u32 *code_ptr = cheat->cheat_codes;
  u32 address, value;
  u32 i;

  for(i = 0; i < cheat->num_cheat_lines; i++)
  {
    address = code_ptr[0];
    value = code_ptr[1];

    code_ptr += 2;

    cheat_opcode = address >> 28;
    address &= 0xFFFFFFF;

    switch(cheat_opcode)
    {
      case 0x0:
        cheat_opcode = address >> 24;
        address = (address & 0xFFFFF) + ((address << 4) & 0xF000000);

        switch(cheat_opcode)
        {
          case 0x0:
          {
            u32 iterations = value >> 24;
            u32 i2;

            value &= 0xFF;

            for(i2 = 0; i2 <= iterations; i2++, address++)
            {
              write_memory8(address, value);
            }
            break;
          }

          case 0x2:
          {
            u32 iterations = value >> 16;
            u32 i2;

            value &= 0xFFFF;

            for(i2 = 0; i2 <= iterations; i2++, address += 2)
            {
              write_memory16(address, value);
            }
            break;
          }

          case 0x4:
            write_memory32(address, value);
            break;
        }
        break;

      case 0x4:
        cheat_opcode = address >> 24;
        address = (address & 0xFFFFF) + ((address << 4) & 0xF000000);

        switch(cheat_opcode)
        {
          case 0x0:
            address = read_memory32(address) + (value >> 24);
            write_memory8(address, value & 0xFF);
            break;

          case 0x2:
            address = read_memory32(address) + ((value >> 16) * 2);
            write_memory16(address, value & 0xFFFF);
            break;

          case 0x4:
            address = read_memory32(address);
            write_memory32(address, value);
            break;

        }
        break;

      case 0x8:
        cheat_opcode = address >> 24;
        address = (address & 0xFFFFF) + ((address << 4) & 0xF000000);

        switch(cheat_opcode)
        {
          case 0x0:
            value = (value & 0xFF) + read_memory8(address);
            write_memory8(address, value);
            break;

          case 0x2:
            value = (value & 0xFFFF) + read_memory16(address);
            write_memory16(address, value);
            break;

          case 0x4:
            value = value + read_memory32(address);
            write_memory32(address, value);
            break;
        }
        break;

      case 0xC:
        cheat_opcode = address >> 24;
        address = (address & 0xFFFFFF) + 0x4000000;

        switch(cheat_opcode)
        {
          case 0x6:
            write_memory16(address, value);
            break;

          case 0x7:
            write_memory32(address, value);
            break;
        }
        break;
    }
  }
}


void process_cheats()
{
  u32 i;

  for(i = 0; i < num_cheats; i++)
  {
    if(cheats[i].cheat_active)
    {
      switch(cheats[i].cheat_variant)
      {
        case CHEAT_TYPE_GAMESHARK_V1:
          process_cheat_gs1(cheats + i);
          break;

        case CHEAT_TYPE_GAMESHARK_V3:
          process_cheat_gs3(cheats + i);
          break;
      }
    }
  }
}
