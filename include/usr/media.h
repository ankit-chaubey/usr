#ifndef USR_MEDIA_H
#define USR_MEDIA_H

#include "usr/bytes.h"
#include "usr/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   File I/O
   ============================================================ */

/* Read an entire file into a heap-allocated buffer.
   Returns a buffer with data!=NULL and len=file_size on success.
   Returns an empty buffer (data=NULL, len=0) on any error. */
usr_bytes usr_media_read_file(const char *path);

/* Write data to a file, creating or truncating it.
   Returns 0 on success, -1 on error (errno is set). */
int usr_media_write_file(const char *path, const usr_bytes_view *data);

/* Append data to a file (creates if not present).
   Returns 0 on success, -1 on error. */
int usr_media_append_file(const char *path, const usr_bytes_view *data);

/* Return file size in bytes, or -1 if the file doesn't exist or isn't accessible. */
int64_t usr_media_file_size(const char *path);

/* Check if a file exists and is readable.
   Returns 1 if yes, 0 if no. */
int usr_media_file_exists(const char *path);

/* Copy file from `src` to `dst`.
   Returns 0 on success, -1 on error. */
int usr_media_copy_file(const char *src, const char *dst);

#ifdef __cplusplus
}
#endif

#endif /* USR_MEDIA_H */
