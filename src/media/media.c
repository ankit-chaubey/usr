#include "usr/media.h"
#include <stdio.h>

usr_bytes usr_media_read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return usr_bytes_alloc(0);

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    usr_bytes b = usr_bytes_alloc(size);
    fread(b.data, 1, size, f);
    b.len = size;

    fclose(f);
    return b;
}

int usr_media_write_file(const char *path, const usr_bytes_view *data) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fwrite(data->data, 1, data->len, f);
    fclose(f);
    return 0;
}
