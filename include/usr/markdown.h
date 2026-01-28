#ifndef USR_MARKDOWN_H
#define USR_MARKDOWN_H

#include <stddef.h>     // size_t
#include <stdint.h>     // uint32_t
#include "entities.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    USR_MD_V1,
    USR_MD_V2
} usr_markdown_version;

/* Markdown → entities */
size_t usr_markdown_parse(
    const char *text,
    usr_markdown_version version,
    usr_entity *out,
    size_t max_entities
);

/* Entities → Markdown (V2 canonical) */
char *usr_entities_to_markdown(
    const char *text,
    const usr_entity *entities,
    size_t count,
    usr_markdown_version version
);

#ifdef __cplusplus
}
#endif

#endif /* USR_MARKDOWN_H */
