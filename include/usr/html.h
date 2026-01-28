#ifndef USR_HTML_H
#define USR_HTML_H

#include <stddef.h>
#include "entities.h"

#ifdef __cplusplus
extern "C" {
#endif

/* HTML → entities */
size_t usr_html_parse(
    const char *html,
    usr_entity *out,
    size_t max_entities
);

/* Entities → HTML (simple) */
char *usr_entities_to_html(
    const char *text,
    const usr_entity *entities,
    size_t count
);

/* 🔑 ADD THIS: round-trip safe version */
char *usr_entities_to_html_rt(
    const char *text,
    usr_entity *entities,
    size_t count
);

#ifdef __cplusplus
}
#endif

#endif
