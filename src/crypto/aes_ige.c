#include "usr/crypto.h"
#include <string.h>
#include <stdint.h>

/* AES block size */
#define AES_BLOCK 16

/* Forward declarations from aes_block.c */
void usr_aes256_key_expand(const uint8_t key[32], uint8_t round_keys[240]);
void usr_aes256_encrypt_block(uint8_t block[16], const uint8_t round_keys[240]);
void usr_aes256_decrypt_block(uint8_t block[16], const uint8_t round_keys[240]); /* future */

/* ===============================
   Helpers
   =============================== */

static inline void xor16(uint8_t *dst, const uint8_t *src) {
    for (int i = 0; i < 16; i++) {
        dst[i] ^= src[i];
    }
}

/* ===============================
   AES-256-IGE ENCRYPT
   =============================== */

void usr_aes256_ige_encrypt(
    uint8_t *data,
    size_t len,
    const uint8_t key[32],
    const uint8_t iv[32]
) {
    if (!data || !key || !iv) return;
    if (len == 0 || (len % AES_BLOCK) != 0) return;

    uint8_t round_keys[240];
    usr_aes256_key_expand(key, round_keys);

    uint8_t prev_plain[16];
    uint8_t prev_cipher[16];

    /* IV is split into two 16-byte blocks */
    memcpy(prev_plain, iv, 16);
    memcpy(prev_cipher, iv + 16, 16);

    for (size_t i = 0; i < len; i += 16) {
        uint8_t block[16];

        /* P_i */
        memcpy(block, data + i, 16);

        /* P_i ⊕ C_{i-1} */
        xor16(block, prev_cipher);

        /* AES encrypt */
        usr_aes256_encrypt_block(block, round_keys);

        /* AES(P_i ⊕ C_{i-1}) ⊕ P_{i-1} */
        xor16(block, prev_plain);

        /* Update chaining values */
        memcpy(prev_plain, data + i, 16);   /* P_i */
        memcpy(prev_cipher, block, 16);     /* C_i */

        /* Write ciphertext */
        memcpy(data + i, block, 16);
    }
}

/* ===============================
   AES-256-IGE DECRYPT
   =============================== */

void usr_aes256_ige_decrypt(
    uint8_t *data,
    size_t len,
    const uint8_t key[32],
    const uint8_t iv[32]
) {
    if (!data || !key || !iv) return;
    if (len == 0 || (len % AES_BLOCK) != 0) return;

    uint8_t round_keys[240];
    usr_aes256_key_expand(key, round_keys);

    uint8_t prev_plain[16];
    uint8_t prev_cipher[16];

    memcpy(prev_plain, iv, 16);
    memcpy(prev_cipher, iv + 16, 16);

    for (size_t i = 0; i < len; i += 16) {
        uint8_t cur_cipher[16];
        uint8_t block[16];

        /* C_i */
        memcpy(cur_cipher, data + i, 16);
        memcpy(block, cur_cipher, 16);

        /* C_i ⊕ P_{i-1} */
        xor16(block, prev_plain);

        /* AES decrypt */
        usr_aes256_decrypt_block(block, round_keys);

        /* AES⁻¹(C_i ⊕ P_{i-1}) ⊕ C_{i-1} */
        xor16(block, prev_cipher);

        /* Update chaining */
        memcpy(prev_plain, block, 16);      /* P_i */
        memcpy(prev_cipher, cur_cipher, 16);/* C_i */

        /* Write plaintext */
        memcpy(data + i, block, 16);
    }
}
