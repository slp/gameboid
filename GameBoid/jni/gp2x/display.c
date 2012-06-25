/*  display.c for GP2X (CPU/LCD/RAM-Tuner Version 2.0)
    Copyright (C) 2006 god_at_hell 
    original CPU-Overclocker (c) by Hermes/PS2Reality 
	parts (c) Rlyehs Work

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
#include <sys/mman.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>

#include "gp2xminilib.h"
#include "cpuctrl.h"
#define WIDTH	320
#define HEIGHT	240

//unsigned TEXTBACK=0x900000; // text-background-color

extern unsigned char msx[];  // define la fuente externa usada para dibujar letras y numeros 

void ClearScreen(unsigned val) // se usa para 'borrar' la pantalla virtual con un color
{
	int n;
	unsigned char *c;
	unsigned short col;
	c=&val;
	col=gp2x_video_color15(c[0],c[1],c[2],0);
	for(n=0;n<320*240;n++)
	{
		gp2x_screen15[n]=col;
	}
}

void DrawBox(unsigned val)
{
	int n;
	unsigned char *c;
	unsigned short col;
	c=&val;
	col=gp2x_video_color15(c[0],c[1],c[2],0);
	
	for(n=320*27+2;n<320*28-1;n++)
	{
		gp2x_screen15[n]=col;
		gp2x_screen15[n+320*209]=col;
	}
	
	for(n=320*29+4;n<320*30-3;n++)
	{
		gp2x_screen15[n]=col;
		gp2x_screen15[n+320*169]=col;
		gp2x_screen15[n+320*205]=col;
	}
	
	for(n=320*28;n<320*237;n=n+320)
	{
		gp2x_screen15[n+2]=col;
		gp2x_screen15[n-2]=col;
	}
	
	for(n=320*30;n<320*235;n=n+320)
	{
		gp2x_screen15[n+4]=col;
		gp2x_screen15[n-4]=col;
	}
	
	for(n=320*30;n<320*199;n=n+320)
	{
		gp2x_screen15[n-120]=col;
	}
	
	for(n=320*55-120;n<320*55-4;n++)
	{
		gp2x_screen15[n]=col;
	}
	
}

void v_putchar( unsigned x, unsigned y, unsigned color, unsigned textback, unsigned char ch) // rutina usada para dibujar caracteres (coordenadas de 8x8)
{
	int i,j,v;
	unsigned char	*font;
	unsigned char *c;
	unsigned short col,col2;
	if(x>=WIDTH || y>=HEIGHT) return;
	c=&color;
	col=gp2x_video_color15(c[0],c[1],c[2],0);
	c=&textback;
	col2=gp2x_video_color15(c[0],c[1],c[2],0);
	v=(y*320*8);
	font = &msx[ (int)ch * 8];
	for (i=0; i < 8; i++, font++)
	{  
		for (j=0; j < 8; j++)
		{
			if ((*font & (128 >> j)))
			{ 
				gp2x_screen15[v+(((x<<3)+j))]=col;
			}
			else gp2x_screen15[v+(((x<<3)+j))]=col2;
		}
		v+=WIDTH;
	}
}

// display array of chars

void v_putcad(int x,int y,unsigned color,unsigned textback,char *cad)  // dibuja una cadena de texto
{	
	while(cad[0]!=0) {v_putchar(x,y,color,textback,cad[0]);cad++;x++;}
}


void gp2x_sound_frame(void *unused, unsigned char *stream, int samples)
{
	int n;
	short *pu;
	pu=stream;
	for(n=0;n<(samples);n++)
	{
		*pu++=0;*pu++=0;
	}
}
