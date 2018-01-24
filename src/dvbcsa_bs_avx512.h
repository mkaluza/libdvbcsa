/*

    This file is part of libdvbcsa.

    libdvbcsa is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation; either version 2 of the License,
    or (at your option) any later version.

    libdvbcsa is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libdvbcsa; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA

    Based on FFdecsa, Copyright (C) 2003-2004  fatih89r

    (c) 2006-2008 Alexandre Becoulet <alexandre.becoulet@free.fr>

*/

#ifndef DVBCSA_AVX_H_
#define DVBCSA_AVX_H_

#include <immintrin.h>

typedef __m512i dvbcsa_bs_word_t;

#define BS_BATCH_SIZE 512
#define BS_BATCH_BYTES 64

#define BS_VAL(n, m)    _mm512_set_epi64(n, m, n, m, n, m, n, m)
#define BS_VAL64(n)     BS_VAL(0x##n##ULL, 0x##n##ULL)
#define BS_VAL32(n)     BS_VAL64(n##n)
#define BS_VAL16(n)     BS_VAL32(n##n)
#define BS_VAL8(n)      BS_VAL16(n##n)

#define BS_AND(a, b)    _mm512_and_si512((a), (b))
#define BS_OR(a, b)     _mm512_or_si512((a), (b))
#define BS_XOR(a, b)    _mm512_xor_si512((a), (b))
#define BS_NOT(a)       _mm512_andnot_si512((a), BS_VAL8(ff))

#define BS_SHL(a, n)    _mm512_slli_epi64(a, n)
#define BS_SHR(a, n)    _mm512_srli_epi64(a, n)

#define BS_EXTRACT8(a, n) ((dvbcsa_u8_aliasing_t *)&(a))[n]

#define BS_EMPTY()

/* block cipher 2-word load with byte-deinterleaving */
/*
#define BS_LOAD_DEINTERLEAVE_8(ptr, var_lo, var_hi) \
      {\
      dvbcsa_bs_word_t a, b; \
      a = _mm256_load_si256((ptr)); \
      b = _mm256_load_si256((ptr) + 1); \
      a = _mm256_shuffle_epi8(a, _mm256_set_epi8(15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0)); \
      b = _mm256_shuffle_epi8(b, _mm256_set_epi8(15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0)); \
      var_lo = _mm256_unpacklo_epi64(a, b); \
      var_hi = _mm256_unpackhi_epi64(a, b); \
      var_lo = _mm256_permute4x64_epi64(var_lo, 0xD8); \
      var_hi = _mm256_permute4x64_epi64(var_hi, 0xD8); \
      }
*/

static inline void block_sbox_avx1(dvbcsa_bs_word_t *src, dvbcsa_bs_word_t *dst) {
}

static inline void block_sbox_avx2(dvbcsa_bs_word_t *src, dvbcsa_bs_word_t *dst) {
}

static inline void block_sbox_avx3(dvbcsa_bs_word_t *src, dvbcsa_bs_word_t *dst) {
}

#define BLOCK_SBOX(in_buf, out_buf) block_sbox_avx2(in_buf, out_buf);

// BS_LOAD_DEINTERLEAVE_8 replacement that only loads data that are already uninterleaved

#define BS_LOAD_DEINTERLEAVE_8(ptr, var_lo, var_hi) \
      {\
      var_lo = _mm512_load_si512((ptr)); \
      var_hi = _mm512_load_si512((ptr) + 1); \
      }

#ifndef DVBCSA_AVX_USE_WIDE_LUT
#define BLOCK_SBOX_PERMUTE_LOOP_ITEM(i, sbox_out, perm_out) \
{ \
	dvbcsa_bs_word_t a, b; \
	dvbcsa_bs_word_t lsb_mask = _mm512_set_epi32(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff); \
	dvbcsa_bs_word_t lsw_mask = _mm512_set_epi32(0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff); \
	/* part 1 */ \
	a = BS_AND(i, lsb_mask); \
	b = _mm512_i32gather_epi32(a, (const int *)dvbcsa_block_sbox_perm, 2); \
	sbox_out = BS_AND(b, lsw_mask); \
 \
	i = _mm512_srli_epi32(i, 8); \
	a = BS_AND(i, lsb_mask); \
	b = _mm512_i32gather_epi32(a, (const int *)dvbcsa_block_sbox_perm, 2); \
	b = _mm512_slli_epi32(b, 16); \
	sbox_out = BS_OR(sbox_out, b); \
 \
	/* part 2 */ \
	i = _mm512_srli_epi32(i, 8); \
	a = BS_AND(i, lsb_mask); \
	b = _mm512_i32gather_epi32(a, (const int *)dvbcsa_block_sbox_perm, 2); \
	perm_out = BS_AND(b, lsw_mask); \
 \
	i = _mm512_srli_epi32(i, 8); \
	a = BS_AND(i, lsb_mask); \
	b = _mm512_i32gather_epi32(a, (const int *)dvbcsa_block_sbox_perm, 2); \
	b = _mm512_slli_epi32(b, 16); \
	perm_out = BS_OR(perm_out, b); \
 \
	/* unpack */ \
	/* FIXME gcc 6.3 doesn't seem to support _mm512_set_epi8 \
	a = _mm512_shuffle_epi8(sbox_out, _mm512_set_epi8(15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0)); \
	b = _mm512_shuffle_epi8(perm_out, _mm512_set_epi8(15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0)); \
	*/ \
	a = _mm512_shuffle_epi8(sbox_out, _mm512_set_epi64(0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200)); \
	b = _mm512_shuffle_epi8(perm_out, _mm512_set_epi64(0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200)); \
	sbox_out = _mm512_unpacklo_epi16(a, b); \
	perm_out = _mm512_unpackhi_epi16(a, b); \
}

extern const uint16_t dvbcsa_block_sbox_perm[256];
#else
#define BLOCK_SBOX_PERMUTE_LOOP_ITEM(i, sbox_out, perm_out) \
{ \
	dvbcsa_bs_word_t a, b; \
	dvbcsa_bs_word_t lsw_mask = _mm512_set_epi32(0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff); \
	/* part 1 */ \
	a = BS_AND(i, lsw_mask); \
	sbox_out = _mm512_i32gather_epi32(a, (const int *)dvbcsa_block_sbox_perm_w, 4); \
 \
	/* part 2 */ \
	a = _mm512_srli_epi32(i, 16); \
	perm_out = _mm512_i32gather_epi32(a, (const int *)dvbcsa_block_sbox_perm, 2); \
 \
	/* unpack */ \
	/* FIXME gcc 6.3 doesn't seem to support _mm512_set_epi8 \
	a = _mm512_shuffle_epi8(sbox_out, _mm512_set_epi8(15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0)); \
	b = _mm512_shuffle_epi8(perm_out, _mm512_set_epi8(15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0, 15,13,11,9,7,5,3,1, 14,12,10,8,6,4,2,0)); \
	*/ \
	a = _mm512_shuffle_epi8(sbox_out, _mm512_set_epi64(0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200)); \
	b = _mm512_shuffle_epi8(perm_out, _mm512_set_epi64(0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200)); \
	sbox_out = _mm512_unpacklo_epi16(a, b); \
	perm_out = _mm512_unpackhi_epi16(a, b); \
}
extern const uint32_t dvbcsa_block_sbox_perm_w[256*256];
#endif
static inline void block_sbox_permute_interleave_avx(dvbcsa_bs_word_t *src, dvbcsa_bs_word_t *dst) {
	int j;
	dvbcsa_bs_word_t i, res1, res2;
	for (j = 0; j < 8; j++) {
		i = _mm512_load_si512(src + j);

		BLOCK_SBOX_PERMUTE_LOOP_ITEM(i, res1, res2)

		_mm512_store_si512(dst + 2*j, res1);
		_mm512_store_si512(dst + 2*j + 1, res2);
	}
}

#define BLOCK_SBOX_PERMUTE(in_buf, out_buf) block_sbox_permute_interleave_avx(in_buf, out_buf);

#endif

