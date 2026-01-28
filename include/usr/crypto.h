#ifndef USR_CRYPTO_H
#define USR_CRYPTO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================
   SHA-256
   ========================= */

void usr_sha256(
    const uint8_t *data,
    size_t len,
    uint8_t out[32]
);

/* =========================
   AES-256-IGE
   ========================= */

void usr_aes256_ige_encrypt(
    uint8_t *data,
    size_t len,
    const uint8_t key[32],
    const uint8_t iv[32]
);

void usr_aes256_ige_decrypt(
    uint8_t *data,
    size_t len,
    const uint8_t key[32],
    const uint8_t iv[32]
);

#ifdef __cplusplus
}
#endif

#endif
