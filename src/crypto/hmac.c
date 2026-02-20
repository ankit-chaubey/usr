#include "usr/crypto.h"
#include <string.h>
#include <stdint.h>

/* ============================================================
   HMAC-SHA256
   RFC 2104: HMAC = H((K XOR opad) || H((K XOR ipad) || message))
   ============================================================ */

void usr_hmac_sha256_init(usr_hmac_sha256_ctx *ctx,
                           const uint8_t *key, size_t key_len)
{
    uint8_t k[64];
    memset(k, 0, sizeof(k));

    /* If key > block size, hash it first */
    if (key_len > 64) {
        usr_sha256(key, key_len, k);
    } else {
        memcpy(k, key, key_len);
    }

    uint8_t ipad[64], opad[64];
    for (int i = 0; i < 64; i++) {
        ipad[i] = k[i] ^ 0x36;
        opad[i] = k[i] ^ 0x5c;
    }

    usr_sha256_init(&ctx->inner);
    usr_sha256_update(&ctx->inner, ipad, 64);

    usr_sha256_init(&ctx->outer);
    usr_sha256_update(&ctx->outer, opad, 64);
}

void usr_hmac_sha256_update(usr_hmac_sha256_ctx *ctx,
                             const uint8_t *data, size_t len)
{
    usr_sha256_update(&ctx->inner, data, len);
}

void usr_hmac_sha256_final(usr_hmac_sha256_ctx *ctx, uint8_t out[32]) {
    uint8_t inner_hash[32];
    usr_sha256_final(&ctx->inner, inner_hash);
    usr_sha256_update(&ctx->outer, inner_hash, 32);
    usr_sha256_final(&ctx->outer, out);
    memset(inner_hash, 0, sizeof(inner_hash));
}

void usr_hmac_sha256(const uint8_t *key, size_t key_len,
                     const uint8_t *data, size_t data_len,
                     uint8_t out[32])
{
    usr_hmac_sha256_ctx ctx;
    usr_hmac_sha256_init(&ctx, key, key_len);
    usr_hmac_sha256_update(&ctx, data, data_len);
    usr_hmac_sha256_final(&ctx, out);
}

/* ============================================================
   PBKDF2-HMAC-SHA256
   RFC 2898 §5.2
   ============================================================ */

int usr_pbkdf2_sha256(
    const uint8_t *password, size_t password_len,
    const uint8_t *salt,     size_t salt_len,
    uint32_t       iterations,
    uint8_t       *out,      size_t out_len
) {
    if (!password || !salt || !out) return -1;
    if (iterations == 0 || out_len == 0) return -1;

    /* PBKDF2 block count */
    size_t num_blocks = (out_len + 31) / 32;  /* ceil(out_len / hLen) */

    for (size_t block = 1; block <= num_blocks; block++) {
        uint8_t U[32], T[32];

        /* U1 = HMAC(password, salt || INT(block)) */
        uint8_t block_be[4] = {
            (uint8_t)(block >> 24),
            (uint8_t)(block >> 16),
            (uint8_t)(block >>  8),
            (uint8_t)(block      )
        };

        usr_hmac_sha256_ctx ctx;
        usr_hmac_sha256_init(&ctx, password, password_len);
        usr_hmac_sha256_update(&ctx, salt, salt_len);
        usr_hmac_sha256_update(&ctx, block_be, 4);
        usr_hmac_sha256_final(&ctx, U);

        memcpy(T, U, 32);

        /* Iterations 2..c */
        for (uint32_t i = 1; i < iterations; i++) {
            usr_hmac_sha256(password, password_len, U, 32, U);
            for (int j = 0; j < 32; j++) T[j] ^= U[j];
        }

        /* Copy T to output (possibly partial for last block) */
        size_t offset = (block - 1) * 32;
        size_t copy   = (out_len - offset < 32) ? (out_len - offset) : 32;
        memcpy(out + offset, T, copy);
    }

    return 0;
}
