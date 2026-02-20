#ifndef USR_ENCODING_H
#define USR_ENCODING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   Base64  (RFC 4648 standard alphabet + URL-safe variant)
   ============================================================ */

/* Returns the number of bytes needed to encode `in_len` bytes.
   Includes NUL terminator. */
size_t usr_base64_enc_size(size_t in_len);

/* Returns the maximum number of bytes decoded from `enc_len` encoded bytes.
   Actual decoded length may be less (due to padding). */
size_t usr_base64_dec_size(size_t enc_len);

/* Encode binary data to standard Base64 (with '=' padding).
   `out` must be at least usr_base64_enc_size(in_len) bytes.
   Returns number of characters written (excluding NUL). */
size_t usr_base64_encode(const uint8_t *in, size_t in_len, char *out);

/* Decode standard Base64 (with or without padding).
   `out` must be at least usr_base64_dec_size(in_len) bytes.
   Returns number of bytes decoded, or (size_t)-1 on invalid input. */
size_t usr_base64_decode(const char *in, size_t in_len, uint8_t *out);

/* URL-safe Base64: uses '-' and '_' instead of '+' and '/'. No padding. */
size_t usr_base64url_encode(const uint8_t *in, size_t in_len, char *out);
size_t usr_base64url_decode(const char *in, size_t in_len, uint8_t *out);

/* Heap-allocated convenience wrappers (caller must free) */
char    *usr_base64_encode_alloc(const uint8_t *in, size_t in_len);
uint8_t *usr_base64_decode_alloc(const char *in, size_t in_len, size_t *out_len);
char    *usr_base64url_encode_alloc(const uint8_t *in, size_t in_len);
uint8_t *usr_base64url_decode_alloc(const char *in, size_t in_len, size_t *out_len);

/* ============================================================
   Hex (lowercase and uppercase)
   ============================================================ */

/* Encode binary to lowercase hex. `out` needs 2*in_len + 1 bytes. */
void usr_hex_encode(const uint8_t *in, size_t in_len, char *out);

/* Encode binary to uppercase hex. `out` needs 2*in_len + 1 bytes. */
void usr_hex_encode_upper(const uint8_t *in, size_t in_len, char *out);

/* Decode hex string (case-insensitive) to binary.
   `out` needs in_len/2 bytes.
   Returns number of bytes decoded, or (size_t)-1 on invalid hex. */
size_t usr_hex_decode(const char *in, size_t in_len, uint8_t *out);

/* Heap-allocated convenience wrappers (caller must free) */
char    *usr_hex_encode_alloc(const uint8_t *in, size_t in_len);
uint8_t *usr_hex_decode_alloc(const char *in, size_t in_len, size_t *out_len);

/* ============================================================
   URL Encoding  (application/x-www-form-urlencoded + RFC 3986)
   ============================================================ */

/* URL-encode a string. Encodes all non-unreserved chars as %XX.
   `out` needs at most 3*in_len + 1 bytes.
   Returns number of bytes written (excluding NUL). */
size_t usr_url_encode(const char *in, size_t in_len, char *out);

/* URL-decode. `out` needs at most in_len + 1 bytes.
   Returns number of bytes decoded, or (size_t)-1 on invalid %XX. */
size_t usr_url_decode(const char *in, size_t in_len, char *out);

/* Heap-allocated wrappers (caller must free) */
char *usr_url_encode_alloc(const char *in, size_t in_len);
char *usr_url_decode_alloc(const char *in, size_t in_len);

/* ============================================================
   HTML Entity Escaping
   ============================================================ */

/* Escape &, <, >, ", ' to HTML entities.
   `out` needs at most 6*in_len + 1 bytes.
   Returns bytes written (excluding NUL). */
size_t usr_html_escape(const char *in, size_t in_len, char *out);

/* Unescape &amp; &lt; &gt; &quot; &apos; and numeric &#NNN; &#xNN;.
   `out` needs at most in_len + 1 bytes.
   Returns bytes written (excluding NUL), or (size_t)-1 on error. */
size_t usr_html_unescape(const char *in, size_t in_len, char *out);

/* Heap-allocated wrappers */
char *usr_html_escape_alloc(const char *in, size_t in_len);
char *usr_html_unescape_alloc(const char *in, size_t in_len);

#ifdef __cplusplus
}
#endif

#endif /* USR_ENCODING_H */
