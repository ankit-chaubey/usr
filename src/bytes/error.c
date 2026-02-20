#include "usr/error.h"

const char *usr_strerror(usr_err e) {
    switch (e) {
        case USR_OK:           return "Success";
        case USR_ERR:          return "Generic error";
        case USR_ERR_NULL:     return "Null pointer";
        case USR_ERR_OOM:      return "Out of memory";
        case USR_ERR_INVAL:    return "Invalid argument";
        case USR_ERR_OVERFLOW: return "Buffer overflow";
        case USR_ERR_IO:       return "I/O error";
        case USR_ERR_ENCODING: return "Encoding error";
        case USR_ERR_CRYPTO:   return "Cryptographic error";
        case USR_ERR_TRUNCATED:return "Truncated input";
        case USR_ERR_FORMAT:   return "Malformed input";
        default:               return "Unknown error";
    }
}
