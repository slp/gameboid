// Betting on GCC aligning this for efficiency.
#include <stdio.h>

int main()
{
  unsigned short int read_16 = 0xF1F2;
  unsigned int read_32 = 0xF1F2F3F4;

  unsigned short int write_16 = 0xF00D;
  unsigned int write_32 = 0xF00DFEED;
  // 16bit unsigned reads, we expect 0xF1F2 and 0xF20000F1
  fprintf(stderr, "%04x %04x\n",
   *((unsigned short int *)((char *)&read_16)),
   *((unsigned short int *)((char *)&read_16 + 1)));

  // 16bit signed reads, we expect 0xFFFFF1F2 and 0xFFFFFFF1
  fprintf(stderr, "%04x %04x\n",
   *((short int *)((char *)&read_16)),
   *((short int *)((char *)&read_16 + 1)));

  // 32bit reads, we expect 0xF1F2F3F4, 0xF4F1F2F3, 0xF3F4F1F2,
  // and 0xF2F3F4F1

  fprintf(stderr, "%08x %08x %08x %08x\n",
   *((int *)((char *)&read_32)),
   *((int *)((char *)&read_32 + 1)),
   *((int *)((char *)&read_32 + 2)),
   *((int *)((char *)&read_32 + 3)));

  // 16bit writes, we expect write_16 to remain 0xF00D

  *((short int *)((char *)&write_16)) = 0xF00D;
  *((short int *)((char *)&write_16) + 1) = 0xF00D;

  fprintf(stderr, "%04x\n", write_16);

  // 32bit writes, we expect write_32 to remain 0xF00DFEED

  *((int *)((char *)&write_16)) = 0xF00DFEED;
  *((int *)((char *)&write_16) + 1) = 0xF00DFEED;
  *((int *)((char *)&write_16) + 2) = 0xF00DFEED;
  *((int *)((char *)&write_16) + 3) = 0xF00DFEED;

  fprintf(stderr, "%08x\n", write_32);

  return 0;
}

