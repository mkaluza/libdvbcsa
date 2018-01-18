/*

    This file is part of libdvbcsa.

    libdvbcsa is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    libdvbcsa is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libdvbcsa; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <time.h>
#include <stdio.h>

#include <dvbcsa/dvbcsa.h>
#include "dvbcsa_pv.h"
#include "dvbcsa_bs.h"

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#define TS_SIZE         184     /* stream size generation */

static void
hexdump (const char *str, const void *data, uint32_t len)
{
  const uint8_t *p = (const uint8_t *)data;
  uint32_t i;

  printf ("- %s - %u bytes\n", str, len);
  for (i = 0; i < len; i++)
    printf ("0x%02x,%c", p[i], (i + 1) % 16 ? ' ' : '\n');
  puts ("");
}
static void
hexdump2 (const char *str, const void *data, uint32_t len, uint8_t row_len)
{
  const uint8_t *p = (const uint8_t *)data;
  uint32_t i;

  printf ("- %s - %u bytes\n", str, len);
  for (i = 0; i < len; i++)
    printf ("0x%02x,%c", p[i], (i + 1) % row_len ? ' ' : '\n');
  puts ("");
}

static int
memtest (uint8_t *data, unsigned int val, unsigned int len)
{
  unsigned int  i;

  for (i = 0; i < len; i++)
    if (data[i] != val)
      return -1;

  return 0;
}

static unsigned int  gs;
static uint8_t       **data;
static unsigned int  *rnd;
static struct        dvbcsa_bs_batch_s *pcks;
static uint8_t       cws[BS_BATCH_BYTES][8];

static int           n_tests, n_failed;

static void init_data(void)
{
  unsigned int i, j;

  //  srand(time(0));

  n_tests = 0;
  n_failed = 0;

  gs = dvbcsa_bs_batch_size();
  rnd = (unsigned int *)malloc(gs * sizeof(unsigned int));
  pcks = (struct dvbcsa_bs_batch_s *)malloc((gs + 1) * sizeof(struct dvbcsa_bs_batch_s));
  data = (uint8_t **)malloc(gs * sizeof(uint8_t *));
#ifdef HAVE_ASSERT_H
  assert(rnd != NULL);
  assert(pcks != NULL);
  assert(data != NULL);
#endif
  for (i = 0; i < gs; i++)
    {
      data[i] = (uint8_t *)malloc(TS_SIZE);
#ifdef HAVE_ASSERT_H
      assert(data[i] != NULL);
#endif
    }

  for (i = 0; i < BS_BATCH_BYTES; i++)
    for (j = 0; j < 8; j++)
      cws[i][j] = 4*i + j;
  hexdump2("CWs", cws, sizeof(cws), 8);
}

static void free_data(void)
{
  unsigned int i;

  for (i = 0; i < gs; i++)
    free(data[i]);
  free(data);
  free(pcks);
  free(rnd);
}

static void
generate_packets(unsigned int minlen, unsigned int maxlen,
                 int random_size, int pattern)
{
  unsigned int i, curlen = minlen;

  printf(" - Generating batch with %i packets, size range: %d-%d%s, pattern: 0x%02x%s\n",
      gs, minlen, maxlen, (random_size)? " (random)" : "", pattern & 0xff,
      (pattern < 0)? " (random)" : "");

  for (i = 0; i < gs; i++)
    {
      if (pattern < 0)
        rnd[i] = rand() & 0xff;
      else
        rnd[i] = pattern & 0xff;

      if (random_size)
        {
          int offs, len;

          len = minlen + rand() % (maxlen - minlen + 1);
          offs = rand() % (TS_SIZE - len + 1);
          pcks[i].data = data[i] + offs;
          pcks[i].len = len;
        }
      else
        {
          if (curlen > maxlen)
            curlen = minlen;
          pcks[i].data = data[i] + (TS_SIZE - curlen);
          pcks[i].len = curlen;
          curlen++;
        }
      memset(pcks[i].data, rnd[i], pcks[i].len);
    }

  pcks[i].data = NULL;
}

static int
run_test(void)
{
  unsigned int i, j;
  int ret = 0;

  struct dvbcsa_bs_key_s *ffkey = dvbcsa_bs_key_alloc();
  struct dvbcsa_key_s *key[BS_BATCH_BYTES];

  for(i = 0; i < BS_BATCH_BYTES; i++) {
    key[i] = dvbcsa_key_alloc();
    dvbcsa_key_set (cws[i], key[i]);

    dvbcsa_bs_key_set_mx (cws[i], ffkey, 4*i, 4);		//FIXME why 4?
  }

#ifdef HAVE_ASSERT_H
  assert(ffkey != NULL);
  assert(key != NULL);
#endif

  /* Bitslice decrypt test */

  printf(" - Encrypting each packet using dvbcsa_encrypt()\n");
  /* assuming BS_BATCH_BYTES streams 8 pkts each*/

  for (i = 0; i < BS_BATCH_BYTES; i++)
    for (j = 0; j < 8; j++)
      dvbcsa_encrypt (key[i], pcks[i+j*BS_BATCH_BYTES].data, pcks[i+j*BS_BATCH_BYTES].len);

  printf(" - Decrypting batch using _bitslice_ dvbcsa_bs_decrypt()\n");

  dvbcsa_bs_decrypt(ffkey, pcks, 184);

  printf(" - Checking results...\n");

  for (i = 0; pcks[i].data; i++)
    {
      if (memtest(pcks[i].data, rnd[i], pcks[i].len))
        {
          printf (" - #%u Failed !\n", i);
          hexdump("failed", pcks[i].data, pcks[i].len);
          ret = -1;
          goto cleanup;
        }
    }

  /* Bitslice encrypt test */

  printf(" - Decrypting each packet using dvbcsa_decrypt()\n");

  for (i = 0; i < BS_BATCH_BYTES; i++)
    for (j = 0; j < 8; j++)
      dvbcsa_encrypt (key[i], pcks[i+j*BS_BATCH_BYTES].data, pcks[i+j*BS_BATCH_BYTES].len);

  printf(" - Encrypting batch using _bitslice_ dvbcsa_bs_encrypt()\n");

  dvbcsa_bs_encrypt(ffkey, pcks, 184);

  printf(" - Checking results...\n");

  for (i = 0; pcks[i].data; i++)
    {
      if (memtest(pcks[i].data, rnd[i], pcks[i].len))
        {
          puts (" - Failed !");
          hexdump("failed", pcks[i].data, pcks[i].len);
          ret = -1;
          goto cleanup;
        }
    }

  puts (" - Ok !");

cleanup:
  for(i = 0; i < BS_BATCH_BYTES; i++)
    dvbcsa_key_free(key[i]);
  dvbcsa_bs_key_free(ffkey);

  if (ret != 0)
    n_failed++;

  n_tests++;

  return ret;
}

int
main (void)
{
  unsigned int minl;

  printf("* DVBCSA test *\n");

  init_data();

  generate_packets(TS_SIZE, TS_SIZE, 0, 0xa5);
  run_test();

  generate_packets(100, TS_SIZE, 1, -1);
  run_test();

  for (minl = 0; minl < TS_SIZE; minl += gs)
    {
      unsigned int maxl = minl + gs - 1;
      if (maxl > TS_SIZE)
        maxl = TS_SIZE;

      generate_packets(minl, maxl, 0, -1);
      run_test();
    }

  free_data();

  printf("=======================\n");
  if (n_failed)
    printf("%d out of %d tests FAILED.\n", n_failed, n_tests);
  else
    printf("OK! All tests passed.\n");

  return (n_failed != 0);
}

