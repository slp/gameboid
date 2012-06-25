/*  speedtest.c for GP2X (CPU/LCD/RAM-Tuner Version 2.0)
    Copyright (C) 2006 god_at_hell 

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
#include <fcntl.h>
#include <math.h>                                             
#include <unistd.h>

#include "display.h"
#include "cpuctrl.h"
#include "gp2xminilib.h"

void prim()
{
	//FILE *primout;
	unsigned char cad[256];
    char p = 0;
    int primnumber;
    int l = 1;
    float g = 0;
    int i = 3;
    
    while(i != 500000)
    {
			int m = 2;
            float temp = sqrt(i);
            float ifloat = (float)i;
            
            if(temp == (int)temp)
            {
                      m = i;
                      p = 1;
            }
            
            while(m < temp)
            {
                    g = ifloat/m;
                    if (g == (int)g)
                    {
                          m = i;
                          p = 1;
                    }
                   	m++;
            }

            if (p == 0)
            {
                  l += 1;
                  primnumber = i;
                  sprintf(cad,"%u is primnumber",i);
				  v_putcad(1,13,0xffffff,0xB00000,cad);
				  //primout = fopen("/mnt/sd/primnumber.txt", "a");
				  //fprintf(primout,"%u: %u\n", l, i);
				  //fclose(primout);
				  //execl("sync",NULL);
				  gp2x_video_flip();
            }
            p = 0;
            i++;
    }               
}

void ant()
{
	int i,n;
	unsigned char cad[256];
	short direction = 0; //clockwise ... 0 = Up, 1 = Right
	unsigned short col1, col2;
	col1=gp2x_video_color15(0,0,0,0);
	col2=gp2x_video_color15(0xFF,0xFF,0xFF,0);
	short antx = 200;
	short anty = 140;
	for(i = 0; i < 9000; i++)
	{
		for(n = 0; n < 500000; n++);
		if(gp2x_screen15[(320*anty) + antx]==col1)
		{	
			for(n = 0; n < 3; n++)
			{
				int m = 0;
				for(m = 0; m < 3; m++)
				{
					gp2x_screen15[320*(anty+n)+antx+m] = col2;
				}
			}
			sprintf(cad,"%u steps left ",8999-i);
			v_putcad(1,3,0x000000,0xffffff,cad);
			gp2x_video_flip();
			for(n = 0; n < 3; n++)
			{
				int m = 0;
				for(m = 0; m < 3; m++)
				{
					gp2x_screen15[320*(anty+n)+antx+m] = col2;
				}
			}
			sprintf(cad,"%u steps left ",8999-i);
			v_putcad(1,3,0x000000,0xffffff,cad);
			gp2x_video_flip();
			if(direction == 0) antx-=3;
			if(direction == 1) anty-=3;
			if(direction == 2) antx+=3;
			if(direction == 3) anty+=3;
			direction--;
			if(direction < 0) direction=3;
		}
		if(gp2x_screen15[(320*anty) + antx]==col2)
		{
			for(n = 0; n < 3; n++)
			{
				int m = 0;
				for(m = 0; m < 3; m++)
				{
					gp2x_screen15[320*(anty+n)+antx+m] = col1;
				}
			}
			sprintf(cad,"%u steps left ",8999-i);
			v_putcad(1,3,0x000000,0xffffff,cad);
			gp2x_video_flip();
			for(n = 0; n < 3; n++)
			{
				int m = 0;
				for(m = 0; m < 3; m++)
				{
					gp2x_screen15[320*(anty+n)+antx+m] = col1;
				}
			}
			sprintf(cad,"%u steps left ",8999-i);
			v_putcad(1,3,0x000000,0xffffff,cad);
			gp2x_video_flip();
			if(direction == 0) antx+=3;
			if(direction == 1) anty+=3;
			if(direction == 2) antx-=3;
			if(direction == 3) anty-=3;
			direction++;
			if(direction > 3) direction=0;
		}
	}
}

void speedtest(short test)
{
	unsigned BACKGROUND;
	if(test == 0) BACKGROUND=0xB00000;
	if(test == 1) BACKGROUND=0xFFFFFF;
	short start = 240;
	short cpuspeed = start;
	unsigned char cad[256];
	FILE *speed;
	
	do
	{	
		speed = fopen("/mnt/sd/speed.txt", "w");
		ClearScreen(BACKGROUND);
		if(test == 0) v_putcad(1,1,0x00ff00,BACKGROUND,"Prim-Speedtest");
		if(test == 1) v_putcad(1,1,0x006600,BACKGROUND,"Ant-Speedtest");
		v_putcad(1,6,0xffffff,BACKGROUND,"Testing Speed");
		if(cpuspeed > start) 
		{
			sprintf(cad,"%uMhz checked",cpuspeed-5);
			v_putcad(1,9,0xffffff,BACKGROUND,cad);
		}
		gp2x_video_flip();
		ClearScreen(BACKGROUND);
		if(test == 0) v_putcad(1,1,0x00ff00,BACKGROUND,"Prim-Speedtest");
		if(test == 1) v_putcad(1,1,0x006600,BACKGROUND,"Ant-Speedtest");
		v_putcad(1,6,0xffffff,BACKGROUND,"Testing Speed");
		if(cpuspeed > start) 
		{
			sprintf(cad,"%uMhz checked",cpuspeed-5);
			if(test == 0) v_putcad(1,9,0xffffff,BACKGROUND,cad);
			if(test == 1) v_putcad(1,9,0x000000,BACKGROUND,cad);
		}
		gp2x_video_flip();
		fprintf (speed,"set CPU-Frequency = %uMHz\r\n",cpuspeed);
		set_FCLK(cpuspeed);
		
		if(test == 0) prim();
		if(test == 1) ant();
		
		fprintf(speed,"%uMhz checked\n\n", cpuspeed);
		cpuspeed = cpuspeed + 5;
		fclose(speed);
		execl("sync",NULL);
	}
	while(1);	
}
