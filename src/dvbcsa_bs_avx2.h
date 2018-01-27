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

#ifndef DVBCSA_AVX2_H_
#define DVBCSA_AVX2_H_

#include <immintrin.h>

typedef __m256i dvbcsa_bs_word_t;

#define BS_BATCH_SIZE 256
#define BS_BATCH_BYTES 32

#define BS_VAL(n, m)    _mm256_set_epi64x(n, m, n, m)
#define BS_VAL64(n)     BS_VAL(0x##n##ULL, 0x##n##ULL)
#define BS_VAL32(n)     BS_VAL64(n##n)
#define BS_VAL16(n)     BS_VAL32(n##n)
#define BS_VAL8(n)      BS_VAL16(n##n)

#define BS_AND(a, b)    _mm256_and_si256((a), (b))
#define BS_OR(a, b)     _mm256_or_si256((a), (b))
#define BS_XOR(a, b)    _mm256_xor_si256((a), (b))
#define BS_NOT(a)       _mm256_andnot_si256((a), BS_VAL8(ff))

#define BS_SHL(a, n)    _mm256_slli_epi64(a, n)
#define BS_SHR(a, n)    _mm256_srli_epi64(a, n)

#define BS_EXTRACT8(a, n) ((dvbcsa_u8_aliasing_t *)&(a))[n]

#define BS_EMPTY()

/* AVX2+ specific macros */
#define BS_SHL32(a, n)  _mm256_slli_epi32(a, n)
#define BS_SHR32(a, n)  _mm256_srli_epi32(a, n)

#define BS_GATHER32(p, i, s)     _mm256_i32gather_epi32((p), (i), (s))
#define BS_SHUF8(r, m)           _mm256_shuffle_epi8((r), (m))

#define BS_LOAD(p)               _mm256_load_si256((p))
#define BS_STORE(p, v)           _mm256_store_si256((p), (v))
#define BS_UNPACKLO16(a, b)      _mm256_unpacklo_epi16((a), (b))
#define BS_UNPACKHI16(a, b)      _mm256_unpackhi_epi16((a), (b))

#include "dvbcsa_bs_avx_common.h"

#endif

