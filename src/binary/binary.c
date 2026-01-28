#include "usr/binary.h"
#include <string.h>
#include <stdlib.h>

/* Original API */
usr_bytes usr_binary_from_text(const char *text) {
    size_t len = strlen(text);
    usr_bytes b = usr_bytes_alloc(len);
    memcpy(b.data, text, len);
    b.len = len;
    return b;
}

/* ABI-safe wrapper for Python / ctypes */
void usr_binary_from_text_out(const char *text, usr_bytes *out) {
    if (!out) return;
    *out = usr_binary_from_text(text);
}

/* Binary → text */
char* usr_text_from_binary(const usr_bytes_view *bin) {
    if (!bin || !bin->data) return NULL;
    char *out = (char*)malloc(bin->len + 1);
    memcpy(out, bin->data, bin->len);
    out[bin->len] = 0;
    return out;
}
