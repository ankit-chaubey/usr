#ifndef USR_CRYPTO_H
#define USR_CRYPTO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   SHA-256
   ============================================================ */

#define USR_SHA256_DIGEST_SIZE 32
#define USR_SHA256_BLOCK_SIZE  64

typedef struct {
    uint32_t state[8];
    uint8_t  buf[64];
    uint64_t bitcount;
    uint32_t buflen;
} usr_sha256_ctx;

/* One-shot SHA-256 */
void usr_sha256(const uint8_t *data, size_t len, uint8_t out[32]);

/* Streaming SHA-256 */
void usr_sha256_init(usr_sha256_ctx *ctx);
void usr_sha256_update(usr_sha256_ctx *ctx, const uint8_t *data, size_t len);
void usr_sha256_final(usr_sha256_ctx *ctx, uint8_t out[32]);

/* ============================================================
   SHA-512
   ============================================================ */

#define USR_SHA512_DIGEST_SIZE 64
#define USR_SHA512_BLOCK_SIZE 128

typedef struct {
    uint64_t state[8];
    uint8_t  buf[128];
    uint64_t bitcount_hi;
    uint64_t bitcount_lo;
    uint32_t buflen;
} usr_sha512_ctx;

/* One-shot SHA-512 */
void usr_sha512(const uint8_t *data, size_t len, uint8_t out[64]);

/* Streaming SHA-512 */
void usr_sha512_init(usr_sha512_ctx *ctx);
void usr_sha512_update(usr_sha512_ctx *ctx, const uint8_t *data, size_t len);
void usr_sha512_final(usr_sha512_ctx *ctx, uint8_t out[64]);

/* ============================================================
   HMAC-SHA256
   ============================================================ */

#define USR_HMAC_SHA256_SIZE 32

typedef struct {
    usr_sha256_ctx inner;
    usr_sha256_ctx outer;
} usr_hmac_sha256_ctx;

void usr_hmac_sha256_init(usr_hmac_sha256_ctx *ctx,
                           const uint8_t *key, size_t key_len);
void usr_hmac_sha256_update(usr_hmac_sha256_ctx *ctx,
                             const uint8_t *data, size_t len);
void usr_hmac_sha256_final(usr_hmac_sha256_ctx *ctx, uint8_t out[32]);

/* One-shot HMAC-SHA256 */
void usr_hmac_sha256(const uint8_t *key, size_t key_len,
                     const uint8_t *data, size_t data_len,
                     uint8_t out[32]);

/* ============================================================
   PBKDF2-HMAC-SHA256
   ============================================================ */

/* Derive `out_len` bytes from password+salt with `iterations` rounds.
   out_len can be any value up to (2^32 - 1) * 32 bytes.
   Returns 0 on success, -1 on error (invalid args). */
int usr_pbkdf2_sha256(
    const uint8_t *password, size_t password_len,
    const uint8_t *salt,     size_t salt_len,
    uint32_t       iterations,
    uint8_t       *out,      size_t out_len
);

/* ============================================================
   AES-256 — Internal Block Operations
   ============================================================ */

/* AES-256 key schedule: 240 bytes of round keys (15 round keys × 16 bytes) */
void usr_aes256_key_expand(const uint8_t key[32], uint8_t round_keys[240]);

/* Encrypt a single 16-byte block in-place */
void usr_aes256_encrypt_block(uint8_t block[16], const uint8_t round_keys[240]);

/* Decrypt a single 16-byte block in-place */
void usr_aes256_decrypt_block(uint8_t block[16], const uint8_t round_keys[240]);

/* ============================================================
   AES-256-IGE  (Telegram MTProto)
   ============================================================ */

/* Encrypt `data` in-place.
   `len` MUST be a multiple of 16.
   `key` = 32 bytes, `iv` = 32 bytes (split as iv[0..15]=prevC, iv[16..31]=prevP).
   Returns 0 on success, -1 on invalid args. */
int usr_aes256_ige_encrypt(
    uint8_t       *data, size_t len,
    const uint8_t  key[32],
    const uint8_t  iv[32]
);

/* Decrypt `data` in-place.
   Same constraints as encrypt. */
int usr_aes256_ige_decrypt(
    uint8_t       *data, size_t len,
    const uint8_t  key[32],
    const uint8_t  iv[32]
);

/* ============================================================
   AES-256-CBC
   ============================================================ */

/* Encrypt using PKCS#7 padding. Output is written to `out`.
   `out_len` must be at least (len + 16) rounded up to 16.
   Call with out=NULL to get required output size in *out_len.
   Returns 0 on success, -1 on error. */
int usr_aes256_cbc_encrypt(
    const uint8_t *in,  size_t in_len,
    const uint8_t  key[32],
    const uint8_t  iv[16],
    uint8_t       *out, size_t *out_len
);

/* Decrypt and strip PKCS#7 padding.
   Returns 0 on success, -1 on error (bad padding, wrong key). */
int usr_aes256_cbc_decrypt(
    const uint8_t *in,  size_t in_len,
    const uint8_t  key[32],
    const uint8_t  iv[16],
    uint8_t       *out, size_t *out_len
);

/* ============================================================
   AES-256-CTR  (no padding, arbitrary length)
   ============================================================ */

/* Encrypt or decrypt (CTR is symmetric) `data` in-place.
   `nonce` = 16-byte counter/nonce block (big-endian counter in last 4 bytes).
   Returns 0 on success, -1 on error. */
int usr_aes256_ctr_crypt(
    uint8_t       *data, size_t len,
    const uint8_t  key[32],
    uint8_t        nonce[16]   /* updated in-place for streaming */
);

/* ============================================================
   CRC32  (IEEE 802.3 / zlib polynomial)
   ============================================================ */

/* Compute CRC-32 of data. Initial value is 0xFFFFFFFF.
   Chain calls: crc = usr_crc32_update(crc, data, len). */
uint32_t usr_crc32(const uint8_t *data, size_t len);
uint32_t usr_crc32_update(uint32_t crc, const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* USR_CRYPTO_H */
