#include "usr/rand.h"
#include <stdint.h>
#include <string.h>

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#  ifdef __linux__
#    include <sys/syscall.h>
#    ifdef SYS_getrandom
#      include <linux/random.h>
#      define USE_GETRANDOM 1
#    endif
#  endif
#  define USE_DEVURANDOM 1
#elif defined(_WIN32)
#  include <windows.h>
#  include <wincrypt.h>
#  define USE_WINCRYPT 1
#endif

/* ============================================================
   Fill `len` bytes with cryptographically random data.
   ============================================================ */

int usr_rand_bytes(uint8_t *out, size_t len) {
    if (!out || len == 0) return 0;

#if defined(USE_GETRANDOM)
    /* getrandom() syscall (Linux 3.17+, Android 8+) */
    ssize_t total = 0;
    while ((size_t)total < len) {
        long n = syscall(SYS_getrandom, out + total, len - total, 0);
        if (n < 0) goto fallback_urandom;
        total += n;
    }
    return 0;
fallback_urandom:;
#endif

#if defined(USE_DEVURANDOM)
    {
        int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
        if (fd < 0) return -1;
        size_t total = 0;
        while (total < len) {
            ssize_t n = read(fd, out + total, len - total);
            if (n <= 0) { close(fd); return -1; }
            total += (size_t)n;
        }
        close(fd);
        return 0;
    }
#elif defined(USE_WINCRYPT)
    {
        HCRYPTPROV prov;
        if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL,
                                 CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
            return -1;
        }
        BOOL ok = CryptGenRandom(prov, (DWORD)len, (BYTE *)out);
        CryptReleaseContext(prov, 0);
        return ok ? 0 : -1;
    }
#else
    /* No secure random source available */
    (void)out; (void)len;
    return -1;
#endif
}

uint64_t usr_rand_u64(void) {
    uint64_t v = 0;
    usr_rand_bytes((uint8_t *)&v, sizeof(v));
    return v;
}

uint32_t usr_rand_u32(void) {
    uint32_t v = 0;
    usr_rand_bytes((uint8_t *)&v, sizeof(v));
    return v;
}

uint64_t usr_rand_range(uint64_t max) {
    if (max == 0) return 0;
    /* Rejection sampling to avoid modulo bias */
    uint64_t threshold = (uint64_t)(-(int64_t)max) % max;
    uint64_t v;
    do {
        v = usr_rand_u64();
    } while (v < threshold);
    return v % max;
}
