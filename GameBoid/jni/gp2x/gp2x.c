/*  Parts used from cpuctrl */
/*  cpuctrl for GP2X
    Copyright (C) 2005  Hermes/PS2Reality

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include "../common.h"
#include "gp2x.h"

extern int main_cpuspeed(int argc, char *argv[]);
extern SDL_Surface* screen;

u32 gp2x_audio_volume = 74;
u32 gpsp_gp2x_dev_audio = 0;
u32 gpsp_gp2x_dev = 0;

volatile u16 *gpsp_gp2x_memregs;
volatile u32 *gpsp_gp2x_memregl;

static volatile u16 *MEM_REG;

s32 gp2x_load_mmuhack()
{
  s32 mmufd = open("/dev/mmuhack", O_RDWR);

  if(mmufd < 0)
  {
    system("/sbin/insmod mmuhack.o");
    mmufd = open("/dev/mmuhack", O_RDWR);
  }

  if(mmufd < 0)
    return -1;

  close(mmufd);
  return 0;
}

void gp2x_overclock()
{
  gpsp_gp2x_dev = open("/dev/mem",   O_RDWR);
  gpsp_gp2x_dev_audio = open("/dev/mixer", O_RDWR);
  gpsp_gp2x_memregl =
   (unsigned long  *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED,
   gpsp_gp2x_dev, 0xc0000000);
  gpsp_gp2x_memregs = (unsigned short *)gpsp_gp2x_memregl;

  clear_screen(0);
  main_cpuspeed(0, NULL);
  gp2x_sound_volume(1);
}

void gp2x_quit()
{
  munmap((void *)gpsp_gp2x_memregl, 0x10000);
  close(gpsp_gp2x_dev_audio);
  close(gpsp_gp2x_dev);
  chdir("/usr/gp2x");
  execl("gp2xmenu", "gp2xmenu", NULL);
}

void gp2x_sound_volume(u32 volume_up)
{
  u32 volume;
  if((volume_up == 0) && (gp2x_audio_volume > 0))
    gp2x_audio_volume--;

  if((volume_up != 0)  && (gp2x_audio_volume < 100))
    gp2x_audio_volume++;

  volume = (gp2x_audio_volume * 0x50) / 100;
  volume = (gp2x_audio_volume << 8) | gp2x_audio_volume;
  ioctl(gpsp_gp2x_dev_audio, SOUND_MIXER_WRITE_PCM, &volume);
}

