#ifndef USR_AES_TABLES_H
#define USR_AES_TABLES_H

#include <stdint.h>

extern const uint8_t sbox[256];
extern const uint8_t inv_sbox[256];
extern const uint8_t mul2[256];
extern const uint8_t mul3[256];

/* These are filled at runtime by usr_aes_tables_init() */
extern uint8_t mul9[256];
extern uint8_t mul11[256];
extern uint8_t mul13[256];
extern uint8_t mul14[256];

void usr_aes_tables_init(void);

#endif /* USR_AES_TABLES_H */
