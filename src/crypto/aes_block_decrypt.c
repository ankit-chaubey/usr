#include "aes_tables.h"
#include <stdint.h>
#include <string.h>
#include "aes_tables.h"

extern const uint8_t sbox[256];

static int inv_init = 0;


static uint8_t xtime(uint8_t x) {
    return (x << 1) ^ ((x >> 7) * 0x1b);
}

static uint8_t mul(uint8_t x, uint8_t y) {
    uint8_t r = 0;
    while (y) {
        if (y & 1) r ^= x;
        x = xtime(x);
        y >>= 1;
    }
    return r;
}

static void inv_sub(uint8_t *s) {
    for (int i = 0; i < 16; i++) s[i] = inv_sbox[s[i]];
}

static void inv_shift(uint8_t *s) {
    uint8_t t[16];
    memcpy(t, s, 16);
    s[0]=t[0];  s[4]=t[4];  s[8]=t[8];   s[12]=t[12];
    s[1]=t[13]; s[5]=t[1];  s[9]=t[5];   s[13]=t[9];
    s[2]=t[10]; s[6]=t[14]; s[10]=t[2];  s[14]=t[6];
    s[3]=t[7];  s[7]=t[11]; s[11]=t[15]; s[15]=t[3];
}

static void inv_mix(uint8_t *s) {
    for (int i = 0; i < 4; i++) {
        uint8_t *c = s + i*4;
        uint8_t a=c[0], b=c[1], d=c[2], e=c[3];
        c[0]=mul(a,14)^mul(b,11)^mul(d,13)^mul(e,9);
        c[1]=mul(a,9)^mul(b,14)^mul(d,11)^mul(e,13);
        c[2]=mul(a,13)^mul(b,9)^mul(d,14)^mul(e,11);
        c[3]=mul(a,11)^mul(b,13)^mul(d,9)^mul(e,14);
    }
}

static void add_key(uint8_t *s, const uint8_t *rk) {
    for (int i = 0; i < 16; i++) s[i] ^= rk[i];
}

void usr_aes256_decrypt_block(uint8_t block[16], const uint8_t rk[240]) {

    inv_shift(block);
    inv_sub(block);
    add_key(block, rk);
}
