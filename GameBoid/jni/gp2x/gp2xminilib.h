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

/* .h by Hermes/PS2Reality*/

#if !defined(GP2XMINILIB)
#define GP2XMINILIB

enum  { GP2X_UP=0x1,       GP2X_LEFT=0x4,       GP2X_DOWN=0x10,  GP2X_RIGHT=0x40,
        GP2X_START=1<<8,   GP2X_SELECT=1<<9,    GP2X_L=1<<10,    GP2X_R=1<<11,
        GP2X_A=1<<12,      GP2X_B=1<<13,        GP2X_X=1<<14,    GP2X_Y=1<<15,
        GP2X_VOL_UP=1<<22, GP2X_VOL_DOWN=1<<23, GP2X_PUSH=1<<27, };
 
#define gp2x_video_color15(R,G,B,A)  (((R&0xF8)<<8)|((G&0xF8)<<3)|((B&0xF8)>>3)|(A<<5))
#define gp2x_video_color8 (C,R,G,B)  gp2x_palette[C][0]=(G<<8)|B,gp2x_palette[C][1]=R;




     
extern unsigned short *gp2x_memregs, *gp2x_screen15, *gp2x_logvram15[2], gp2x_sound_buffer[4+(44100*2)*4]; //*2=stereo, *4=max buffers

        
 
extern unsigned long   gp2x_dev[4];


void gp2x_video_flip(void);
void gp2x_video_setpalette(void);
unsigned long gp2x_joystick_read(void);
void *gp2x_sound_play(void *blah);
void gp2x_deinit(void);
void gp2x_init(int bpp, int rate, int bits, int stereo, int Hz);



#endif
