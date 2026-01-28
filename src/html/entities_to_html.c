#include "usr/html.h"
#include <stdlib.h>
#include <string.h>

static const char *open_tag(usr_entity_type t, const char *extra) {
    switch (t) {
        case USR_ENTITY_BOLD: return "<b>";
        case USR_ENTITY_ITALIC: return "<i>";
        case USR_ENTITY_UNDERLINE: return "<u>";
        case USR_ENTITY_STRIKETHROUGH: return "<s>";
        case USR_ENTITY_SPOILER: return "<tg-spoiler>";
        case USR_ENTITY_CODE: return "<code>";
        case USR_ENTITY_PRE: return "<pre>";
        case USR_ENTITY_TEXT_LINK: return "<a href=\"";
        default: return "";
    }
}

static const char *close_tag(usr_entity_type t) {
    switch (t) {
        case USR_ENTITY_TEXT_LINK: return "</a>";
        case USR_ENTITY_SPOILER: return "</tg-spoiler>";
        case USR_ENTITY_PRE: return "</pre>";
        case USR_ENTITY_CODE: return "</code>";
        default: return "</>";
    }
}

char *usr_entities_to_html(
    const char *text,
    const usr_entity *e,
    size_t n
) {
    char *out = malloc(strlen(text)*4);
    strcpy(out, text);

    /* Simplified v1: assumes sorted entities */
    for (size_t i = 0; i < n; i++) {
        strcat(out, open_tag(e[i].type, e[i].extra));
        strcat(out, close_tag(e[i].type));
    }

    return out;
}
