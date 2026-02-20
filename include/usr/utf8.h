#ifndef USR_UTF8_H
#define USR_UTF8_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   UTF-8 / UTF-16 Utilities
   ============================================================ */

/* Decode one UTF-8 codepoint from `s` (at most `len` bytes).
   On success: fills *codepoint and *advance, returns 0.
   On error: returns -1 (invalid sequence, truncated, etc.).
   *advance is always set to at least 1 to allow safe iteration. */
int usr_utf8_decode(
    const uint8_t *s,
    size_t         len,
    uint32_t      *codepoint,
    size_t        *advance
);

/* Encode a single Unicode codepoint into UTF-8.
   `out` must have at least 4 bytes.
   Returns number of bytes written, or -1 if codepoint is invalid. */
int usr_utf8_encode(uint32_t codepoint, uint8_t *out);

/* Validate an entire UTF-8 string.
   Returns 0 if valid, -1 if invalid.
   `len` = number of bytes (not including any NUL). */
int usr_utf8_validate(const uint8_t *s, size_t len);

/* Count the number of Unicode codepoints in a UTF-8 string.
   Returns -1 if the string contains invalid sequences. */
int64_t usr_utf8_codepoint_count(const uint8_t *s, size_t len);

/* Return the UTF-16 code unit count for a UTF-8 string.
   Codepoints > U+FFFF count as 2 UTF-16 units (surrogate pairs).
   Returns -1 on invalid UTF-8. */
int64_t usr_utf8_utf16_units(const uint8_t *s, size_t len);

/* Convert a UTF-16 offset (in code units) to a byte offset in a UTF-8 string.
   Returns the byte offset, or (size_t)-1 if utf16_offset is out of range. */
size_t usr_utf8_byte_offset_from_utf16(
    const uint8_t *s,
    size_t         byte_len,
    uint32_t       utf16_offset
);

/* Convert a byte offset in a UTF-8 string to a UTF-16 offset.
   Returns the UTF-16 offset, or (uint32_t)-1 if byte_offset is out of range. */
uint32_t usr_utf8_utf16_offset_from_byte(
    const uint8_t *s,
    size_t         byte_len,
    size_t         byte_offset
);

/* How many UTF-16 units does this codepoint occupy? */
static inline int usr_codepoint_utf16_units(uint32_t cp) {
    return (cp > 0xFFFFu) ? 2 : 1;
}

#ifdef __cplusplus
}
#endif

#endif /* USR_UTF8_H */
