#ifndef USR_RAND_H
#define USR_RAND_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   Cryptographically Secure Random Bytes
   Uses /dev/urandom on POSIX, getrandom() where available.
   ============================================================ */

/* Fill `out` with `len` cryptographically random bytes.
   Returns 0 on success, -1 on failure (entropy source unavailable). */
int usr_rand_bytes(uint8_t *out, size_t len);

/* Generate a random 64-bit unsigned integer. Returns 0 on failure. */
uint64_t usr_rand_u64(void);

/* Generate a random 32-bit unsigned integer. */
uint32_t usr_rand_u32(void);

/* Generate a random value in [0, max). Returns 0 if max == 0. */
uint64_t usr_rand_range(uint64_t max);

#ifdef __cplusplus
}
#endif

#endif /* USR_RAND_H */
