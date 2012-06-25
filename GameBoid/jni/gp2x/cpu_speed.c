
/*  CPU/LCD/RAM-Tuner for GP2X Version 2.0
    Copyright (C) 2006 god_at_hell
    original CPU-Overclocker (c) by Hermes/PS2Reality
  the gamma-routine was provided by theoddbot

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include "display.h"
#include "cpuctrl.h"
#include "gp2xminilib.h"
#include "speedtest.h"
#include "cmdline.h"
#include "daemon.h"

int cpuspeed_exit = 0;

unsigned COLORFONDO=0xB00000; // background-color
unsigned WHITE=0xFFFFFF;
unsigned TEXTBACK=0x0000B0;

unsigned gp2x_nKeys=0;
int oldtime=0;
char pageshift;
short menupoint;
unsigned char cad[256];
short cpusettings[2],dispsettings[3],ramsettings[10];
extern int daemonsettings[8];
extern unsigned MDIV,PDIV,SCALE;

void cleardisp()
{
    ClearScreen(COLORFONDO);
    sprintf(cad,"gpSP2X v0.9008 with CPU_SPEED");
    v_putcad(1,1,WHITE,COLORFONDO,cad);
    v_putcad(28,5,WHITE,COLORFONDO,"Item Help");
    v_putcad(2,26,WHITE,COLORFONDO,"Start:Quit Stick:UP/DOWN");
    
    DrawBox(WHITE);
}

void itemhelp(char item[])
{
  if(strcmp(item, "menu") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Enter submenu");
    v_putcad(26,9,0xffffff,COLORFONDO,"with B.");
  }
  if(strcmp(item, "cpuclk") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Choose a");
    v_putcad(26,9,0xffffff,COLORFONDO,"clockspeed");
    v_putcad(26,10,0xffffff,COLORFONDO,"with R/L or");
    v_putcad(26,11,0xffffff,COLORFONDO,"Vol UP/Down.");
    v_putcad(26,13,0xffffff,COLORFONDO,"Valid speeds");
    v_putcad(26,14,0xffffff,COLORFONDO,"are:");
    v_putcad(26,15,0xffffff,COLORFONDO,"33 to 340Mhz");
  }
  if(strcmp(item, "cpudiv") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Choose with");
    v_putcad(26,9,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,11,0xffffff,COLORFONDO,"The CPU-clock");
    v_putcad(26,12,0xffffff,COLORFONDO,"will be");
    v_putcad(26,13,0xffffff,COLORFONDO,"divided by");
    v_putcad(26,14,0xffffff,COLORFONDO,"this value.");
    v_putcad(26,16,0xffffff,COLORFONDO,"Valid values");
    v_putcad(26,17,0xffffff,COLORFONDO,"are from");
    v_putcad(26,18,0xffffff,COLORFONDO,"1 to 8");
  }
  if(strcmp(item, "test") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Change with");
    v_putcad(26,9,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,11,0xffffff,COLORFONDO,"The test");
    v_putcad(26,12,0xffffff,COLORFONDO,"checks how");
    v_putcad(26,13,0xffffff,COLORFONDO,"high you can");
    v_putcad(26,14,0xffffff,COLORFONDO,"clock the");
    v_putcad(26,15,0xffffff,COLORFONDO,"CPU.");
    v_putcad(26,17,0xffffff,COLORFONDO,"Check the");
    v_putcad(26,18,0xffffff,COLORFONDO,"Readme for");
    v_putcad(26,19,0xffffff,COLORFONDO,"more infos.");
  }
  if(strcmp(item, "gotest") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Start the");
    v_putcad(26,9,0xffffff,COLORFONDO,"test with B.");
  }
  if(strcmp(item, "clockgen") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Change the");
    v_putcad(26,9,0xffffff,COLORFONDO,"clockgen with");
    v_putcad(26,10,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,12,0xffffff,COLORFONDO,"FPLL is for");
    v_putcad(26,13,0xffffff,COLORFONDO,"FW 1.0");
    v_putcad(26,14,0xffffff,COLORFONDO,"UPLL for the");
    v_putcad(26,15,0xffffff,COLORFONDO,"rest.");
  }
  if(strcmp(item, "timing") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Choose the");
    v_putcad(26,9,0xffffff,COLORFONDO,"LCD-Timing");
    v_putcad(26,10,0xffffff,COLORFONDO,"with R/L.");
    v_putcad(26,12,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,13,0xffffff,COLORFONDO,"values from");
    v_putcad(26,14,0xffffff,COLORFONDO,"-20 to 36");
    v_putcad(26,15,0xffffff,COLORFONDO,"for FPLL");
    v_putcad(26,16,0xffffff,COLORFONDO,"and -6 to 10");
    v_putcad(26,17,0xffffff,COLORFONDO,"for UPLL");
  }
  if(strcmp(item, "gamma") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Change gamma");
    v_putcad(26,9,0xffffff,COLORFONDO,"with R/L or");
    v_putcad(26,10,0xffffff,COLORFONDO,"Vol UP/DOWN");
    v_putcad(26,12,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,13,0xffffff,COLORFONDO,"values from");
    v_putcad(26,14,0xffffff,COLORFONDO,"0.01 to 10.00");
  }
  if(strcmp(item, "CAS") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Choose the");
    v_putcad(26,9,0xffffff,COLORFONDO,"CAS Latency");
    v_putcad(26,10,0xffffff,COLORFONDO,"with R/L");
    v_putcad(26,12,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,13,0xffffff,COLORFONDO,"2 or 3 cycles");
    v_putcad(26,15,0xffffff,COLORFONDO,"Delay between");
    v_putcad(26,16,0xffffff,COLORFONDO,"registration");
    v_putcad(26,17,0xffffff,COLORFONDO,"of a READ");
    v_putcad(26,18,0xffffff,COLORFONDO,"command and");
    v_putcad(26,19,0xffffff,COLORFONDO,"outputdata.");
    v_putcad(27,21,0x0000DD,COLORFONDO,"!INSTABLE!");
  }
  if(strcmp(item, "tRC") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Change with");
    v_putcad(26,9,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,11,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,12,0xffffff,COLORFONDO,"1 to 16");
    v_putcad(26,13,0xffffff,COLORFONDO,"cycles.");
    v_putcad(26,15,0xffffff,COLORFONDO,"ACTIVE to");
    v_putcad(26,16,0xffffff,COLORFONDO,"ACTIVE/");
    v_putcad(26,17,0xffffff,COLORFONDO,"AUTOREFRESH");
    v_putcad(26,18,0xffffff,COLORFONDO,"command");
    v_putcad(26,19,0xffffff,COLORFONDO,"period delay.");
  }
  if(strcmp(item, "tRAS") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Change with");
    v_putcad(26,9,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,11,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,12,0xffffff,COLORFONDO,"1 to 16");
    v_putcad(26,13,0xffffff,COLORFONDO,"cycles.");
    v_putcad(26,15,0xffffff,COLORFONDO,"Delay between");
    v_putcad(26,16,0xffffff,COLORFONDO,"the ACTIVE");
    v_putcad(26,17,0xffffff,COLORFONDO,"and PRECHARGE");
    v_putcad(26,18,0xffffff,COLORFONDO,"commands.");
  }
  if(strcmp(item, "tWR") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Change with");
    v_putcad(26,9,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,11,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,12,0xffffff,COLORFONDO,"1 to 16");
    v_putcad(26,13,0xffffff,COLORFONDO,"cycles.");
    v_putcad(26,15,0xffffff,COLORFONDO,"Write");
    v_putcad(26,16,0xffffff,COLORFONDO,"recovery time");
    v_putcad(26,17,0xffffff,COLORFONDO,"in cycles.");
  }
  if(strcmp(item, "tMRD") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Change with");
    v_putcad(26,9,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,11,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,12,0xffffff,COLORFONDO,"1 to 16");
    v_putcad(26,13,0xffffff,COLORFONDO,"cycles.");
    v_putcad(26,15,0xffffff,COLORFONDO,"LOAD MODE");
    v_putcad(26,16,0xffffff,COLORFONDO,"REGISTER");
    v_putcad(26,17,0xffffff,COLORFONDO,"command cycle");
    v_putcad(26,18,0xffffff,COLORFONDO,"time.");
  }
  if(strcmp(item, "tRFC") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Change with");
    v_putcad(26,9,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,11,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,12,0xffffff,COLORFONDO,"1 to 16");
    v_putcad(26,13,0xffffff,COLORFONDO,"cycles.");
    v_putcad(26,15,0xffffff,COLORFONDO,"AUTOREFRESH");
    v_putcad(26,16,0xffffff,COLORFONDO,"command");
    v_putcad(26,17,0xffffff,COLORFONDO,"period.");
  }
  if(strcmp(item, "tRP") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Change with");
    v_putcad(26,9,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,11,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,12,0xffffff,COLORFONDO,"1 to 16");
    v_putcad(26,13,0xffffff,COLORFONDO,"cycles.");
    v_putcad(26,15,0xffffff,COLORFONDO,"PRECHARGE");
    v_putcad(26,16,0xffffff,COLORFONDO,"command");
    v_putcad(26,17,0xffffff,COLORFONDO,"period.");
  }
  if(strcmp(item, "tRCD") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Change with");
    v_putcad(26,9,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,11,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,12,0xffffff,COLORFONDO,"1 to 16");
    v_putcad(26,13,0xffffff,COLORFONDO,"cycles.");
    v_putcad(26,15,0xffffff,COLORFONDO,"RAS to CAS");
    v_putcad(26,16,0xffffff,COLORFONDO,"Delay in");
    v_putcad(26,17,0xffffff,COLORFONDO,"cycles.");
  }
  if(strcmp(item, "REFPERD") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Choose the");
    v_putcad(26,9,0xffffff,COLORFONDO,"RAM Refresh");
    v_putcad(26,9,0xffffff,COLORFONDO,"Period");
    v_putcad(26,10,0xffffff,COLORFONDO,"with R/L");
    v_putcad(26,12,0xffffff,COLORFONDO,"Valid are");
    v_putcad(26,13,0xffffff,COLORFONDO,"1 to 65535 ");
    v_putcad(26,14,0xffffff,COLORFONDO,"cycles.");
    v_putcad(26,16,0xffffff,COLORFONDO,"Max. cycles");
    v_putcad(26,17,0xffffff,COLORFONDO,"between");
    v_putcad(26,18,0xffffff,COLORFONDO,"AUTOREFRESH");
    v_putcad(26,19,0xffffff,COLORFONDO,"commands.");
    v_putcad(26,21,0xFFFF,COLORFONDO,"Default ~250");
  }
  if(strcmp(item, "DCLK_Div") == 0)
  {
    v_putcad(26,8,0xffffff,COLORFONDO,"Choose with");
    v_putcad(26,9,0xffffff,COLORFONDO,"R/L.");
    v_putcad(26,11,0xffffff,COLORFONDO,"The CPU-clock");
    v_putcad(26,12,0xffffff,COLORFONDO,"will be");
    v_putcad(26,13,0xffffff,COLORFONDO,"divided by");
    v_putcad(26,14,0xffffff,COLORFONDO,"this value.");
    v_putcad(26,15,0xffffff,COLORFONDO,"Valid values");
    v_putcad(26,17,0xffffff,COLORFONDO,"are from");
    v_putcad(26,18,0xffffff,COLORFONDO,"1 to 8");
    v_putcad(26,20,0xffffff,COLORFONDO,"RAM-Clock =");
    v_putcad(26,21,0xffffff,COLORFONDO,"CPU-Clock / 2");
  }
}

short cpumenu()
{
  unsigned sysfreq=0, cpufreq, cpu_div;
  
  short test = 1;
  
  if(cpusettings[0] == -1)
  {
    //get the setted values from system
    cpu_div = get_920_Div();
    sysfreq=get_freq_920_CLK();
    sysfreq*=cpu_div+1;
    cpufreq=sysfreq/1000000;
  }
  else
  {
    //use old settings
    cpufreq = cpusettings[0];
    cpu_div = cpusettings[1];
  }
  
  menupoint = 0;
  
  do
  { 
    cleardisp();
    
    //cpu menu
    v_putcad(12,2,WHITE,COLORFONDO,"CPU Clock Setup");
    sprintf(cad,"CPU Frequency = %uMhz",cpufreq);
    if(menupoint==0){v_putcad(2,5,0xffffff,TEXTBACK,cad);itemhelp("cpuclk");}
    else v_putcad(2,5,0xffff,COLORFONDO,cad);
    sprintf(cad,"CPU Divider: %u",cpu_div+1);
    if(menupoint==1){v_putcad(2,7,0xffffff,TEXTBACK,cad);itemhelp("cpudiv");}
    else v_putcad(2,7,0xffff,COLORFONDO,cad);
    v_putcad(2,12,0xffff00,COLORFONDO,"CPU Speedtest");
    if(test==0) sprintf(cad,"Test 2: Primnumber");
    if(test==1) sprintf(cad,"Test 1: Walking Ant");
    if(menupoint==2){v_putcad(2,15,0xffffff,TEXTBACK,cad);itemhelp("test");}
    else v_putcad(2,15,0xffff,COLORFONDO,cad);
    if(menupoint==3){v_putcad(8,17,0xffffff,TEXTBACK,"GO!!!");itemhelp("gotest");}
    else v_putcad(8,17,0xffff,COLORFONDO,"GO!!!");

    gp2x_video_flip();

    while(1)
    {
      gp2x_nKeys=gp2x_joystick_read();

      if((gp2x_nKeys & GP2X_START))
      {
        while(1)
        {
          gp2x_nKeys=gp2x_joystick_read();
          if(!(gp2x_nKeys & GP2X_START)) break;
        }
        cpusettings[0] = cpufreq;
        cpusettings[1] = cpu_div;
        return 0;
      }

      if((gp2x_nKeys & GP2X_DOWN))
      {
        menupoint++;
        if(menupoint>3) menupoint=0;
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_UP))
      {
        menupoint--;
        if(menupoint<0) menupoint=3;
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_R))
      {
        if (menupoint == 0)
        {
          cpufreq+=10;
          if (cpufreq > 340) cpufreq = 33;
        }
        if (menupoint == 1)
        {
          cpu_div++;
          if (cpu_div == 8) cpu_div = 0;
        }
        if (menupoint == 2)
        {
          test++;
          if (test == 2) test = 0;
        }
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_L))
      {
        if (menupoint == 0)
        {
          cpufreq-=10;
          if (cpufreq < 33) cpufreq = 340;
        }
        if (menupoint == 1)
        {
          cpu_div--;
          if (cpu_div == -1) cpu_div = 7;
        }
        if (menupoint == 2)
        {
          cpu_div--;
          if (cpu_div == -1) cpu_div = 1;
        }
        delay_us(200000);
        break;
      }
      
      if((gp2x_nKeys & GP2X_VOL_DOWN))
      {
        if (menupoint == 0)
        {
          cpufreq++;
          if (cpufreq == 340) cpufreq = 33;
        }
        delay_us(200000);
        break;
      }
            
      if((gp2x_nKeys & GP2X_VOL_UP))
      {
        if (menupoint == 0)
        {
          cpufreq--;
          if (cpufreq == 32) cpufreq = 340;
        }
        delay_us(200000);
        break;
      }
      
      if((gp2x_nKeys & GP2X_B))
      {
        if (menupoint == 3) speedtest(test);
        delay_us(200000);
        break;
      }
    }
  }
  while(1);
}

short lcdmenu()
{
  short clockgen, LCDClk;
  float gamma;
  
  if(dispsettings[0] == -1)
  {
    //get the setted values from system
    clockgen = get_Clkgen();
    LCDClk = get_LCDClk(clockgen);
    gamma = 1.0;
  }
  else
  {
    //use old settings
    clockgen = dispsettings[0];
    LCDClk = dispsettings[1];
    gamma = dispsettings[2]/100;
  }
  
  menupoint = 0;
  
  do
  { 
    cleardisp();
    
    //display menu
    v_putcad(9,2,WHITE,COLORFONDO,"Display Setting Setup");
    if (clockgen == 0)
    {
      sprintf(cad,"Clockgen = FPLL");
      if(menupoint==0){v_putcad(2,5,0xffffff,TEXTBACK,cad);itemhelp("clockgen");}
      else v_putcad(2,5,0xffff,COLORFONDO,cad);
    }
    else
    {
      sprintf(cad,"Clockgen = UPLL");
      if(menupoint==0){v_putcad(2,5,0xffffff,TEXTBACK,cad);itemhelp("clockgen");}
      else v_putcad(2,5,0xffff,COLORFONDO,cad);
    }
    sprintf(cad,"Timing: %i",LCDClk);
    if(menupoint==1){v_putcad(2,7,0xffffff,TEXTBACK,cad);itemhelp("timing");}
    else v_putcad(2,7,0xffff,COLORFONDO,cad);
    sprintf(cad,"Gamma: %.2f",gamma);
    if(menupoint==2){v_putcad(2,9,0xffffff,TEXTBACK,cad);itemhelp("gamma");}
    else v_putcad(2,9,0xffff,COLORFONDO,cad); 

    gp2x_video_flip();

    while(1)
    {
      gp2x_nKeys=gp2x_joystick_read();

      if((gp2x_nKeys & GP2X_START))
      {
        while(1)
        {
          gp2x_nKeys=gp2x_joystick_read();
          if(!(gp2x_nKeys & GP2X_START)) break;
        }
        dispsettings[0] = clockgen;
        dispsettings[1] = LCDClk;
        dispsettings[2] = (int)(gamma*100);
        return 0;
      }

      if((gp2x_nKeys & GP2X_DOWN))
      {
        menupoint++;
        if(menupoint>2) menupoint=0;
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_UP))
      {
        menupoint--;
        if(menupoint<0) menupoint=2;
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_R))
      {
        if (menupoint == 0)
        {
          clockgen++;
          if (clockgen == 2) clockgen = 0;
        }
        if (menupoint == 1)
        {
          LCDClk++;
          if (clockgen == 0)
          {
            if (LCDClk == 37) LCDClk = -20;
            set_add_FLCDCLK(LCDClk);
          }
          else
          {
            if (LCDClk == 11) LCDClk = -6;
            set_add_ULCDCLK(LCDClk);
          }
        }
        if (menupoint == 2)
        {
          gamma += 0.2;
          if (gamma > 10) gamma = 0.2;
          set_gamma(gamma);
        }
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_L))
      {
        if (menupoint == 0)
        {
          clockgen--;
          if (clockgen == -1) clockgen = 1;
        }
        if (menupoint == 1)
        {
          LCDClk--;
          if(clockgen == 0)
          {
            if (LCDClk == -21) LCDClk = 36;
            set_add_FLCDCLK(LCDClk);
          }
          else
          {
            if (LCDClk == -7) LCDClk = 10;
            set_add_ULCDCLK(LCDClk);
          }
        }
        if (menupoint == 2)
        {
          gamma -= 0.2;
          if (gamma < 0.01) gamma = 10;
          set_gamma(gamma);
        }
        delay_us(200000);
        break;
      }
      
      if((gp2x_nKeys & GP2X_VOL_DOWN))
      {
        if (menupoint == 2)
        {
          gamma += 0.01;
          if (gamma > 10) gamma = 0.01;
          set_gamma(gamma);
        }
        delay_us(200000);
        break;
      }
            
      if((gp2x_nKeys & GP2X_VOL_UP))
      {
        if (menupoint == 2)
        {
          gamma -= 0.01;
          if (gamma < 0.01) gamma = 10;
          set_gamma(gamma);
        }
        delay_us(200000);
        break;
      }
    }
  }
  while(1);
}

short rammenu()
{
  short CAS,tRC,tRAS,tWR,tMRD,tRFC,tRP,tRCD,DCLK_Div;
  int REFPERD;
  
  if(ramsettings[0] == -1)
  {
    //get the setted values from system
    CAS = get_CAS();
    tRC = get_tRC();
    tRAS = get_tRAS();
    tWR = get_tWR();
    tMRD = get_tMRD();
    tRFC = get_tRFC();
    tRP = get_tRP();
    tRCD = get_tRCD();
    REFPERD = get_REFPERD();
    DCLK_Div = get_DCLK_Div();
  }
  else
  {
    //use old settings
    CAS = ramsettings[0];
    tRC = ramsettings[1];
    tRAS = ramsettings[2];
    tWR = ramsettings[3];
    tMRD = ramsettings[4];
    tRFC = ramsettings[5];
    tRP = ramsettings[6];
    tRCD = ramsettings[7];
    REFPERD = ramsettings[8];
    DCLK_Div = ramsettings[9];
  }
  
  pageshift = 0;
  menupoint = 0;
  
  do
  { 
    cleardisp();
    
    //ram menu
    v_putcad(11,2,WHITE,COLORFONDO,"RAM Setting Setup");
    if(pageshift == 0)
    { 
      sprintf(cad,"CAS: %u",CAS+2);
      if(menupoint==0){v_putcad(2,5,0xffffff,TEXTBACK,cad);itemhelp("CAS");}
      else v_putcad(2,5,0xffff,COLORFONDO,cad);
    }
    else v_putcad(13,5,0xffff,COLORFONDO,"^");
    sprintf(cad,"tRC: %u",tRC+1);
    if(menupoint==1){v_putcad(2,7-pageshift,0xffffff,TEXTBACK,cad);itemhelp("tRC");}
    else v_putcad(2,7-pageshift,0xffff,COLORFONDO,cad);
    sprintf(cad,"tRAS: %u",tRAS+1);
    if(menupoint==2){v_putcad(2,9-pageshift,0xffffff,TEXTBACK,cad);itemhelp("tRAS");}
    else v_putcad(2,9-pageshift,0xffff,COLORFONDO,cad);
    sprintf(cad,"tWR: %u",tWR+1);
    if(menupoint==3){v_putcad(2,11-pageshift,0xffffff,TEXTBACK,cad);itemhelp("tWR");}
    else v_putcad(2,11-pageshift,0xffff,COLORFONDO,cad);
    sprintf(cad,"tMRD: %u",tMRD+1);
    if(menupoint==4){v_putcad(2,13-pageshift,0xffffff,TEXTBACK,cad);itemhelp("tMRD");}
    else v_putcad(2,13-pageshift,0xffff,COLORFONDO,cad);
    sprintf(cad,"tRFC: %u",tRFC+1);
    if(menupoint==5){v_putcad(2,15-pageshift,0xffffff,TEXTBACK,cad);itemhelp("tRFC");}
    else v_putcad(2,15-pageshift,0xffff,COLORFONDO,cad);
    sprintf(cad,"tRP: %u",tRP+1);
    if(menupoint==6){v_putcad(2,17-pageshift,0xffffff,TEXTBACK,cad);itemhelp("tRP");}
    else v_putcad(2,17-pageshift,0xffff,COLORFONDO,cad);
    sprintf(cad,"tRCD: %u",tRCD+1);
    if(menupoint==7){v_putcad(2,19-pageshift,0xffffff,TEXTBACK,cad);itemhelp("tRCD");}
    else v_putcad(2,19-pageshift,0xffff,COLORFONDO,cad);
    sprintf(cad,"Refresh Period: %u",REFPERD+1);
    if(menupoint==8){v_putcad(2,21-pageshift,0xffffff,TEXTBACK,cad);itemhelp("REFPERD");}
    else v_putcad(2,21-pageshift,0xffff,COLORFONDO,cad);
    if(pageshift == 1)
    { 
      sprintf(cad,"RAM Divider: %u",DCLK_Div+1);
      if(menupoint==9){v_putcad(2,23,0xffffff,TEXTBACK,cad);itemhelp("DCLK_Div");}
      else v_putcad(2,23,0xffff,COLORFONDO,cad);
    }
    else v_putcad(13,23,0xffff,COLORFONDO,"v");
    

    gp2x_video_flip();

    while(1)
    {
      gp2x_nKeys=gp2x_joystick_read();

      if((gp2x_nKeys & GP2X_START))
      {
        while(1)
        {
          gp2x_nKeys=gp2x_joystick_read();
          if(!(gp2x_nKeys & GP2X_START)) break;
        }
        ramsettings[0] = CAS;
        ramsettings[1] = tRC;
        ramsettings[2] = tRAS;
        ramsettings[3] = tWR;
        ramsettings[4] = tMRD;
        ramsettings[5] = tRFC;
        ramsettings[6] = tRP;
        ramsettings[7] = tRCD;
        ramsettings[8] = REFPERD;
        ramsettings[9] = DCLK_Div;
        return 0;
      }

      if((gp2x_nKeys & GP2X_DOWN))
      {
        menupoint++;
        if(menupoint==9) pageshift = 1;
        if(menupoint>9) {menupoint=0; pageshift = 0;} 
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_UP))
      {
        menupoint--;
        if(menupoint==0) pageshift = 0;
        if(menupoint<0) { menupoint=9; pageshift = 1;}  
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_R))
      {
        if (menupoint == 0)
        {
          CAS++;
          if (CAS == 2) CAS = 0;
        }
        if (menupoint == 1)
        {
          tRC++;
          if (tRC == 16) tRC = 0;
        }
        if (menupoint == 2)
        {
          tRAS++;
          if (tRAS == 16) tRAS = 0;
        }
        if (menupoint == 3)
        {
          tWR++;
          if (tWR == 16) tWR = 0;
        }
        if (menupoint == 4)
        {
          tMRD++;
          if (tMRD == 16) tMRD = 0;
        }
        if (menupoint == 5)
        {
          tRFC++;
          if (tRFC == 16) tRFC = 0;
        }
        if (menupoint == 6)
        {
          tRP++;
          if (tRP == 16) tRP = 0;
        }
        if (menupoint == 7)
        {
          tRCD++;
          if (tRCD == 16) tRCD = 0;
        }
        if (menupoint == 8)
        {
          REFPERD += 10;
          if (REFPERD == 0xffff) REFPERD = 0;
        }
        if (menupoint == 9)
        {
          DCLK_Div++;
          if (DCLK_Div == 8) DCLK_Div = 0;
        }
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_L))
      {
        if (menupoint == 0)
        {
          CAS--;
          if (CAS == -1) CAS = 1;
        }
        if (menupoint == 1)
        {
          tRC--;
          if (tRC == -1) tRC = 15;
        }
        if (menupoint == 2)
        {
          tRAS--;
          if (tRAS == -1) tRAS = 15;
        }
        if (menupoint == 3)
        {
          tWR--;
          if (tWR == -1) tWR = 15;
        }
        if (menupoint == 4)
        {
          tMRD--;
          if (tMRD == -1) tMRD = 15;
        }
        if (menupoint == 5)
        {
          tRFC--;
          if (tRFC == -1) tRFC = 15;
        }
        if (menupoint == 6)
        {
          tRP--;
          if (tRP == -1) tRP = 15;
        }
        if (menupoint == 7)
        {
          tRCD--;
          if (tRCD == -1) tRCD = 15;
        }
        if (menupoint == 8)
        {
          REFPERD -= 10;
          if (REFPERD == -1) REFPERD = 0xfffe;
        }
        if (menupoint == 9)
        {
          DCLK_Div--;
          if (DCLK_Div == -1) DCLK_Div = 7;
        }
        delay_us(200000);
        break;
      }
    }
  }
  while(1);
}

int exit_cpu_speed(char* runfile)
{
  gp2x_deinit();
  cpuspeed_exit = 1;
#if 0
  system("sync");
  if(strcmp(runfile, "gp2xmenu") == 0) chdir("/usr/gp2x");
  execl(runfile,NULL);
  exit(0);
#endif
} 

/****************************************************************************************************************************************/
//  MAIN
/****************************************************************************************************************************************/

int main_cpuspeed(int argc, char *argv[])
{
  unsigned sysfreq=0;
  int cpufreq, n;
  short run = 0;
  int start_daemon = 0;
  short mainmenupoint = 0;
  ramsettings[0] = -1;
  ramsettings[1] = -1;
  ramsettings[2] = -1;
  ramsettings[3] = -1;
  ramsettings[4] = -1;
  ramsettings[5] = -1;
  ramsettings[6] = -1;
  ramsettings[7] = -1;
  ramsettings[8] = -1;
  ramsettings[9] = -1;
  dispsettings[0] = -1;
  dispsettings[1] = -100;
  dispsettings[2] = -1;
  cpusettings[0] = -1;
  cpusettings[1] = -1;
  daemonsettings[0] = -1;
  daemonsettings[1] = 260;
  daemonsettings[2] = 5;
  daemonsettings[3] = 5;
  daemonsettings[4] = GP2X_L | GP2X_R;
  daemonsettings[5] = GP2X_VOL_DOWN;
  daemonsettings[6] = GP2X_VOL_UP;
  daemonsettings[7] = 1;
  
  char *runfile;

  FILE *settings;
  char sets[41][40];
  char* comps[25] = {
      "CPU-Clock\n","CPU-Div\n","FPLL\n","UPLL\n","Gamma\n","CAS\n","tRC\n",
      "tRAS\n","tWR\n","tMRD\n","tRFC\n","tRP\n","tRCD\n","Refresh-Period\n","RAM-Div\n","Run\n",
      "Daemon-Min\n","Daemon-Max\n","Daemon-Step\n","Daemon-Delay\n","Daemon-Hotkey\n","Daemon-Incr-Key\n","Daemon-Decr-Key\n",
      "Daemon-OSD\n","Daemon-Run\n"
  };
  
  gp2x_init(16,44100,16,1,50);
  cpuctrl_init();  // ATENCION: si no se hace esto, cuelgue seguro (aprovecho la definicion de rlyeh y debe ir despues de gp2x_init)
  
  settings = fopen ("./cpu_speed.cfg", "r");
  
  if (settings == NULL)
    {
    printf("No file found\n");
    }
    else
    {
    char *ReturnCode;
    for(n=0; n<40; n++)
    {
      ReturnCode = fgets(sets[n],40,settings);
      if (ReturnCode == NULL)
      { 
        sets[40][0] = (char)n;
        break;
      }
    }
    
    for(n=0; n<(int)sets[40][0]; n=n+2)
    {
      if(strcmp(sets[n],comps[0]) == 0)
      {
        if(atoi(sets[n+1]) > 33)
        if(atoi(sets[n+1]) < 341)
        cpusettings[0] = atoi(sets[n+1]);
      }
      if(strcmp(sets[n],comps[1]) == 0)
      {
        if(atoi(sets[n+1]) > 0)
        if(atoi(sets[n+1]) < 9)
        cpusettings[1] = atoi(sets[n+1])-1;
      }
      if(strcmp(sets[n],comps[2]) == 0)
      {
        if(atoi(sets[n+1]) > -21)
        if(atoi(sets[n+1]) < 37)
        dispsettings[0] = 0;
        dispsettings[1] = atoi(sets[n+1]);
        
      }
      if(strcmp(sets[n],comps[3]) == 0)
      {
        if(atoi(sets[n+1]) > -7)
        if(atoi(sets[n+1]) < 11)
        dispsettings[0] = 1;
        dispsettings[1] = atoi(sets[n+1]);
      }
      if(strcmp(sets[n],comps[4]) == 0)
      {
        if(atof(sets[n+1]) > 0)
        if(atof(sets[n+1]) <= 10)
        dispsettings[2] = (int)(atof(sets[n+1])*100);
      }
      if(strcmp(sets[n],comps[5]) == 0)
      {
        if(atoi(sets[n+1]) > 1)
        if(atoi(sets[n+1]) < 4)
        ramsettings[0] = atoi(sets[n+1])-2;
      }
      if(strcmp(sets[n],comps[6]) == 0)
      {
        if(atoi(sets[n+1]) > 0)
        if(atoi(sets[n+1]) < 17)
        ramsettings[1] = atoi(sets[n+1])-1;
      }
      if(strcmp(sets[n],comps[7]) == 0)
      {
        if(atoi(sets[n+1]) > 0)
        if(atoi(sets[n+1]) < 17)
        ramsettings[2] = atoi(sets[n+1])-1;
      }
      if(strcmp(sets[n],comps[8]) == 0)
      {
        if(atoi(sets[n+1]) > 0)
        if(atoi(sets[n+1]) < 17)
        ramsettings[3] = atoi(sets[n+1])-1;
      }
      if(strcmp(sets[n],comps[9]) == 0)
      {
        if(atoi(sets[n+1]) > 0)
        if(atoi(sets[n+1]) < 17)
        ramsettings[4] = atoi(sets[n+1])-1;
      }
      if(strcmp(sets[n],comps[10]) == 0)
      {
        if(atoi(sets[n+1]) > 0)
        if(atoi(sets[n+1]) < 17)
        ramsettings[5] = atoi(sets[n+1])-1;
      }
      if(strcmp(sets[n],comps[11]) == 0)
      {
        if(atoi(sets[n+1]) > 0)
        if(atoi(sets[n+1]) < 17)
        ramsettings[6] = atoi(sets[n+1])-1;
      }
      if(strcmp(sets[n],comps[12]) == 0)
      {
        if(atoi(sets[n+1]) > 0)
        if(atoi(sets[n+1]) < 17)
        ramsettings[7] = atoi(sets[n+1])-1;
      }
      if(strcmp(sets[n],comps[13]) == 0)
      {
        if(atoi(sets[n+1]) > 0)
        if(atoi(sets[n+1]) < 65536)
        ramsettings[8] = atoi(sets[n+1])-1;
      }
      if(strcmp(sets[n],comps[14]) == 0)
      {
        if(atoi(sets[n+1]) > 0)
        if(atoi(sets[n+1]) < 9)
        ramsettings[9] = atoi(sets[n+1])-1;
      }
      if(strcmp(sets[n],comps[15]) == 0)
      {
        run = 1;
        runfile = sets[n+1];
      }
      if(strcmp(sets[n],comps[16]) == 0)
      {
        if(atoi(sets[n+1]) >= 33)
        if(atoi(sets[n+1]) <= 340)
        daemonsettings[0] = atoi(sets[n+1]);
      }
      if(strcmp(sets[n],comps[17]) == 0)
      {
        if(atoi(sets[n+1]) >= 33)
        if(atoi(sets[n+1]) <= 340)
        daemonsettings[1] = atoi(sets[n+1]);
      }
      if(strcmp(sets[n],comps[18]) == 0)
      {
        if(atoi(sets[n+1]) >= daemonsettings[1])
        if(atoi(sets[n+1]) <= 340)
        daemonsettings[2] = atoi(sets[n+1]);
      }
      if(strcmp(sets[n],comps[19]) == 0)
      {
        daemonsettings[3] = (int)(atof(sets[n+1])*10+0.5);
        if(daemonsettings[3] < 1)
            daemonsettings[3] = 5;
      }
      if(strcmp(sets[n],comps[20]) == 0)
      {
        daemonsettings[4] = parse_key_sequence(sets[n+1]);
      }
      if(strcmp(sets[n],comps[21]) == 0)
      {
        daemonsettings[5] = parse_key_sequence(sets[n+1]);
      }
      if(strcmp(sets[n],comps[22]) == 0)
      {
        daemonsettings[6] = parse_key_sequence(sets[n+1]);
      }
      if(strcmp(sets[n],comps[23]) == 0)
      {
        daemonsettings[7] = (sets[n+1][1] == 'n' ? 1 : 0);
      }
      if(strcmp(sets[n],comps[24]) == 0)
      {
        start_daemon = 1;
      }
    }
  }
  
  if(run == 1)
  {
    if (cpusettings[0] > -1) set_FCLK(cpusettings[0]);
    if (cpusettings[1] > -1) set_920_Div(cpusettings[1]);
    if (dispsettings[0] == 0)
    if (dispsettings[1] > -100) set_add_FLCDCLK(dispsettings[1]);
    if (dispsettings[0] == 1)
    if (dispsettings[1] > -100) set_add_ULCDCLK(dispsettings[1]);
    if (dispsettings[2] > -1) set_gamma((float)(dispsettings[2]/100));
    if (ramsettings[0] > -1) set_CAS(ramsettings[0]);
    if (ramsettings[1] > -1) set_tRC(ramsettings[1]);
    if (ramsettings[2] > -1) set_tRAS(ramsettings[2]);
    if (ramsettings[3] > -1) set_tWR(ramsettings[3]);
    if (ramsettings[4] > -1) set_tMRD(ramsettings[4]);
    if (ramsettings[5] > -1) set_tRFC(ramsettings[5]);
    if (ramsettings[6] > -1) set_tRP(ramsettings[6]);
    if (ramsettings[7] > -1) set_tRCD(ramsettings[7]);
    if (ramsettings[8] > -1) set_REFPERD(ramsettings[8]);
    if (ramsettings[9] > -1) set_DCLK_Div(ramsettings[9]);
    exit_cpu_speed(runfile);
    return 0;
  }

  sysfreq=get_freq_920_CLK();
  sysfreq*=get_920_Div()+1;
  cpufreq=sysfreq/1000000;
  
#if 0
  if(argc > 1)
  {
    if(strcmp(argv[1], "0") == 0) fallback(argc, argv);
    else if(strcmp(argv[1], "1") == 0) fallback(argc, argv);
    else if(strcmp(argv[1], "--help") == 0) cmdhelp();
    else if(strcmp(argv[1], "--daemon") == 0) cmd_daemon(argc, argv);
    else if(strcmp(argv[1], "--kill-daemon") == 0) kill_running_daemon() || printf("no daemon running\r\n");
    else if(strcmp(argv[1], "--kill") == 0) kill_running_daemon() || printf("no daemon running\r\n");
    else cmdline(argc, argv);
    gp2x_deinit();
    return 0;
  }
#endif
  
  if(start_daemon)
      start_daemon_by_settings();
  
  set_gamma(1.0);
  
  do
  { 
    cleardisp();
    
    //main menu
    v_putcad(27,26,WHITE,COLORFONDO,"Y:Save to SD");
    v_putcad(2,27,WHITE,COLORFONDO,"Select: Set setting and quit");
    itemhelp("menu");
    if(mainmenupoint==0) v_putcad(2,6,0xffffff,TEXTBACK,"> CPU Clock");
    else v_putcad(2,6,0xffff,COLORFONDO,"> CPU Clock");
    if(mainmenupoint==1) v_putcad(2,8,0xffffff,TEXTBACK,"> Display Settings");
    else v_putcad(2,8,0xffff,COLORFONDO,"> Display Settings");
    if(mainmenupoint==2) v_putcad(2,10,0xffffff,TEXTBACK,"> RAM Settings");
    else v_putcad(2,10,0xffff,COLORFONDO,"> RAM Settings");
    if(mainmenupoint==3) v_putcad(2,12,0xffffff,TEXTBACK,"> Daemon");
    else v_putcad(2,12,0xffff,COLORFONDO,"> Daemon");
    v_putcad(2,17,0xffff00,COLORFONDO,"Information:");
    sprintf(cad,"Sys.-Frq: %u Hz",sysfreq);
    v_putcad(2,19,0xffffff,COLORFONDO,cad);
    sprintf(cad,"UCLK-Frq:  %u Hz",get_freq_UCLK());
    v_putcad(2,20,0xffffff,COLORFONDO,cad);
    sprintf(cad,"ACLK-Frq: %u Hz",get_freq_ACLK());
    v_putcad(2,21,0xffffff,COLORFONDO,cad);
    gp2x_video_flip();

    while(1)
    {
      gp2x_nKeys=gp2x_joystick_read();

      if((gp2x_nKeys & GP2X_START))
      {
        while(1)
        {
          gp2x_nKeys=gp2x_joystick_read();
          if(!(gp2x_nKeys & GP2X_START)) break;
        }
        exit_cpu_speed("gp2xmenu");
        return 0;
      }

      if((gp2x_nKeys & GP2X_SELECT))
      {
        if (cpusettings[0] > -1)
        {
          set_FCLK(cpusettings[0]);
          set_920_Div(cpusettings[1]);
        }
        if (ramsettings[0] > -1)
        {
          set_CAS(ramsettings[0]);
          set_tRC(ramsettings[1]);
          set_tRAS(ramsettings[2]);
          set_tWR(ramsettings[3]);
          set_tMRD(ramsettings[4]);
          set_tRFC(ramsettings[5]);
          set_tRP(ramsettings[6]);
          set_tRCD(ramsettings[7]);
          set_REFPERD(ramsettings[8]);
          set_DCLK_Div(ramsettings[9]);
        }
        while(1)
        {
          gp2x_nKeys=gp2x_joystick_read();
          if(!(gp2x_nKeys & GP2X_START)) break;
        }
        exit_cpu_speed("gp2xmenu");
        return 0;
      }

      if((gp2x_nKeys & GP2X_DOWN))
      {
        mainmenupoint=mainmenupoint+1;
        if(mainmenupoint>3) mainmenupoint=0;  
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_UP))
      {
        mainmenupoint=mainmenupoint-1;
        if(mainmenupoint<0) mainmenupoint=3;  
        delay_us(200000);
        break;
      }

      if((gp2x_nKeys & GP2X_Y))
      {
        settings = fopen("./cpu_speed.cfg", "w");
        if(cpusettings[0] > -1)
        {
          fprintf(settings,"CPU-Clock\n%u\n", cpusettings[0]);
          fprintf(settings,"CPU-Div\n%i\n", cpusettings[1]+1);
        }
        if(dispsettings[0] > -1)
        {
          if(dispsettings[0] == 0) fprintf(settings,"FPLL\n%i\n", dispsettings[1]);
          if(dispsettings[0] == 1) fprintf(settings,"UPLL\n%i\n", dispsettings[1]);
        fprintf(settings,"Gamma\n%.2f\n", (float)(dispsettings[2]/100));
        }
        if(ramsettings[0] > -1)
        {
        fprintf(settings,"CAS\n%i\n", ramsettings[0]+2);
        fprintf(settings,"tRC\n%i\n", ramsettings[1]+1);
        fprintf(settings,"tRAS\n%i\n", ramsettings[2]+1);
        fprintf(settings,"tWR\n%i\n", ramsettings[3]+1);
        fprintf(settings,"tMRD\n%i\n", ramsettings[4]+1);
        fprintf(settings,"tRFC\n%i\n", ramsettings[5]+1);
        fprintf(settings,"tRP\n%i\n", ramsettings[6]+1);
        fprintf(settings,"tRCD\n%i\n", ramsettings[7]+1);
        fprintf(settings,"Refresh-Period\n%i\n", ramsettings[8]+1);
        fprintf(settings,"RAM-Div\n%i\n", ramsettings[9]+1);
        }
        if(daemonsettings[0] > -1)
        {
            fprintf(settings,"Daemon-Min\n%i\n", daemonsettings[0]);
            fprintf(settings,"Daemon-Max\n%i\n", daemonsettings[1]);
            fprintf(settings,"Daemon-Step\n%i\n", daemonsettings[2]);
            fprintf(settings,"Daemon-Delay\n%.1f\n", daemonsettings[3]/10.f);
        
            char test[256];
            sprintf(test,"");
            formatkey(test,daemonsettings[4]);
            fprintf(settings,"Daemon-Hotkey\n%s\n",test);
            sprintf(test,"");
            formatkey(test,daemonsettings[5]);
            fprintf(settings,"Daemon-Incr-Key\n%s\n",test);
            sprintf(test,"");
            formatkey(test,daemonsettings[6]);
            fprintf(settings,"Daemon-Decr-Key\n%s\n",test);
        
            fprintf(settings,"Daemon-OSD\n%s\n",(daemonsettings[7]?"On":"Off"));
            if(!access("/tmp/cpu_daemon.pid",R_OK))
                fprintf(settings,"Daemon-Run\nOn\n");
        
        }
        
        
        fclose(settings);
        system("sync");
        while(1)
        {
          gp2x_nKeys=gp2x_joystick_read();
          if(!(gp2x_nKeys & GP2X_Y)) break;
        } 
        break;
      }


      if((gp2x_nKeys & GP2X_B))
      {
        if(mainmenupoint==0) cpumenu();
        if(mainmenupoint==1) lcdmenu();
        if(mainmenupoint==2) rammenu();
        if(mainmenupoint==3) daemonmenu();
        while(1)
        {
          gp2x_nKeys=gp2x_joystick_read();
          if(!(gp2x_nKeys & GP2X_B)) break;
        } 
        break;
      }
    }
  }
  while(cpuspeed_exit == 0);  
}
