#include "usr/html.h"
#include "usr/utf8.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    usr_entity_type type;
    uint32_t start;
    const char *extra;
} tag_ctx;

static int starts(const char *s, const char *t) {
    return strncmp(s, t, strlen(t)) == 0;
}

size_t usr_html_parse(
    const char *html,
    usr_entity *out,
    size_t max
) {
    tag_ctx stack[32];
    int sp = 0;
    size_t count = 0;

    uint32_t utf16_pos = 0;

    for (size_t i = 0; html[i]; ) {

        /* ---------- TAG OPEN ---------- */
        if (html[i] == '<') {

            /* BOLD */
            if (starts(&html[i], "<b>") || starts(&html[i], "<strong>")) {
                stack[sp++] = (tag_ctx){USR_ENTITY_BOLD, utf16_pos, NULL};
                i += html[i+2]=='>' ? 3 : 8;
                continue;
            }

            /* ITALIC */
            if (starts(&html[i], "<i>") || starts(&html[i], "<em>")) {
                stack[sp++] = (tag_ctx){USR_ENTITY_ITALIC, utf16_pos, NULL};
                i += html[i+2]=='>' ? 3 : 4;
                continue;
            }

            /* UNDERLINE */
            if (starts(&html[i], "<u>")) {
                stack[sp++] = (tag_ctx){USR_ENTITY_UNDERLINE, utf16_pos, NULL};
                i += 3;
                continue;
            }

            /* STRIKE */
            if (starts(&html[i], "<s>") ||
                starts(&html[i], "<del>") ||
                starts(&html[i], "<strike>")) {
                stack[sp++] = (tag_ctx){USR_ENTITY_STRIKETHROUGH, utf16_pos, NULL};
                i += (html[i+2]=='>' ? 3 : 8);
                continue;
            }

            /* SPOILER */
            if (starts(&html[i], "<tg-spoiler>")) {
                stack[sp++] = (tag_ctx){USR_ENTITY_SPOILER, utf16_pos, NULL};
                i += 13;
                continue;
            }

            /* LINK */
            if (starts(&html[i], "<a href=\"")) {
                const char *h = &html[i+9];
                const char *e = strchr(h, '"');
                stack[sp++] = (tag_ctx){
                    USR_ENTITY_TEXT_LINK,
                    utf16_pos,
                    strndup(h, e-h)
                };
                i = (e - html) + 2;
                continue;
            }

            /* TAG CLOSE */
            if (html[i+1] == '/' && sp > 0) {
                tag_ctx ctx = stack[--sp];
                out[count++] = (usr_entity){
                    ctx.type,
                    ctx.start,
                    utf16_pos - ctx.start,
                    ctx.extra
                };
                i = strchr(&html[i], '>') - html + 1;
                continue;
            }

            /* Unknown tag → skip */
            i = strchr(&html[i], '>') - html + 1;
            continue;
        }

        /* ---------- TEXT ---------- */
        uint32_t cp;
        size_t adv;
        usr_utf8_decode((uint8_t*)&html[i], 4, &cp, &adv);
        utf16_pos += (cp > 0xFFFF) ? 2 : 1;
        i += adv;
    }

    return count;
}
