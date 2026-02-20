#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "usr/crypto.h"
#include "usr/encoding.h"

#define MB (1024*1024)

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static void bench_sha256(size_t data_size, int iters) {
    uint8_t *data = (uint8_t*)malloc(data_size);
    uint8_t  out[32];
    memset(data, 0xAB, data_size);

    double t0 = now_ms();
    for (int i = 0; i < iters; i++) {
        usr_sha256(data, data_size, out);
    }
    double elapsed = now_ms() - t0;
    double mbps = (data_size * iters / MB) / (elapsed / 1000.0);

    printf("SHA-256  %4zuKB x %5d = %7.2f ms  |  %.1f MB/s\n",
           data_size/1024, iters, elapsed, mbps);
    free(data);
}

static void bench_aes_ige(size_t data_size, int iters) {
    uint8_t *data = (uint8_t*)malloc(data_size);
    uint8_t  key[32], iv[32];
    memset(data, 0, data_size);
    memset(key,  0x11, 32);
    memset(iv,   0x22, 32);

    /* Round up to AES block multiple */
    if (data_size % 16 != 0) data_size = (data_size/16+1)*16;

    double t0 = now_ms();
    for (int i = 0; i < iters; i++) {
        usr_aes256_ige_encrypt(data, data_size, key, iv);
    }
    double elapsed = now_ms() - t0;
    double mbps = (data_size * iters / MB) / (elapsed / 1000.0);

    printf("AES-IGE  %4zuKB x %5d = %7.2f ms  |  %.1f MB/s\n",
           data_size/1024, iters, elapsed, mbps);
    free(data);
}

static void bench_base64(size_t data_size, int iters) {
    uint8_t *data = (uint8_t*)malloc(data_size);
    char    *enc  = (char*)malloc(data_size * 2);
    memset(data, 0xCD, data_size);

    double t0 = now_ms();
    for (int i = 0; i < iters; i++) {
        usr_base64_encode(data, data_size, enc);
    }
    double elapsed = now_ms() - t0;
    double mbps = (data_size * iters / MB) / (elapsed / 1000.0);

    printf("Base64   %4zuKB x %5d = %7.2f ms  |  %.1f MB/s\n",
           data_size/1024, iters, elapsed, mbps);
    free(data); free(enc);
}

int main(void) {
    printf("====== USR Benchmark ======\n");
    printf("(MB/s = megabytes per second throughput)\n\n");

    bench_sha256(64,    100000);
    bench_sha256(1024,  10000);
    bench_sha256(64*1024, 1000);

    printf("\n");
    bench_aes_ige(64,      100000);
    bench_aes_ige(4*1024,  10000);
    bench_aes_ige(64*1024, 1000);

    printf("\n");
    bench_base64(1024,   50000);
    bench_base64(64*1024, 2000);

    printf("\n====== Done ======\n");
    return 0;
}
