#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "usr/encoding.h"

static int pass = 0, fail = 0;

static void check_str(const char *name, const char *got, const char *expected) {
    if (strcmp(got, expected) == 0) {
        printf("  ✅ %s\n", name); pass++;
    } else {
        printf("  ❌ %s\n     got:      [%s]\n     expected: [%s]\n", name, got, expected); fail++;
    }
}

static void check_bytes(const char *name, const uint8_t *got, size_t got_len,
                        const uint8_t *exp, size_t exp_len) {
    if (got_len == exp_len && memcmp(got, exp, exp_len) == 0) {
        printf("  ✅ %s\n", name); pass++;
    } else {
        printf("  ❌ %s (len %zu vs %zu)\n", name, got_len, exp_len); fail++;
    }
}

static void test_base64(void) {
    printf("\n── Base64 ──\n");

    char enc[256];
    uint8_t dec[256];

    /* RFC 4648 test vectors */
    usr_base64_encode((uint8_t*)"", 0, enc);          check_str("encode \"\"", enc, "");
    usr_base64_encode((uint8_t*)"f", 1, enc);          check_str("encode \"f\"", enc, "Zg==");
    usr_base64_encode((uint8_t*)"fo", 2, enc);         check_str("encode \"fo\"", enc, "Zm8=");
    usr_base64_encode((uint8_t*)"foo", 3, enc);        check_str("encode \"foo\"", enc, "Zm9v");
    usr_base64_encode((uint8_t*)"foob", 4, enc);       check_str("encode \"foob\"", enc, "Zm9vYg==");
    usr_base64_encode((uint8_t*)"fooba", 5, enc);      check_str("encode \"fooba\"", enc, "Zm9vYmE=");
    usr_base64_encode((uint8_t*)"foobar", 6, enc);     check_str("encode \"foobar\"", enc, "Zm9vYmFy");

    size_t n;
    n = usr_base64_decode("Zm9vYmFy", 8, dec);
    check_bytes("decode \"foobar\"", dec, n, (uint8_t*)"foobar", 6);

    n = usr_base64_decode("Zg==", 4, dec);
    check_bytes("decode \"f\"", dec, n, (uint8_t*)"f", 1);

    /* URL-safe */
    uint8_t data[] = {0xFB, 0xFF, 0xFE};
    usr_base64url_encode(data, 3, enc);
    check_str("base64url encode", enc, "-__-");
}

static void test_hex(void) {
    printf("\n── Hex ──\n");

    char hex[32];
    uint8_t bin[16];

    uint8_t input[] = {0xDE, 0xAD, 0xBE, 0xEF};
    usr_hex_encode(input, 4, hex);
    check_str("hex encode", hex, "deadbeef");

    usr_hex_encode_upper(input, 4, hex);
    check_str("hex encode upper", hex, "DEADBEEF");

    size_t n = usr_hex_decode("deadbeef", 8, bin);
    check_bytes("hex decode lowercase", bin, n, input, 4);

    n = usr_hex_decode("DEADBEEF", 8, bin);
    check_bytes("hex decode uppercase", bin, n, input, 4);

    n = usr_hex_decode("0xDEADBEEF", 10, bin);
    check_bytes("hex decode with 0x prefix", bin, n, input, 4);
}

static void test_url(void) {
    printf("\n── URL Encoding ──\n");

    char out[512];

    usr_url_encode("hello world", 11, out);
    check_str("url encode space", out, "hello%20world");

    usr_url_encode("a+b=c&d", 7, out);
    check_str("url encode special chars", out, "a%2Bb%3Dc%26d");

    usr_url_decode("hello%20world", 13, out);
    check_str("url decode %20", out, "hello world");

    usr_url_decode("a%2Bb", 5, out);
    check_str("url decode %2B", out, "a+b");
}

static void test_html_escape(void) {
    printf("\n── HTML Escape ──\n");

    char out[256];

    usr_html_escape("<b>hello & world</b>", 20, out);
    check_str("html escape", out, "&lt;b&gt;hello &amp; world&lt;/b&gt;");

    usr_html_unescape("&lt;b&gt;hello &amp; world&lt;/b&gt;", 36, out);
    check_str("html unescape", out, "<b>hello & world</b>");

    const char *dec_str = "&#72;&#101;&#108;&#108;&#111;";
    usr_html_unescape(dec_str, strlen(dec_str), out);
    check_str("html unescape numeric decimal", out, "Hello");

    const char *hex_str = "&#x48;&#x65;&#x6c;&#x6c;&#x6f;";
    usr_html_unescape(hex_str, strlen(hex_str), out);
    check_str("html unescape numeric hex", out, "Hello");
}

int main(void) {
    printf("====== USR Encoding Tests ======\n");
    test_base64();
    test_hex();
    test_url();
    test_html_escape();
    printf("\n══════════════════════════════\n");
    printf("Results: %d passed, %d failed\n", pass, fail);
    return (fail > 0) ? 1 : 0;
}
