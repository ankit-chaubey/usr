#include "usr/encoding.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* ============================================================
   BASE64
   ============================================================ */

static const char B64_STD[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char B64_URL[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static const int8_t B64_DEC[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 0-15 */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 16-31 */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,62,-1,63, /* 32-47: +,-,/ */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1, /* 48-63: 0-9,= */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, /* 64-79: A-O */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,63, /* 80-95: P-Z,_ */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, /* 96-111: a-o */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1, /* 112-127: p-z */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

size_t usr_base64_enc_size(size_t in_len) {
    return ((in_len + 2) / 3) * 4 + 1;
}

size_t usr_base64_dec_size(size_t enc_len) {
    return ((enc_len + 3) / 4) * 3 + 1;
}

static size_t base64_encode_impl(const uint8_t *in, size_t in_len,
                                  char *out, const char *alpha, int pad) {
    size_t i = 0, o = 0;
    while (i + 3 <= in_len) {
        uint32_t v = ((uint32_t)in[i] << 16)
                   | ((uint32_t)in[i+1] << 8)
                   |  (uint32_t)in[i+2];
        out[o++] = alpha[(v >> 18) & 0x3F];
        out[o++] = alpha[(v >> 12) & 0x3F];
        out[o++] = alpha[(v >>  6) & 0x3F];
        out[o++] = alpha[(v      ) & 0x3F];
        i += 3;
    }
    size_t rem = in_len - i;
    if (rem == 1) {
        out[o++] = alpha[(in[i] >> 2) & 0x3F];
        out[o++] = alpha[(in[i] << 4) & 0x3F];
        if (pad) { out[o++] = '='; out[o++] = '='; }
    } else if (rem == 2) {
        uint32_t v = ((uint32_t)in[i] << 8) | in[i+1];
        out[o++] = alpha[(v >> 10) & 0x3F];
        out[o++] = alpha[(v >>  4) & 0x3F];
        out[o++] = alpha[(v <<  2) & 0x3F];
        if (pad) out[o++] = '=';
    }
    out[o] = '\0';
    return o;
}

static size_t base64_decode_impl(const char *in, size_t in_len, uint8_t *out) {
    size_t o = 0;
    size_t i = 0;
    while (i < in_len) {
        /* Skip whitespace */
        if (in[i] == '\r' || in[i] == '\n' || in[i] == ' ') { i++; continue; }

        int a = B64_DEC[(uint8_t)in[i]];
        int b = (i+1 < in_len) ? B64_DEC[(uint8_t)in[i+1]] : -2;
        int c = (i+2 < in_len) ? B64_DEC[(uint8_t)in[i+2]] : -2;
        int d = (i+3 < in_len) ? B64_DEC[(uint8_t)in[i+3]] : -2;

        if (a < 0 || b < 0) return (size_t)-1;

        out[o++] = (uint8_t)((a << 2) | (b >> 4));

        if (c == -2 || in[i+2] == '=') { i += 4; break; }
        if (c < 0) return (size_t)-1;
        out[o++] = (uint8_t)((b << 4) | (c >> 2));

        if (d == -2 || in[i+3] == '=') { i += 4; break; }
        if (d < 0) return (size_t)-1;
        out[o++] = (uint8_t)((c << 6) | d);

        i += 4;
    }
    out[o] = '\0';
    return o;
}

size_t usr_base64_encode(const uint8_t *in, size_t in_len, char *out) {
    return base64_encode_impl(in, in_len, out, B64_STD, 1);
}

size_t usr_base64_decode(const char *in, size_t in_len, uint8_t *out) {
    return base64_decode_impl(in, in_len, out);
}

size_t usr_base64url_encode(const uint8_t *in, size_t in_len, char *out) {
    return base64_encode_impl(in, in_len, out, B64_URL, 0);
}

size_t usr_base64url_decode(const char *in, size_t in_len, uint8_t *out) {
    return base64_decode_impl(in, in_len, out);
}

char *usr_base64_encode_alloc(const uint8_t *in, size_t in_len) {
    size_t sz = usr_base64_enc_size(in_len);
    char *out = (char *)malloc(sz);
    if (!out) return NULL;
    usr_base64_encode(in, in_len, out);
    return out;
}

uint8_t *usr_base64_decode_alloc(const char *in, size_t in_len, size_t *out_len) {
    size_t sz = usr_base64_dec_size(in_len);
    uint8_t *out = (uint8_t *)malloc(sz);
    if (!out) return NULL;
    size_t n = usr_base64_decode(in, in_len, out);
    if (n == (size_t)-1) { free(out); return NULL; }
    if (out_len) *out_len = n;
    return out;
}

char *usr_base64url_encode_alloc(const uint8_t *in, size_t in_len) {
    size_t sz = usr_base64_enc_size(in_len);
    char *out = (char *)malloc(sz);
    if (!out) return NULL;
    usr_base64url_encode(in, in_len, out);
    return out;
}

uint8_t *usr_base64url_decode_alloc(const char *in, size_t in_len, size_t *out_len) {
    size_t sz = usr_base64_dec_size(in_len);
    uint8_t *out = (uint8_t *)malloc(sz);
    if (!out) return NULL;
    size_t n = usr_base64url_decode(in, in_len, out);
    if (n == (size_t)-1) { free(out); return NULL; }
    if (out_len) *out_len = n;
    return out;
}

/* ============================================================
   HEX ENCODING
   ============================================================ */

void usr_hex_encode(const uint8_t *in, size_t in_len, char *out) {
    static const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < in_len; i++) {
        out[i*2]   = hex[in[i] >> 4];
        out[i*2+1] = hex[in[i] & 0x0F];
    }
    out[in_len*2] = '\0';
}

void usr_hex_encode_upper(const uint8_t *in, size_t in_len, char *out) {
    static const char hex[] = "0123456789ABCDEF";
    for (size_t i = 0; i < in_len; i++) {
        out[i*2]   = hex[in[i] >> 4];
        out[i*2+1] = hex[in[i] & 0x0F];
    }
    out[in_len*2] = '\0';
}

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

size_t usr_hex_decode(const char *in, size_t in_len, uint8_t *out) {
    /* Skip optional "0x" prefix */
    if (in_len >= 2 && in[0] == '0' && (in[1] == 'x' || in[1] == 'X')) {
        in += 2; in_len -= 2;
    }
    if (in_len % 2 != 0) return (size_t)-1;
    for (size_t i = 0; i < in_len; i += 2) {
        int hi = hex_nibble(in[i]);
        int lo = hex_nibble(in[i+1]);
        if (hi < 0 || lo < 0) return (size_t)-1;
        out[i/2] = (uint8_t)((hi << 4) | lo);
    }
    return in_len / 2;
}

char *usr_hex_encode_alloc(const uint8_t *in, size_t in_len) {
    char *out = (char *)malloc(in_len * 2 + 1);
    if (!out) return NULL;
    usr_hex_encode(in, in_len, out);
    return out;
}

uint8_t *usr_hex_decode_alloc(const char *in, size_t in_len, size_t *out_len) {
    uint8_t *out = (uint8_t *)malloc(in_len / 2 + 1);
    if (!out) return NULL;
    size_t n = usr_hex_decode(in, in_len, out);
    if (n == (size_t)-1) { free(out); return NULL; }
    if (out_len) *out_len = n;
    return out;
}

/* ============================================================
   URL ENCODING  (RFC 3986 unreserved chars: A-Z a-z 0-9 - _ . ~)
   ============================================================ */

static int is_unreserved(unsigned char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '-' || c == '_' || c == '.' || c == '~';
}

size_t usr_url_encode(const char *in, size_t in_len, char *out) {
    static const char hex[] = "0123456789ABCDEF";
    size_t o = 0;
    for (size_t i = 0; i < in_len; i++) {
        unsigned char c = (unsigned char)in[i];
        if (is_unreserved(c)) {
            out[o++] = (char)c;
        } else {
            out[o++] = '%';
            out[o++] = hex[c >> 4];
            out[o++] = hex[c & 0x0F];
        }
    }
    out[o] = '\0';
    return o;
}

size_t usr_url_decode(const char *in, size_t in_len, char *out) {
    size_t o = 0;
    for (size_t i = 0; i < in_len; ) {
        if (in[i] == '%' && i + 2 < in_len) {
            int hi = hex_nibble(in[i+1]);
            int lo = hex_nibble(in[i+2]);
            if (hi >= 0 && lo >= 0) {
                out[o++] = (char)((hi << 4) | lo);
                i += 3;
                continue;
            }
        }
        if (in[i] == '+') {
            out[o++] = ' ';
        } else {
            out[o++] = in[i];
        }
        i++;
    }
    out[o] = '\0';
    return o;
}

char *usr_url_encode_alloc(const char *in, size_t in_len) {
    char *out = (char *)malloc(in_len * 3 + 1);
    if (!out) return NULL;
    usr_url_encode(in, in_len, out);
    return out;
}

char *usr_url_decode_alloc(const char *in, size_t in_len) {
    char *out = (char *)malloc(in_len + 1);
    if (!out) return NULL;
    usr_url_decode(in, in_len, out);
    return out;
}

/* ============================================================
   HTML ENTITY ESCAPING
   ============================================================ */

size_t usr_html_escape(const char *in, size_t in_len, char *out) {
    size_t o = 0;
    for (size_t i = 0; i < in_len; i++) {
        unsigned char c = (unsigned char)in[i];
        switch (c) {
            case '&': memcpy(out+o, "&amp;",  5); o += 5; break;
            case '<': memcpy(out+o, "&lt;",   4); o += 4; break;
            case '>': memcpy(out+o, "&gt;",   4); o += 4; break;
            case '"': memcpy(out+o, "&quot;", 6); o += 6; break;
            case '\'':memcpy(out+o, "&#39;",  5); o += 5; break;
            default:  out[o++] = (char)c; break;
        }
    }
    out[o] = '\0';
    return o;
}

size_t usr_html_unescape(const char *in, size_t in_len, char *out) {
    size_t o = 0;
    for (size_t i = 0; i < in_len; ) {
        if (in[i] != '&') { out[o++] = in[i++]; continue; }

        /* Find semicolon */
        size_t end = i + 1;
        while (end < in_len && end < i + 12 && in[end] != ';') end++;
        if (end >= in_len || in[end] != ';') { out[o++] = in[i++]; continue; }

        const char *entity = in + i + 1;
        size_t elen = end - i - 1;

        if (elen == 3 && memcmp(entity, "amp", 3) == 0) { out[o++] = '&'; }
        else if (elen == 2 && memcmp(entity, "lt", 2) == 0) { out[o++] = '<'; }
        else if (elen == 2 && memcmp(entity, "gt", 2) == 0) { out[o++] = '>'; }
        else if (elen == 4 && memcmp(entity, "quot", 4) == 0) { out[o++] = '"'; }
        else if (elen == 4 && memcmp(entity, "apos", 4) == 0) { out[o++] = '\''; }
        else if (elen == 4 && memcmp(entity, "nbsp", 4) == 0) {
            /* Non-breaking space: U+00A0 → UTF-8 0xC2 0xA0 */
            out[o++] = (char)0xC2; out[o++] = (char)0xA0;
        }
        else if (elen > 1 && entity[0] == '#') {
            /* Numeric entity: &#NNN; or &#xNN; */
            uint32_t cp = 0;
            if (elen > 2 && (entity[1] == 'x' || entity[1] == 'X')) {
                for (size_t j = 2; j < elen; j++) {
                    int v = hex_nibble(entity[j]);
                    if (v < 0) { cp = 0; break; }
                    cp = (cp << 4) | (uint32_t)v;
                }
            } else {
                for (size_t j = 1; j < elen; j++) {
                    if (entity[j] < '0' || entity[j] > '9') { cp = 0; break; }
                    cp = cp * 10 + (uint32_t)(entity[j] - '0');
                }
            }
            /* Encode cp as UTF-8 */
            if (cp > 0 && cp <= 0x10FFFF) {
                if (cp < 0x80) {
                    out[o++] = (char)cp;
                } else if (cp < 0x800) {
                    out[o++] = (char)(0xC0 | (cp >> 6));
                    out[o++] = (char)(0x80 | (cp & 0x3F));
                } else if (cp < 0x10000) {
                    out[o++] = (char)(0xE0 | (cp >> 12));
                    out[o++] = (char)(0x80 | ((cp >> 6) & 0x3F));
                    out[o++] = (char)(0x80 | (cp & 0x3F));
                } else {
                    out[o++] = (char)(0xF0 | (cp >> 18));
                    out[o++] = (char)(0x80 | ((cp >> 12) & 0x3F));
                    out[o++] = (char)(0x80 | ((cp >> 6) & 0x3F));
                    out[o++] = (char)(0x80 | (cp & 0x3F));
                }
            } else {
                /* Passthrough invalid entity */
                memcpy(out+o, in+i, end-i+1); o += end-i+1;
                i = end + 1; continue;
            }
        } else {
            /* Unknown entity: pass through */
            memcpy(out+o, in+i, end-i+1); o += end-i+1;
            i = end + 1; continue;
        }

        i = end + 1;
    }
    out[o] = '\0';
    return o;
}

char *usr_html_escape_alloc(const char *in, size_t in_len) {
    char *out = (char *)malloc(in_len * 6 + 1);
    if (!out) return NULL;
    usr_html_escape(in, in_len, out);
    return out;
}

char *usr_html_unescape_alloc(const char *in, size_t in_len) {
    char *out = (char *)malloc(in_len + 1);
    if (!out) return NULL;
    usr_html_unescape(in, in_len, out);
    return out;
}
