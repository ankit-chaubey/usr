#include "usr/html.h"
#include "usr/utf8.h"
#include "usr/strbuilder.h"
#include "usr/encoding.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

/* ============================================================
   Internal helpers
   ============================================================ */

static int starts_with_ci(const char *s, const char *t) {
    while (*t) {
        if (tolower((unsigned char)*s) != tolower((unsigned char)*t)) return 0;
        s++; t++;
    }
    return 1;
}

static const char *skip_to(const char *s, char c) {
    while (*s && *s != c) s++;
    return s;
}

/* Extract attribute value: src='...' or src="..." */
static char *extract_attr(const char *tag_start, const char *tag_end,
                           const char *attr_name) {
    size_t nlen = strlen(attr_name);
    const char *p = tag_start;
    while (p < tag_end) {
        /* Skip whitespace */
        while (p < tag_end && isspace((unsigned char)*p)) p++;
        if (p >= tag_end) break;

        /* Try to match attr_name */
        if ((size_t)(tag_end - p) > nlen &&
            strncasecmp(p, attr_name, nlen) == 0) {
            const char *q = p + nlen;
            while (q < tag_end && isspace((unsigned char)*q)) q++;
            if (q < tag_end && *q == '=') {
                q++;
                while (q < tag_end && isspace((unsigned char)*q)) q++;
                char delim = (*q == '"' || *q == '\'') ? *q++ : 0;
                const char *val_start = q;
                const char *val_end;
                if (delim) {
                    val_end = val_start;
                    while (val_end < tag_end && *val_end != delim) val_end++;
                } else {
                    val_end = val_start;
                    while (val_end < tag_end && !isspace((unsigned char)*val_end) &&
                           *val_end != '>') val_end++;
                }
                size_t vlen = (size_t)(val_end - val_start);
                char *result = (char *)malloc(vlen + 1);
                if (!result) return NULL;
                memcpy(result, val_start, vlen);
                result[vlen] = '\0';
                return result;
            }
        }
        /* Skip to next word */
        while (p < tag_end && !isspace((unsigned char)*p) && *p != '=') p++;
        if (p < tag_end && *p == '=') {
            p++;
            if (p < tag_end && (*p == '"' || *p == '\'')) {
                char d = *p++;
                while (p < tag_end && *p != d) p++;
                if (p < tag_end) p++;
            } else {
                while (p < tag_end && !isspace((unsigned char)*p)) p++;
            }
        }
    }
    return NULL;
}

/* ============================================================
   Tag context for the open-tag stack
   ============================================================ */

typedef struct {
    usr_entity_type type;
    uint32_t        utf16_start;
    char           *extra;       /* heap-allocated, may be NULL */
} tag_ctx;

/* ============================================================
   usr_html_parse
   HTML → plain text + entities
   ============================================================ */

size_t usr_html_parse(
    const char  *html,
    char       **plain_out,
    usr_entity  *entities_out,
    size_t       max_entities
) {
    if (!html) return (size_t)-1;

    usr_sb    plain;
    usr_sb_init(&plain, strlen(html));

    tag_ctx   stack[64];
    int       sp    = 0;
    size_t    count = 0;
    uint32_t  utf16_pos = 0;
    int       in_pre  = 0;  /* inside <pre> */
    int       in_code = 0;  /* inside <code> */

    const char *p = html;

    while (*p) {
        if (*p != '<') {
            /* ---------- Text content ---------- */
            /* Handle &amp; &lt; &gt; &quot; &apos; &#...; */
            if (*p == '&') {
                const char *semi = skip_to(p + 1, ';');
                if (*semi == ';' && (size_t)(semi - p) < 16) {
                    /* Decode entity into temp buffer */
                    char entity_raw[20];
                    size_t elen = (size_t)(semi - p) + 1;
                    memcpy(entity_raw, p, elen);
                    entity_raw[elen] = '\0';
                    char decoded[8];
                    size_t dlen = usr_html_unescape(entity_raw, elen, decoded);
                    if (dlen > 0 && dlen != (size_t)-1) {
                        usr_sb_append(&plain, decoded, dlen);
                        /* Count UTF-16 units in decoded */
                        size_t di = 0;
                        while (di < dlen) {
                            uint32_t cp; size_t adv;
                            if (usr_utf8_decode((const uint8_t*)decoded + di,
                                                dlen - di, &cp, &adv) == 0) {
                                utf16_pos += (cp > 0xFFFFu) ? 2 : 1;
                                di += adv;
                            } else { di++; }
                        }
                        p = semi + 1;
                        continue;
                    }
                }
            }

            /* Regular UTF-8 character */
            uint32_t cp; size_t adv;
            if (usr_utf8_decode((const uint8_t*)p, 8, &cp, &adv) == 0) {
                usr_sb_append(&plain, p, adv);
                utf16_pos += (cp > 0xFFFFu) ? 2 : 1;
                p += adv;
            } else {
                usr_sb_appendc(&plain, *p);
                p++;
            }
            continue;
        }

        /* ---------- Tag ---------- */
        const char *tag_start = p + 1;
        const char *tag_end   = skip_to(tag_start, '>');
        if (!*tag_end) break; /* malformed, bail */

        /* Check self-closing */
        /* int self_close = (tag_end > tag_start && tag_end[-1] == '/'); */

        /* Tag name */
        const char *tn = tag_start;
        int is_close = (*tn == '/');
        if (is_close) tn++;

        /* Find end of tag name */
        const char *tn_end = tn;
        while (tn_end < tag_end && !isspace((unsigned char)*tn_end) &&
               *tn_end != '>' && *tn_end != '/') tn_end++;
        size_t tnlen = (size_t)(tn_end - tn);

        /* Match tag name */
#define TAG_IS(s) (tnlen == strlen(s) && strncasecmp(tn, (s), tnlen) == 0)

        if (!is_close) {
            usr_entity_type etype = _USR_ENTITY_TYPE_COUNT; /* sentinel = unknown */
            char *extra = NULL;

            if (TAG_IS("b") || TAG_IS("strong")) {
                etype = USR_ENTITY_BOLD;
            } else if (TAG_IS("i") || TAG_IS("em")) {
                etype = USR_ENTITY_ITALIC;
            } else if (TAG_IS("u") || TAG_IS("ins")) {
                etype = USR_ENTITY_UNDERLINE;
            } else if (TAG_IS("s") || TAG_IS("del") || TAG_IS("strike")) {
                etype = USR_ENTITY_STRIKETHROUGH;
            } else if (TAG_IS("tg-spoiler")) {
                etype = USR_ENTITY_SPOILER;
            } else if (TAG_IS("code")) {
                etype = USR_ENTITY_CODE;
                in_code++;
            } else if (TAG_IS("pre")) {
                etype = USR_ENTITY_PRE;
                in_pre++;
                /* Check for <pre><code class="language-xxx"> */
                /* Look for language in class attr of next <code> — handled at </pre> */
                extra = extract_attr(tn_end, tag_end, "class");
                /* Remove "language-" prefix if present */
                if (extra && strncmp(extra, "language-", 9) == 0) {
                    memmove(extra, extra + 9, strlen(extra + 9) + 1);
                }
            } else if (TAG_IS("a")) {
                etype = USR_ENTITY_TEXT_LINK;
                extra = extract_attr(tn_end, tag_end, "href");
            } else if (TAG_IS("tg-emoji")) {
                etype = USR_ENTITY_CUSTOM_EMOJI;
                extra = extract_attr(tn_end, tag_end, "emoji-id");
            } else if (TAG_IS("blockquote")) {
                etype = USR_ENTITY_BLOCKQUOTE;
            }

            if (etype != _USR_ENTITY_TYPE_COUNT && sp < 64) {
                stack[sp].type        = etype;
                stack[sp].utf16_start = utf16_pos;
                stack[sp].extra       = extra;
                sp++;
            } else {
                if (extra) free(extra);
            }
        } else {
            /* Closing tag: pop matching open tag from stack */
            usr_entity_type etype = _USR_ENTITY_TYPE_COUNT;

            if (TAG_IS("b") || TAG_IS("strong")) etype = USR_ENTITY_BOLD;
            else if (TAG_IS("i") || TAG_IS("em")) etype = USR_ENTITY_ITALIC;
            else if (TAG_IS("u") || TAG_IS("ins")) etype = USR_ENTITY_UNDERLINE;
            else if (TAG_IS("s") || TAG_IS("del") || TAG_IS("strike")) etype = USR_ENTITY_STRIKETHROUGH;
            else if (TAG_IS("tg-spoiler")) etype = USR_ENTITY_SPOILER;
            else if (TAG_IS("code")) { etype = USR_ENTITY_CODE; if (in_code > 0) in_code--; }
            else if (TAG_IS("pre"))  { etype = USR_ENTITY_PRE; if (in_pre > 0) in_pre--; }
            else if (TAG_IS("a"))    etype = USR_ENTITY_TEXT_LINK;
            else if (TAG_IS("tg-emoji")) etype = USR_ENTITY_CUSTOM_EMOJI;
            else if (TAG_IS("blockquote")) etype = USR_ENTITY_BLOCKQUOTE;

            if (etype != _USR_ENTITY_TYPE_COUNT) {
                /* Find matching open tag in stack (top-down) */
                for (int k = sp - 1; k >= 0; k--) {
                    if (stack[k].type == etype) {
                        uint32_t length = utf16_pos - stack[k].utf16_start;
                        if (length > 0 && count < max_entities) {
                            entities_out[count].type   = etype;
                            entities_out[count].offset = stack[k].utf16_start;
                            entities_out[count].length = length;
                            entities_out[count].extra  = stack[k].extra;
                            count++;
                        } else {
                            if (stack[k].extra) free(stack[k].extra);
                        }
                        /* Remove from stack by shifting */
                        for (int m = k; m < sp - 1; m++) stack[m] = stack[m+1];
                        sp--;
                        break;
                    }
                }
            }
        }

#undef TAG_IS
        p = tag_end + 1;
    }

    /* Clean up unclosed stack entries */
    for (int k = 0; k < sp; k++) {
        if (stack[k].extra) free(stack[k].extra);
    }

    if (plain_out) {
        *plain_out = usr_sb_detach(&plain);
    } else {
        usr_sb_free(&plain);
    }

    return count;
}

/* ============================================================
   Event-based renderer for entities → HTML
   ============================================================ */

typedef struct {
    uint32_t        utf16_pos;
    int             is_open;   /* 1=open, 0=close */
    usr_entity_type type;
    const char     *extra;
    int             entity_idx; /* for ordering ties */
} html_event;

static int event_cmp(const void *a, const void *b) {
    const html_event *x = (const html_event *)a;
    const html_event *y = (const html_event *)b;
    if (x->utf16_pos != y->utf16_pos)
        return (x->utf16_pos < y->utf16_pos) ? -1 : 1;
    /* At same position: opens before closes */
    if (x->is_open != y->is_open)
        return (x->is_open > y->is_open) ? -1 : 1;
    /* Among opens: outer (lower index) first */
    if (x->is_open)
        return x->entity_idx - y->entity_idx;
    /* Among closes: inner (higher index) first */
    return y->entity_idx - x->entity_idx;
}

static void append_open_tag(usr_sb *sb, usr_entity_type t, const char *extra) {
    switch (t) {
        case USR_ENTITY_BOLD:          usr_sb_appends(sb, "<b>"); break;
        case USR_ENTITY_ITALIC:        usr_sb_appends(sb, "<i>"); break;
        case USR_ENTITY_UNDERLINE:     usr_sb_appends(sb, "<u>"); break;
        case USR_ENTITY_STRIKETHROUGH: usr_sb_appends(sb, "<s>"); break;
        case USR_ENTITY_SPOILER:       usr_sb_appends(sb, "<tg-spoiler>"); break;
        case USR_ENTITY_CODE:          usr_sb_appends(sb, "<code>"); break;
        case USR_ENTITY_PRE:
            if (extra && extra[0]) {
                usr_sb_appends(sb, "<pre><code class=\"language-");
                usr_sb_appends(sb, extra);
                usr_sb_appends(sb, "\">");
            } else {
                usr_sb_appends(sb, "<pre>");
            }
            break;
        case USR_ENTITY_TEXT_LINK:
            usr_sb_appends(sb, "<a href=\"");
            if (extra) usr_sb_appends(sb, extra);
            usr_sb_appends(sb, "\">");
            break;
        case USR_ENTITY_CUSTOM_EMOJI:
            usr_sb_appends(sb, "<tg-emoji emoji-id=\"");
            if (extra) usr_sb_appends(sb, extra);
            usr_sb_appends(sb, "\">");
            break;
        case USR_ENTITY_BLOCKQUOTE:
            usr_sb_appends(sb, "<blockquote>");
            break;
        default: break;
    }
}

static void append_close_tag(usr_sb *sb, usr_entity_type t, const char *extra) {
    switch (t) {
        case USR_ENTITY_BOLD:          usr_sb_appends(sb, "</b>"); break;
        case USR_ENTITY_ITALIC:        usr_sb_appends(sb, "</i>"); break;
        case USR_ENTITY_UNDERLINE:     usr_sb_appends(sb, "</u>"); break;
        case USR_ENTITY_STRIKETHROUGH: usr_sb_appends(sb, "</s>"); break;
        case USR_ENTITY_SPOILER:       usr_sb_appends(sb, "</tg-spoiler>"); break;
        case USR_ENTITY_CODE:          usr_sb_appends(sb, "</code>"); break;
        case USR_ENTITY_PRE:
            if (extra && extra[0]) {
                usr_sb_appends(sb, "</code></pre>");
            } else {
                usr_sb_appends(sb, "</pre>");
            }
            break;
        case USR_ENTITY_TEXT_LINK:     usr_sb_appends(sb, "</a>"); break;
        case USR_ENTITY_CUSTOM_EMOJI:  usr_sb_appends(sb, "</tg-emoji>"); break;
        case USR_ENTITY_BLOCKQUOTE:    usr_sb_appends(sb, "</blockquote>"); break;
        default: break;
    }
}

char *usr_entities_to_html(
    const char       *text,
    const usr_entity *entities,
    size_t            count
) {
    if (!text) return NULL;

    /* Build event list */
    size_t event_count = count * 2;
    html_event *events = NULL;
    if (event_count > 0) {
        events = (html_event *)malloc(event_count * sizeof(html_event));
        if (!events) return NULL;
        for (size_t i = 0; i < count; i++) {
            events[2*i] = (html_event){
                entities[i].offset,
                1,
                entities[i].type,
                entities[i].extra,
                (int)i
            };
            events[2*i+1] = (html_event){
                entities[i].offset + entities[i].length,
                0,
                entities[i].type,
                entities[i].extra,
                (int)i
            };
        }
        qsort(events, event_count, sizeof(html_event), event_cmp);
    }

    usr_sb sb;
    size_t text_len = strlen(text);
    usr_sb_init(&sb, text_len * 2 + event_count * 32);

    size_t   ei       = 0;   /* event index */
    size_t   ti       = 0;   /* byte index into text */
    uint32_t utf16    = 0;

    while (ti < text_len || (events && ei < event_count)) {

        /* Emit tags at current utf16 position */
        while (events && ei < event_count && events[ei].utf16_pos == utf16) {
            if (events[ei].is_open) {
                append_open_tag(&sb, events[ei].type, events[ei].extra);
            } else {
                append_close_tag(&sb, events[ei].type, events[ei].extra);
            }
            ei++;
        }

        if (ti >= text_len) break;

        /* Decode one UTF-8 character */
        uint32_t cp; size_t adv;
        if (usr_utf8_decode((const uint8_t*)text + ti, text_len - ti, &cp, &adv) < 0) {
            adv = 1; cp = (uint32_t)(unsigned char)text[ti];
        }

        /* HTML-escape special characters in text */
        switch (cp) {
            case '&': usr_sb_appends(&sb, "&amp;"); break;
            case '<': usr_sb_appends(&sb, "&lt;");  break;
            case '>': usr_sb_appends(&sb, "&gt;");  break;
            default:
                usr_sb_append(&sb, text + ti, adv);
                break;
        }

        ti    += adv;
        utf16 += (cp > 0xFFFFu) ? 2 : 1;
    }

    /* Flush remaining close tags */
    while (events && ei < event_count) {
        if (!events[ei].is_open) {
            append_close_tag(&sb, events[ei].type, events[ei].extra);
        }
        ei++;
    }

    free(events);
    return usr_sb_detach(&sb);
}
