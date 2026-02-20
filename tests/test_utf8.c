#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "usr/utf8.h"

static int pass = 0, fail = 0;

#define CHECK(cond, name) \
    do { if (cond) { printf("  ✅ %s\n", (name)); pass++; } \
         else { printf("  ❌ %s\n", (name)); fail++; } } while(0)

int main(void) {
    printf("====== USR UTF-8 Tests ======\n\n");

    /* Decode ASCII */
    uint32_t cp; size_t adv;
    usr_utf8_decode((uint8_t*)"A", 1, &cp, &adv);
    CHECK(cp == 'A' && adv == 1, "decode ASCII 'A'");

    /* Decode 2-byte: U+00E9 é */
    uint8_t e_acute[] = {0xC3, 0xA9};
    usr_utf8_decode(e_acute, 2, &cp, &adv);
    CHECK(cp == 0xE9 && adv == 2, "decode U+00E9 é");

    /* Decode 3-byte: U+4E2D 中 */
    uint8_t chinese[] = {0xE4, 0xB8, 0xAD};
    usr_utf8_decode(chinese, 3, &cp, &adv);
    CHECK(cp == 0x4E2D && adv == 3, "decode U+4E2D 中");

    /* Decode 4-byte: U+1F600 😀 */
    uint8_t emoji[] = {0xF0, 0x9F, 0x98, 0x80};
    usr_utf8_decode(emoji, 4, &cp, &adv);
    CHECK(cp == 0x1F600 && adv == 4, "decode U+1F600 😀");

    /* Emoji counts as 2 UTF-16 units */
    CHECK(usr_codepoint_utf16_units(0x1F600) == 2, "emoji = 2 UTF-16 units");
    CHECK(usr_codepoint_utf16_units(0x41) == 1, "ASCII = 1 UTF-16 unit");

    /* Encode */
    uint8_t enc[4];
    int n = usr_utf8_encode(0x4E2D, enc);
    CHECK(n == 3 && enc[0]==0xE4 && enc[1]==0xB8 && enc[2]==0xAD,
          "encode U+4E2D 中");

    n = usr_utf8_encode(0x1F600, enc);
    CHECK(n == 4 && enc[0]==0xF0 && enc[1]==0x9F && enc[2]==0x98 && enc[3]==0x80,
          "encode U+1F600 😀");

    /* Validate */
    CHECK(usr_utf8_validate((uint8_t*)"hello", 5) == 0, "validate ASCII");
    CHECK(usr_utf8_validate(emoji, 4) == 0, "validate emoji");
    uint8_t bad[] = {0x80}; /* lone continuation byte */
    CHECK(usr_utf8_validate(bad, 1) != 0, "reject lone continuation");
    uint8_t bad2[] = {0xED, 0xA0, 0x80}; /* surrogate D800 */
    CHECK(usr_utf8_validate(bad2, 3) != 0, "reject surrogate");

    /* Count */
    const char *s = "Hello 🙂!"; /* 7 chars: H e l l o space emoji ! */
    int64_t cpc = usr_utf8_codepoint_count((uint8_t*)s, strlen(s));
    CHECK(cpc == 8, "codepoint count (Hello 🙂!)");

    int64_t u16 = usr_utf8_utf16_units((uint8_t*)s, strlen(s));
    CHECK(u16 == 9, "utf16 units (emoji counts 2)");

    /* Offset conversion */
    size_t byte_off = usr_utf8_byte_offset_from_utf16((uint8_t*)s, strlen(s), 6);
    /* 6 UTF-16 units = "Hello " = 6 bytes */
    CHECK(byte_off == 6, "byte offset from utf16 offset 6");

    printf("\n══════════════════════════════\n");
    printf("Results: %d passed, %d failed\n", pass, fail);
    return (fail > 0) ? 1 : 0;
}
