#include "usr/html.h"
#include "usr/utf8.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t pos;      /* UTF-16 position */
    int open;          /* 1 = open, 0 = close */
    usr_entity_type type;
    const char *extra;
} event;

static const char *open_tag(usr_entity_type t, const char *e) {
    switch (t) {
        case USR_ENTITY_BOLD:          return "<b>";
        case USR_ENTITY_ITALIC:        return "<i>";
        case USR_ENTITY_UNDERLINE:     return "<u>";
        case USR_ENTITY_STRIKETHROUGH: return "<s>";
        case USR_ENTITY_SPOILER:       return "<tg-spoiler>";
        case USR_ENTITY_CODE:          return "<code>";
        case USR_ENTITY_PRE:           return "<pre>";
        case USR_ENTITY_TEXT_LINK:     return "<a href=\"";
        default:                       return "";
    }
}

static const char *close_tag(usr_entity_type t) {
    switch (t) {
        case USR_ENTITY_TEXT_LINK: return "</a>";
        case USR_ENTITY_SPOILER:   return "</tg-spoiler>";
        case USR_ENTITY_PRE:       return "</pre>";
        case USR_ENTITY_CODE:      return "</code>";
        case USR_ENTITY_BOLD:      return "</b>";
        case USR_ENTITY_ITALIC:    return "</i>";
        case USR_ENTITY_UNDERLINE: return "</u>";
        case USR_ENTITY_STRIKETHROUGH: return "</s>";
        default:                  return "";
    }
}

char *usr_entities_to_html_rt(
    const char *text,
    usr_entity *e,
    size_t n
) {
    size_t evn = n * 2;
    event *ev = calloc(evn, sizeof(event));

    for (size_t i = 0; i < n; i++) {
        ev[2*i]     = (event){ e[i].offset, 1, e[i].type, e[i].extra };
        ev[2*i + 1] = (event){ e[i].offset + e[i].length, 0, e[i].type, e[i].extra };
    }

    /* sort events: by pos, open before close */
    for (size_t i = 0; i < evn; i++) {
        for (size_t j = i + 1; j < evn; j++) {
            if (ev[j].pos < ev[i].pos ||
               (ev[j].pos == ev[i].pos && ev[j].open > ev[i].open)) {
                event tmp = ev[i];
                ev[i] = ev[j];
                ev[j] = tmp;
            }
        }
    }

    char *out = malloc(strlen(text) * 6 + 32); /* safe headroom */
    size_t oi = 0;
    size_t ti = 0;
    uint32_t utf16 = 0;

    for (size_t i = 0; text[ti]; ) {

        while (i < evn && ev[i].pos == utf16 && ev[i].open) {
            const char *t = open_tag(ev[i].type, ev[i].extra);
            size_t l = strlen(t);
            memcpy(out + oi, t, l);
            oi += l;
            i++;
        }

        /* UTF-8 safe copy */
        uint32_t cp;
        size_t adv;
        usr_utf8_decode((const uint8_t *)&text[ti], 4, &cp, &adv);

        memcpy(out + oi, text + ti, adv);
        oi += adv;
        ti += adv;

        utf16 += (cp > 0xFFFF) ? 2 : 1;

        while (i < evn && ev[i].pos == utf16 && !ev[i].open) {
            const char *t = close_tag(ev[i].type);
            size_t l = strlen(t);
            memcpy(out + oi, t, l);
            oi += l;
            i++;
        }
    }

    out[oi] = '\0';
    free(ev);
    return out;
}
