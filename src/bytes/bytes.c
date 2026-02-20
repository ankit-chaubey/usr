#include "usr/bytes.h"
#include <stdlib.h>
#include <string.h>

#define USR_BYTES_INITIAL_CAP 16

usr_bytes usr_bytes_alloc(size_t cap) {
    usr_bytes b;
    if (cap == 0) cap = USR_BYTES_INITIAL_CAP;
    b.data = (uint8_t *)malloc(cap);
    if (!b.data) {
        b.len = 0;
        b.cap = 0;
    } else {
        b.len = 0;
        b.cap = cap;
    }
    return b;
}

void usr_bytes_free(usr_bytes *b) {
    if (!b) return;
    if (b->data) {
        free(b->data);
        b->data = NULL;
    }
    b->len = 0;
    b->cap = 0;
}

usr_bytes usr_bytes_clone(const usr_bytes *src) {
    if (!src || !src->data || src->len == 0) {
        return usr_bytes_empty();
    }
    usr_bytes b = usr_bytes_alloc(src->len);
    if (!b.data) return b;
    memcpy(b.data, src->data, src->len);
    b.len = src->len;
    return b;
}

int usr_bytes_reserve(usr_bytes *b, size_t cap) {
    if (!b) return -1;
    if (b->cap >= cap) return 0;

    /* grow by doubling */
    size_t new_cap = (b->cap < USR_BYTES_INITIAL_CAP) ? USR_BYTES_INITIAL_CAP : b->cap;
    while (new_cap < cap) new_cap *= 2;

    uint8_t *p = (uint8_t *)realloc(b->data, new_cap);
    if (!p) return -1;
    b->data = p;
    b->cap  = new_cap;
    return 0;
}

int usr_bytes_append(usr_bytes *b, const void *data, size_t len) {
    if (!b || (!data && len > 0)) return -1;
    if (len == 0) return 0;
    if (usr_bytes_reserve(b, b->len + len) < 0) return -1;
    memcpy(b->data + b->len, data, len);
    b->len += len;
    return 0;
}

int usr_bytes_push(usr_bytes *b, uint8_t byte) {
    return usr_bytes_append(b, &byte, 1);
}

void usr_bytes_clear(usr_bytes *b) {
    if (b) b->len = 0;
}

usr_bytes_view usr_bytes_view_from(const void *data, size_t len) {
    usr_bytes_view v;
    v.data = (const uint8_t *)data;
    v.len  = len;
    return v;
}

usr_bytes_view usr_bytes_as_view(const usr_bytes *b) {
    usr_bytes_view v;
    if (!b || !b->data) {
        v.data = NULL;
        v.len  = 0;
    } else {
        v.data = b->data;
        v.len  = b->len;
    }
    return v;
}

int usr_bytes_eq(const usr_bytes_view *a, const usr_bytes_view *b) {
    if (!a || !b) return 0;
    if (a->len != b->len) return 0;
    if (a->len == 0) return 1;
    if (!a->data || !b->data) return 0;
    return memcmp(a->data, b->data, a->len) == 0;
}
