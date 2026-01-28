#include "usr/markdown.h"
#include "usr/utf8.h"
#include <stdlib.h>
#include <string.h>

/*
 Best-effort Telegram-style Markdown parser.

 Rules:
 - Markers are visible characters (counted in UTF-16)
 - Malformed formatting is ignored
 - No formatting inside code
 - Overlaps resolved by normalization
*/

static int match2(const char *s, char a, char b) {
    return s[0] == a && s[1] == b;
}

size_t usr_markdown_parse(
    const char *text,
    usr_markdown_version version,
    usr_entity *out,
    size_t max
) {
    size_t count = 0;
    uint32_t utf16_pos = 0;

    int bold = 0, italic = 0, strike = 0, code = 0;
    int underline = 0, spoiler = 0;

    uint32_t sb = 0, si = 0, ss = 0, sc = 0, su = 0, sp = 0;

    (void)version;

    for (size_t i = 0; text[i] && count < max; ) {

        /* ---------- CODE (`) ---------- */
        if (text[i] == '`') {
            if (!code) {
                code = 1;
                sc = utf16_pos;
            } else if (utf16_pos > sc) {
                out[count++] = (usr_entity){
                    USR_ENTITY_CODE, sc, utf16_pos - sc, NULL
                };
                code = 0;
            }
            i++;
            utf16_pos++;   /* marker is visible */
            continue;
        }

        /* Inside code: everything is literal */
        if (code) {
            uint32_t cp; size_t adv;
            usr_utf8_decode((const uint8_t*)&text[i], 4, &cp, &adv);
            utf16_pos += (cp > 0xFFFF) ? 2 : 1;
            i += adv;
            continue;
        }

        /* ---------- SPOILER (||) ---------- */
        if (match2(&text[i], '|', '|')) {
            if (!spoiler) {
                spoiler = 1;
                sp = utf16_pos;
            } else if (utf16_pos > sp) {
                out[count++] = (usr_entity){
                    USR_ENTITY_SPOILER, sp, utf16_pos - sp, NULL
                };
                spoiler = 0;
            }
            i += 2;
            utf16_pos += 2;
            continue;
        }

        /* ---------- UNDERLINE (__) ---------- */
        if (match2(&text[i], '_', '_')) {
            if (!underline) {
                underline = 1;
                su = utf16_pos;
            } else if (utf16_pos > su) {
                out[count++] = (usr_entity){
                    USR_ENTITY_UNDERLINE, su, utf16_pos - su, NULL
                };
                underline = 0;
            }
            i += 2;
            utf16_pos += 2;
            continue;
        }

        /* ---------- BOLD (*) ---------- */
        if (text[i] == '*') {
            if (!bold) {
                bold = 1;
                sb = utf16_pos;
            } else if (utf16_pos > sb) {
                out[count++] = (usr_entity){
                    USR_ENTITY_BOLD, sb, utf16_pos - sb, NULL
                };
                bold = 0;
            }
            i++;
            utf16_pos++;
            continue;
        }

        /* ---------- ITALIC (_) ---------- */
        if (text[i] == '_') {
            if (!italic) {
                italic = 1;
                si = utf16_pos;
            } else if (utf16_pos > si) {
                out[count++] = (usr_entity){
                    USR_ENTITY_ITALIC, si, utf16_pos - si, NULL
                };
                italic = 0;
            }
            i++;
            utf16_pos++;
            continue;
        }

        /* ---------- STRIKE (~) ---------- */
        if (text[i] == '~') {
            if (!strike) {
                strike = 1;
                ss = utf16_pos;
            } else if (utf16_pos > ss) {
                out[count++] = (usr_entity){
                    USR_ENTITY_STRIKETHROUGH, ss, utf16_pos - ss, NULL
                };
                strike = 0;
            }
            i++;
            utf16_pos++;
            continue;
        }

        /* ---------- NORMAL UTF-8 ---------- */
        uint32_t cp;
        size_t adv;
        usr_utf8_decode((const uint8_t*)&text[i], 4, &cp, &adv);
        utf16_pos += (cp > 0xFFFF) ? 2 : 1;
        i += adv;
    }

    /* Unclosed markers are ignored (best-effort) */
    return count;
}
