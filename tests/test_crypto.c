#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "usr/crypto.h"
#include "usr/encoding.h"

static int pass = 0, fail = 0;

static void check_hex(const char *test_name,
                       const uint8_t *got, size_t got_len,
                       const char *expected_hex) {
    char got_hex[256] = {0};
    usr_hex_encode(got, got_len, got_hex);
    if (strcmp(got_hex, expected_hex) == 0) {
        printf("  ✅ %s\n", test_name);
        pass++;
    } else {
        printf("  ❌ %s\n     got:      %s\n     expected: %s\n",
               test_name, got_hex, expected_hex);
        fail++;
    }
}

static void test_sha256(void) {
    printf("\n── SHA-256 ──\n");
    uint8_t out[32];

    /* NIST test vector 1: "" */
    usr_sha256(NULL, 0, out);
    check_hex("SHA256(\"\") empty",
        out, 32,
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    /* NIST test vector 2: "abc" */
    usr_sha256((uint8_t*)"abc", 3, out);
    check_hex("SHA256(\"abc\")",
        out, 32,
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    /* "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" (448-bit msg) */
    const char *msg2 = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    usr_sha256((uint8_t*)msg2, strlen(msg2), out);
    check_hex("SHA256(448-bit msg)",
        out, 32,
        "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");

    /* Streaming test */
    usr_sha256_ctx ctx;
    usr_sha256_init(&ctx);
    usr_sha256_update(&ctx, (uint8_t*)"ab", 2);
    usr_sha256_update(&ctx, (uint8_t*)"c", 1);
    usr_sha256_final(&ctx, out);
    check_hex("SHA256 streaming (abc)",
        out, 32,
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    /* Large input: 1,000,000 'a' chars */
    {
        uint8_t *big = (uint8_t*)malloc(1000000);
        memset(big, 'a', 1000000);
        usr_sha256(big, 1000000, out);
        free(big);
        check_hex("SHA256(1M 'a')",
            out, 32,
            "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
    }
}

static void test_sha512(void) {
    printf("\n── SHA-512 ──\n");
    uint8_t out[64];

    usr_sha512(NULL, 0, out);
    check_hex("SHA512(\"\")",
        out, 64,
        "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");

    usr_sha512((uint8_t*)"abc", 3, out);
    check_hex("SHA512(\"abc\")",
        out, 64,
        "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
}

static void test_hmac(void) {
    printf("\n── HMAC-SHA256 ──\n");
    uint8_t out[32];

    /* RFC 4231 test vector 1 */
    uint8_t key1[20];
    memset(key1, 0x0b, 20);
    const char *data1 = "Hi There";
    usr_hmac_sha256(key1, 20, (uint8_t*)data1, strlen(data1), out);
    check_hex("HMAC-SHA256 RFC4231 #1",
        out, 32,
        "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");
}

static void test_pbkdf2(void) {
    printf("\n── PBKDF2-HMAC-SHA256 ──\n");
    uint8_t out[32];

    /* RFC 6070 test vector (adapted for SHA-256) */
    usr_pbkdf2_sha256((uint8_t*)"password", 8,
                      (uint8_t*)"salt", 4,
                      1, out, 32);
    char hex[65];
    usr_hex_encode(out, 32, hex);
    /* Known output for PBKDF2-HMAC-SHA256(password, salt, 1, 32) */
    const char *expected = "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b";
    if (strcmp(hex, expected) == 0) {
        printf("  ✅ PBKDF2 c=1\n"); pass++;
    } else {
        printf("  ❌ PBKDF2 c=1\n     got: %s\n     exp: %s\n", hex, expected); fail++;
    }
}

static void test_aes_ige(void) {
    printf("\n── AES-256-IGE ──\n");

    /* Round-trip test */
    uint8_t key[32], iv[32], msg[64], orig[64];
    for (int i = 0; i < 32; i++) key[i] = (uint8_t)(i * 3 + 7);
    for (int i = 0; i < 32; i++) iv[i]  = (uint8_t)(i * 5 + 11);
    for (int i = 0; i < 64; i++) msg[i] = orig[i] = (uint8_t)(i + 1);

    int r1 = usr_aes256_ige_encrypt(msg, 64, key, iv);
    int r2 = usr_aes256_ige_decrypt(msg, 64, key, iv);

    if (r1 == 0 && r2 == 0 && memcmp(msg, orig, 64) == 0) {
        printf("  ✅ AES-256-IGE encrypt/decrypt round-trip\n"); pass++;
    } else {
        printf("  ❌ AES-256-IGE round-trip FAILED (r1=%d r2=%d)\n", r1, r2); fail++;
    }

    /* Error cases */
    if (usr_aes256_ige_encrypt(NULL, 16, key, iv) != 0) { printf("  ✅ NULL data rejected\n"); pass++; }
    else { printf("  ❌ NULL data not rejected\n"); fail++; }

    if (usr_aes256_ige_encrypt(msg, 17, key, iv) != 0) { printf("  ✅ non-block size rejected\n"); pass++; }
    else { printf("  ❌ non-block size not rejected\n"); fail++; }
}

static void test_aes_cbc(void) {
    printf("\n── AES-256-CBC ──\n");

    uint8_t key[32], iv[16];
    memset(key, 0x11, 32);
    memset(iv,  0x22, 16);

    const char *plain = "Hello, AES-CBC!";
    size_t      plain_len = strlen(plain);

    uint8_t enc[64]; size_t enc_len = sizeof(enc);
    uint8_t dec[64]; size_t dec_len = sizeof(dec);

    int r1 = usr_aes256_cbc_encrypt((uint8_t*)plain, plain_len, key, iv, enc, &enc_len);
    int r2 = usr_aes256_cbc_decrypt(enc, enc_len, key, iv, dec, &dec_len);

    if (r1 == 0 && r2 == 0 &&
        dec_len == plain_len &&
        memcmp(dec, plain, plain_len) == 0) {
        printf("  ✅ AES-256-CBC encrypt/decrypt round-trip\n"); pass++;
    } else {
        printf("  ❌ AES-256-CBC round-trip FAILED (r1=%d r2=%d dec_len=%zu)\n",
               r1, r2, dec_len); fail++;
    }
}

static void test_aes_ctr(void) {
    printf("\n── AES-256-CTR ──\n");

    uint8_t key[32], nonce[16];
    memset(key, 0x33, 32);
    memset(nonce, 0x00, 16);

    const char *plain = "CTR mode: arbitrary length!";
    size_t len = strlen(plain);

    uint8_t enc[64], dec[64];
    memcpy(enc, plain, len);

    uint8_t n1[16], n2[16];
    memcpy(n1, nonce, 16);
    memcpy(n2, nonce, 16);

    usr_aes256_ctr_crypt(enc, len, key, n1);
    memcpy(dec, enc, len);
    usr_aes256_ctr_crypt(dec, len, key, n2);

    if (memcmp(dec, plain, len) == 0) {
        printf("  ✅ AES-256-CTR encrypt/decrypt symmetric\n"); pass++;
    } else {
        printf("  ❌ AES-256-CTR round-trip FAILED\n"); fail++;
    }
}

static void test_crc32(void) {
    printf("\n── CRC-32 ──\n");

    uint32_t crc = usr_crc32((uint8_t*)"123456789", 9);
    if (crc == 0xCBF43926u) {
        printf("  ✅ CRC32(\"123456789\") = 0x%08X\n", crc); pass++;
    } else {
        printf("  ❌ CRC32 expected 0xCBF43926, got 0x%08X\n", crc); fail++;
    }
}

int main(void) {
    printf("====== USR Crypto Tests ======\n");

    test_sha256();
    test_sha512();
    test_hmac();
    test_pbkdf2();
    test_aes_ige();
    test_aes_cbc();
    test_aes_ctr();
    test_crc32();

    printf("\n══════════════════════════════\n");
    printf("Results: %d passed, %d failed\n", pass, fail);
    return (fail > 0) ? 1 : 0;
}
