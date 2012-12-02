
//Gp2x/gp2x/mnt/nand/bios_cache.bin:     file format binary

Disassembly of section .data:

00000000 <.data>:
   0: e3a00f02  mov r0, #8  ; 0x8
   4: ebcb1afd  bl  step_debug
   8: e25cc001  subs  ip, ip, #1  ; 0x1
   c: 5a000001  bpl 0x18
  10: e3a00d05  mov r0, #320  ; 0x140
  14: ebcb18d0  bl  0xff2c635c
  18: eaffffff  b 0x1c
  1c: e3a00d05  mov r0, #320  ; 0x140
  20: ebcb1af6  bl  step_debug
  24: e59a9034  ldr r9, [sl, #52]
  28: e249900c  sub r9, r9, #12 ; 0xc
  2c: e59a0034  ldr r0, [sl, #52]
  30: e240000c  sub r0, r0, #12 ; 0xc
  34: e58a0034  str r0, [sl, #52]
  38: e3c99003  bic r9, r9, #3  ; 0x3
  3c: e2890000  add r0, r9, #0  ; 0x0
  40: e59a102c  ldr r1, [sl, #44]
  44: ebcb1a8e  bl  0xff2c6a84
  48: e2890004  add r0, r9, #4  ; 0x4
  4c: e1a01007  mov r1, r7
  50: ebcb1a8b  bl  0xff2c6a84
  54: e2890008  add r0, r9, #8  ; 0x8
  58: e1a01008  mov r1, r8
  5c: e3a02f51  mov r2, #324  ; 0x144
  60: ebcb1a6f  bl  0xff2c6a24
  64: e3a00f51  mov r0, #324  ; 0x144
  68: ebcb1ae4  bl  step_debug
  6c: e2480002  sub r0, r8, #2  ; 0x2
  70: ebcb1a95  bl  0xff2c6acc
  74: e1a07000  mov r7, r0
  78: e3a00f52  mov r0, #328  ; 0x148
  7c: ebcb1adf  bl  step_debug
  80: e3a01e15  mov r1, #336  ; 0x150
  84: e2810078  add r0, r1, #120  ; 0x78
  88: e58a002c  str r0, [sl, #44]
  8c: e3a00f53  mov r0, #332  ; 0x14c
  90: ebcb1ada  bl  step_debug
  94: e59a002c  ldr r0, [sl, #44]
  98: e0800107  add r0, r0, r7, lsl #2
  9c: ebcb1ac9  bl  0xff2c6bc8
  a0: e1a07000  mov r7, r0
  a4: e3a00e15  mov r0, #336  ; 0x150
  a8: ebcb1ad4  bl  step_debug
  ac: ebcb194d  bl  0xff2c65e8
  b0: e58a002c  str r0, [sl, #44]
  b4: e3a00f55  mov r0, #340  ; 0x154
  b8: ebcb1ad0  bl  step_debug
  bc: e59a9034  ldr r9, [sl, #52]
  c0: e2499004  sub r9, r9, #4  ; 0x4
  c4: e59a0034  ldr r0, [sl, #52]
  c8: e2400004  sub r0, r0, #4  ; 0x4
  cc: e58a0034  str r0, [sl, #52]
  d0: e3c99003  bic r9, r9, #3  ; 0x3
  d4: e2890000  add r0, r9, #0  ; 0x0
  d8: e59a102c  ldr r1, [sl, #44]
  dc: e3a02f56  mov r2, #344  ; 0x158
  e0: ebcb1a4f  bl  0xff2c6a24
  e4: e3a00f56  mov r0, #344  ; 0x158
  e8: ebcb1ac4  bl  step_debug
  ec: e59a102c  ldr r1, [sl, #44]
  f0: e2010080  and r0, r1, #128  ; 0x80
  f4: e58a002c  str r0, [sl, #44]
  f8: e3a00f57  mov r0, #348  ; 0x15c
  fc: ebcb1abf  bl  step_debug
 100: e59a102c  ldr r1, [sl, #44]
 104: e381001f  orr r0, r1, #31 ; 0x1f
 108: e58a002c  str r0, [sl, #44]
 10c: e3a00e16  mov r0, #352  ; 0x160
 110: ebcb1aba  bl  step_debug
 114: e59a002c  ldr r0, [sl, #44]
 118: e3a02e16  mov r2, #352  ; 0x160
 11c: e3a010ff  mov r1, #255  ; 0xff
 120: e38114ff  orr r1, r1, #-16777216  ; 0xff000000
 124: ebcb190a  bl  0xff2c6554
 128: e3a00f59  mov r0, #356  ; 0x164
 12c: ebcb1ab3  bl  step_debug
 130: e59a9034  ldr r9, [sl, #52]
 134: e2499008  sub r9, r9, #8  ; 0x8
 138: e59a0034  ldr r0, [sl, #52]
 13c: e2400008  sub r0, r0, #8  ; 0x8
 140: e58a0034  str r0, [sl, #52]
 144: e3c99003  bic r9, r9, #3  ; 0x3
 148: e2890000  add r0, r9, #0  ; 0x0
 14c: e59a1008  ldr r1, [sl, #8]
 150: ebcb1a4b  bl  0xff2c6a84
 154: e2890004  add r0, r9, #4  ; 0x4
 158: e1a01008  mov r1, r8
 15c: e3a02f5a  mov r2, #360  ; 0x168
 160: ebcb1a2f  bl  0xff2c6a24
 164: e3a00f5a  mov r0, #360  ; 0x168
 168: ebcb1aa4  bl  step_debug
 16c: e3a01e17  mov r1, #368  ; 0x170
 170: e2818000  add r8, r1, #0  ; 0x0
 174: e3a00f5b  mov r0, #364  ; 0x16c
 178: ebcb1aa0  bl  step_debug
 17c: e1a00007  mov r0, r7
 180: e24cc016  sub ip, ip, #22 ; 0x16
 184: eacb18be  b 0xff2c6484
 188: e3a00ff1  mov r0, #964  ; 0x3c4
 18c: e3800b02  orr r0, r0, #2048 ; 0x800
 190: ebcb1a9a  bl  step_debug
 194: e59a9034  ldr r9, [sl, #52]
 198: e2499020  sub r9, r9, #32 ; 0x20
 19c: e59a0034  ldr r0, [sl, #52]
 1a0: e2400020  sub r0, r0, #32 ; 0x20
 1a4: e58a0034  str r0, [sl, #52]
 1a8: e3c99003  bic r9, r9, #3  ; 0x3
 1ac: e2890000  add r0, r9, #0  ; 0x0
 1b0: e59a1010  ldr r1, [sl, #16]
 1b4: ebcb1a32  bl  0xff2c6a84
 1b8: e2890004  add r0, r9, #4  ; 0x4
 1bc: e59a1014  ldr r1, [sl, #20]
 1c0: ebcb1a2f  bl  0xff2c6a84
 1c4: e2890008  add r0, r9, #8  ; 0x8
 1c8: e1a01005  mov r1, r5
 1cc: ebcb1a2c  bl  0xff2c6a84
 1d0: e289000c  add r0, r9, #12 ; 0xc
 1d4: e59a101c  ldr r1, [sl, #28]
 1d8: ebcb1a29  bl  0xff2c6a84
 1dc: e2890010  add r0, r9, #16 ; 0x10
 1e0: e59a1020  ldr r1, [sl, #32]
 1e4: ebcb1a26  bl  0xff2c6a84
 1e8: e2890014  add r0, r9, #20 ; 0x14
 1ec: e1a01006  mov r1, r6
 1f0: ebcb1a23  bl  0xff2c6a84
 1f4: e2890018  add r0, r9, #24 ; 0x18
 1f8: e59a1028  ldr r1, [sl, #40]
 1fc: ebcb1a20  bl  0xff2c6a84
 200: e289001c  add r0, r9, #28 ; 0x1c
 204: e1a01008  mov r1, r8
 208: e3a02ff2  mov r2, #968  ; 0x3c8
 20c: e3822b02  orr r2, r2, #2048 ; 0x800
 210: ebcb1a03  bl  0xff2c6a24
 214: e3a00ff2  mov r0, #968  ; 0x3c8
 218: e3800b02  orr r0, r0, #2048 ; 0x800
 21c: ebcb1a77  bl  step_debug
 220: e59a0008  ldr r0, [sl, #8]
 224: e1a00580  mov r0, r0, lsl #11
 228: e58a0028  str r0, [sl, #40]
 22c: e3a00ff3  mov r0, #972  ; 0x3cc
 230: e3800b02  orr r0, r0, #2048 ; 0x800
 234: ebcb1a71  bl  step_debug
 238: e59a0028  ldr r0, [sl, #40]
 23c: e128f00b  msr CPSR_f, fp
 240: e1b074a0  movs  r7, r0, lsr #9
 244: e10fb000  mrs fp, CPSR
 248: e3a00ebd  mov r0, #3024 ; 0xbd0
 24c: ebcb1a6b  bl  step_debug
 250: e3a00ff5  mov r0, #980  ; 0x3d4
 254: e3800b02  orr r0, r0, #2048 ; 0x800
 258: e1a08000  mov r8, r0
 25c: e25cc00c  subs  ip, ip, #12 ; 0xc
 260: 5a000002  bpl 0x270
 264: e3a00fe9  mov r0, #932  ; 0x3a4
 268: e3800b02  orr r0, r0, #2048 ; 0x800
 26c: ebcb183a  bl  0xff2c635c
 270: eaffffff  b 0x274
 274: e3a00fe9  mov r0, #932  ; 0x3a4
 278: e3800b02  orr r0, r0, #2048 ; 0x800
 27c: ebcb1a5f  bl  step_debug
 280: e3570000  cmp r7, #0  ; 0x0
 284: e10fb000  mrs fp, CPSR
 288: e3a00fea  mov r0, #936  ; 0x3a8
 28c: e3800b02  orr r0, r0, #2048 ; 0x800
 290: ebcb1a5a  bl  step_debug
 294: e24cc002  sub ip, ip, #2  ; 0x2
 298: e128f00b  msr CPSR_f, fp
 29c: 1a000007  bne 0x2c0
 2a0: 5a000002  bpl 0x2b0
 2a4: e3a00fef  mov r0, #956  ; 0x3bc
 2a8: e3800b02  orr r0, r0, #2048 ; 0x800
 2ac: ebcb182a  bl  0xff2c635c
 2b0: ea000013  b 0x304
 2b4: e3a00feb  mov r0, #940  ; 0x3ac
 2b8: e3800b02  orr r0, r0, #2048 ; 0x800
 2bc: ebcb1a4f  bl  step_debug
 2c0: e3c774fe  bic r7, r7, #-33554432  ; 0xfe000000
 2c4: e3a00ebb  mov r0, #2992 ; 0xbb0
 2c8: ebcb1a4c  bl  step_debug
 2cc: e0837007  add r7, r3, r7
 2d0: e3a00fed  mov r0, #948  ; 0x3b4
 2d4: e3800b02  orr r0, r0, #2048 ; 0x800
 2d8: ebcb1a48  bl  step_debug
 2dc: e313040e  tst r3, #234881024  ; 0xe000000
 2e0: e10fb000  mrs fp, CPSR
 2e4: e3a00fee  mov r0, #952  ; 0x3b8
 2e8: e3800b02  orr r0, r0, #2048 ; 0x800
 2ec: ebcb1a43  bl  step_debug
 2f0: e24cc004  sub ip, ip, #4  ; 0x4
 2f4: e128f00b  msr CPSR_f, fp
 2f8: 0a000004  beq 0x310
 2fc: e317040e  tst r7, #234881024  ; 0xe000000
 300: e10fb000  mrs fp, CPSR
 304: e3a00fef  mov r0, #956  ; 0x3bc
 308: e3800b02  orr r0, r0, #2048 ; 0x800
 30c: ebcb1a3b  bl  step_debug
 310: e1a00008  mov r0, r8
 314: e24cc001  sub ip, ip, #1  ; 0x1
 318: eacb1859  b 0xff2c6484
 31c: e3a00ff5  mov r0, #980  ; 0x3d4
 320: e3800b02  orr r0, r0, #2048 ; 0x800
 324: ebcb1a35  bl  step_debug
 328: e24cc001  sub ip, ip, #1  ; 0x1
 32c: e128f00b  msr CPSR_f, fp
 330: 1a000007  bne 0x354
 334: 5a000002  bpl 0x344
 338: e3a00f09  mov r0, #36 ; 0x24
 33c: e3800b03  orr r0, r0, #3072 ; 0xc00
 340: ebcb1805  bl  0xff2c635c
 344: ea0000d6  b 0x6a4
 348: e3a00ff6  mov r0, #984  ; 0x3d8
 34c: e3800b02  orr r0, r0, #2048 ; 0x800
 350: ebcb1a2a  bl  step_debug
 354: e59a0028  ldr r0, [sl, #40]
 358: e08404a0  add r0, r4, r0, lsr #9
 35c: e58a0028  str r0, [sl, #40]
 360: e3a00ff7  mov r0, #988  ; 0x3dc
 364: e3800b02  orr r0, r0, #2048 ; 0x800
 368: ebcb1a24  bl  step_debug
 36c: e59a0008  ldr r0, [sl, #8]
 370: e128f00b  msr CPSR_f, fp
 374: e1b00ca0  movs  r0, r0, lsr #25
 378: e10fb000  mrs fp, CPSR
 37c: e58a0008  str r0, [sl, #8]
 380: e3a00ebe  mov r0, #3040 ; 0xbe0
 384: ebcb1a1d  bl  step_debug
 388: e24cc003  sub ip, ip, #3  ; 0x3
 38c: e128f00b  msr CPSR_f, fp
 390: 2a000007  bcs 0x3b4
 394: 5a000002  bpl 0x3a4
 398: e3a00f05  mov r0, #20 ; 0x14
 39c: e3800b03  orr r0, r0, #3072 ; 0xc00
 3a0: ebcb17ed  bl  0xff2c635c
 3a4: ea00006a  b 0x554
 3a8: e3a00ff9  mov r0, #996  ; 0x3e4
 3ac: e3800b02  orr r0, r0, #2048 ; 0x800
 3b0: ebcb1a12  bl  step_debug
 3b4: e2830000  add r0, r3, #0  ; 0x0
 3b8: ebcb1a02  bl  0xff2c6bc8
 3bc: e58a0008  str r0, [sl, #8]
 3c0: e3a00ffa  mov r0, #1000 ; 0x3e8
 3c4: e3800b02  orr r0, r0, #2048 ; 0x800
 3c8: ebcb1a0c  bl  step_debug
 3cc: e59a0008  ldr r0, [sl, #8]
 3d0: e1a00000  nop     (mov r0,r0)
 3d4: e58a000c  str r0, [sl, #12]
 3d8: e3a00ffb  mov r0, #1004 ; 0x3ec
 3dc: e3800b02  orr r0, r0, #2048 ; 0x800
 3e0: ebcb1a06  bl  step_debug
 3e4: e59a0008  ldr r0, [sl, #8]
 3e8: e1a00000  nop     (mov r0,r0)
 3ec: e58a0010  str r0, [sl, #16]
 3f0: e3a00ebf  mov r0, #3056 ; 0xbf0
 3f4: ebcb1a01  bl  step_debug
 3f8: e59a0008  ldr r0, [sl, #8]
 3fc: e1a00000  nop     (mov r0,r0)
 400: e58a0014  str r0, [sl, #20]
 404: e3a00ffd  mov r0, #1012 ; 0x3f4
 408: e3800b02  orr r0, r0, #2048 ; 0x800
 40c: ebcb19fb  bl  step_debug
 410: e59a0008  ldr r0, [sl, #8]
 414: e1a05000  mov r5, r0
 418: e3a00ffe  mov r0, #1016 ; 0x3f8
 41c: e3800b02  orr r0, r0, #2048 ; 0x800
 420: ebcb19f6  bl  step_debug
 424: e59a0008  ldr r0, [sl, #8]
 428: e1a00000  nop     (mov r0,r0)
 42c: e58a001c  str r0, [sl, #28]
 430: e3a00fff  mov r0, #1020 ; 0x3fc
 434: e3800b02  orr r0, r0, #2048 ; 0x800
 438: ebcb19f0  bl  step_debug
 43c: e59a0008  ldr r0, [sl, #8]
 440: e1a00000  nop     (mov r0,r0)
 444: e58a0020  str r0, [sl, #32]
 448: e3a00b03  mov r0, #3072 ; 0xc00
 44c: ebcb19eb  bl  step_debug
 450: e59a0008  ldr r0, [sl, #8]
 454: e1a06000  mov r6, r0
 458: e24cc00a  sub ip, ip, #10 ; 0xa

 45c: e3a00f01  mov r0, #4  ; 0x4
 460: e3800b03  orr r0, r0, #3072 ; 0xc00
 464: ebcb19e5  bl  step_debug

 468: e59a0028  ldr r0, [sl, #40]
 46c: e1540000  cmp r4, r0
 470: e10fb000  mrs fp, CPSR

 474: e3a00f02  mov r0, #8  ; 0x8
 478: e3800b03  orr r0, r0, #3072 ; 0xc00

 47c: ebcb19df  bl  step_debug
 480: e24cc002  sub ip, ip, #2  ; 0x2
 484: e128f00b  msr CPSR_f, fp

 488: aa000021  bge 0x514
 48c: e1a09004  mov r9, r4

 490: e1a00004  mov r0, r4
 494: e2800020  add r0, r0, #32 ; 0x20
 498: e1a04000  mov r4, r0
 49c: e3c99003  bic r9, r9, #3  ; 0x3
 4a0: e2890000  add r0, r9, #0  ; 0x0
 4a4: e59a1008  ldr r1, [sl, #8]
 4a8: ebcb1975  bl  0xff2c6a84
 4ac: e2890004  add r0, r9, #4  ; 0x4
 4b0: e59a100c  ldr r1, [sl, #12]
 4b4: ebcb1972  bl  0xff2c6a84
 4b8: e2890008  add r0, r9, #8  ; 0x8
 4bc: e59a1010  ldr r1, [sl, #16]
 4c0: ebcb196f  bl  0xff2c6a84
 4c4: e289000c  add r0, r9, #12 ; 0xc
 4c8: e59a1014  ldr r1, [sl, #20]
 4cc: ebcb196c  bl  0xff2c6a84
 4d0: e2890010  add r0, r9, #16 ; 0x10
 4d4: e1a01005  mov r1, r5
 4d8: ebcb1969  bl  0xff2c6a84
 4dc: e2890014  add r0, r9, #20 ; 0x14
 4e0: e59a101c  ldr r1, [sl, #28]
 4e4: ebcb1966  bl  0xff2c6a84
 4e8: e2890018  add r0, r9, #24 ; 0x18
 4ec: e59a1020  ldr r1, [sl, #32]
 4f0: ebcb1963  bl  0xff2c6a84
 4f4: e289001c  add r0, r9, #28 ; 0x1c
 4f8: e1a01006  mov r1, r6
 4fc: e3a02f03  mov r2, #12 ; 0xc
 500: e3822b03  orr r2, r2, #3072 ; 0xc00
 504: ebcb1946  bl  0xff2c6a24
 508: e3a00f03  mov r0, #12 ; 0xc
 50c: e3800b03  orr r0, r0, #3072 ; 0xc00
 510: ebcb19ba  bl  step_debug
 514: e24cc009  sub ip, ip, #9  ; 0x9
 518: e128f00b  msr CPSR_f, fp
 51c: aa000006  bge 0x53c
 520: 5a000002  bpl 0x530
 524: e3a00f01  mov r0, #4  ; 0x4
 528: e3800b03  orr r0, r0, #3072 ; 0xc00
 52c: ebcb178a  bl  0xff2c635c
 530: eaffffc9  b 0x45c
 534: e3a00ec1  mov r0, #3088 ; 0xc10
 538: ebcb19b0  bl  step_debug
 53c: e25cc001  subs  ip, ip, #1  ; 0x1
 540: 5a000002  bpl 0x550
 544: e3a00f09  mov r0, #36 ; 0x24
 548: e3800b03  orr r0, r0, #3072 ; 0xc00
 54c: ebcb1782  bl  0xff2c635c
 550: ea000053  b 0x6a4
 554: e3a00f05  mov r0, #20 ; 0x14
 558: e3800b03  orr r0, r0, #3072 ; 0xc00
 55c: ebcb19a7  bl  step_debug
 560: e59a0028  ldr r0, [sl, #40]
 564: e1540000  cmp r4, r0
 568: e10fb000  mrs fp, CPSR
 56c: e3a00f06  mov r0, #24 ; 0x18
 570: e3800b03  orr r0, r0, #3072 ; 0xc00
 574: ebcb19a1  bl  step_debug
 578: e24cc002  sub ip, ip, #2  ; 0x2
 57c: e128f00b  msr CPSR_f, fp
 580: aa00003f  bge 0x684
 584: e1a09003  mov r9, r3
 588: e1a00003  mov r0, r3
 58c: e2800020  add r0, r0, #32 ; 0x20
 590: e1a03000  mov r3, r0
 594: e3c99003  bic r9, r9, #3  ; 0x3
 598: e2890000  add r0, r9, #0  ; 0x0
 59c: ebcb1989  bl  0xff2c6bc8
 5a0: e58a0008  str r0, [sl, #8]
 5a4: e2890004  add r0, r9, #4  ; 0x4
 5a8: ebcb1986  bl  0xff2c6bc8
 5ac: e58a000c  str r0, [sl, #12]
 5b0: e2890008  add r0, r9, #8  ; 0x8
 5b4: ebcb1983  bl  0xff2c6bc8
 5b8: e58a0010  str r0, [sl, #16]
 5bc: e289000c  add r0, r9, #12 ; 0xc
 5c0: ebcb1980  bl  0xff2c6bc8
 5c4: e58a0014  str r0, [sl, #20]
 5c8: e2890010  add r0, r9, #16 ; 0x10
 5cc: ebcb197d  bl  0xff2c6bc8
 5d0: e1a05000  mov r5, r0
 5d4: e2890014  add r0, r9, #20 ; 0x14
 5d8: ebcb197a  bl  0xff2c6bc8
 5dc: e58a001c  str r0, [sl, #28]
 5e0: e2890018  add r0, r9, #24 ; 0x18
 5e4: ebcb1977  bl  0xff2c6bc8
 5e8: e58a0020  str r0, [sl, #32]
 5ec: e289001c  add r0, r9, #28 ; 0x1c
 5f0: ebcb1974  bl  0xff2c6bc8
 5f4: e1a06000  mov r6, r0
 5f8: e3a00f07  mov r0, #28 ; 0x1c
 5fc: e3800b03  orr r0, r0, #3072 ; 0xc00
 600: ebcb197e  bl  step_debug
 604: e1a09004  mov r9, r4
 608: e1a00004  mov r0, r4
 60c: e2800020  add r0, r0, #32 ; 0x20
 610: e1a04000  mov r4, r0
 614: e3c99003  bic r9, r9, #3  ; 0x3
 618: e2890000  add r0, r9, #0  ; 0x0
 61c: e59a1008  ldr r1, [sl, #8]
 620: ebcb1917  bl  0xff2c6a84
 624: e2890004  add r0, r9, #4  ; 0x4
 628: e59a100c  ldr r1, [sl, #12]
 62c: ebcb1914  bl  0xff2c6a84
 630: e2890008  add r0, r9, #8  ; 0x8
 634: e59a1010  ldr r1, [sl, #16]
 638: ebcb1911  bl  0xff2c6a84
 63c: e289000c  add r0, r9, #12 ; 0xc
 640: e59a1014  ldr r1, [sl, #20]
 644: ebcb190e  bl  0xff2c6a84
 648: e2890010  add r0, r9, #16 ; 0x10
 64c: e1a01005  mov r1, r5
 650: ebcb190b  bl  0xff2c6a84
 654: e2890014  add r0, r9, #20 ; 0x14
 658: e59a101c  ldr r1, [sl, #28]
 65c: ebcb1908  bl  0xff2c6a84
 660: e2890018  add r0, r9, #24 ; 0x18
 664: e59a1020  ldr r1, [sl, #32]
 668: ebcb1905  bl  0xff2c6a84
 66c: e289001c  add r0, r9, #28 ; 0x1c
 670: e1a01006  mov r1, r6
 674: e3a02ec2  mov r2, #3104 ; 0xc20
 678: ebcb18e9  bl  0xff2c6a24

 67c: e3a00ec2  mov r0, #3104 ; 0xc20
 680: ebcb195e  bl  step_debug
 684: e24cc012  sub ip, ip, #18 ; 0x12
 688: e128f00b  msr CPSR_f, fp
 68c: aa000007  bge 0x6b0
 690: 5a000002  bpl 0x6a0
 694: e3a00f05  mov r0, #20 ; 0x14
 698: e3800b03  orr r0, r0, #3072 ; 0xc00
 69c: ebcb172e  bl  0xff2c635c
 6a0: eaffffab  b 0x554

6a4:  e3a00f09  mov r0, #36 ; 0x24
 6a8: e3800b03  orr r0, r0, #3072 ; 0xc00
 6ac: ebcb1953  bl  step_debug
 6b0: e59a9034  ldr r9, [sl, #52]
 6b4: e59a0034  ldr r0, [sl, #52]
 6b8: e2800020  add r0, r0, #32 ; 0x20
 6bc: e58a0034  str r0, [sl, #52]
 6c0: e3c99003  bic r9, r9, #3  ; 0x3
 6c4: e2890000  add r0, r9, #0  ; 0x0
 6c8: ebcb193e  bl  0xff2c6bc8
 6cc: e58a0010  str r0, [sl, #16]
 6d0: e2890004  add r0, r9, #4  ; 0x4
 6d4: ebcb193b  bl  0xff2c6bc8
 6d8: e58a0014  str r0, [sl, #20]
 6dc: e2890008  add r0, r9, #8  ; 0x8
 6e0: ebcb1938  bl  0xff2c6bc8
 6e4: e1a05000  mov r5, r0
 6e8: e289000c  add r0, r9, #12 ; 0xc
 6ec: ebcb1935  bl  0xff2c6bc8
 6f0: e58a001c  str r0, [sl, #28]
 6f4: e2890010  add r0, r9, #16 ; 0x10
 6f8: ebcb1932  bl  0xff2c6bc8
 6fc: e58a0020  str r0, [sl, #32]
 700: e2890014  add r0, r9, #20 ; 0x14
 704: ebcb192f  bl  0xff2c6bc8
 708: e1a06000  mov r6, r0
 70c: e2890018  add r0, r9, #24 ; 0x18
 710: ebcb192c  bl  0xff2c6bc8
 714: e58a0028  str r0, [sl, #40]
 718: e289001c  add r0, r9, #28 ; 0x1c
 71c: ebcb1929  bl  0xff2c6bc8
 720: e1a08000  mov r8, r0

724:  e3a00f0a  mov r0, #40 ; 0x28
 728: e3800b03  orr r0, r0, #3072 ; 0xc00
 72c: ebcb1933  bl  step_debug
 730: e1a00008  mov r0, r8
 734: e24cc00a  sub ip, ip, #10 ; 0xa
 738: eacb1751  b 0xff2c6484
