#ifndef USR_BYTES_H
#define USR_BYTES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   usr_bytes — owned growable byte buffer
   ============================================================ */
typedef struct {
    uint8_t *data;
    size_t   len;
    size_t   cap;
} usr_bytes;

/* ============================================================
   usr_bytes_view — non-owning read-only slice
   ============================================================ */
typedef struct {
    const uint8_t *data;
    size_t         len;
} usr_bytes_view;

/* ---------- Lifecycle ---------- */

/* Allocate a buffer with at least `cap` bytes capacity.
   Returns a zero-length buffer. data may be NULL on OOM. */
usr_bytes usr_bytes_alloc(size_t cap);

/* Free an owned buffer. Safe to call on zero-init / already-freed. */
void usr_bytes_free(usr_bytes *b);

/* Deep-copy a bytes object. Returns zero buffer on OOM. */
usr_bytes usr_bytes_clone(const usr_bytes *src);

/* ---------- Mutation ---------- */

/* Append bytes to a buffer, growing as needed.
   Returns 0 on success, -1 on OOM. */
int usr_bytes_append(usr_bytes *b, const void *data, size_t len);

/* Append a single byte.  Returns 0 on success, -1 on OOM. */
int usr_bytes_push(usr_bytes *b, uint8_t byte);

/* Ensure capacity is at least `cap`. Returns 0 on success, -1 on OOM. */
int usr_bytes_reserve(usr_bytes *b, size_t cap);

/* Reset length to zero without freeing. */
void usr_bytes_clear(usr_bytes *b);

/* ---------- Views ---------- */

/* Create a view from arbitrary memory. Does NOT take ownership. */
usr_bytes_view usr_bytes_view_from(const void *data, size_t len);

/* Create a view from an owned buffer. */
usr_bytes_view usr_bytes_as_view(const usr_bytes *b);

/* ---------- Comparison ---------- */

/* Returns 1 if contents are equal, 0 otherwise. */
int usr_bytes_eq(const usr_bytes_view *a, const usr_bytes_view *b);

/* ---------- Zero-init helpers ---------- */

/* Returns a zero-initialized empty buffer (data=NULL, len=0, cap=0). */
static inline usr_bytes usr_bytes_empty(void) {
    usr_bytes b;
    b.data = (uint8_t *)0;
    b.len  = 0;
    b.cap  = 0;
    return b;
}

#ifdef __cplusplus
}
#endif

#endif /* USR_BYTES_H */
