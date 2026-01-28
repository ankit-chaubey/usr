#include "aes_tables.h"
#include <stdint.h>
#include <string.h>
#include "aes_tables.h"

#define AES_BLOCK 16
#define AES_ROUNDS 14

/* =======================
   AES CONSTANTS
   ======================= */


static const uint8_t rcon[15] = {
  0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8,0xAB,0x4D
};

/* =======================
   AES HELPERS
   ======================= */

static uint8_t xtime(uint8_t x) {
    return (x << 1) ^ ((x >> 7) * 0x1b);
}

static void sub_bytes(uint8_t *s) {
    for (int i = 0; i < 16; i++) s[i] = sbox[s[i]];
}

static void shift_rows(uint8_t *s) {
    uint8_t t[16];
    memcpy(t, s, 16);

    s[0]=t[0];  s[4]=t[4];  s[8]=t[8];   s[12]=t[12];
    s[1]=t[5];  s[5]=t[9];  s[9]=t[13];  s[13]=t[1];
    s[2]=t[10]; s[6]=t[14]; s[10]=t[2];  s[14]=t[6];
    s[3]=t[15]; s[7]=t[3];  s[11]=t[7];  s[15]=t[11];
}

static void mix_columns(uint8_t *s) {
    for (int i = 0; i < 4; i++) {
        uint8_t *c = s + i*4;
        uint8_t a = c[0], b = c[1], d = c[2], e = c[3];
        uint8_t t = a ^ b ^ d ^ e;
        c[0] ^= t ^ xtime(a ^ b);
        c[1] ^= t ^ xtime(b ^ d);
        c[2] ^= t ^ xtime(d ^ e);
        c[3] ^= t ^ xtime(e ^ a);
    }
}

static void add_round_key(uint8_t *s, const uint8_t *rk) {
    for (int i = 0; i < 16; i++) s[i] ^= rk[i];
}

/* =======================
   KEY EXPANSION (AES-256)
   ======================= */

void usr_aes256_key_expand(const uint8_t key[32], uint8_t round_keys[240]) {
    memcpy(round_keys, key, 32);
    uint8_t tmp[4];
    int i = 8, r = 1;

    while (i < 60) {
        memcpy(tmp, &round_keys[(i - 1) * 4], 4);

        if (i % 8 == 0) {
            uint8_t t = tmp[0];
            tmp[0] = sbox[tmp[1]] ^ rcon[r++];
            tmp[1] = sbox[tmp[2]];
            tmp[2] = sbox[tmp[3]];
            tmp[3] = sbox[t];
        } else if (i % 8 == 4) {
            for (int j = 0; j < 4; j++)
                tmp[j] = sbox[tmp[j]];
        }

        for (int j = 0; j < 4; j++)
            round_keys[i*4 + j] =
                round_keys[(i - 8)*4 + j] ^ tmp[j];
        i++;
    }
}

/* =======================
   AES-256 ENCRYPT BLOCK
   ======================= */

void usr_aes256_encrypt_block(uint8_t block[16], const uint8_t rk[240]) {
    add_round_key(block, rk);

    for (int r = 1; r < AES_ROUNDS; r++) {
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, rk + r*16);
    }

    sub_bytes(block);
    shift_rows(block);
    add_round_key(block, rk + AES_ROUNDS*16);
}
