#ifndef USR_VERSION_H
#define USR_VERSION_H

#define USR_VERSION_MAJOR 0
#define USR_VERSION_MINOR 1
#define USR_VERSION_PATCH 3
#define USR_VERSION_STR   "0.1.3"

/* Build a single 32-bit version integer for easy comparison */
#define USR_VERSION_NUM \
    ((USR_VERSION_MAJOR << 16) | (USR_VERSION_MINOR << 8) | USR_VERSION_PATCH)

#endif /* USR_VERSION_H */
