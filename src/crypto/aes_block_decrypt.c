#include "aes_tables.h"
#include "usr/crypto.h"
#include <stdint.h>
#include <string.h>

extern void usr_aes_tables_init(void);

/* ============================================================
   AES-256 Decrypt Block — Full 14-round Inverse Cipher
   ============================================================ */

#define AES256_ROUNDS 14

/* Inverse SubBytes: apply inv_sbox to each byte */
static void inv_sub_bytes(uint8_t s[16]) {
    for (int i = 0; i < 16; i++) s[i] = inv_sbox[s[i]];
}

/*
 * Inverse ShiftRows:
 *   Row 0: no shift
 *   Row 1: right 1
 *   Row 2: right 2
 *   Row 3: right 3
 *
 * Column-major layout (state[col*4+row]):
 *   s[0] s[4] s[8]  s[12]   row 0
 *   s[1] s[5] s[9]  s[13]   row 1
 *   s[2] s[6] s[10] s[14]   row 2
 *   s[3] s[7] s[11] s[15]   row 3
 */
static void inv_shift_rows(uint8_t s[16]) {
    uint8_t t;
    /* Row 1: right 1 */
    t = s[13]; s[13] = s[9]; s[9] = s[5]; s[5] = s[1]; s[1] = t;
    /* Row 2: right 2 */
    t = s[2]; s[2] = s[10]; s[10] = t;
    t = s[6]; s[6] = s[14]; s[14] = t;
    /* Row 3: right 3 (= left 1) */
    t = s[3]; s[3] = s[7]; s[7] = s[11]; s[11] = s[15]; s[15] = t;
}

/*
 * Inverse MixColumns.
 * Uses precomputed mul9, mul11, mul13, mul14 tables from aes_tables.c.
 *
 * Matrix:
 *   14  11  13   9
 *    9  14  11  13
 *   13   9  14  11
 *   11  13   9  14
 */
static void inv_mix_columns(uint8_t s[16]) {
    for (int col = 0; col < 4; col++) {
        uint8_t *c = s + col * 4;
        uint8_t a0 = c[0], a1 = c[1], a2 = c[2], a3 = c[3];
        c[0] = mul14[a0] ^ mul11[a1] ^ mul13[a2] ^  mul9[a3];
        c[1] =  mul9[a0] ^ mul14[a1] ^ mul11[a2] ^ mul13[a3];
        c[2] = mul13[a0] ^  mul9[a1] ^ mul14[a2] ^ mul11[a3];
        c[3] = mul11[a0] ^ mul13[a1] ^  mul9[a2] ^ mul14[a3];
    }
}

static void add_round_key(uint8_t s[16], const uint8_t *rk) {
    for (int i = 0; i < 16; i++) s[i] ^= rk[i];
}

/* ============================================================
   Full AES-256 Decrypt Block
   Equivalent Inverse Cipher (AES standard FIPS 197 §5.3)
   ============================================================ */

void usr_aes256_decrypt_block(uint8_t block[16], const uint8_t rk[240]) {
    usr_aes_tables_init();

    /* Initial AddRoundKey with last round key */
    add_round_key(block, rk + AES256_ROUNDS * 16);

    /* Rounds 13 down to 1 */
    for (int r = AES256_ROUNDS - 1; r >= 1; r--) {
        inv_shift_rows(block);
        inv_sub_bytes(block);
        add_round_key(block, rk + r * 16);
        inv_mix_columns(block);
    }

    /* Final round (no InvMixColumns) */
    inv_shift_rows(block);
    inv_sub_bytes(block);
    add_round_key(block, rk);
}
