#include "usr/crypto.h"
#include <string.h>
#include <stdint.h>

void usr_aes256_key_expand(const uint8_t key[32], uint8_t round_keys[240]);
void usr_aes256_encrypt_block(uint8_t block[16], const uint8_t round_keys[240]);

/* ============================================================
   AES-256-CTR  (Encrypt = Decrypt, arbitrary length)

   Counter block layout (big-endian):
     nonce[0..11] = 96-bit nonce
     nonce[12..15] = 32-bit counter (incremented after each block)

   The caller may pass any 16-byte nonce; last 4 bytes are the counter.
   nonce[] is updated in-place to allow streaming across multiple calls.
   ============================================================ */

int usr_aes256_ctr_crypt(
    uint8_t       *data,
    size_t         len,
    const uint8_t  key[32],
    uint8_t        nonce[16]
) {
    if (!data || !key || !nonce) return -1;
    if (len == 0) return 0;

    uint8_t round_keys[240];
    usr_aes256_key_expand(key, round_keys);

    uint8_t keystream[16];
    size_t  i = 0;

    while (i < len) {
        /* Encrypt the current counter block to get keystream */
        memcpy(keystream, nonce, 16);
        usr_aes256_encrypt_block(keystream, round_keys);

        /* XOR keystream with data (partial block at end) */
        size_t chunk = (len - i < 16) ? (len - i) : 16;
        for (size_t j = 0; j < chunk; j++) {
            data[i + j] ^= keystream[j];
        }
        i += chunk;

        /* Increment counter (big-endian, last 4 bytes) */
        for (int k = 15; k >= 12; k--) {
            if (++nonce[k]) break;
        }
    }

    return 0;
}
