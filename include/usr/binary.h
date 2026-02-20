#ifndef USR_BINARY_H
#define USR_BINARY_H

#include "usr/bytes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   Binary ↔ Text Conversion
   ============================================================ */

/* Convert a NUL-terminated UTF-8 string to a usr_bytes buffer.
   The returned buffer contains the raw UTF-8 bytes (no NUL).
   Returns an empty buffer (data=NULL) on OOM. */
usr_bytes usr_binary_from_text(const char *text);

/* ABI-safe ctypes/FFI wrapper: writes result into *out.
   Safe to call from Python/ctypes. */
void usr_binary_from_text_out(const char *text, usr_bytes *out);

/* Convert a binary buffer to a heap-allocated NUL-terminated string.
   The caller must free() the returned pointer.
   Assumes the buffer contains valid UTF-8.
   Returns NULL on OOM or NULL input. */
char *usr_text_from_binary(const usr_bytes_view *bin);

/* Same but takes raw pointer + length (convenient for C callers). */
char *usr_text_from_ptr(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* USR_BINARY_H */
