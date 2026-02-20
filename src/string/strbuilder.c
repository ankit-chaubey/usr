#include "usr/strbuilder.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define SB_INITIAL 64

void usr_sb_init(usr_sb *sb, size_t hint) {
    if (!sb) return;
    size_t cap = (hint > 0) ? hint + 1 : SB_INITIAL;
    sb->buf = (char *)malloc(cap);
    if (sb->buf) {
        sb->buf[0] = '\0';
        sb->len = 0;
        sb->cap = cap;
    } else {
        sb->buf = NULL;
        sb->len = 0;
        sb->cap = 0;
    }
}

void usr_sb_free(usr_sb *sb) {
    if (!sb) return;
    free(sb->buf);
    sb->buf = NULL;
    sb->len = 0;
    sb->cap = 0;
}

void usr_sb_clear(usr_sb *sb) {
    if (!sb) return;
    sb->len = 0;
    if (sb->buf) sb->buf[0] = '\0';
}

int usr_sb_reserve(usr_sb *sb, size_t extra) {
    if (!sb) return -1;
    size_t needed = sb->len + extra + 1; /* +1 for NUL */
    if (sb->cap >= needed) return 0;

    size_t new_cap = (sb->cap < SB_INITIAL) ? SB_INITIAL : sb->cap;
    while (new_cap < needed) new_cap *= 2;

    char *p = (char *)realloc(sb->buf, new_cap);
    if (!p) return -1;
    sb->buf = p;
    sb->cap = new_cap;
    return 0;
}

int usr_sb_append(usr_sb *sb, const char *data, size_t len) {
    if (!sb || (!data && len > 0)) return -1;
    if (len == 0) return 0;
    if (usr_sb_reserve(sb, len) < 0) return -1;
    memcpy(sb->buf + sb->len, data, len);
    sb->len += len;
    sb->buf[sb->len] = '\0';
    return 0;
}

int usr_sb_appends(usr_sb *sb, const char *s) {
    if (!s) return -1;
    return usr_sb_append(sb, s, strlen(s));
}

int usr_sb_appendc(usr_sb *sb, char c) {
    return usr_sb_append(sb, &c, 1);
}

int usr_sb_appendf(usr_sb *sb, const char *fmt, ...) {
    if (!sb || !fmt) return -1;

    /* Two-pass: measure, then write */
    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    int n = vsnprintf(NULL, 0, fmt, args1);
    va_end(args1);

    if (n < 0) { va_end(args2); return -1; }

    if (usr_sb_reserve(sb, (size_t)n) < 0) { va_end(args2); return -1; }

    vsnprintf(sb->buf + sb->len, (size_t)n + 1, fmt, args2);
    va_end(args2);

    sb->len += (size_t)n;
    return 0;
}

const char *usr_sb_str(const usr_sb *sb) {
    if (!sb || !sb->buf) return "";
    return sb->buf;
}

size_t usr_sb_len(const usr_sb *sb) {
    if (!sb) return 0;
    return sb->len;
}

char *usr_sb_detach(usr_sb *sb) {
    if (!sb) return NULL;
    char *p = sb->buf;
    if (!p) {
        /* Return a valid empty string if nothing was written */
        p = (char *)malloc(1);
        if (p) p[0] = '\0';
    }
    sb->buf = NULL;
    sb->len = 0;
    sb->cap = 0;
    return p;
}
