
/*
  GP2X minimal library v0.5 by rlyeh, 2005.
 
  + GP2X video library with double buffering.
  + GP2X soundring buffer library with double buffering.
  + GP2X joystick library.
 
  Thanks to Squidge, Robster, snaff and NK, for the help & previous work! :-)
 
 
  What's new
  ==========
 
  0.5: patched sound for real stereo (using NK's solution); better init code.
 
  0.4: lots of cleanups; sound is threaded now, double buffered too; 8 bpp video support; better exiting code.
 
  0.3: shorter library; improved joystick diagonal detection.
 
  0.2: better code layout; public release.
 
  0.1: beta release
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <linux/fb.h>
#include <pthread.h>
#include "gp2xminilib.h"

extern void gp2x_sound_frame(void *blah, void *bufferg, int samples);
 


         unsigned long   gp2x_dev[4]={0,0,0,0}, gp2x_physvram[4];
         unsigned short *gp2x_memregs, *gp2x_screen15, *gp2x_logvram15[2], gp2x_sound_buffer[4+(44100*2)*4]; //*2=stereo, *4=max buffers
volatile unsigned short  gp2x_palette[512][2];
         unsigned char  *gp2x_screen8, *gp2x_logvram8[2];
         pthread_t       gp2x_sound_thread=0, gp2x_sound_thread_exit=0;

void gp2x_video_flip(void)
{
  unsigned long address=gp2x_physvram[gp2x_physvram[3]];
 
  gp2x_screen15=gp2x_logvram15[gp2x_physvram[3]^=1]; 
  gp2x_screen8 =gp2x_logvram8 [gp2x_physvram[3]   ]; 
 
  gp2x_memregs[0x290E>>1]=(unsigned short)(address & 0xffff);
  gp2x_memregs[0x2910>>1]=(unsigned short)(address >> 16);
  gp2x_memregs[0x2912>>1]=(unsigned short)(address & 0xffff);
  gp2x_memregs[0x2914>>1]=(unsigned short)(address >> 16);
}
 
void gp2x_video_setpalette(void)
{int i;
 gp2x_memregs[0x2958>>1]=0; 
 for(i=0; i<512; i++) gp2x_memregs[0x295A>>1]=gp2x_palette[i][0], gp2x_memregs[0x295A>>1]=gp2x_palette[i][1];
}
 
unsigned long gp2x_joystick_read(void)
{
  unsigned long value=(gp2x_memregs[0x1198>>1] & 0x00FF);
 
  if(value==0xFD) value=0xFA;
  if(value==0xF7) value=0xEB;
  if(value==0xDF) value=0xAF;
  if(value==0x7F) value=0xBE;
 
  return ~((gp2x_memregs[0x1184>>1] & 0xFF00) | value | (gp2x_memregs[0x1186>>1] << 16));
}
 
#if 0
void *gp2x_sound_play(void *blah)
{
  struct timespec ts;
  int flip=0;
 
  ts.tv_sec=0, ts.tv_nsec=gp2x_sound_buffer[2];
 
  while(! gp2x_sound_thread_exit)
  {
   gp2x_sound_frame(blah, (void *)(&gp2x_sound_buffer[4+flip]), gp2x_sound_buffer[0]);
   write(gp2x_dev[3], (void *)(&gp2x_sound_buffer[4+flip]), gp2x_sound_buffer[1]);
 
   flip^=gp2x_sound_buffer[1];
 
   //nanosleep(&ts, NULL);
  }
 
  return NULL;
}
#endif

void gp2x_deinit(void)
{int i;
  if(gp2x_sound_thread) { gp2x_sound_thread_exit=1; for(i=0;i<1000000;i++); }
 
  gp2x_memregs[0x28DA>>1]=0x4AB; 
  gp2x_memregs[0x290C>>1]=640;   
 
  close(gp2x_dev[0]);
  close(gp2x_dev[1]);
  close(gp2x_dev[2]);
  //close(gp2x_dev[3]);
  //fcloseall();
}
 
void gp2x_init(int bpp, int rate, int bits, int stereo, int Hz)
{
  struct fb_fix_screeninfo fixed_info;
 
  if(!gp2x_dev[0]) gp2x_dev[0] = open("/dev/fb0", O_RDWR);
  if(!gp2x_dev[1]) gp2x_dev[1] = open("/dev/fb1", O_RDWR);
  if(!gp2x_dev[2]) gp2x_dev[2] = open("/dev/mem", O_RDWR);
  //if(!gp2x_dev[3]) gp2x_dev[3] = open("/dev/dsp", O_WRONLY);
 
  gp2x_memregs=(unsigned short *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev[2], 0xc0000000);
 
  if(!gp2x_sound_thread) { gp2x_memregs[0x0F16>>1] = 0x830a; sleep(1); 
                           gp2x_memregs[0x0F58>>1] = 0x100c; sleep(1); }
 
                ioctl (gp2x_dev[0], FBIOGET_FSCREENINFO, &fixed_info); 
   gp2x_screen15=gp2x_logvram15[0]=(unsigned short *)mmap(0,  320*240*2, PROT_WRITE, MAP_SHARED, gp2x_dev[0], 0);
     gp2x_screen8=gp2x_logvram8[0]=(unsigned char *)gp2x_logvram15[0];
                  gp2x_physvram[0]=fixed_info.smem_start;
 
                ioctl (gp2x_dev[1], FBIOGET_FSCREENINFO, &fixed_info);
                 gp2x_logvram15[1]=(unsigned short *)mmap(0,  320*240*2, PROT_WRITE, MAP_SHARED, gp2x_dev[1], 0);
                  gp2x_logvram8[1]=(unsigned char *)gp2x_logvram15[1];
                  gp2x_physvram[1]=fixed_info.smem_start;
 
  gp2x_memregs[0x28DA>>1]=(((bpp+1)/8)<<9)|0xAB; /*8/15/16/24bpp...*/
  gp2x_memregs[0x290C>>1]=320*((bpp+1)/8);       /*line width in bytes*/
 
  ioctl(gp2x_dev[3], SNDCTL_DSP_SPEED,  &rate);
  ioctl(gp2x_dev[3], SNDCTL_DSP_SETFMT, &bits);
  ioctl(gp2x_dev[3], SNDCTL_DSP_STEREO, &stereo);
 
  gp2x_sound_buffer[1]=(gp2x_sound_buffer[0]=(rate/Hz)) << (stereo + (bits==16));
  gp2x_sound_buffer[2]=(1000000/Hz);
 
  if(!gp2x_sound_thread) { gp2x_sound_thread = 1; //pthread_create( &gp2x_sound_thread, NULL, gp2x_sound_play, NULL);
                           atexit(gp2x_deinit); }
}
 
 
 
/*
 
EXAMPLE
=======
 
   now supply your own function for 16 bits, stereo:
 
   void gp2x_sound_frame(void *blah, void *bufferg, int samples)
   {
     signed short *buffer=(signed short *)bufferg;
     while(samples--)
     {
       *buffer++=0; //Left channel
       *buffer++=0; //Right channel
     }
   }
 
   or 16 bits mono:
 
   void gp2x_sound_frame(void *blah, void *bufferg, int samples)
   {
     signed short *buffer=(signed short *)bufferg;
     while(samples--)
     {
       *buffer++=0; //Central channel
     }
   }
 
  now the main program...
 
  hicolor example:
 
  int main(int argc, char *argv[])
  {
   //this sets video to hicolor (16 bpp)
   //it also sets sound to 44100,16bits,stereo and syncs audio to 50 Hz (PAL timing)
 
   //Warning: GP2X does not support 8bit sound sampling! (at least within Linux)
 
   gp2x_init(16,44100,16,1,50);
 
   while(1)
   {
    unsigned long  pad=gp2x_joystick_read();
    unsigned short color=gp2x_video_color15(255,255,255,0);
 
    if(pad & GP2X_L) if(pad & GP2X_R) exit();           
 
    if(pad & GP2X_A) color=gp2x_color15(255,255,255,0);  //white
     else            color=gp2x_color15(255,0,0,0);      //red
 
    gp2x_screen15[160+120*320]=color;                    //x=160, y=120
    gp2x_video_flip();
   }
  }
 
  palettized example:
 
  int main(int argc, char *argv[])
  {
   //this sets video to palette mode (8 bpp)
   //it also sets sound to 11025,16bits,stereo and syncs audio to 60 Hz (NSTC timing)
 
   //Warning: GP2X does not support 8bit sound sampling! (at least within Linux)
 
   gp2x_init(8,11025,16,1,60);
 
   gp2x_video_color8(0,0,0,0);       //color #0 is black for us
   gp2x_video_color8(1,255,255,255); //color #1 is white for us
   gp2x_video_color8(2,255,0,0);     //color #2 is red   for us
   gp2x_video_setpalette();
 
   while(1)
   {
    unsigned long pad=gp2x_joystick_read();
    unsigned char color;
 
    if(pad & GP2X_L) if(pad & GP2X_R) exit();           
 
    if(pad & GP2X_A) color=1;  //white
     else            color=2;  //red
 
    gp2x_screen8[160+120*320]=color;                      //x=160, y=120
    gp2x_video_flip();
   }
  }
 
*/

