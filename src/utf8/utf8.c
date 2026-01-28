#include "usr/utf8.h"

int usr_utf8_decode(
    const uint8_t *s,
    size_t len,
    uint32_t *codepoint,
    size_t *advance
) {
    if (len == 0) return -1;

    uint8_t c = s[0];

    if (c < 0x80) {
        *codepoint = c;
        *advance = 1;
        return 0;
    }

    if ((c & 0xE0) == 0xC0 && len >= 2) {
        *codepoint = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        *advance = 2;
        return 0;
    }

    if ((c & 0xF0) == 0xE0 && len >= 3) {
        *codepoint =
            ((c & 0x0F) << 12) |
            ((s[1] & 0x3F) << 6) |
            (s[2] & 0x3F);
        *advance = 3;
        return 0;
    }

    if ((c & 0xF8) == 0xF0 && len >= 4) {
        *codepoint =
            ((c & 0x07) << 18) |
            ((s[1] & 0x3F) << 12) |
            ((s[2] & 0x3F) << 6) |
            (s[3] & 0x3F);
        *advance = 4;
        return 0;
    }

    return -1;
}
