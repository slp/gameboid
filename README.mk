GameBoid is free and open source GBA emulator for Android, based on 
[gpSP](http://notaz.gp2x.de/cgi-bin/gitweb.cgi?p=gpsp.git).

It has most of gpSP and [Android shared emulator routines
library](http://libemudroid.sf.net) features including:

* zipped games support
* on-screen controls
* save-load support

Building:

GameBoid depends on shared emulator routines library, which can be 
obtained [from Sourceforge](/p/libemudroid/code).

To build debug version execute following commands (taking, that you are 
in project directory and checked out library source is under 
../emudroid-common):

$ android update project -p . -l ../emudroid-common
$ NDK_DEBUG=1 ndk-build
$ ant debug
