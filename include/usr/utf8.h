#ifndef USR_UTF8_H
#define USR_UTF8_H

#include <stddef.h>
#include <stdint.h>

int usr_utf8_decode(
    const uint8_t *s,
    size_t len,
    uint32_t *codepoint,
    size_t *advance
);

#endif
