#include "usr/crypto.h"
#include <string.h>
#include <stdint.h>

#define AES_BLOCK 16

/* Forward declarations */
void usr_aes256_key_expand(const uint8_t key[32], uint8_t round_keys[240]);
void usr_aes256_encrypt_block(uint8_t block[16], const uint8_t round_keys[240]);
void usr_aes256_decrypt_block(uint8_t block[16], const uint8_t round_keys[240]);

static inline void xor16(uint8_t *dst, const uint8_t *a, const uint8_t *b) {
    /* Unrolled 16-byte XOR — compiler will SIMD this with -O2 */
    dst[ 0] = a[ 0] ^ b[ 0]; dst[ 1] = a[ 1] ^ b[ 1];
    dst[ 2] = a[ 2] ^ b[ 2]; dst[ 3] = a[ 3] ^ b[ 3];
    dst[ 4] = a[ 4] ^ b[ 4]; dst[ 5] = a[ 5] ^ b[ 5];
    dst[ 6] = a[ 6] ^ b[ 6]; dst[ 7] = a[ 7] ^ b[ 7];
    dst[ 8] = a[ 8] ^ b[ 8]; dst[ 9] = a[ 9] ^ b[ 9];
    dst[10] = a[10] ^ b[10]; dst[11] = a[11] ^ b[11];
    dst[12] = a[12] ^ b[12]; dst[13] = a[13] ^ b[13];
    dst[14] = a[14] ^ b[14]; dst[15] = a[15] ^ b[15];
}

/* ============================================================
   AES-256-IGE ENCRYPT
   IGE mode (Infinite Garble Extension) as used by Telegram MTProto.

   C_i = AES_enc(P_i XOR C_{i-1}) XOR P_{i-1}
   where C_{-1} = iv[16..31], P_{-1} = iv[0..15]

   iv[0..15]  = previous plaintext (P_{-1})
   iv[16..31] = previous ciphertext (C_{-1})
   ============================================================ */

int usr_aes256_ige_encrypt(
    uint8_t       *data,
    size_t         len,
    const uint8_t  key[32],
    const uint8_t  iv[32]
) {
    if (!data || !key || !iv) return -1;
    if (len == 0 || (len % AES_BLOCK) != 0) return -1;

    uint8_t round_keys[240];
    usr_aes256_key_expand(key, round_keys);

    uint8_t prev_plain[16];   /* P_{i-1} */
    uint8_t prev_cipher[16];  /* C_{i-1} */

    /* iv[0..15]  = P_{-1},  iv[16..31] = C_{-1} */
    memcpy(prev_plain,  iv,      16);
    memcpy(prev_cipher, iv + 16, 16);

    for (size_t i = 0; i < len; i += AES_BLOCK) {
        uint8_t block[16];
        uint8_t plain_save[16];

        /* Save P_i */
        memcpy(plain_save, data + i, AES_BLOCK);

        /* block = P_i XOR C_{i-1} */
        xor16(block, data + i, prev_cipher);

        /* block = AES_enc(block) */
        usr_aes256_encrypt_block(block, round_keys);

        /* C_i = block XOR P_{i-1} */
        xor16(data + i, block, prev_plain);

        /* Update chaining values */
        memcpy(prev_plain,  plain_save, AES_BLOCK);
        memcpy(prev_cipher, data + i,  AES_BLOCK);
    }

    return 0;
}

/* ============================================================
   AES-256-IGE DECRYPT
   P_i = AES_dec(C_i XOR P_{i-1}) XOR C_{i-1}
   ============================================================ */

int usr_aes256_ige_decrypt(
    uint8_t       *data,
    size_t         len,
    const uint8_t  key[32],
    const uint8_t  iv[32]
) {
    if (!data || !key || !iv) return -1;
    if (len == 0 || (len % AES_BLOCK) != 0) return -1;

    uint8_t round_keys[240];
    usr_aes256_key_expand(key, round_keys);

    uint8_t prev_plain[16];   /* P_{i-1} */
    uint8_t prev_cipher[16];  /* C_{i-1} */

    memcpy(prev_plain,  iv,      16);
    memcpy(prev_cipher, iv + 16, 16);

    for (size_t i = 0; i < len; i += AES_BLOCK) {
        uint8_t block[16];
        uint8_t cipher_save[16];

        /* Save C_i */
        memcpy(cipher_save, data + i, AES_BLOCK);

        /* block = C_i XOR P_{i-1} */
        xor16(block, data + i, prev_plain);

        /* block = AES_dec(block) */
        usr_aes256_decrypt_block(block, round_keys);

        /* P_i = block XOR C_{i-1} */
        xor16(data + i, block, prev_cipher);

        /* Update chaining values */
        memcpy(prev_plain,  data + i,    AES_BLOCK);
        memcpy(prev_cipher, cipher_save, AES_BLOCK);
    }

    return 0;
}
