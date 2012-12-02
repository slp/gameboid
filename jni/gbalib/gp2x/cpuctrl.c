/*  cpuctrl.c for GP2X (CPU/LCD/RAM-Tuner Version 2.0)
    Copyright (C) 2006 god_at_hell 
    original CPU-Overclocker (c) by Hermes/PS2Reality 
	the gamma-routine was provided by theoddbot
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


/****************************************************************************************************************************************/
// CPU CONTROL
/****************************************************************************************************************************************/

#include <sys/mman.h>
#include <math.h>
#include <stdio.h>
#include "gp2xminilib.h"

#define SYS_CLK_FREQ 7372800

//from minimal library rlyeh

extern unsigned long   gp2x_dev[4];
extern unsigned short *gp2x_memregs;

// system registers
static struct 
{
	unsigned short SYSCLKENREG,SYSCSETREG,FPLLVSETREG,DUALINT920,DUALINT940,DUALCTRL940,DISPCSETREG,MEMTIMEX0;
	unsigned short MEMTIMEX1,MEMREFX,MLC_GAMM_BYPATH,MLC_GAMMA_A,MLC_GAMMA_D,YBNKLVL;
}
system_reg;

volatile unsigned short *MEM_REG;
unsigned MDIV,PDIV,SCALE;
volatile unsigned *arm940code;

void cpuctrl_init()
{
	MEM_REG=&gp2x_memregs[0];
}

void save_system_regs()
{
	system_reg.SYSCSETREG=MEM_REG[0x91c>>1];
	system_reg.FPLLVSETREG=MEM_REG[0x912>>1];
	system_reg.SYSCLKENREG=MEM_REG[0x904>>1];
	system_reg.DUALINT920=MEM_REG[0x3B40>>1];	
	system_reg.DUALINT940=MEM_REG[0x3B42>>1];
	system_reg.DUALCTRL940=MEM_REG[0x3B48>>1];
	system_reg.DISPCSETREG=MEM_REG[0x924>>1];
	system_reg.MEMTIMEX0=MEM_REG[0x3802>>1];
	system_reg.MEMTIMEX1=MEM_REG[0x3804>>1];
	system_reg.MEMREFX=MEM_REG[0x3808>>1];
	system_reg.MLC_GAMM_BYPATH=MEM_REG[0x2880>>1];
	system_reg.MLC_GAMMA_A=MEM_REG[0x295C>>1];
	system_reg.MLC_GAMMA_D=MEM_REG[0x295E>>1];
	system_reg.YBNKLVL=MEM_REG[0x283A>>1];
}

void load_system_regs()
{
	MEM_REG[0x91c>>1]=system_reg.SYSCSETREG;
	MEM_REG[0x910>>1]=system_reg.FPLLVSETREG;
	MEM_REG[0x3B40>>1]=system_reg.DUALINT920;
	MEM_REG[0x3B42>>1]=system_reg.DUALINT940;
	MEM_REG[0x3B48>>1]=system_reg.DUALCTRL940;
	MEM_REG[0x904>>1]=system_reg.SYSCLKENREG;
	/* Set UPLLSETVREG to 0x4F02, which gives 80MHz */
	MEM_REG[0x0914>>1] = 0x4F02;
	/* Wait for clock change to start */
	while (MEM_REG[0x0902>>1] & 2);
	/* Wait for clock change to be verified */
	while (MEM_REG[0x0916>>1] != 0x4F02);
	MEM_REG[0x3802>>1]=system_reg.MEMTIMEX0;
	MEM_REG[0x3804>>1]=system_reg.MEMTIMEX1;
	MEM_REG[0x3808>>1]=system_reg.MEMREFX;
	MEM_REG[0x2880>>1]=system_reg.MLC_GAMM_BYPATH;
	MEM_REG[0x295C>>1]=system_reg.MLC_GAMMA_A;
	MEM_REG[0x295E>>1]=system_reg.MLC_GAMMA_D;
	MEM_REG[0x283A>>1]=system_reg.YBNKLVL;
}


void set_FCLK(unsigned MHZ)
{
	printf ("set CPU-Frequency = %uMHz\r\n",MHZ);
	unsigned v;
	unsigned mdiv,pdiv=3,scale=0;
	MHZ*=1000000;
	mdiv=(MHZ*pdiv)/SYS_CLK_FREQ;
	//printf ("Old value = %04X\r",MEM_REG[0x924>>1]," ");
	//printf ("APLL = %04X\r",MEM_REG[0x91A>>1]," ");
	mdiv=((mdiv-8)<<8) & 0xff00;
	pdiv=((pdiv-2)<<2) & 0xfc;
	scale&=3;
	v=mdiv | pdiv | scale;
	MEM_REG[0x910>>1]=v;
}

unsigned get_FCLK()
{
	return MEM_REG[0x910>>1];
}

void set_add_FLCDCLK(int addclock)
{
	//Set LCD controller to use FPLL
	printf ("...set to FPLL-Clockgen...\r\n");
	printf ("set Timing-Prescaler = %i\r\n",addclock);
	MEM_REG[0x924>>1]= 0x5A00 + ((addclock)<<8); 
	//If you change the initial timing, don't forget to shift your intervall-borders in "cpu_speed.c"
}

void set_add_ULCDCLK(int addclock)
{
	//Set LCD controller to use UPLL
	printf ("...set to UPLL-Clockgen...\r\n");
	printf ("set Timing-Prescaler = %i\r\n",addclock);
	MEM_REG[0x0924>>1] = 0x8900 + ((addclock)<<8);
	//If you change the initial timing, don't forget to shift your intervall-borders in "cpu_speed.c"
}

unsigned get_LCDClk()
{
	if (MEM_REG[0x0924>>1] < 0x7A01) return((MEM_REG[0x0924>>1] - 0x5A00)>>8);
	else return((MEM_REG[0x0924>>1] - 0x8900)>>8);
}

char get_Clkgen()
{
	if (MEM_REG[0x0924>>1] < 0x7A01) return(0);
	else return(1);
}

unsigned get_freq_UCLK()
{
	unsigned i;
	unsigned reg,mdiv,pdiv,scale;
	i = MEM_REG[0x900>>1];
	i = ((i >> 7) & 1) ;
	if(i) return 0;
	reg=MEM_REG[0x916>>1];
	mdiv = ((reg & 0xff00) >> 8) + 8;
	pdiv = ((reg & 0xfc) >> 2) + 2;
	scale = reg & 3;	
	return ((SYS_CLK_FREQ * mdiv) / (pdiv << scale));
}

unsigned get_freq_ACLK()
{
	unsigned i;
	unsigned reg,mdiv,pdiv,scale;
	i = MEM_REG[0x900>>1];
	i = ((i >> 8) & 1) ;
	if(i) return 0;
	reg=MEM_REG[0x918>>1];
	mdiv = ((reg & 0xff00) >> 8) + 8;
	pdiv = ((reg & 0xfc) >> 2) + 2;
	scale = reg & 3;
	return ((SYS_CLK_FREQ * mdiv)/(pdiv << scale));
}

unsigned get_freq_920_CLK()
{
	unsigned i;
	unsigned reg,mdiv,pdiv,scale;
	reg=MEM_REG[0x912>>1];
	mdiv = ((reg & 0xff00) >> 8) + 8;
	pdiv = ((reg & 0xfc) >> 2) + 2;
	scale = reg & 3;
	MDIV=mdiv;
	PDIV=pdiv;
	SCALE=scale;
	i = (MEM_REG[0x91c>>1] & 7)+1;
	return ((SYS_CLK_FREQ * mdiv)/(pdiv << scale))/i;
}

unsigned get_freq_940_CLK()
{
	unsigned i;
	unsigned reg,mdiv,pdiv,scale;
	reg=MEM_REG[0x912>>1];
	mdiv = ((reg & 0xff00) >> 8) + 8;
	pdiv = ((reg & 0xfc) >> 2) + 2;
	scale = reg & 3;
	i = ((MEM_REG[0x91c>>1]>>3) & 7)+1;
	return ((SYS_CLK_FREQ * mdiv) / (pdiv << scale))/i;
}

unsigned get_freq_DCLK()
{
	unsigned i;
	unsigned reg,mdiv,pdiv,scale;
	reg=MEM_REG[0x912>>1];
	mdiv = ((reg & 0xff00) >> 8) + 8;
	pdiv = ((reg & 0xfc) >> 2) + 2;
	scale = reg & 3;
	i = ((MEM_REG[0x91c>>1]>>6) & 7)+1;
	return ((SYS_CLK_FREQ * mdiv) / (pdiv << scale))/i;
}

void set_920_Div(unsigned short div)
{
	printf ("set divider for CPU-Clock = %u\r\n",div+1);
	unsigned short v;
	v = MEM_REG[0x91c>>1] & (~0x3);
	MEM_REG[0x91c>>1] = (div & 0x7) | v; 
}

unsigned short get_920_Div()
{
	return (MEM_REG[0x91c>>1] & 0x7); 
}

void set_940_Div(unsigned short div)
{	
	unsigned short v;
	v = (unsigned short)( MEM_REG[0x91c>>1] & (~(0x7 << 3)));
	MEM_REG[0x91c>>1] = ((div & 0x7) << 3) | v; 
}

unsigned short get_940_Div()
{
	return ((MEM_REG[0x91c>>1] >> 3) & 0x7); 
}

void set_DCLK_Div( unsigned short div )
{
	printf ("set divider for RAM-Clock = %u\r\n",div+1);
	unsigned short v;
	v = (unsigned short)( MEM_REG[0x91c>>1] & (~(0x7 << 6)));
	MEM_REG[0x91c>>1] = ((div & 0x7) << 6) | v; 
}

unsigned short get_DCLK_Div()
{
	return ((MEM_REG[0x91c>>1] >> 6) & 0x7); 
}

unsigned short Disable_Int_920()
{
	unsigned short ret;
	ret=MEM_REG[0x3B40>>1];
	MEM_REG[0x3B40>>1]=0;
	MEM_REG[0x3B44>>1]=0xffff;	
	return ret;	
}

unsigned short Disable_Int_940()
{
	unsigned short ret;
	ret=MEM_REG[0x3B42>>1];
	MEM_REG[0x3B42>>1]=0;
	MEM_REG[0x3B46>>1]=0xffff;	
	return ret;	
}

unsigned get_state940()
{
	return MEM_REG[0x904>>1];	
}


void Enable_Int_920(unsigned short flag)
{
	MEM_REG[0x3B40>>1]=flag;
}

void Enable_Int_940(unsigned short flag)
{
	MEM_REG[0x3B42>>1]=flag;
}

void Disable_940()
{
	Disable_Int_940();
	MEM_REG[0x3B48>>1]|= (1 << 7);
	MEM_REG[0x904>>1]&=0xfffe;
}

void Load_940_code(unsigned *code,int size)
{
	unsigned *cp;
	int i;
	arm940code=(unsigned short *)mmap(0, 0x100000, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev[2], 0x03000000);
	Disable_940();
	cp=(unsigned *) code;
	for (i = 0; i < size/4; i ++)
	{
		arm940code[i] = cp[i];
	}
	for (i = 0; i < 64; i ++)
	{
		arm940code[0x3FC0+i] = 0;
	}
	MEM_REG[0x3B48>>1]=(MEM_REG[0x3B48>>1]  & 0xFF00) | 0x03; // allow 940
}

void clock_940_off()
{
	MEM_REG[0x904>>1]&=0xfffe;
}

void clock_940_on()
{
	MEM_REG[0x904>>1]|=1;
}


//--------------
//Memory Timings
//--------------

//get

unsigned get_CAS()
{
	return ((MEM_REG[0x3804>>1] >> 12) & 0x1);	
}

unsigned get_tRC()
{
	return ((MEM_REG[0x3804>>1] >> 8) & 0xF);	
}

unsigned get_tRAS()
{
	return ((MEM_REG[0x3804>>1] >> 4) & 0xF);	
}

unsigned get_tWR()
{
	return (MEM_REG[0x3804>>1] & 0xF);	
}

unsigned get_tMRD()
{
	return ((MEM_REG[0x3802>>1] >> 12) & 0xF);	
}

unsigned get_tRFC()
{
	return ((MEM_REG[0x3802>>1] >> 8) & 0xF);	
}

unsigned get_tRP()
{
	return ((MEM_REG[0x3802>>1] >> 4) & 0xF);	
}

unsigned get_tRCD()
{
	return (MEM_REG[0x3802>>1] & 0xF);	
}

unsigned get_REFPERD()
{
	return MEM_REG[0x3808>>1];	
}


//set

void set_CAS(unsigned short timing)
{
	printf ("set CAS = %u\r\n",timing+2);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3804>>1] & (~(0x1 << 12)));
	MEM_REG[0x3804>>1] = ((timing & 0x1) << 12) | v;	
}

void set_tRC(unsigned short timing)
{
	printf ("set tRC = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3804>>1] & (~(0xF << 8)));
	MEM_REG[0x3804>>1] = ((timing & 0xF) << 8) | v;	
}

void set_tRAS(unsigned short timing)
{
	printf ("set tRAS = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3804>>1] & (~(0xF << 4)));
	MEM_REG[0x3804>>1] = ((timing & 0xF) << 4) | v;	
}

void set_tWR(unsigned short timing)
{	
	printf ("set tWR = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3804>>1] & (~(0xF)));
	MEM_REG[0x3804>>1] = (timing & 0xF) | v;	
}

void set_tMRD(unsigned short timing)
{
	printf ("set tMRD = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3802>>1] & (~(0xF << 12)));
	MEM_REG[0x3802>>1] = ((timing & 0xF) << 12) | v;	
}

void set_tRFC(unsigned short timing)
{
	printf ("set tRFC = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3802>>1] & (~(0xF << 8)));
	MEM_REG[0x3802>>1] = ((timing & 0xF) << 8) | v;	
}

void set_tRP(unsigned short timing)
{
	printf ("set tRP = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3802>>1] & (~(0xF << 4)));
	MEM_REG[0x3802>>1] = ((timing & 0xF) << 4) | v;	
}

void set_tRCD(unsigned short timing)
{
	printf ("set tRCD = %u\r\n",timing+1);
	unsigned short v;
	v = (unsigned short)(MEM_REG[0x3802>>1] & (~(0xF)));
	MEM_REG[0x3802>>1] = (timing & 0xF) | v;	
}

void set_REFPERD(unsigned short timing)
{
	printf ("set Refresh Period = %u\r\n",timing+1);
	MEM_REG[0x3808>>1] = timing;	
}


//-----
//Gamma
//-----

void set_gamma(float gamma)
{
	printf ("set gamma = %f\r\n",gamma);
	int i;
	gamma = 1/gamma;

    //enable gamma
    MEM_REG[0x2880>>1]&=~(1<<12);

    MEM_REG[0x295C>>1]=0;
    for(i=0; i<256; i++)
    {
		unsigned char g;
        unsigned short s;
        g =(unsigned char)(255.0*pow(i/255.0,gamma));
        s = (g<<8) | g;
		MEM_REG[0x295E>>1]= s;
        MEM_REG[0x295E>>1]= g;
    }
}

unsigned get_YBNKLVL()
{
	return (MEM_REG[0x283A>>1] & 0x3FF);	
}

void set_YBNKLVL(unsigned short val)
{
	unsigned short temp = (unsigned short)(MEM_REG[0x3808>>1] & (~(0x3FF)));
	MEM_REG[0x3808>>1] = (val & 0x3FF) | temp;	
}
