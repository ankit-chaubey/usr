#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* core */
#include "usr/utf8.h"
#include "usr/markdown.h"
#include "usr/entities.h"
#include "usr/html.h"

/* binary / bytes */
#include "usr/binary.h"
#include "usr/bytes.h"

/* crypto */
#include "usr/crypto.h"

/* helper: entity name */
static const char *etype(usr_entity_type t) {
    switch (t) {
        case USR_ENTITY_BOLD: return "BOLD";
        case USR_ENTITY_ITALIC: return "ITALIC";
        case USR_ENTITY_UNDERLINE: return "UNDERLINE";
        case USR_ENTITY_STRIKETHROUGH: return "STRIKE";
        case USR_ENTITY_CODE: return "CODE";
        case USR_ENTITY_SPOILER: return "SPOILER";
        default: return "OTHER";
    }
}

int main(void) {

    printf("===== USR FULL DEMO =====\n\n");

    /* =====================================================
       1️⃣ INPUT TEXT
    ===================================================== */
    const char *input =
        "*bold* _italic_ ~strike~ `code` ||spoiler|| 🙂";

    printf("INPUT:\n%s\n\n", input);

    /* =====================================================
       2️⃣ UTF-8 DECODING
    ===================================================== */
    printf("UTF-8 CODEPOINTS:\n");
    for (size_t i = 0; input[i]; ) {
        uint32_t cp;
        size_t adv;
        usr_utf8_decode((const uint8_t*)&input[i], 4, &cp, &adv);
        printf("U+%04X ", cp);
        i += adv;
    }
    printf("\n\n");

    /* =====================================================
       3️⃣ MARKDOWN → ENTITIES
    ===================================================== */
    usr_entity ents[64];
    size_t n = usr_markdown_parse(
        input,
        USR_MD_V2,
        ents,
        64
    );
    n = usr_entities_normalize(ents, n);

    printf("ENTITIES:\n");
    for (size_t i = 0; i < n; i++) {
        printf(
            "  %s | offset=%u length=%u\n",
            etype(ents[i].type),
            ents[i].offset,
            ents[i].length
        );
    }
    printf("\n");

    /* =====================================================
       4️⃣ ENTITIES → HTML
    ===================================================== */
    char *html = usr_entities_to_html_rt(input, ents, n);
    printf("HTML OUTPUT:\n%s\n\n", html);

    /* =====================================================
       5️⃣ HTML → ENTITIES (BEST-EFFORT ROUND TRIP)
    ===================================================== */
    usr_entity ents2[64];
    size_t n2 = usr_html_parse(html, ents2, 64);
    n2 = usr_entities_normalize(ents2, n2);

    printf("ENTITIES FROM HTML:\n");
    for (size_t i = 0; i < n2; i++) {
        printf(
            "  %s | offset=%u length=%u\n",
            etype(ents2[i].type),
            ents2[i].offset,
            ents2[i].length
        );
    }
    printf("\n");

    /* =====================================================
       6️⃣ TEXT → BINARY → TEXT (CORRECT API)
    ===================================================== */
    usr_bytes bin = usr_binary_from_text("Hello");
    printf("BINARY SIZE: %zu bytes\n", bin.len);

    usr_bytes_view view = {
        .data = bin.data,
        .len  = bin.len
    };

    char *text_back = usr_text_from_binary(&view);
    printf("BINARY → TEXT: %s\n\n", text_back);

    free(text_back);
    usr_bytes_free(&bin);

    /* =====================================================
       7️⃣ CRYPTO — SHA256
    ===================================================== */
    uint8_t hash[32];
    usr_sha256(
        (const uint8_t *)input,
        strlen(input),
        hash
    );

    printf("SHA256:\n");
    for (int i = 0; i < 32; i++)
        printf("%02X", hash[i]);
    printf("\n\n");

    /* =====================================================
       8️⃣ CRYPTO — AES-256-IGE
    ===================================================== */
    uint8_t key[32] = {0};
    uint8_t iv[32]  = {0};

    uint8_t buf[32] = "secret-message";
    size_t buflen = 16; /* AES block multiple */

    usr_aes256_ige_encrypt(buf, buflen, key, iv);
    printf("AES-IGE ENCRYPTED: ");
    for (size_t i = 0; i < buflen; i++)
        printf("%02X", buf[i]);
    printf("\n");

    usr_aes256_ige_decrypt(buf, buflen, key, iv);
    printf("AES-IGE DECRYPTED: %s\n\n", buf);

    free(html);

    printf("===== DONE =====\n");
    return 0;
}
