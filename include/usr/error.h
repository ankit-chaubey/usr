#ifndef USR_ERROR_H
#define USR_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   Universal Systems Runtime — Error Codes
   ============================================================ */

typedef enum {
    USR_OK              =  0,   /* Success */
    USR_ERR             = -1,   /* Generic error */
    USR_ERR_NULL        = -2,   /* Null pointer argument */
    USR_ERR_OOM         = -3,   /* Out of memory */
    USR_ERR_INVAL       = -4,   /* Invalid argument (wrong size, bad data) */
    USR_ERR_OVERFLOW    = -5,   /* Output buffer too small */
    USR_ERR_IO          = -6,   /* I/O / file error */
    USR_ERR_ENCODING    = -7,   /* Encoding error (bad UTF-8, bad base64, etc.) */
    USR_ERR_CRYPTO      = -8,   /* Cryptographic error */
    USR_ERR_TRUNCATED   = -9,   /* Input was truncated / incomplete */
    USR_ERR_FORMAT      = -10,  /* Malformed input */
} usr_err;

/* Returns a human-readable string for an error code */
const char *usr_strerror(usr_err e);

#ifdef __cplusplus
}
#endif

#endif /* USR_ERROR_H */
