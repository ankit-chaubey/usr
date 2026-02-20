#include "usr/media.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

usr_bytes usr_media_read_file(const char *path) {
    if (!path) return usr_bytes_empty();

    FILE *f = fopen(path, "rb");
    if (!f) return usr_bytes_empty();

    /* Get file size */
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return usr_bytes_empty(); }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return usr_bytes_empty(); }
    rewind(f);

    usr_bytes b = usr_bytes_alloc((size_t)sz + 1); /* +1 for optional NUL */
    if (!b.data) { fclose(f); return b; }

    size_t n = fread(b.data, 1, (size_t)sz, f);
    fclose(f);

    b.len = n;
    return b;
}

int usr_media_write_file(const char *path, const usr_bytes_view *data) {
    if (!path || !data) return -1;
    FILE *f = fopen(path, "wb");
    if (!f) return -1;

    if (data->data && data->len > 0) {
        size_t written = fwrite(data->data, 1, data->len, f);
        fclose(f);
        return (written == data->len) ? 0 : -1;
    }

    fclose(f);
    return 0;
}

int usr_media_append_file(const char *path, const usr_bytes_view *data) {
    if (!path || !data) return -1;
    FILE *f = fopen(path, "ab");
    if (!f) return -1;

    if (data->data && data->len > 0) {
        size_t written = fwrite(data->data, 1, data->len, f);
        fclose(f);
        return (written == data->len) ? 0 : -1;
    }

    fclose(f);
    return 0;
}

int64_t usr_media_file_size(const char *path) {
    if (!path) return -1;
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (int64_t)st.st_size;
}

int usr_media_file_exists(const char *path) {
    if (!path) return 0;
    struct stat st;
    return (stat(path, &st) == 0) ? 1 : 0;
}

int usr_media_copy_file(const char *src, const char *dst) {
    if (!src || !dst) return -1;

    FILE *in  = fopen(src, "rb");
    if (!in) return -1;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return -1; }

    char buf[65536];
    int  ok = 1;
    size_t n;

    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) { ok = 0; break; }
    }
    if (ferror(in)) ok = 0;

    fclose(in);
    fclose(out);
    return ok ? 0 : -1;
}
