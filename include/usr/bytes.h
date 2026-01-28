#ifndef USR_BYTES_H
#define USR_BYTES_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *data;
    size_t len;
    size_t cap;
} usr_bytes;

typedef struct {
    const uint8_t *data;
    size_t len;
} usr_bytes_view;

usr_bytes usr_bytes_alloc(size_t cap);
void usr_bytes_free(usr_bytes *b);
usr_bytes_view usr_bytes_view_from(const void *data, size_t len);

#endif
