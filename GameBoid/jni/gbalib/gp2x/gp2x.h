#ifndef GP2X_H
#define GP2X_H

enum
{
  GP2X_UP       = 1 << 0,
  GP2X_LEFT     = 1 << 2,
  GP2X_DOWN     = 1 << 4,
  GP2X_RIGHT    = 1 << 6,
  GP2X_START    = 1 << 8,
  GP2X_SELECT   = 1 << 9,
  GP2X_L        = 1 << 10,
  GP2X_R        = 1 << 11,
  GP2X_A        = 1 << 12,
  GP2X_B        = 1 << 13,
  GP2X_X        = 1 << 14,
  GP2X_Y        = 1 << 15,
  GP2X_VOL_DOWN = 1 << 22,
  GP2X_VOL_UP   = 1 << 23,
  GP2X_PUSH     = 1 << 27
};


extern u32 gpsp_gp2x_dev_audio;
extern u32 gpsp_gp2x_dev;
extern volatile u16 *gpsp_gp2x_memregs;
extern volatile u32 *gpsp_gp2x_memregl;

void gp2x_sound_volume(u32 volume_up);
void gp2x_quit();

// call this at first
void cpuctrl_init(void);
void save_system_regs(void);
void cpuctrl_deinit(void);
void set_display_clock_div(unsigned div);

void set_FCLK(u32 MHZ);
// 0 to 7 divider (freq = FCLK / (1 + div))
void set_920_Div(u16 div);
void set_DCLK_Div(u16 div);

void Disable_940(void);
void gp2x_video_wait_vsync(void);
unsigned short get_920_Div();
void set_940_Div(u16 div);

s32 gp2x_load_mmuhack();

#endif
