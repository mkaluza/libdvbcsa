#ifndef _AVX_COMMON_H
#define _AVX_COMMON_H

/* BS_LOAD_DEINTERLEAVE_8 stub: deinterleaving is done by BLOCK_SBOX_PERMUTE */
#define BS_LOAD_DEINTERLEAVE_8(ptr, var_lo, var_hi) \
      {\
      var_lo = BS_LOAD((ptr)); \
      var_hi = BS_LOAD((ptr) + 1); \
      }

static const uint16_t dvbcsa_block_sbox_perm[256];

static inline void avx_block_sbox_permute(dvbcsa_bs_word_t *src,
                                           dvbcsa_bs_word_t *dst)
{
  int j;
  dvbcsa_bs_word_t a, i, b, res1, res2;
  dvbcsa_bs_word_t lsb_mask = BS_VAL32(000000ff);
  dvbcsa_bs_word_t lsw_mask = BS_VAL32(0000ffff);
  dvbcsa_bs_word_t shuffle_mask = BS_VAL(0x0f0d0b0907050301ULL, \
                                         0x0e0c0a0806040200ULL);
  /* 15,13,11,9,7,5,3,1,  14,12,10,8,6,4,2,0); */
  for (j = 0; j < 8; j++)
    {
      i = BS_LOAD(src + j);

      a = BS_AND(i, lsb_mask);
      b = BS_GATHER32((const int *)dvbcsa_block_sbox_perm, a, 2);
      res1 = BS_AND(b, lsw_mask);

      i = BS_SHR32(i, 8);
      a = BS_AND(i, lsb_mask);
      b = BS_GATHER32((const int *)dvbcsa_block_sbox_perm, a, 2);
      b = BS_SHL32(b, 16);
      res1 = BS_OR(res1, b);

      i = BS_SHR32(i, 8);
      a = BS_AND(i, lsb_mask);
      b = BS_GATHER32((const int *)dvbcsa_block_sbox_perm, a, 2);
      res2 = BS_AND(b, lsw_mask);

      i = BS_SHR32(i, 8);
      a = BS_AND(i, lsb_mask);
      b = BS_GATHER32((const int *)dvbcsa_block_sbox_perm, a, 2);
      b = BS_SHL32(b, 16);
      res2 = BS_OR(res2, b);

      a = BS_SHUF8(res1, shuffle_mask);
      b = BS_SHUF8(res2, shuffle_mask);
      res1 = BS_UNPACKLO16(a, b);
      res2 = BS_UNPACKHI16(a, b);

      BS_STORE(dst + 2 * j, res1);
      BS_STORE(dst + 2 * j + 1, res2);
    }
}

#define BLOCK_SBOX_PERMUTE(in_buf, out_buf) avx_block_sbox_permute(in_buf, out_buf);
#endif
