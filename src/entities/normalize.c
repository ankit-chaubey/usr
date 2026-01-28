#include "usr/entities.h"
#include <stdlib.h>

/*
 Sort rule:
 1) smaller offset first
 2) if same offset → longer entity first
*/
static int cmp(const void *a, const void *b) {
    const usr_entity *x = a;
    const usr_entity *y = b;

    if (x->offset != y->offset)
        return (int)x->offset - (int)y->offset;

    /* longer first */
    return (int)y->length - (int)x->length;
}

size_t usr_entities_normalize(usr_entity *e, size_t n) {
    if (!e || n == 0) return 0;

    qsort(e, n, sizeof(usr_entity), cmp);

    /*
     Telegram rule:
     - ANY overlapping entities are illegal
     - Leftmost + longest wins
     - Drop all inner overlaps (even same type)
    */
    for (size_t i = 1; i < n; i++) {
        uint32_t prev_start = e[i - 1].offset;
        uint32_t prev_end   = e[i - 1].offset + e[i - 1].length;

        if (e[i].offset < prev_end) {
            /* overlapping → drop */
            e[i].length = 0;
        }
    }

    /* compact in-place */
    size_t out = 0;
    for (size_t i = 0; i < n; i++) {
        if (e[i].length == 0) continue;
        e[out++] = e[i];
    }

    return out;
}
