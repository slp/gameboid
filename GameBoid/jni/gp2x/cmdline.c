
/*  commandline.c for GP2X Version 2.0
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
#include "cpuctrl.h"
#include "gp2xminilib.h"
#include "cpuctrl.h"


void fallback(int argc, char *argv[])
{
	//beginning commandline-utilizing
	if (argc == 3)
	{
		if (atoi(argv[1]) == 0)
		{
			if(atoi(argv[2]) > 36){gp2x_deinit();exit(1);}
			if(atoi(argv[2]) < -20){gp2x_deinit();exit(1);}
			set_add_FLCDCLK(atoi(argv[2]));
		}
		else
		{
			if(atoi(argv[2]) > 10){gp2x_deinit();exit(1);}
			if(atoi(argv[2]) < -5){gp2x_deinit();exit(1);}
			set_add_ULCDCLK(atoi(argv[2]));
		}
	}

	if (argc == 4)
	{
		if(atoi(argv[3]) > 320){gp2x_deinit();exit(1);}
		if(atoi(argv[3]) < 33){gp2x_deinit();exit(1);}
		if (atoi(argv[1]) == 0)
		{
			if(atoi(argv[2]) > 36){gp2x_deinit();exit(1);}
			if(atoi(argv[2]) < -20){gp2x_deinit();exit(1);}
			set_add_FLCDCLK(atoi(argv[2]));
		}
		else
		{
			if(atoi(argv[2]) > 10){gp2x_deinit();exit(1);}
			if(atoi(argv[2]) < -5){gp2x_deinit();exit(1);}
			set_add_ULCDCLK(atoi(argv[2]));
		}
		set_FCLK(atoi(argv[3]));
	}
	
	if (argc == 5)
	{
		if(atoi(argv[3]) > 320){gp2x_deinit();exit(1);}
		if(atoi(argv[3]) < 33){gp2x_deinit();exit(1);}
		if(atof(argv[4]) > 10){gp2x_deinit();exit(1);}
		if(atof(argv[4]) < 0.02){gp2x_deinit();exit(1);}
		if (atoi(argv[1]) == 0)
		{
			if(atoi(argv[2]) > 36){gp2x_deinit();exit(1);}
			if(atoi(argv[2]) < -20){gp2x_deinit();exit(1);}
			set_add_FLCDCLK(atoi(argv[2]));
		}
		else
		{
			if(atoi(argv[2]) > 10){gp2x_deinit();exit(1);}
			if(atoi(argv[2]) < -5){gp2x_deinit();exit(1);}
			set_add_ULCDCLK(atoi(argv[2]));
		}
		set_FCLK(atoi(argv[3]));
		set_gamma(atof(argv[4]));
	}
}

void cmdhelp()
{
	printf ("\ngpSP2X v0.9003 Beta by Exophase/ZodTTD\r\n");
	printf ("cpu_speed by god_at_hell\r\n");
	printf ("Usage: cpu_speed.gpe [option1] [value1] [option2]...\r\n");
	printf ("Options:\r\n");
	printf (" RAM-Options\r\n");
	printf (" -----------\r\n");
	printf (" --cas 		CAS Latency. Delay in clock cycles between the registration \n\t\tof a READ command and the first bit of output data. \n\t\tValid values are 2 and 3 cycles.\r\n");
	printf (" --trc 		ACTIVE to ACTIVE /AUTOREFRESH command delay. Defines ACTIVE \n\t\tto ACTIVE/auto refresh command period delay. \n\t\tValid values are from 1 to 16 cycles.\r\n");
	printf (" --tras 	ACTIVE to PRECHARGE delay. Defines the delay between the ACTIVE \n\t\tand PRECHARGE commands. \n\t\tValid values are from 1 to 16 cycles.\r\n");
	printf (" --twr 		Write recovery time in cycles.\n\t\tValid values are from 1 to 16 cycles.\r\n");
	printf (" --tmrd 	LOAD MODE REGISTER command cycle time.\n\t\tValid values are from 1 to 16 cycles.\r\n");
	printf (" --trfc 	AUTO REFRESH command period in cycles.\n\t\tValid values are from 1 to 16 cycles.\r\n");
	printf (" --trp 		PRECHARGE command period in cycles.\n\t\tValid values are from 1 to 16 cycles.\r\n");
	printf (" --trcd 	RAS to CAS Delay in cycles.\n\t\tValid values are from 1 to 16 cycles.\r\n");
	printf (" --refperd 	Refresh Period. Defines maximum time period between \n\t\tAUTOREFRESH commands.\n\t\tValid values are from 1 to 65535 (default ~ 250) cycles.\r\n");
	printf (" --ramdiv 	Divider for the Memory-Clock which is 1/2 of the CPU-Clock. \n\t\tValid values are from 1 to 8.\r\n");
	printf ("\n CPU-Options\r\n");
	printf (" -----------\r\n");
	printf (" --cpuclk	Sets the CPU-Frequency in Mhz. \n\t\tValid values are from 33 to 340.\r\n");
	printf (" --cpudiv 	Divider for the CPU-Clock. \n\t\tValid values are from 1 to 8.\r\n");
	printf ("\n Display-Options\r\n");
	printf ("----------------\r\n");
	printf (" --fpll		Sets clockgenerator to fpll (for firmware 1.0 - 1.0.1).\r\n");
	printf (" --upll		Sets clockgenerator to upll (for the rest).\r\n");
	printf (" --timing	Timing Prescaler to eliminate flickering. \n\t\tValid values are: -20 to 36 with fpll.\n\t\t\t\t  -6 to 10 with upll.\r\n");
	printf (" --gamma	Regulates the gamma. \n\t\tValid values are from 0.0001 to 15.0000.\r\n");
	printf ("\n Daemon-Mode \r\n");
	printf ("----------------\r\n");
	printf ("Usage: cpu_speed.gpe --daemon [option1] [value1] [option2]...\r\n");
	printf ("Shutdown: cpu_speed.gpe --kill[-daemon]\r\n");
	printf ("Options:\r\n");
	printf (" --min	Sets the minimum CPU-Frequency in Mhz. \n\t\tValid values are from 33 to 340.\r\n");
	printf (" --max	Sets the maximum CPU-Frequency in Mhz. \n\t\tValid values are from 33 to 340.\r\n");
	printf (" --start	Sets the CPU-Frequency in Mhz. \n\t\tValid values are from 33 to 340.\r\n");
	printf (" --step	Sets the CPU-Frequency step in Mhz. \n\t\tValid values are from 1 to 340.\r\n");
	printf (" --hotkey	Sets the hotkey. (Default: LR) \n\t\tValid values are a combination of LRXYZAB+-S/@ or ``None''\n\t\t (+- are volume, S is Start, / is Select, @ is Stick).\r\n");
	printf (" --incr	Sets the increment key. (Default: +) \n\t\tValid values are a combination of LRXYZAB+-S/@ or ``None''\n\t\t (+- are volume, S is Start, / is Select, @ is Stick).\r\n");
	printf (" --decr	Sets the decrement key. (Default: -) \n\t\tValid values are a combination of LRXYZAB+-S/@ or ``None''\n\t\t (+- are volume, S is Start, / is Select, @ is Stick).\r\n");
	printf (" --no-hotkey	Alias for --hotkey None.\r\n");
	printf (" --no-incr	Alias for --incr None.\r\n");
	printf (" --no-decr	Alias for --decr None.\r\n");
	printf (" --foreground	Do not switch to daemon mode. (Useful for debugging)\r\n");
	printf (" --background	Switch to daemon mode. (Default)\r\n");
	printf (" --display	Enable on screen display. COMING SOON!\r\n");
	printf (" --no-display	Disable on screen display.\r\n");
	printf ("\nNOTE:\nThe old commandline-settings are working ... read more about this in the readme\n\n");
}

void cmdline(int argc, char *argv[])
{
	short i,n;
	short varis = 11;
	char clockgen = get_Clkgen();
	char var[11][9]={"--cas","--trc","--tras","--twr","--tmrd","--trfc","--trp","--trcd","--ramdiv","--cpuclk","--cpudiv"};

	short val[varis];
	for(n=0;n<varis;n++) //initialize the variable-array
	{
		val[n] = -1;
	}

	short timing = -100;
	int refperd = -1;
	float gamma = -1.;

	
	for(i=1; i<argc; i++)
	{
		if(strcmp(argv[i], "--fpll") == 0) clockgen = 0;
		if(strcmp(argv[i], "--upll") == 0) clockgen = 1;
		
		for(n=0; n<varis; n++)
		{
			if(strcmp(argv[i], var[n]) == 0)
			{
				if(i+1 == argc){printf ("%s is missing it's parameter\r\n",var[n]);gp2x_deinit();exit(1);}
				val[n] = atoi(argv[i+1]);
			}
		}
		if(strcmp(argv[i], "--refperd") == 0)
		{
			if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
			refperd = atoi(argv[i+1]);
		} 
		if(strcmp(argv[i], "--gamma") == 0)
		{
			if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
			gamma = atof(argv[i+1]);
		}
		if(strcmp(argv[i], "--timing") == 0)
		{
			if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
			timing = atoi(argv[i+1]);
		}
	}
	
	if(clockgen == 0)
	{
		if(timing > -21)
		{
			if(timing < 37) set_add_FLCDCLK(timing);
		}
		else set_add_FLCDCLK(get_LCDClk(clockgen));
	}
	if(clockgen == 1)
	{
		if(timing > -7)
		{
			if(timing < 11) set_add_ULCDCLK(timing);
		}
		else set_add_ULCDCLK(get_LCDClk(clockgen));
	}
	if(refperd-1 > -1)
	{
		if(refperd-1 < 0xffff) set_REFPERD(refperd-1);
	}
	if(gamma > 0.)
	{
		if(gamma < 15.) set_gamma(gamma);
	}
	if(val[0]-2 > -1)
	{
		if(val[0]-2 < 2) set_CAS(val[0]-2);
	}
	if(val[1]-1 > -1)
	{
		if(val[1]-1 < 16) set_tRC(val[1]-1);
	}
	if(val[2]-1 > -1)
	{
		if(val[2]-1 < 16) set_tRAS(val[2]-1);
	}
	if(val[3]-1 > -1)
	{
		if(val[3]-1 < 16) set_tWR(val[3]-1);
	}
	if(val[4]-1 > -1)
	{
		if(val[4]-1 < 16) set_tMRD(val[4]-1);
	}
	if(val[5]-1 > -1)
	{
		if(val[5]-1 < 16) set_tRFC(val[5]-1);
	}
	if(val[6]-1 > -1)
	{
		if(val[6] < 16) set_tRP(val[6]-1);
	}
	if(val[7]-1 > -1)
	{
		if(val[7]-1 < 16) set_tRCD(val[7]-1);
	}
	if(val[8]-1 > -1)
	{
		if(val[8]-1 < 8) set_DCLK_Div(val[8]-1);
	}
	if(val[9] > 32)
	{
		if(val[9] < 341) set_FCLK(val[9]);
	}
	if(val[10]-1 > -1)
	{
		if(val[10]-1 < 8) set_920_Div(val[10]-1);
	}
}
