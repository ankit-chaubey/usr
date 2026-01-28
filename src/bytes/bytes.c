#include "usr/bytes.h"
#include <stdlib.h>

usr_bytes usr_bytes_alloc(size_t cap) {
    usr_bytes b;
    b.data = (uint8_t*)malloc(cap);
    b.len = 0;
    b.cap = cap;
    return b;
}

void usr_bytes_free(usr_bytes *b) {
    if (!b || !b->data) return;
    free(b->data);
    b->data = NULL;
    b->len = 0;
    b->cap = 0;
}

usr_bytes_view usr_bytes_view_from(const void *data, size_t len) {
    usr_bytes_view v;
    v.data = (const uint8_t*)data;
    v.len = len;
    return v;
}
