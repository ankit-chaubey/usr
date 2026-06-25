/**
 * full_demo.c - USR library v0.1.4 comprehensive demo
 * Exercises: UTF-8, Markdown parsing, HTML round-trip,
 *            SHA-256, HMAC-SHA256, AES-256-IGE/CBC/CTR,
 *            Base64, Hex, URL encoding, CRC-32.
 *
 * Build: gcc -Iinclude -Isrc/crypto examples/full_demo.c build/libusr.a -o demo
 * Run:   ./demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "usr/usr.h"


static void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("  %-20s: ", label);
    for (size_t i = 0; i < len; i++) printf("%02x", data[i]);
    printf("\n");
}

static void print_entities(const usr_entity *e, size_t n) {
    for (size_t i = 0; i < n; i++) {
        printf("    [%zu] %-16s offset=%-4u length=%u",
               i, usr_entity_type_name(e[i].type), e[i].offset, e[i].length);
        if (e[i].extra) printf("  extra=%s", e[i].extra);
        printf("\n");
    }
}


int main(void) {
    printf("╔══════════════════════════════════════╗\n");
    printf("║      USR v0.1.4 - Full Demo          ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

   
    printf("1. UTF-8 Codepoints\n");
    const char *sample = "Hello 🙂 世界";
    printf("  Input: %s\n  Codepoints: ", sample);
    for (size_t i = 0; sample[i]; ) {
        uint32_t cp; size_t adv;
        usr_utf8_decode((const uint8_t*)sample + i, strlen(sample) - i, &cp, &adv);
        printf("U+%04X ", cp);
        i += adv;
    }
    int64_t u16 = usr_utf8_utf16_units((const uint8_t*)sample, strlen(sample));
    printf("\n  UTF-16 units: %lld\n\n", (long long)u16);

   
    printf("2. Markdown V2 Parse\n");
    const char *md = "*bold* _italic_ ~strike~ `code` ||spoiler|| [Link](https://example.com)";
    printf("  Input:  %s\n", md);

    usr_entity ents[64];
    char *plain = NULL;
    size_t n = usr_markdown_parse(md, USR_MD_V2, &plain, ents, 64);
    n = usr_entities_normalize(ents, n);
    printf("  Plain:  %s\n", plain);
    printf("  Entities (%zu):\n", n);
    print_entities(ents, n);

   
    printf("\n3. Entities → HTML\n");
    char *html = usr_entities_to_html(plain, ents, n);
    printf("  %s\n", html);

   
    printf("\n4. HTML → Entities (round-trip check)\n");
    usr_entity ents2[64];
    char *plain2 = NULL;
    size_t n2 = usr_html_parse(html, &plain2, ents2, 64);
    n2 = usr_entities_normalize(ents2, n2);
    int match = (plain2 && strcmp(plain, plain2) == 0) ? 1 : 0;
    printf("  Plain match: %s\n", match ? "✅ YES" : "❌ NO");
    printf("  Entity count: %zu → %zu\n", n, n2);

   
    printf("\n5. Entities → MarkdownV2\n");
    char *md_out = usr_entities_to_markdown(plain, ents, n, USR_MD_V2);
    printf("  %s\n", md_out);

   
    printf("\n6. SHA-256\n");
    uint8_t digest[32];
    usr_sha256((uint8_t*)"Hello, world!", 13, digest);
    print_hex("SHA256(\"Hello, world!\")", digest, 32);

   
    printf("\n7. SHA-512\n");
    uint8_t digest512[64];
    usr_sha512((uint8_t*)"Hello, world!", 13, digest512);
    print_hex("SHA512", digest512, 64);

   
    printf("\n8. HMAC-SHA256\n");
    uint8_t hmac[32];
    usr_hmac_sha256((uint8_t*)"secret-key", 10, (uint8_t*)"message", 7, hmac);
    print_hex("HMAC-SHA256", hmac, 32);

   
    printf("\n9. AES-256-IGE (Telegram MTProto)\n");
    uint8_t key[32], iv[32], msg[32];
    memset(key, 0x11, 32); memset(iv, 0x22, 32);
    memcpy(msg, "secret-message!!", 16);
    memset(msg+16, 0, 16);
    print_hex("plaintext", msg, 16);
    usr_aes256_ige_encrypt(msg, 32, key, iv);
    print_hex("encrypted", msg, 16);
    memset(iv, 0x22, 32);
    usr_aes256_ige_decrypt(msg, 32, key, iv);
    print_hex("decrypted", msg, 16);

   
    printf("\n10. AES-256-CBC\n");
    uint8_t cbc_key[32], cbc_iv[16];
    memset(cbc_key, 0x33, 32); memset(cbc_iv, 0x44, 16);
    uint8_t cbc_plain[] = "Hello, AES-CBC!";
    uint8_t cbc_enc[64], cbc_dec[64];
    size_t enc_len = sizeof(cbc_enc), dec_len = sizeof(cbc_dec);
    usr_aes256_cbc_encrypt(cbc_plain, 15, cbc_key, cbc_iv, cbc_enc, &enc_len);
    memset(cbc_iv, 0x44, 16);
    usr_aes256_cbc_decrypt(cbc_enc, enc_len, cbc_key, cbc_iv, cbc_dec, &dec_len);
    printf("  Original:  %s\n", cbc_plain);
    printf("  Decrypted: %.*s\n", (int)dec_len, cbc_dec);

   
    printf("\n11. Base64\n");
    char b64[128];
    usr_base64_encode((uint8_t*)"usr v0.1.4!", 11, b64);
    printf("  encode: %s\n", b64);
    uint8_t b64dec[64]; size_t b64n = usr_base64_decode(b64, strlen(b64), b64dec);
    b64dec[b64n] = 0; printf("  decode: %s\n", (char*)b64dec);

   
    printf("\n12. Hex encoding\n");
    char hexbuf[64];
    uint8_t hexdata[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE};
    usr_hex_encode(hexdata, 6, hexbuf);
    printf("  encode: %s\n", hexbuf);

   
    printf("\n13. URL encoding\n");
    char urlbuf[256];
    usr_url_encode("hello world & foo=bar", 21, urlbuf);
    printf("  encode: %s\n", urlbuf);
    char urldec[256];
    usr_url_decode(urlbuf, strlen(urlbuf), urldec);
    printf("  decode: %s\n", urldec);

   
    printf("\n14. CRC-32\n");
    uint32_t crc = usr_crc32((uint8_t*)"123456789", 9);
    printf("  CRC32(\"123456789\") = 0x%08X (expected 0xCBF43926) %s\n",
           crc, (crc == 0xCBF43926u) ? "✅" : "❌");

   
    free(plain); free(plain2); free(html); free(md_out);

    printf("\n╔══════════════════════════════════════╗\n");
    printf("║         Demo complete ✅              ║\n");
    printf("╚══════════════════════════════════════╝\n");
    return 0;
}
