#ifndef USR_MEDIA_H
#define USR_MEDIA_H

#include "bytes.h"

usr_bytes usr_media_read_file(const char *path);
int usr_media_write_file(const char *path, const usr_bytes_view *data);

#endif
