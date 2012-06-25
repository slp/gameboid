/*  daemon.c for GP2X Version 2.0
    Copyright (C) 2006 jannis harder 

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


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "gp2xminilib.h"
#include "cpuctrl.h"
#include "display.h"

#include "daemon.h"


extern unsigned COLORFONDO; // background-color
extern unsigned WHITE;
extern unsigned TEXTBACK;

extern unsigned char cad[256];

extern unsigned short *gp2x_memregs;
extern pthread_t       gp2x_sound_thread;

int start_daemon(
        unsigned int minimal_cpu_speed, unsigned int maximal_cpu_speed, unsigned int start_cpu_speed, int cpu_speed_step,
        unsigned long hotkey, unsigned long incrementkey, unsigned long decrementkey,
        int speed_display, int foreground,
        unsigned long delay)
{
    pid_t pid, sid;
    
    if(!foreground) {
        kill_running_daemon();
    
    
        FILE * pidfile = fopen("/tmp/cpu_daemon.pid","w");
    
        if(!pidfile) {
            printf("couldn't write pidfile\r\n");
            exit(-2);
        }
    
        pid = fork();
        
        if(pid > 0) {
            fprintf(pidfile,"%i\n",pid);
            fclose(pidfile);
        }
        if(pid != 0)
            return pid;

    
        fclose(pidfile);
    
        umask(0);
        sid = setsid();
    
    
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }
    
    if(foreground)
        printf("daemon ready\r\n");
    
    nano_setup(); // loading the full minilib would be overkill and i guess some games/emus wouldn't like it
    
    
    unsigned int current_cpu_speed = start_cpu_speed;
    
    while(1) {
        usleep(delay);
        unsigned long keystate = gp2x_joystick_read();
        
        unsigned int last_cpu_speed = 0;
            
        while(
            (hotkey && (keystate & hotkey) == hotkey) ||
            ((!hotkey) && (
                (incrementkey && (keystate & incrementkey) == incrementkey) || 
                (decrementkey && (keystate & decrementkey) == decrementkey)
            ))
        ) {
            if(foreground && !last_cpu_speed)
                printf("cpu daemon activated!\r\n");
                
            if(incrementkey && (keystate & incrementkey) == incrementkey) {
                current_cpu_speed += cpu_speed_step;
                while((keystate & incrementkey) == incrementkey) usleep(100000),keystate = gp2x_joystick_read();
            }
            else if(decrementkey && (keystate & decrementkey) == decrementkey) {
                current_cpu_speed -= cpu_speed_step;
                while((keystate & decrementkey) == decrementkey) usleep(100000),keystate = gp2x_joystick_read();
            }
            
            if(current_cpu_speed < minimal_cpu_speed)
                current_cpu_speed = minimal_cpu_speed;
            if(current_cpu_speed > maximal_cpu_speed)
                current_cpu_speed = maximal_cpu_speed;
                

            
            if(last_cpu_speed != current_cpu_speed) {
                set_FCLK(current_cpu_speed);
            }
            last_cpu_speed = current_cpu_speed;
            keystate = gp2x_joystick_read();
        }

    }
    
}

int kill_running_daemon() {
    
    FILE * pidfile = fopen("/tmp/cpu_daemon.pid","r");
    char pid_buffer[14];
    pid_buffer[0] = 'k';
    pid_buffer[1] = 'i';
    pid_buffer[2] = 'l';
    pid_buffer[3] = 'l';
    pid_buffer[4] = ' ';
    pid_buffer[5] = 0;
    if(pidfile) {
        printf("found pidfile\r\n");
        fgets(&(pid_buffer[5]),10,pidfile);
        fclose(pidfile);
        int return_code = system(pid_buffer);
        if(return_code) 
            printf("daemon wasn't running\r\n");
        else
            printf("killed old daemon\r\n");
        unlink("/tmp/cpu_daemon.pid");
        return 1;
    }
    return 0;
}


void nano_setup() {
    if(!gp2x_sound_thread) {
        gp2x_memregs=(unsigned short *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR), 0xc0000000);
        cpuctrl_init();
    }
}

void cmd_daemon(int argc, char *argv[]) {
    
    int cpu_div = get_920_Div();
	int sysfreq=get_freq_920_CLK();
	sysfreq*=cpu_div+1;
	int cpufreq=sysfreq/1000000;
    
    unsigned int minimal_value = 33;
    unsigned int maximal_value = 260;
    unsigned int start_value = cpufreq;
    unsigned int step = 10;
    unsigned long hotkey = GP2X_L | GP2X_R;
    unsigned long downkey = GP2X_VOL_UP;
    unsigned long upkey = GP2X_VOL_DOWN;
    int foreground = 0;
    int display = 1;
    float delay = 1;
    
    int i;
    for( i = 2; i < argc; i++) {
        if(!strcmp(argv[i],"--min")) {
            if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
            minimal_value = atoi(argv[i+1]);
            if(minimal_value < 33)
                minimal_value = 33;
        }
        else if(!strcmp(argv[i],"--max")) {
            if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
            maximal_value = atoi(argv[i+1]);
            if(maximal_value > 340)
                maximal_value = 340;
        }
        else if(!strcmp(argv[i],"--start")) {
            if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
            start_value = atoi(argv[i+1]);
        }
        else if(!strcmp(argv[i],"--step")) {
            if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
            step = atoi(argv[i+1]);
        }
        else if(!strcmp(argv[i],"--hotkey")) {
            if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
            hotkey = parse_key_sequence(argv[i+1]);
        }
        else if(!strcmp(argv[i],"--incr")) {
            if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
            upkey = parse_key_sequence(argv[i+1]);
        }
        else if(!strcmp(argv[i],"--decr")) {
            if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
            downkey = parse_key_sequence(argv[i+1]);
        }
        else if(!strcmp(argv[i],"--delay")) {
            if(i+1 == argc){printf ("%s is missing it's parameter\r\n",argv[i]);gp2x_deinit();exit(1);}
            delay = atof(argv[i+1]);
        }
        else if(!strcmp(argv[i],"--no-incr")) {
            upkey = 0;
        }
        else if(!strcmp(argv[i],"--no-decr")) {
            downkey = 0;
        }
        else if(!strcmp(argv[i],"--no-hotkey")) {
            hotkey = 0;
        }
        else if(!strcmp(argv[i],"--foreground")) {
            foreground = 1;
        }
        else if(!strcmp(argv[i],"--background")) {
            foreground = 0;
        }
        else if(!strcmp(argv[i],"--display")) {
            display = 1;
        }
        else if(!strcmp(argv[i],"--no-display")) {
            display = 0;
        }
    }
    
    if((hotkey & downkey) == downkey)
        printf("warning: hotkey includes decrement keypress!\r\n");
    if((hotkey & upkey) == upkey)
        printf("warning: hotkey includes increment keypress!\r\n");
    
    int pid = start_daemon(minimal_value, maximal_value, start_value, step, hotkey, upkey, downkey, display, foreground, delay* 1000000);
    
        if(pid < 0) {
            printf("couldn't start daemon\r\n");
            exit(1);
        }
        else if(pid > 0) {
            printf("daemon started\r\n");
            exit(0);
        }
}

unsigned long parse_key_sequence(char *key_sequence) {
    unsigned long hotkey = 0;
    if(!strcmp(key_sequence,"None"))
        return 0;
    char *mask = key_sequence;
    while(*mask) {
        switch(*mask) {
        case 'l':
        case 'L':
            hotkey |= GP2X_L;
            break;
        case 'r':
        case 'R':
            hotkey |= GP2X_R;
            break;
        case 'a':
        case 'A':
            hotkey |= GP2X_A;
            break;
        case 'b':
        case 'B':
            hotkey |= GP2X_B;
            break;
        case 'x':
        case 'X':
            hotkey |= GP2X_X;
            break;
        case 'y':
        case 'Y':
            hotkey |= GP2X_Y;
            break;
        case '+':
            hotkey |= GP2X_VOL_DOWN;
            break;
        case '-':
            hotkey |= GP2X_VOL_UP;
            break;
        case 'S':
        case 's':
            hotkey |= GP2X_START;
            break;
        case '/':
            hotkey |= GP2X_SELECT;
            break;
        case '@':
            hotkey |= GP2X_PUSH;
            break;
        case '\n':
            break;
        default:
            printf("unknown key %c\r\n",*mask);
        }
        mask++;
    }
    return hotkey;
}

int daemonsettings[8];


void cleardisp();


void formatkey(char * base, unsigned long keyseq) {
    
    if(!keyseq)
        strcat(base,"None");
        
    if(keyseq & GP2X_L)
        strcat(base,"L");
    if(keyseq & GP2X_R)    
        strcat(base,"R");
    if(keyseq & GP2X_A)    
        strcat(base,"A");
    if(keyseq & GP2X_B)    
        strcat(base,"B");
    if(keyseq & GP2X_X)    
        strcat(base,"X");
    if(keyseq & GP2X_Y)    
        strcat(base,"Y");
    if(keyseq & GP2X_VOL_DOWN)
        strcat(base,"+");
    if(keyseq & GP2X_VOL_UP)  
        strcat(base,"-");
    if(keyseq & GP2X_START)
        strcat(base,"S");
    if(keyseq & GP2X_SELECT)
        strcat(base,"/");
    if(keyseq & GP2X_PUSH)
        strcat(base,"@");
}



#define VALID_KEYS ((GP2X_L) | (GP2X_R) | (GP2X_X) | (GP2X_Y) | (GP2X_A) | (GP2X_B) | (GP2X_START) | (GP2X_SELECT) | (GP2X_VOL_UP) | (GP2X_VOL_DOWN) | (GP2X_PUSH) )

int running;

void daemon_itemhelp(int menuitem)
{
	switch(menuitem) {
	case 0:
		v_putcad(26,8,0xffffff,COLORFONDO,"Choose a");
		v_putcad(26,9,0xffffff,COLORFONDO,"minimal");
		v_putcad(26,10,0xffffff,COLORFONDO,"clockspeed");
		v_putcad(26,11,0xffffff,COLORFONDO,"with R/L or");
		v_putcad(26,12,0xffffff,COLORFONDO,"Vol UP/Down.");
		v_putcad(26,14,0xffffff,COLORFONDO,"Valid speeds");
		v_putcad(26,15,0xffffff,COLORFONDO,"are:");
		v_putcad(26,16,0xffffff,COLORFONDO,"33 to 340Mhz");
	    break;
	case 1:
		v_putcad(26,8,0xffffff,COLORFONDO,"Choose a");
		v_putcad(26,9,0xffffff,COLORFONDO,"maximal");
		v_putcad(26,10,0xffffff,COLORFONDO,"clockspeed");
		v_putcad(26,11,0xffffff,COLORFONDO,"with R/L or");
		v_putcad(26,12,0xffffff,COLORFONDO,"Vol UP/Down.");
		v_putcad(26,14,0xffffff,COLORFONDO,"Valid speeds");
		v_putcad(26,15,0xffffff,COLORFONDO,"are:");
		v_putcad(26,16,0xffffff,COLORFONDO,"33 to 340Mhz");
	    break;
	case 2:
		v_putcad(26,8,0xffffff,COLORFONDO,"Choose a step");
		v_putcad(26,9,0xffffff,COLORFONDO,"width for");
		v_putcad(26,10,0xffffff,COLORFONDO,"changing the");
		v_putcad(26,11,0xffffff,COLORFONDO,"clockspeed.");
		v_putcad(26,13,0xffffff,COLORFONDO,"Use R/L or");
		v_putcad(26,14,0xffffff,COLORFONDO,"Vol UP/Down.");
		break;
    case 3:
		v_putcad(26,8,0xffffff,COLORFONDO,"Choose a");
		v_putcad(26,9,0xffffff,COLORFONDO,"delay between");
		v_putcad(26,10,0xffffff,COLORFONDO,"each hotkey");
		v_putcad(26,11,0xffffff,COLORFONDO,"check");
		v_putcad(26,13,0xffffff,COLORFONDO,"Use R/L or");
		v_putcad(26,14,0xffffff,COLORFONDO,"Vol UP/Down.");
		break;
    case 4:
		v_putcad(26,8,0xffffff,COLORFONDO,"Choose a");
		v_putcad(26,9,0xffffff,COLORFONDO,"hotkey.");
		v_putcad(26,10,0xffffff,COLORFONDO,"Add or delete");
		v_putcad(26,11,0xffffff,COLORFONDO,"a button by");
		v_putcad(26,12,0xffffff,COLORFONDO,"pressing it.");
		v_putcad(26,14,0x0000DD,COLORFONDO,"Joystick is");
		v_putcad(26,15,0x0000DD,COLORFONDO,"not allowed.");
		break;
    case 5:
		v_putcad(26,8,0xffffff,COLORFONDO,"Choose a");
		v_putcad(26,9,0xffffff,COLORFONDO,"key for");
		v_putcad(26,10,0xffffff,COLORFONDO,"incrementing");
		v_putcad(26,11,0xffffff,COLORFONDO,"the clkspeed.");
		v_putcad(26,12,0xffffff,COLORFONDO,"Add or delete");
		v_putcad(26,13,0xffffff,COLORFONDO,"a button by");
		v_putcad(26,14,0xffffff,COLORFONDO,"pressing it.");
		v_putcad(26,16,0x0000DD,COLORFONDO,"Joystick is");
		v_putcad(26,17,0x0000DD,COLORFONDO,"not allowed.");
		break;
    case 6:
		v_putcad(26,8,0xffffff,COLORFONDO,"Choose a");
		v_putcad(26,9,0xffffff,COLORFONDO,"key for");
		v_putcad(26,10,0xffffff,COLORFONDO,"decrementing");
		v_putcad(26,11,0xffffff,COLORFONDO,"the clkspeed.");
		v_putcad(26,12,0xffffff,COLORFONDO,"Add or delete");
		v_putcad(26,13,0xffffff,COLORFONDO,"a button by");
		v_putcad(26,14,0xffffff,COLORFONDO,"pressing it.");
		v_putcad(26,16,0x0000DD,COLORFONDO,"Joystick is");
		v_putcad(26,17,0x0000DD,COLORFONDO,"not allowed.");
		break;
    case 7:
	/*	v_putcad(26,8,0xffffff,COLORFONDO,"Enable or");
		v_putcad(26,9,0xffffff,COLORFONDO,"disable");
		v_putcad(26,10,0xffffff,COLORFONDO,"on screen");
		v_putcad(26,11,0xffffff,COLORFONDO,"display.");
		v_putcad(26,13,0x0000DD,COLORFONDO,"May cause");
		v_putcad(26,14,0x0000DD,COLORFONDO,"conflicts");
		v_putcad(26,15,0x0000DD,COLORFONDO,"with");
		v_putcad(26,16,0x0000DD,COLORFONDO,"some apps!");*/
		v_putcad(26,8,0x0000DD,COLORFONDO,"COMING SOON");
		break;
    case 8:
        if(running) {
		    v_putcad(26,8,0xffffff,COLORFONDO,"Press B to");
		    v_putcad(26,9,0xffffff,COLORFONDO,"kill the");
		    v_putcad(26,10,0xffffff,COLORFONDO,"running");
		    v_putcad(26,11,0xffffff,COLORFONDO,"daemon");
		    v_putcad(26,12,0xffffff,COLORFONDO,"process.");
	    }
	    else {
	        v_putcad(26,8,0xffffff,COLORFONDO,"Press B to");
		    v_putcad(26,9,0xffffff,COLORFONDO,"start the ");
		    v_putcad(26,10,0xffffff,COLORFONDO,"daemon in the");
		    v_putcad(26,11,0xffffff,COLORFONDO, "background.");
	    }
		break;
	}
}

void daemonmenu() {
    
    int menupoint = 0;
    running = !access("/tmp/cpu_daemon.pid",R_OK);

    
    unsigned long gp2x_nKeys;
    while(1) {
        
        	if(daemonsettings[0] < 33)
			    daemonsettings[0] = 33;
			if(daemonsettings[1] > 340)
			    daemonsettings[1] = 340;
			if(daemonsettings[1] < daemonsettings[0])
			    daemonsettings[1] = daemonsettings[0];
			if(daemonsettings[0] > daemonsettings[1])
			    daemonsettings[0] = daemonsettings[1];
			if(daemonsettings[2] < 1)
			    daemonsettings[2] = 1;
			if(daemonsettings[3] < 1)
			    daemonsettings[3] = 1;
			//if(daemonsettings[7] == 10 || daemonsettings[7] == -10)
			 //   daemonsettings[7] = 1;
			//if(daemonsettings[7] == 11 || daemonsettings[7] == -9)
			    daemonsettings[7] = 0;
        
            
        
        cleardisp();
        v_putcad(13,2,WHITE,COLORFONDO,"Daemon Setup");
        
        v_putcad(2,5,0xffff00,COLORFONDO,"CPU Clockspeed:");
        
        sprintf(cad,"From: %huMhz",daemonsettings[0]);
        
        v_putcad(2,7,0xffff,COLORFONDO,cad);
        if(menupoint == 0)
            v_putcad(2,7,0xffff,TEXTBACK,cad);
            
        sprintf(cad,"To: %huMhz",daemonsettings[1]);
        
        v_putcad(2,8,0xffff,COLORFONDO,cad);
        if(menupoint == 1)
            v_putcad(2,8,0xffff,TEXTBACK,cad);
        
        sprintf(cad,"Step: %huMhz",daemonsettings[2]);
        
        v_putcad(2,9,0xffff,COLORFONDO,cad);
        if(menupoint == 2)
            v_putcad(2,9,0xffff,TEXTBACK,cad);
            
            
        v_putcad(2,11,0xffff00,COLORFONDO,"Buttons:");
            
        sprintf(cad,"Delay: %0.1fsec",daemonsettings[3]/10.0f);
        
        v_putcad(2,13,0xffff,COLORFONDO,cad);
        if(menupoint == 3)
            v_putcad(2,13,0xffff,TEXTBACK,cad);
            
        sprintf(cad,"Hotkey: ");
        
        formatkey(cad,daemonsettings[4]);
        
        v_putcad(2,15,0xffff,COLORFONDO,cad);
        if(menupoint == 4)
            v_putcad(2,15,0xffff,TEXTBACK,cad);
        
        
        sprintf(cad,"IncrKey: ");
        
        formatkey(cad,daemonsettings[5]);
        
        v_putcad(2,16,0xffff,COLORFONDO,cad);
        if(menupoint == 5)
            v_putcad(2,16,0xffff,TEXTBACK,cad);
        
        sprintf(cad,"DecrKey: ");
        
        formatkey(cad,daemonsettings[6]);
        
        v_putcad(2,17,0xffff,COLORFONDO,cad);
        if(menupoint == 6)
            v_putcad(2,17,0xffff,TEXTBACK,cad);
        
        if(menupoint >= 4 && menupoint <=6)
            v_putcad(2,26,WHITE,COLORFONDO,"---------- Stick:UP/DOWN");
        
        v_putcad(2,19,0xffff00,COLORFONDO,"Misc:");
        
        
        v_putcad(2,21,0xffff,COLORFONDO,(daemonsettings[7] ? "On Screen Display: On" : "On Screen Display: Off"));
        if(menupoint == 7)
            v_putcad(2,21,0xffff,TEXTBACK,(daemonsettings[7] ? "On Screen Display: On" : "On Screen Display: Off"));
        

        v_putcad(2,23,0xffff,COLORFONDO,(running ? "Kill Running Daemon" : "Start Daemon"));
        if(menupoint == 8)
            v_putcad(2,23,0xffff,TEXTBACK,(running ? "Kill Running Daemon" : "Start Daemon"));

        
        
        daemon_itemhelp(menupoint);
        
        gp2x_video_flip();
        while(1)
		{
			gp2x_nKeys=gp2x_joystick_read();

			
			
			if((gp2x_nKeys & GP2X_DOWN)) 
			{
				menupoint++; 
				if(menupoint>8) menupoint=0;
				usleep(200000);
				break;
			}

			if((gp2x_nKeys & GP2X_UP)) 
			{ 
				menupoint--; 
				if(menupoint<0) menupoint=8;
				usleep(200000);
				break;
			}
			
			if((menupoint >= 4) && (menupoint <= 6) && (gp2x_nKeys & VALID_KEYS))
			{
			    daemonsettings[menupoint] ^= (gp2x_nKeys & VALID_KEYS);
			    usleep(200000);  
			    break;
			}
			
			if(menupoint < 8 &&(gp2x_nKeys & GP2X_R))
			{
			    daemonsettings[menupoint] += 10;
			    usleep(200000);
			    break;
			}
			
			if(menupoint < 4 && (gp2x_nKeys & GP2X_VOL_UP))
			{
			    daemonsettings[menupoint] -= 1;
			    usleep(200000); 
			    break;
			}
			
			if(menupoint < 4 && (gp2x_nKeys & GP2X_VOL_DOWN))
			{
			    daemonsettings[menupoint] += 1;
			    usleep(200000); 
			    break;
			}
			
			if(menupoint < 8 && (gp2x_nKeys & GP2X_L))
			{
			    daemonsettings[menupoint] -= 10;
			    usleep(200000);
			    break;
			}
			if(menupoint == 8 && (gp2x_nKeys & GP2X_B))
			{
			    if(running)
			        kill_running_daemon();
			    else {
			        int cpu_div = get_920_Div();
	                int sysfreq=get_freq_920_CLK();
                    sysfreq*=cpu_div+1;
	                int cpufreq=sysfreq/1000000;
	                
			        start_daemon_by_settings();
			    }
			    usleep(200000);
				running = !access("/tmp/cpu_daemon.pid",R_OK);
				break; 
			}

			
			if((gp2x_nKeys & GP2X_START)) 
			{
				while(1)
				{
					gp2x_nKeys=gp2x_joystick_read();
					if(!(gp2x_nKeys & GP2X_START)) break;
				}
				
				if(running) { // update values!
				    start_daemon_by_settings();
				}
				
				
				return;
			}
			
		}
    }
}

void start_daemon_by_settings() {
    int cpu_div = get_920_Div();
    int sysfreq=get_freq_920_CLK();
    sysfreq*=cpu_div+1;
    int cpufreq=sysfreq/1000000;
    
    start_daemon(daemonsettings[0], daemonsettings[1], cpufreq, daemonsettings[2], daemonsettings[4], daemonsettings[5],
                    daemonsettings[6], daemonsettings[7], 0, daemonsettings[3] * 100000);
}
