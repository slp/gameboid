#include <stdio.h>

typedef unsigned int u32;

u32 arm_imm_find_nonzero(u32 imm, u32 start_bit)
{
  u32 i;

  for(i = start_bit; i < 32; i += 2)
  {
    if((imm >> i) & 0x03)
      break;
  }

  return i;
}

u32 arm_disect_imm_32bit(u32 imm, u32 *stores, u32 *rotations)
{
  u32 store_count = 0;
  u32 left_shift = 0;

  // Otherwise it'll return 0 things to store because it'll never
  // find anything.
  if(imm == 0)
  {
    rotations[0] = 0;
    stores[0] = 0;
    return 1;
  }

  // Find chunks of non-zero data at 2 bit alignments.
  while(1)
  {
    left_shift = arm_imm_find_nonzero(imm, left_shift);

    if(left_shift == 32)
    {
      // We've hit the end of the useful data.
      return store_count;
    }

    // Hit the end, it might wrap back around to the beginning.
    if(left_shift >= 24)
    {
      // Make a mask for the residual bits. IE, if we have
      // 5 bits of data at the end we can wrap around to 3
      // bits of data in the beginning. Thus the first
      // thing, after being shifted left, has to be less
      // than 111b, 0x7, or (1 << 3) - 1.
      u32 top_bits = 32 - left_shift;
      u32 residual_bits = 8 - top_bits;
      u32 residual_mask = (1 << residual_bits) - 1;

      if((store_count > 1) && (left_shift > 24) &&
       ((stores[0] << (32 - rotations[0])) < residual_mask))
      {
        // Then we can throw out the last bit and tack it on
        // to the first bit.
        u32 initial_bits = rotations[0];
        stores[0] = (stores[0] << (top_bits + (32 - rotations[0]))) |
         ((imm >> left_shift) & 0xFF);
        rotations[0] = top_bits;

        return store_count;
      }
      else
      {
        // There's nothing to wrap over to in the beginning
        stores[store_count] = (imm >> left_shift) & 0xFF;
        rotations[store_count] = (32 - left_shift) & 0x1F;
        return store_count + 1;
      }
      break;
    }

    stores[store_count] = (imm >> left_shift) & 0xFF;
    rotations[store_count] = (32 - left_shift) & 0x1F;

    store_count++;
    left_shift += 8;
  }
}

#define ror(value, shift)                                                     \
  ((value) >> shift) | ((value) << (32 - shift))                              \

u32 arm_assemble_imm_32bit(u32 *stores, u32 *rotations, u32 store_count)
{
  u32 n = ror(stores[0], rotations[0]);
  u32 i;
  printf("%x : %x\n", stores[0], rotations[0]);

  for(i = 1; i < store_count; i++)
  {
    printf("%x : %x\n", stores[i], rotations[i]);
    n |= ror(stores[i], rotations[i]);
  }

  return n;
}


int main(int argc, char *argv[])
{
  u32 n = 0;
  u32 stores[4];
  u32 rotations[4];
  u32 store_count;
  u32 n2;

  if(argc != 1)
  {
    n = strtoul(argv[1], NULL, 16);
    store_count = arm_disect_imm_32bit(n, stores, rotations);
    n2 = arm_assemble_imm_32bit(stores, rotations, store_count);
    printf("%08x -> %08x (%d stores)\n", n, n2, store_count);
    return 0;
  }

  do
  {
    store_count = arm_disect_imm_32bit(n, stores, rotations);
    n2 = arm_assemble_imm_32bit(stores, rotations, store_count);
    if(n != n2)
    {
      printf("Failure: %08x -/-> %08x\n", n, n2);
      return -1;
    }
    n++;
  } while(n != 0);

  printf("Done!\n");
  return 0;
}
