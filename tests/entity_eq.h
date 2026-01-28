#ifndef USR_ENTITY_EQ_H
#define USR_ENTITY_EQ_H

#include "usr/entities.h"

static int usr_entities_equal(
    const usr_entity *a, size_t na,
    const usr_entity *b, size_t nb
) {
    if (na != nb) return 0;
    for (size_t i = 0; i < na; i++) {
        if (a[i].type   != b[i].type)   return 0;
        if (a[i].offset != b[i].offset) return 0;
        if (a[i].length != b[i].length) return 0;
    }
    return 1;
}

#endif
