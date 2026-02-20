#include "usr/utf8.h"
#include <string.h>

/* ============================================================
   Decode one UTF-8 code point
   ============================================================ */

int usr_utf8_decode(
    const uint8_t *s,
    size_t         len,
    uint32_t      *codepoint,
    size_t        *advance
) {
    if (!s || !codepoint || !advance) return -1;
    if (len == 0) { *advance = 0; return -1; }

    uint8_t c = s[0];

    /* 1-byte: U+0000 .. U+007F */
    if (c < 0x80) {
        *codepoint = c;
        *advance   = 1;
        return 0;
    }

    /* Invalid leading bytes: 0x80..0xBF are continuation bytes */
    if (c < 0xC2) { *advance = 1; return -1; }

    /* 2-byte: U+0080 .. U+07FF */
    if (c < 0xE0) {
        if (len < 2 || (s[1] & 0xC0) != 0x80) { *advance = 1; return -1; }
        *codepoint = ((uint32_t)(c & 0x1F) << 6) | (s[1] & 0x3F);
        if (*codepoint < 0x80) { *advance = 1; return -1; } /* overlong */
        *advance = 2;
        return 0;
    }

    /* 3-byte: U+0800 .. U+FFFF (excluding surrogates) */
    if (c < 0xF0) {
        if (len < 3 ||
            (s[1] & 0xC0) != 0x80 ||
            (s[2] & 0xC0) != 0x80) { *advance = 1; return -1; }
        *codepoint = ((uint32_t)(c & 0x0F) << 12)
                   | ((uint32_t)(s[1] & 0x3F) << 6)
                   |  (uint32_t)(s[2] & 0x3F);
        if (*codepoint < 0x800) { *advance = 1; return -1; } /* overlong */
        /* Reject surrogates U+D800..U+DFFF */
        if (*codepoint >= 0xD800 && *codepoint <= 0xDFFF) {
            *advance = 1; return -1;
        }
        *advance = 3;
        return 0;
    }

    /* 4-byte: U+10000 .. U+10FFFF */
    if (c < 0xF5) {
        if (len < 4 ||
            (s[1] & 0xC0) != 0x80 ||
            (s[2] & 0xC0) != 0x80 ||
            (s[3] & 0xC0) != 0x80) { *advance = 1; return -1; }
        *codepoint = ((uint32_t)(c & 0x07) << 18)
                   | ((uint32_t)(s[1] & 0x3F) << 12)
                   | ((uint32_t)(s[2] & 0x3F) << 6)
                   |  (uint32_t)(s[3] & 0x3F);
        if (*codepoint < 0x10000 || *codepoint > 0x10FFFF) {
            *advance = 1; return -1; /* overlong or out of range */
        }
        *advance = 4;
        return 0;
    }

    /* 0xF5+ are invalid */
    *advance = 1;
    return -1;
}

/* ============================================================
   Encode one codepoint to UTF-8
   ============================================================ */

int usr_utf8_encode(uint32_t cp, uint8_t *out) {
    if (!out) return -1;

    if (cp < 0x80) {
        out[0] = (uint8_t)cp;
        return 1;
    }
    if (cp < 0x800) {
        out[0] = (uint8_t)(0xC0 | (cp >> 6));
        out[1] = (uint8_t)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp >= 0xD800 && cp <= 0xDFFF) return -1; /* surrogates invalid */
    if (cp < 0x10000) {
        out[0] = (uint8_t)(0xE0 | (cp >> 12));
        out[1] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (uint8_t)(0x80 | (cp & 0x3F));
        return 3;
    }
    if (cp <= 0x10FFFF) {
        out[0] = (uint8_t)(0xF0 | (cp >> 18));
        out[1] = (uint8_t)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (uint8_t)(0x80 | (cp & 0x3F));
        return 4;
    }
    return -1;
}

/* ============================================================
   Validate a UTF-8 string
   ============================================================ */

int usr_utf8_validate(const uint8_t *s, size_t len) {
    if (!s) return -1;
    size_t i = 0;
    while (i < len) {
        uint32_t cp;
        size_t   adv;
        if (usr_utf8_decode(s + i, len - i, &cp, &adv) < 0) return -1;
        i += adv;
    }
    return 0;
}

/* ============================================================
   Count codepoints
   ============================================================ */

int64_t usr_utf8_codepoint_count(const uint8_t *s, size_t len) {
    if (!s) return -1;
    int64_t count = 0;
    size_t  i     = 0;
    while (i < len) {
        uint32_t cp;
        size_t   adv;
        if (usr_utf8_decode(s + i, len - i, &cp, &adv) < 0) return -1;
        count++;
        i += adv;
    }
    return count;
}

/* ============================================================
   Count UTF-16 code units
   ============================================================ */

int64_t usr_utf8_utf16_units(const uint8_t *s, size_t len) {
    if (!s) return -1;
    int64_t units = 0;
    size_t  i     = 0;
    while (i < len) {
        uint32_t cp;
        size_t   adv;
        if (usr_utf8_decode(s + i, len - i, &cp, &adv) < 0) return -1;
        units += (cp > 0xFFFFu) ? 2 : 1;
        i     += adv;
    }
    return units;
}

/* ============================================================
   Byte offset from UTF-16 offset
   ============================================================ */

size_t usr_utf8_byte_offset_from_utf16(
    const uint8_t *s,
    size_t         byte_len,
    uint32_t       utf16_offset
) {
    if (!s) return (size_t)-1;
    uint32_t units = 0;
    size_t   i     = 0;

    while (i < byte_len) {
        if (units == utf16_offset) return i;
        uint32_t cp;
        size_t   adv;
        if (usr_utf8_decode(s + i, byte_len - i, &cp, &adv) < 0) {
            return (size_t)-1;
        }
        units += (cp > 0xFFFFu) ? 2u : 1u;
        i     += adv;
    }
    if (units == utf16_offset) return i; /* end of string */
    return (size_t)-1;
}

/* ============================================================
   UTF-16 offset from byte offset
   ============================================================ */

uint32_t usr_utf8_utf16_offset_from_byte(
    const uint8_t *s,
    size_t         byte_len,
    size_t         byte_offset
) {
    if (!s || byte_offset > byte_len) return (uint32_t)-1;
    uint32_t units = 0;
    size_t   i     = 0;

    while (i < byte_offset) {
        uint32_t cp;
        size_t   adv;
        if (usr_utf8_decode(s + i, byte_len - i, &cp, &adv) < 0) {
            return (uint32_t)-1;
        }
        units += (cp > 0xFFFFu) ? 2u : 1u;
        i     += adv;
    }
    return units;
}
