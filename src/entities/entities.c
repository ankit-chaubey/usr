#include "usr/entities.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================
   Entity type names
   ============================================================ */

const char *usr_entity_type_name(usr_entity_type t) {
    switch (t) {
        case USR_ENTITY_BOLD:          return "bold";
        case USR_ENTITY_ITALIC:        return "italic";
        case USR_ENTITY_UNDERLINE:     return "underline";
        case USR_ENTITY_STRIKETHROUGH: return "strikethrough";
        case USR_ENTITY_SPOILER:       return "spoiler";
        case USR_ENTITY_CODE:          return "code";
        case USR_ENTITY_PRE:           return "pre";
        case USR_ENTITY_TEXT_LINK:     return "text_link";
        case USR_ENTITY_CUSTOM_EMOJI:  return "custom_emoji";
        case USR_ENTITY_BLOCKQUOTE:    return "blockquote";
        case USR_ENTITY_MENTION:       return "mention";
        case USR_ENTITY_HASHTAG:       return "hashtag";
        case USR_ENTITY_CASHTAG:       return "cashtag";
        case USR_ENTITY_BOT_COMMAND:   return "bot_command";
        case USR_ENTITY_URL:           return "url";
        case USR_ENTITY_EMAIL:         return "email";
        case USR_ENTITY_PHONE_NUMBER:  return "phone_number";
        case USR_ENTITY_TEXT_MENTION:  return "text_mention";
        default:                       return "unknown";
    }
}

void usr_entity_copy(usr_entity *dst, const usr_entity *src) {
    if (!dst || !src) return;
    *dst = *src;
    if (src->extra) {
        size_t len = strlen(src->extra);
        char *copy  = (char *)malloc(len + 1);
        if (copy) { memcpy(copy, src->extra, len + 1); }
        dst->extra = copy;
    }
}

void usr_entity_free_extra(usr_entity *e) {
    if (e && e->extra) {
        free((void *)e->extra);
        e->extra = NULL;
    }
}

/* ============================================================
   Sort comparator
   Primary:   smaller offset
   Secondary: larger length (outer span first)
   ============================================================ */

static int entity_cmp(const void *a, const void *b) {
    const usr_entity *x = (const usr_entity *)a;
    const usr_entity *y = (const usr_entity *)b;

    if (x->offset != y->offset)
        return (x->offset < y->offset) ? -1 : 1;

    /* Tie: longer entity first (outer) */
    if (x->length != y->length)
        return (x->length > y->length) ? -1 : 1;

    return 0;
}

/* ============================================================
   usr_entities_normalize
   Sorts and removes invalid/crossing spans.
   Properly-nested entities (inner contained in outer) are KEPT.
   ============================================================ */

size_t usr_entities_normalize(usr_entity *e, size_t n) {
    if (!e || n == 0) return 0;

    /* 1. Drop zero-length entities */
    size_t valid = 0;
    for (size_t i = 0; i < n; i++) {
        if (e[i].length > 0) e[valid++] = e[i];
    }
    n = valid;
    if (n == 0) return 0;

    /* 2. Sort */
    qsort(e, n, sizeof(usr_entity), entity_cmp);

    /*
     * 3. Remove crossing (non-properly-nested) entities.
     *
     * We maintain a stack of "active" open intervals.
     * An entity is CROSSING if it starts inside a previous entity
     * but ends outside it.
     *
     * Properly nested:
     *   outer [0, 10]
     *   inner [2, 6]  → valid: 2 >= 0 and 2+4 = 6 <= 10
     *
     * Crossing:
     *   first [0, 6]
     *   second [4, 10] → second.end (14) > first.end (6) — invalid
     */
    typedef struct {
        uint32_t end;
    } interval;

    interval stack[64];
    int      sp = 0;
    size_t   out = 0;

    for (size_t i = 0; i < n; i++) {
        uint32_t start = e[i].offset;
        uint32_t end   = e[i].offset + e[i].length;

        /* Pop expired intervals */
        while (sp > 0 && stack[sp-1].end <= start) sp--;

        if (sp == 0) {
            /* No active outer span: always accept */
            if (sp < 64) stack[sp++].end = end;
            e[out++] = e[i];
        } else {
            /* Check against the innermost active interval */
            uint32_t outer_end = stack[sp-1].end;
            if (end <= outer_end) {
                /* Fully nested: accept */
                if (sp < 64) stack[sp++].end = end;
                e[out++] = e[i];
            } else {
                /* Crossing: skip */
                (void)0;
            }
        }
    }

    return out;
}
