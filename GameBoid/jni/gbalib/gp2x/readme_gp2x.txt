-- gameplaySP2X  Gameboy Advance emulator for GP2X --

gpSP2X is a version of my (Exophase)'s emulator originally for Sony PSP.
A large amount of effort has been done to make it more optimized for the
ARM CPU present in the GP2X, however it is still very much a work in
progress.

See readme.txt for the PSP version readme, which contains a lot of
information relevant to the GP2X version (note that some of it does
not apply however).


Changelog:

0.9-2xb: 
-- IMPORTANT-- If you're overwriting an old version, be sure to delete the
   gpsp.cfg file first, or be prepared to have a bunch of weird button
   settings that would require fixing.

- Fixed some bugs stunting compatability.
- Optimized alpha blends in renderer.
- Some more optimizations to dynarec output.
- Savestates should work better now.
- Cheat/misc menu won't crash the emulator.
- Main button config window works (not all buttons are in yet)

0.9-2Xa: (Exophase release)
- Redid autoframeskip. Should work more reliably.
- Rewrote dynamic recompiler from x86 source (arm_emit.h, arm_stub.S).
  Has some more sophisticated behavior than the last version, more is
  still to come... Should notice a slight speed improvement over the
  last version.
- Tweaked GUI to be a little more useable. Buttons are now mirroring the
  PSP version's.
- Code unification + cleanup amongst versions.


v9008: (zodttd release)
- Updated the way autoframeskip works. Should be better now. Still has a max
  frameskip value.
- Added a slight performance increase to the dynarec.
- Added sync() to make sure files such as savestates and in-game saves are
  saved properly to the GP2X.

v9006: (zodttd release)
- Initial public release


Installation:

1. Place the "gpsp.gpe" and "game_config.txt" file in a directory on your SD
   card used with the GP2X.

2. Place your GBA BIOS in the directory from step 1. This file must be named
   "gba_bios.bin" in all lowercase as shown, so rename it if needed.

   -- NOTE --

   There are two commonly available BIOSes - one is the correct one used in
   production GBA's worldwide and the other is a prototype BIOS. The latter
   will not cause some games to not work correctly or crash. If you attempt
   to use this BIOS you will be presented with a warning before being
   allowed to continue. This screen will give you a checksum of the real
   BIOS image (see readme.txt for further information).

3. Place your GBA games in the directory from step 1. These files should have
   a ".gba" or ".bin" file extension. Zip compressed games should be supported
   and are recognized with the ".zip" file extension. Note that 32MB ROMs will
   probably not run if zipped. 16MB and smaller should be OK.

4. Done. Run gpsp.gpe.


Controls:

How to use gpSP on the GP2X:
Buttons are mapped as follows (GBA/ingame buttons can be changed in the menu):

GP2X--------------------GBA
X           ->          A
B           ->          B
L TRIG      ->          L TRIG
R TRIG      ->          R TRIG
START       ->          START
SELECT      ->          SELECT

GP2X--------------------------------gpSP

-- IN-GAME --

VOL MIDDLE (UP + DOWN)    ->        menu
PUSH STICK                ->        fps display toggle (second number is
                                     frames actually drawn)

-- IN-MENU --
B                         ->        select option
X                         ->        cancel/exit menu
A                         ->        escape (up one director level in the
                                     file selector)

When gpSP is started, you are presented with the option to overclock your
GP2X. Use the L/R TRIG to change the CPU clockspeed and press START to
continue. You may also change RAM timings here - experiment with what
works well. Note that going too high on overclocking or low on RAM
timings can cause the game to crash or the GP2X to outright freeze up.

If you do not want to overclock, press START without using L/R.
You will now be presented with a menu to choose a game. Press the IN-MENU
"SELECT" button shown above to pick a game to load.

If you would like to test gpSP for the GP2X with a homebrew (free public
domain) game, a game by Russ Prince works very well with gpSP. It is called
Bust-A-Move and is a remake of the classic game it's named after.


How to build from source:

The makefile included in the source is geared towards the Open2x toolchain.
If you use Open2x and installed it in the way recommended then it should
work okay, assuming you also have up to date HW-SDL (and have
arm-linux-sdl-config installed in the right place). The makefile is in the
gp2x directory, so go there first then just type make to build gpsp.gpe.
Might need a little tweaking if your setup is different. If you need help
you can ask me, but I'll probably nag you about why you want to build it in
the first place.


GP2X version FAQ:

Q) Help! This game doesn't work. Am I using a bad version of the ROM?

A) First, make sure you're using the correct BIOS version. If you aren't
   gpSP should tell you. Other than that, there are some games that are
   known to not work now (and will probably work later), and perhaps
   many more games that I don't know about that don't work. I haven't
   launched a full scale compatability test at this version, so it might
   take a while before the compatability levels are high.


Q) Why is this version slower than the PSP version?

A) gpSP is still a work in progress. It might be possible to obtain more
   speed from both this version and the PSP one too (and others in the
   future). With that in mind, know that even a very agressively overclocked
   GP2X is still less powerful than a PSP, generally speaking. Still, I
   have a lot of ideas. It's unlikely that the GP2X version will ever be as
   fast/faster than the PSP version for anyone but anything's possible.


Q) How high does my GP2X have to overclock to enjoy gpSP?

A) That depends on you. Higher overclocking will mean less frames skipped
   on autoframeskip, or less frameskip needed if on manual. Or it can
   make the difference between whether or not virtual 60fps can be reached.
   For some games no GP2X in the world will be able to run them fullspeed,
   with any amount of frameskip. A few might run well with no overclocking
   and a generous level of frameskip (probably manual). If you don't care
   about battery life (or you're plugged into an outlet) you should push
   it as high as you can while still maintaining stability, because
   chances are high that whatever you play will benefit from it. Right now
   you'll probably want 260MHz if you can achieve it, but with a lot of
   luck this number will lower slightly in the future (and is just a vague
   ballpark figure anyway). I don't want to scare anyone off from using the
   emulator, you should give it a try and see how it plays for you
   regardless of how high you can overclock. Just note that this is far
   from a locked smooth experience for everyone on every game.


Q) GBA has an ARM processor, GP2X has an ARM processor. GP2X is more
   powerful than GBA. This emulator should run great without overclocking,
   so therefore you're doing it wrong.

A) That's not a question, but I'll field it anyway. Two things: first,
   "virtualization", or running the GBA code "natively" on the GP2X is
   probably not possible, at least not with the way I want to do things.
   For reasons why go read my blog (see below). So yes, you actually
   do need more than 16.7MHz of ARM9 power to emulate the GBA's CPU.
   Second: there is a whole lot of work behind emulating the pretty 2D
   graphics on the GBA, something it can do in hardware a lot better than
   this platform can.
   End result: GBA emulation on GP2X isn't as easy as you think it is.


Q) What are you working on now? When will you release the next version?

A) See the gpSP development blog:

   http://gpsp-dev.blogspot.com/

   Note that I don't give release dates, ever, unless I'm right on the verge
   of releasing. Be grateful that I've decided to be much more open about
   the development of the emulator now.


Q) Thanks to your blog I heard that you made some improvement. Can I have
   a copy of the new code?

A) No. Builds in transition often have a lot of problems, and I like for
   releases to be relatively substantial. I can probably be bribed out of
   them with donations though. :P


Q) Why do the menu suck so much? Why do half the options not work or not
   make any sense?

A) Sorry, the menu still hasn't been modified very much to fit the GP2X
   version instead of the PSP version.. hopefully this will improve in the
   future.


Q) Who's in charge of the GP2X version anyway?

A) Originally, zodttd was. I, Exophase, have basically usurped control of it
   now to encourage zodttd to work more on his PS1 emulator (that and I'm
   possessive of gpSP and get nervous when people work on it too heavily).
   zodttd will most likely still be around to work on things though.


Q) I'm a super nice person and would like to donate some of my hard earned
   money to this one-off GBA emulator. Where do I send my money to?

A) Exophase: exophase@gmail.com on PayPal
   zodttd: https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=heirloomer
    %40pobox%2ecom&item_number=1&no_shipping=1&no_note=1&tax=0&cy_code=USD&bn=
    PP%2dDonationsBF&charset=UTF%2d8
    ^ Click there for donating on PayPal (remove whitespace/linebreaks).

   GP2X people have already donated a lot more to me than PSP people have,
   even though there's an order of magnitude or two less users. And they've
   donated far more to zodttd than they have to me. So I'm not going to ask
   people to donate..

   However I won't lie: donating ups the chances of me actually working on the
   next version (for which I have a lot of ideas, but not necessarily time to
   dedicate to.. that time might need more incentive to be allotted from other
   things). This could change depending on my employment situation, but right
   now I feel guilty doing anything that doesn't help guarantee that I'll be
   able to buy food a year from now.


Q) Tell me all of your personal information.

A) Again not a question, but why not. I'm Exophase, real name: Gilead Kutnick,
   male, 23 years old, current residence Bloomington, IN; straight/single/not
   actively looking, almost have an MS in Computer Science (do have a BS
   underneath it), likes PSP more than GP2X, will not write a Nintendo DS
   emulator for either, am currently looking for a job for after I graduate.


Q) You said you're looking for a job.

A) Yes. If you have one or know someone who needs a low level oriented
   programmer then I'm up for grabs. And this is my resume:
   http://exophase.devzero.co.uk/resume.pdf


Credits:

Original codebase: Exophase (exophase@gmail.com)
Foundation gp2x code: zodttd
GP2X dynarec/stubs + current code maintainance: Exophase

