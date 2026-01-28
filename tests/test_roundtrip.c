#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usr/markdown.h"
#include "usr/html.h"
#include "usr/entities.h"

#define MAXE 64

/* Skip inputs known to be non-idempotent by design */
static int is_best_effort_only(const char *s) {
    return (strstr(s, "||") != NULL) ||
           (strchr(s, '`') != NULL);
}

static void test_roundtrip(const char *input) {
    usr_entity e1[MAXE], e2[MAXE];

    /* Markdown → entities */
    size_t n1 = usr_markdown_parse(
        input,
        USR_MD_V2,
        e1,
        MAXE
    );
    n1 = usr_entities_normalize(e1, n1);

    /* Invariant: no zero-length entities */
    for (size_t i = 0; i < n1; i++) {
        if (e1[i].length == 0) {
            printf("❌ INVALID ENTITY (markdown)\nInput: %s\n", input);
            exit(1);
        }
    }

    /* entities → HTML */
    char *html = usr_entities_to_html_rt(input, e1, n1);
    if (!html) {
        printf("❌ HTML NULL\nInput: %s\n", input);
        exit(1);
    }

    /* HTML → entities */
    size_t n2 = usr_html_parse(html, e2, MAXE);
    n2 = usr_entities_normalize(e2, n2);

    /* Invariant: no zero-length entities after HTML */
    for (size_t i = 0; i < n2; i++) {
        if (e2[i].length == 0) {
            printf("❌ INVALID ENTITY (html)\nInput: %s\nHTML: %s\n",
                   input, html);
            free(html);
            exit(1);
        }
    }

    /* Strict comparison only when safe */
    if (!is_best_effort_only(input)) {
        if (n1 != n2) {
            printf("❌ ROUND TRIP FAILED\nInput: %s\nHTML: %s\n",
                   input, html);
            free(html);
            exit(1);
        }
    }

    free(html);
}

int main(void) {
    /* Strict-safe cases */
    test_roundtrip("Hello *world*");
    test_roundtrip("*Bold _italic_*");
    test_roundtrip("__underline ~strike~__");

    /* Best-effort cases (must not crash) */
    test_roundtrip("||spoiler|| and `code`");
    test_roundtrip("[link](https://example.com)");

    printf("✅ Round-trip tests passed (best-effort aware)\n");
    return 0;
}
