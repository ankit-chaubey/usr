#include "usr/crypto.h"
#include <string.h>
#include <stdint.h>

#define AES_BLOCK 16

void usr_aes256_key_expand(const uint8_t key[32], uint8_t round_keys[240]);
void usr_aes256_encrypt_block(uint8_t block[16], const uint8_t round_keys[240]);
void usr_aes256_decrypt_block(uint8_t block[16], const uint8_t round_keys[240]);

static inline void xor16(uint8_t *a, const uint8_t *b) {
    for (int i = 0; i < 16; i++) a[i] ^= b[i];
}

/* ============================================================
   AES-256-CBC Encrypt with PKCS#7 padding
   ============================================================ */

int usr_aes256_cbc_encrypt(
    const uint8_t *in,  size_t in_len,
    const uint8_t  key[32],
    const uint8_t  iv[16],
    uint8_t       *out, size_t *out_len
) {
    if (!key || !iv || !out_len) return -1;

    /* Padded length: always add a full padding block if already aligned */
    size_t pad_len = AES_BLOCK - (in_len % AES_BLOCK);
    size_t total   = in_len + pad_len;

    if (!out) {
        *out_len = total;
        return 0;
    }
    if (*out_len < total) { *out_len = total; return -1; }

    uint8_t round_keys[240];
    usr_aes256_key_expand(key, round_keys);

    uint8_t prev[16];
    memcpy(prev, iv, 16);

    /* Copy plaintext and apply PKCS#7 padding */
    uint8_t padded[4096]; /* use heap for large inputs */
    uint8_t *buf = padded;
    if (total > sizeof(padded)) {
        /* Can't stack-allocate; caller should use streaming API.
           For now, handle inline with pointer arithmetic. */
        memcpy(out, in, in_len);
        uint8_t pad = (uint8_t)pad_len;
        for (size_t i = 0; i < pad_len; i++) out[in_len + i] = pad;
        buf = out;
    } else {
        memcpy(buf, in ? in : (const uint8_t *)"", in_len);
        uint8_t pad = (uint8_t)pad_len;
        for (size_t i = 0; i < pad_len; i++) buf[in_len + i] = pad;
    }

    for (size_t i = 0; i < total; i += AES_BLOCK) {
        uint8_t block[16];
        memcpy(block, buf + i, AES_BLOCK);
        xor16(block, prev);
        usr_aes256_encrypt_block(block, round_keys);
        memcpy(out + i, block, AES_BLOCK);
        memcpy(prev, block, AES_BLOCK);
    }

    *out_len = total;
    return 0;
}

/* ============================================================
   AES-256-CBC Decrypt with PKCS#7 unpadding
   ============================================================ */

int usr_aes256_cbc_decrypt(
    const uint8_t *in,  size_t in_len,
    const uint8_t  key[32],
    const uint8_t  iv[16],
    uint8_t       *out, size_t *out_len
) {
    if (!in || !key || !iv || !out_len) return -1;
    if (in_len == 0 || (in_len % AES_BLOCK) != 0) return -1;
    if (!out) { *out_len = in_len; return 0; }
    if (*out_len < in_len) { *out_len = in_len; return -1; }

    uint8_t round_keys[240];
    usr_aes256_key_expand(key, round_keys);

    uint8_t prev[16];
    memcpy(prev, iv, 16);

    for (size_t i = 0; i < in_len; i += AES_BLOCK) {
        uint8_t block[16];
        memcpy(block, in + i, AES_BLOCK);
        usr_aes256_decrypt_block(block, round_keys);
        xor16(block, prev);
        memcpy(out + i, block, AES_BLOCK);
        memcpy(prev, in + i, AES_BLOCK);
    }

    /* Verify and strip PKCS#7 padding */
    uint8_t pad = out[in_len - 1];
    if (pad == 0 || pad > AES_BLOCK) return -1;
    for (size_t i = in_len - pad; i < in_len; i++) {
        if (out[i] != pad) return -1;
    }

    *out_len = in_len - pad;
    return 0;
}
