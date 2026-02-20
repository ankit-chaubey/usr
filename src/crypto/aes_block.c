#include "aes_tables.h"
#include "usr/crypto.h"
#include <stdint.h>
#include <string.h>

extern void usr_aes_tables_init(void);

/* AES-256: 14 rounds, 15 round keys (240 bytes) */
#define AES256_ROUNDS    14
#define AES256_RK_WORDS  60   /* 15 round keys * 4 words each */

/* Rcon values for AES-256 key schedule (indices 1..7 are used) */
static const uint8_t rcon[15] = {
    0x00, /* unused (index 0) */
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,  /* indices 1..7 */
    0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D   /* extras, not needed for AES-256 */
};

/* ============================================================
   AES-256 Key Schedule
   Produces 240 bytes = 60 × 32-bit words = 15 round keys
   ============================================================ */

void usr_aes256_key_expand(const uint8_t key[32], uint8_t round_keys[240]) {
    usr_aes_tables_init();

    /* Copy the original key as the first 8 words (32 bytes) */
    memcpy(round_keys, key, 32);

    uint8_t tmp[4];
    int rcon_idx = 1;

    for (int i = 8; i < AES256_RK_WORDS; i++) {
        /* tmp = previous word */
        memcpy(tmp, &round_keys[(i - 1) * 4], 4);

        if (i % 8 == 0) {
            /* RotWord: rotate left by 1 byte */
            uint8_t t = tmp[0];
            tmp[0] = sbox[tmp[1]] ^ rcon[rcon_idx++];
            tmp[1] = sbox[tmp[2]];
            tmp[2] = sbox[tmp[3]];
            tmp[3] = sbox[t];
        } else if (i % 8 == 4) {
            /* SubWord: apply S-box to each byte */
            tmp[0] = sbox[tmp[0]];
            tmp[1] = sbox[tmp[1]];
            tmp[2] = sbox[tmp[2]];
            tmp[3] = sbox[tmp[3]];
        }

        /* XOR with word 8 positions back */
        for (int j = 0; j < 4; j++) {
            round_keys[i * 4 + j] = round_keys[(i - 8) * 4 + j] ^ tmp[j];
        }
    }
}

/* ============================================================
   AES State helpers
   The AES state is stored column-major:
   state[col*4 + row], col=0..3, row=0..3
   ============================================================ */

static void sub_bytes(uint8_t s[16]) {
    for (int i = 0; i < 16; i++) s[i] = sbox[s[i]];
}

/* ShiftRows (row 0: no shift, row 1: left 1, row 2: left 2, row 3: left 3)
   Column-major layout:
     s[0] s[4] s[8]  s[12]   <- row 0
     s[1] s[5] s[9]  s[13]   <- row 1
     s[2] s[6] s[10] s[14]   <- row 2
     s[3] s[7] s[11] s[15]   <- row 3
*/
static void shift_rows(uint8_t s[16]) {
    uint8_t t;
    /* Row 1: left 1 */
    t = s[1]; s[1] = s[5]; s[5] = s[9]; s[9] = s[13]; s[13] = t;
    /* Row 2: left 2 */
    t = s[2]; s[2] = s[10]; s[10] = t;
    t = s[6]; s[6] = s[14]; s[14] = t;
    /* Row 3: left 3 (= right 1) */
    t = s[15]; s[15] = s[11]; s[11] = s[7]; s[7] = s[3]; s[3] = t;
}

/* MixColumns using precomputed mul2, mul3 tables */
static void mix_columns(uint8_t s[16]) {
    for (int col = 0; col < 4; col++) {
        uint8_t *c = s + col * 4;
        uint8_t a0 = c[0], a1 = c[1], a2 = c[2], a3 = c[3];
        c[0] = mul2[a0] ^ mul3[a1] ^      a2  ^      a3;
        c[1] =      a0  ^ mul2[a1] ^ mul3[a2] ^      a3;
        c[2] =      a0  ^      a1  ^ mul2[a2] ^ mul3[a3];
        c[3] = mul3[a0] ^      a1  ^      a2  ^ mul2[a3];
    }
}

static void add_round_key(uint8_t s[16], const uint8_t *rk) {
    for (int i = 0; i < 16; i++) s[i] ^= rk[i];
}

/* ============================================================
   AES-256 Encrypt Block
   ============================================================ */

void usr_aes256_encrypt_block(uint8_t block[16], const uint8_t rk[240]) {
    /* Initial AddRoundKey */
    add_round_key(block, rk);

    /* Rounds 1..13 */
    for (int r = 1; r < AES256_ROUNDS; r++) {
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, rk + r * 16);
    }

    /* Final round (no MixColumns) */
    sub_bytes(block);
    shift_rows(block);
    add_round_key(block, rk + AES256_ROUNDS * 16);
}
