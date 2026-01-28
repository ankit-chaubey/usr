#ifndef USR_BINARY_H
#define USR_BINARY_H

#include "bytes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Original API (OK for C, NOT for ctypes) */
usr_bytes usr_binary_from_text(const char *text);

/* ABI-safe wrapper (REQUIRED for ctypes / Python) */
void usr_binary_from_text_out(const char *text, usr_bytes *out);

/* Binary → text */
char* usr_text_from_binary(const usr_bytes_view *bin);

#ifdef __cplusplus
}
#endif

#endif
