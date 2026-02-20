#ifndef USR_STRBUILDER_H
#define USR_STRBUILDER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   usr_sb — String Builder
   Grows dynamically, always NUL-terminated.
   ============================================================ */

typedef struct {
    char  *buf;
    size_t len;   /* bytes written (excluding NUL) */
    size_t cap;   /* total allocated bytes (including NUL slot) */
} usr_sb;

/* Initialize a string builder with optional preallocated capacity.
   Pass hint=0 to use a default initial size. */
void usr_sb_init(usr_sb *sb, size_t hint);

/* Free internal buffer. sb is left zeroed. */
void usr_sb_free(usr_sb *sb);

/* Reset length to zero without freeing memory. */
void usr_sb_clear(usr_sb *sb);

/* Append `len` raw bytes. Returns 0 on success, -1 on OOM. */
int usr_sb_append(usr_sb *sb, const char *data, size_t len);

/* Append a NUL-terminated C string. Returns 0 on success, -1 on OOM. */
int usr_sb_appends(usr_sb *sb, const char *s);

/* Append a single character. Returns 0 on success, -1 on OOM. */
int usr_sb_appendc(usr_sb *sb, char c);

/* Append using printf-style format. Returns 0 on success, -1 on OOM. */
int usr_sb_appendf(usr_sb *sb, const char *fmt, ...);

/* Ensure capacity for at least `extra` more bytes.
   Returns 0 on success, -1 on OOM. */
int usr_sb_reserve(usr_sb *sb, size_t extra);

/* Return current NUL-terminated string. Never NULL (may be ""). */
const char *usr_sb_str(const usr_sb *sb);

/* Return current length (excluding NUL). */
size_t usr_sb_len(const usr_sb *sb);

/* Detach and return the internal buffer (caller owns, must free()).
   sb is left zeroed. Returns NULL on OOM if sb was empty. */
char *usr_sb_detach(usr_sb *sb);

#ifdef __cplusplus
}
#endif

#endif /* USR_STRBUILDER_H */
