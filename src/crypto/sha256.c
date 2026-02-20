#include "usr/crypto.h"
#include <string.h>
#include <stdint.h>

/* ============================================================
   SHA-256 Implementation (FIPS 180-4 compliant)
   Supports streaming via init/update/final API.
   One-shot convenience wrapper: usr_sha256().
   ============================================================ */

/* SHA-256 round constants */
static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/* SHA-256 initial hash values (first 32 bits of fractional parts of sqrt of first 8 primes) */
static const uint32_t H0[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

#define ROTR32(x, n)  (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z)   (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)        (ROTR32(x, 2)  ^ ROTR32(x, 13) ^ ROTR32(x, 22))
#define EP1(x)        (ROTR32(x, 6)  ^ ROTR32(x, 11) ^ ROTR32(x, 25))
#define SIG0(x)       (ROTR32(x, 7)  ^ ROTR32(x, 18) ^ ((x) >> 3))
#define SIG1(x)       (ROTR32(x, 17) ^ ROTR32(x, 19) ^ ((x) >> 10))

/* Process one 64-byte block */
static void sha256_compress(uint32_t state[8], const uint8_t block[64]) {
    uint32_t w[64];
    int i;

    /* Prepare message schedule */
    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i*4  ] << 24)
             | ((uint32_t)block[i*4+1] << 16)
             | ((uint32_t)block[i*4+2] <<  8)
             | ((uint32_t)block[i*4+3]);
    }
    for (i = 16; i < 64; i++) {
        w[i] = SIG1(w[i-2]) + w[i-7] + SIG0(w[i-15]) + w[i-16];
    }

    /* Initialize working variables */
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    /* 64 rounds */
    for (i = 0; i < 64; i++) {
        uint32_t t1 = h + EP1(e) + CH(e, f, g) + K[i] + w[i];
        uint32_t t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    /* Add to state */
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

/* ============================================================
   Streaming API
   ============================================================ */

void usr_sha256_init(usr_sha256_ctx *ctx) {
    for (int i = 0; i < 8; i++) ctx->state[i] = H0[i];
    ctx->bitcount = 0;
    ctx->buflen   = 0;
}

void usr_sha256_update(usr_sha256_ctx *ctx, const uint8_t *data, size_t len) {
    if (!ctx || !data || len == 0) return;

    ctx->bitcount += (uint64_t)len << 3;

    size_t i = 0;
    /* Fill the partial buffer first */
    if (ctx->buflen > 0) {
        size_t need = 64 - ctx->buflen;
        size_t fill = (len < need) ? len : need;
        memcpy(ctx->buf + ctx->buflen, data, fill);
        ctx->buflen += (uint32_t)fill;
        i += fill;
        if (ctx->buflen == 64) {
            sha256_compress(ctx->state, ctx->buf);
            ctx->buflen = 0;
        }
    }

    /* Process full blocks directly from input */
    while (i + 64 <= len) {
        sha256_compress(ctx->state, data + i);
        i += 64;
    }

    /* Buffer remaining bytes */
    if (i < len) {
        size_t rem = len - i;
        memcpy(ctx->buf, data + i, rem);
        ctx->buflen = (uint32_t)rem;
    }
}

void usr_sha256_final(usr_sha256_ctx *ctx, uint8_t out[32]) {
    if (!ctx || !out) return;

    uint64_t bitcount = ctx->bitcount;
    uint32_t buflen   = ctx->buflen;

    /* Append 0x80 padding byte */
    ctx->buf[buflen++] = 0x80;

    /* If not enough room for length (8 bytes), flush and start new block */
    if (buflen > 56) {
        memset(ctx->buf + buflen, 0, 64 - buflen);
        sha256_compress(ctx->state, ctx->buf);
        buflen = 0;
    }

    /* Zero-pad to byte 56, then write 64-bit bit count big-endian */
    memset(ctx->buf + buflen, 0, 56 - buflen);
    ctx->buf[56] = (uint8_t)(bitcount >> 56);
    ctx->buf[57] = (uint8_t)(bitcount >> 48);
    ctx->buf[58] = (uint8_t)(bitcount >> 40);
    ctx->buf[59] = (uint8_t)(bitcount >> 32);
    ctx->buf[60] = (uint8_t)(bitcount >> 24);
    ctx->buf[61] = (uint8_t)(bitcount >> 16);
    ctx->buf[62] = (uint8_t)(bitcount >>  8);
    ctx->buf[63] = (uint8_t)(bitcount      );
    sha256_compress(ctx->state, ctx->buf);

    /* Write digest big-endian */
    for (int i = 0; i < 8; i++) {
        out[i*4  ] = (uint8_t)(ctx->state[i] >> 24);
        out[i*4+1] = (uint8_t)(ctx->state[i] >> 16);
        out[i*4+2] = (uint8_t)(ctx->state[i] >>  8);
        out[i*4+3] = (uint8_t)(ctx->state[i]      );
    }

    /* Wipe context */
    memset(ctx, 0, sizeof(*ctx));
}

/* ============================================================
   One-shot convenience wrapper
   ============================================================ */

void usr_sha256(const uint8_t *data, size_t len, uint8_t out[32]) {
    usr_sha256_ctx ctx;
    usr_sha256_init(&ctx);
    usr_sha256_update(&ctx, data, len);
    usr_sha256_final(&ctx, out);
}
