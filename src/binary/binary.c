#include "usr/binary.h"
#include "usr/bytes.h"
#include <string.h>
#include <stdlib.h>

usr_bytes usr_binary_from_text(const char *text) {
    if (!text) return usr_bytes_empty();
    size_t    len = strlen(text);
    usr_bytes b   = usr_bytes_alloc(len > 0 ? len : 1);
    if (!b.data) return b;
    if (len > 0) memcpy(b.data, text, len);
    b.len = len;
    return b;
}

void usr_binary_from_text_out(const char *text, usr_bytes *out) {
    if (!out) return;
    *out = usr_binary_from_text(text);
}

char *usr_text_from_binary(const usr_bytes_view *bin) {
    if (!bin || !bin->data) return NULL;
    char *out = (char *)malloc(bin->len + 1);
    if (!out) return NULL;
    if (bin->len > 0) memcpy(out, bin->data, bin->len);
    out[bin->len] = '\0';
    return out;
}

char *usr_text_from_ptr(const uint8_t *data, size_t len) {
    if (!data && len > 0) return NULL;
    char *out = (char *)malloc(len + 1);
    if (!out) return NULL;
    if (len > 0) memcpy(out, data, len);
    out[len] = '\0';
    return out;
}
