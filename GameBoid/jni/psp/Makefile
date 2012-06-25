# -x assembler-with-cpp
# gpSP makefile
# Gilead Kutnick - Exophase

# Global definitions

PSPSDK          = ${shell psp-config --pspsdk-path}
PREFIX          = ${shell psp-config --psp-prefix}

OBJS            = main.o cpu.o video.o memory.o sound.o input.o \
 	          cpu_threaded.o gui.o zip.o cheats.o mips_stub.o 

TARGET          = gpSP

VPATH           += ..
CFLAGS          += -O3 -DPSP_BUILD -G0 -funsigned-char
CFLAGS          += ${shell ${PREFIX}/bin/sdl-config --cflags}
ASFLAGS         = ${CFLAGS}
PSP_EBOOT_TITLE = gpSP
EXTRA_TARGETS   = EBOOT.PBP

LIBS            += ${shell ${PREFIX}/bin/sdl-config --libs} -lpsppower \
		  -lz

include ${PSPSDK}/lib/build.mak

