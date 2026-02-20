#include "usr/markdown.h"
#include "usr/utf8.h"
#include "usr/strbuilder.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================
   MarkdownV2 special chars (must be escaped in plain text)
   ============================================================ */
static int is_md2_special(char c) {
    return c=='_'||c=='*'||c=='['||c==']'||c=='('||c==')'||
           c=='~'||c=='`'||c=='>'||c=='#'||c=='+'||c=='-'||
           c=='='||c=='|'||c=='{'||c=='}'||c=='.'||c=='!';
}

/* ============================================================
   Marker state
   ============================================================ */
typedef struct { int active; uint32_t start; } mstate;

/* ============================================================
   usr_markdown_parse
   ============================================================ */
size_t usr_markdown_parse(
    const char           *text,
    usr_markdown_version  version,
    char                **plain_out,
    usr_entity           *out,
    size_t                max_out
) {
    if (!text) return (size_t)-1;

    size_t   in_len   = strlen(text);
    size_t   count    = 0;
    uint32_t utf16    = 0; /* UTF-16 units in plain text built so far */
    size_t   i        = 0;

    mstate bold      = {0,0}, italic   = {0,0}, underline = {0,0};
    mstate strike    = {0,0}, spoiler  = {0,0}, code_m    = {0,0};
    int    in_code   = 0, in_pre = 0;
    uint32_t pre_start = 0;
    char     pre_lang[64] = {0};

    usr_sb plain;
    usr_sb_init(&plain, in_len);

#define EMIT(TYPE, OFFSET, LENGTH, EXTRA) \
    do { if ((LENGTH) > 0 && count < max_out) { \
        out[count].type=(TYPE); out[count].offset=(OFFSET); \
        out[count].length=(LENGTH); out[count].extra=(EXTRA); count++; \
    } } while(0)

#define PLAIN_CHAR_ADV(ptr, remaining) \
    do { uint32_t _cp; size_t _adv; \
        if (usr_utf8_decode((const uint8_t*)(ptr),(remaining),&_cp,&_adv)==0) { \
            usr_sb_append(&plain,(ptr),_adv); \
            utf16 += (_cp>0xFFFFu)?2:1; i+=_adv; \
        } else { usr_sb_appendc(&plain,*(ptr)); utf16++; i++; } \
    } while(0)

    while (i < in_len && count < max_out) {
        char c = text[i];

        /* Backslash escape (V2) */
        if (version==USR_MD_V2 && c=='\\' && i+1<in_len && is_md2_special(text[i+1])) {
            i++;
            PLAIN_CHAR_ADV(text+i, in_len-i);
            continue;
        }

        /* Triple backtick pre-block */
        if (!in_code && text[i]=='`' && i+2<in_len &&
            text[i+1]=='`' && text[i+2]=='`') {
            if (!in_pre) {
                i += 3;
                /* optional language on same line */
                size_t ls = i;
                while (i<in_len && text[i]!='\n') i++;
                size_t ll = i - ls;
                if (ll < sizeof(pre_lang)) { memcpy(pre_lang,text+ls,ll); pre_lang[ll]=0; }
                if (i<in_len && text[i]=='\n') i++;
                in_pre=1; pre_start=utf16;
            } else {
                EMIT(USR_ENTITY_PRE, pre_start, utf16-pre_start,
                     (pre_lang[0]) ? strdup(pre_lang) : NULL);
                in_pre=0; memset(pre_lang,0,sizeof(pre_lang));
                i += 3;
            }
            continue;
        }
        /* Inside pre: literal */
        if (in_pre) { PLAIN_CHAR_ADV(text+i, in_len-i); continue; }

        /* Inline code ` */
        if (c=='`' && !in_code) {
            code_m.active=1; code_m.start=utf16; in_code=1; i++; continue;
        }
        if (c=='`' && in_code) {
            EMIT(USR_ENTITY_CODE, code_m.start, utf16-code_m.start, NULL);
            code_m.active=0; in_code=0; i++; continue;
        }
        /* Inside code: literal */
        if (in_code) { PLAIN_CHAR_ADV(text+i, in_len-i); continue; }

        /* Spoiler || (V2) */
        if (version==USR_MD_V2 && c=='|' && i+1<in_len && text[i+1]=='|') {
            if (!spoiler.active) { spoiler.active=1; spoiler.start=utf16; }
            else { EMIT(USR_ENTITY_SPOILER,spoiler.start,utf16-spoiler.start,NULL); spoiler.active=0; }
            i+=2; continue;
        }

        /* Underline __ (V2) */
        if (version==USR_MD_V2 && c=='_' && i+1<in_len && text[i+1]=='_') {
            if (!underline.active) { underline.active=1; underline.start=utf16; }
            else { EMIT(USR_ENTITY_UNDERLINE,underline.start,utf16-underline.start,NULL); underline.active=0; }
            i+=2; continue;
        }

        /* Bold * */
        if (c=='*') {
            if (!bold.active) { bold.active=1; bold.start=utf16; }
            else { EMIT(USR_ENTITY_BOLD,bold.start,utf16-bold.start,NULL); bold.active=0; }
            i++; continue;
        }

        /* Italic _ (V2) */
        if (version==USR_MD_V2 && c=='_') {
            if (!italic.active) { italic.active=1; italic.start=utf16; }
            else { EMIT(USR_ENTITY_ITALIC,italic.start,utf16-italic.start,NULL); italic.active=0; }
            i++; continue;
        }

        /* Strikethrough ~ (V2) */
        if (version==USR_MD_V2 && c=='~') {
            if (!strike.active) { strike.active=1; strike.start=utf16; }
            else { EMIT(USR_ENTITY_STRIKETHROUGH,strike.start,utf16-strike.start,NULL); strike.active=0; }
            i++; continue;
        }

        /* Inline link [text](url) */
        if (c=='[') {
            /* Find closing ] */
            size_t j = i+1, depth=1;
            while (j<in_len && depth>0) {
                if (text[j]=='[') depth++;
                else if (text[j]==']') depth--;
                j++;
            }
            /* j now points past ] */
            if (depth==0 && j<in_len && text[j]=='(') {
                size_t url_s = j+1;
                size_t url_e = url_s;
                while (url_e<in_len && text[url_e]!=')') url_e++;
                if (url_e < in_len) {
                    /* emit link text into plain */
                    uint32_t link_start = utf16;
                    size_t lt_end = j - 1; /* index of ] */
                    for (size_t k = i+1; k < lt_end; ) {
                        if (version==USR_MD_V2 && text[k]=='\\' && k+1<lt_end && is_md2_special(text[k+1])) {
                            k++;
                            uint32_t cp; size_t adv;
                            if (usr_utf8_decode((const uint8_t*)text+k,lt_end-k,&cp,&adv)==0) {
                                usr_sb_append(&plain,text+k,adv); utf16+=(cp>0xFFFFu)?2:1; k+=adv;
                            } else { usr_sb_appendc(&plain,text[k]); utf16++; k++; }
                            continue;
                        }
                        uint32_t cp; size_t adv;
                        if (usr_utf8_decode((const uint8_t*)text+k,lt_end-k,&cp,&adv)==0) {
                            usr_sb_append(&plain,text+k,adv); utf16+=(cp>0xFFFFu)?2:1; k+=adv;
                        } else { usr_sb_appendc(&plain,text[k]); utf16++; k++; }
                    }
                    size_t url_len = url_e - url_s;
                    char *url = (char*)malloc(url_len+1);
                    if (url) { memcpy(url,text+url_s,url_len); url[url_len]=0; }
                    EMIT(USR_ENTITY_TEXT_LINK, link_start, utf16-link_start, url);
                    i = url_e + 1;
                    continue;
                }
            }
        }

        /* Normal char */
        PLAIN_CHAR_ADV(text+i, in_len-i);
    }

#undef EMIT
#undef PLAIN_CHAR_ADV

    if (plain_out) *plain_out = usr_sb_detach(&plain);
    else usr_sb_free(&plain);

    return count;
}

/* ============================================================
   usr_entities_to_markdown
   ============================================================ */

typedef struct {
    uint32_t pos; int open; usr_entity_type type; const char *extra; int idx;
} md_ev;

static int md_ev_cmp(const void *a, const void *b) {
    const md_ev *x=(const md_ev*)a, *y=(const md_ev*)b;
    if (x->pos != y->pos) return (x->pos<y->pos)?-1:1;
    if (x->open != y->open) return (x->open>y->open)?-1:1;
    return x->open ? (x->idx-y->idx) : (y->idx-x->idx);
}

char *usr_entities_to_markdown(
    const char       *text,
    const usr_entity *entities,
    size_t            count,
    usr_markdown_version version
) {
    if (!text) return NULL;
    size_t text_len = strlen(text);

    size_t ev_n = count*2;
    md_ev *ev = NULL;
    if (ev_n > 0) {
        ev = (md_ev*)malloc(ev_n * sizeof(md_ev));
        if (!ev) return NULL;
        for (size_t i=0; i<count; i++) {
            ev[2*i]   = (md_ev){entities[i].offset,         1, entities[i].type, entities[i].extra, (int)i};
            ev[2*i+1] = (md_ev){entities[i].offset+entities[i].length, 0, entities[i].type, entities[i].extra, (int)i};
        }
        qsort(ev, ev_n, sizeof(md_ev), md_ev_cmp);
    }

    usr_sb sb;
    usr_sb_init(&sb, text_len*2 + ev_n*8);

    size_t ei=0, ti=0;
    uint32_t utf16=0;

    while (ti<text_len || (ev&&ei<ev_n)) {
        while (ev && ei<ev_n && ev[ei].pos==utf16) {
            usr_entity_type t=ev[ei].type; const char *extra=ev[ei].extra;
            if (t==USR_ENTITY_TEXT_LINK) {
                if (ev[ei].open) usr_sb_appendc(&sb,'[');
                else { usr_sb_appends(&sb,"]("); if(extra)usr_sb_appends(&sb,extra); usr_sb_appendc(&sb,')'); }
            } else if (t==USR_ENTITY_PRE) {
                usr_sb_appends(&sb,"```");
                if (ev[ei].open && extra && extra[0]) { usr_sb_appends(&sb,extra); usr_sb_appendc(&sb,'\n'); }
                else if (!ev[ei].open) usr_sb_appendc(&sb,'\n');
            } else if (t==USR_ENTITY_CUSTOM_EMOJI) {
                /* No markdown syntax — skip */
            } else {
                const char *m = NULL;
                switch(t) {
                    case USR_ENTITY_BOLD:          m="*";  break;
                    case USR_ENTITY_ITALIC:        m="_";  break;
                    case USR_ENTITY_UNDERLINE:     m="__"; break;
                    case USR_ENTITY_STRIKETHROUGH: m="~";  break;
                    case USR_ENTITY_SPOILER:       m="||"; break;
                    case USR_ENTITY_CODE:          m="`";  break;
                    default: break;
                }
                if (m) usr_sb_appends(&sb,m);
            }
            ei++;
        }
        if (ti>=text_len) break;

        uint32_t cp; size_t adv;
        if (usr_utf8_decode((const uint8_t*)text+ti,text_len-ti,&cp,&adv)<0) { adv=1; cp=(unsigned char)text[ti]; }

        if (version==USR_MD_V2 && cp<128 && is_md2_special((char)cp))
            usr_sb_appendc(&sb,'\\');
        usr_sb_append(&sb,text+ti,adv);
        ti+=adv; utf16+=(cp>0xFFFFu)?2:1;
    }
    while (ev && ei<ev_n) {
        if (!ev[ei].open) {
            usr_entity_type t=ev[ei].type;
            if (t==USR_ENTITY_TEXT_LINK) { usr_sb_appends(&sb,"]("); if(ev[ei].extra)usr_sb_appends(&sb,ev[ei].extra); usr_sb_appendc(&sb,')'); }
            else if (t==USR_ENTITY_PRE) usr_sb_appends(&sb,"\n```");
            else { const char *m=NULL; switch(t){case USR_ENTITY_BOLD:m="*";break;case USR_ENTITY_ITALIC:m="_";break;case USR_ENTITY_UNDERLINE:m="__";break;case USR_ENTITY_STRIKETHROUGH:m="~";break;case USR_ENTITY_SPOILER:m="||";break;case USR_ENTITY_CODE:m="`";break;default:break;} if(m)usr_sb_appends(&sb,m); }
        }
        ei++;
    }
    free(ev);
    return usr_sb_detach(&sb);
}
