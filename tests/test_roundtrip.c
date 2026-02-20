#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usr/markdown.h"
#include "usr/html.h"
#include "usr/entities.h"

#define MAXE 128

static int pass = 0, fail = 0;

static void test_md_html_roundtrip(const char *md_input) {
    usr_entity e1[MAXE], e2[MAXE];
    char *plain1 = NULL, *plain2 = NULL, *html_out = NULL;

    /* Markdown → plain + entities */
    size_t n1 = usr_markdown_parse(md_input, USR_MD_V2, &plain1, e1, MAXE);
    if (n1 == (size_t)-1) {
        printf("  ❌ markdown_parse OOM: [%s]\n", md_input); fail++; return;
    }
    n1 = usr_entities_normalize(e1, n1);

    /* Zero-length entity check */
    for (size_t i = 0; i < n1; i++) {
        if (e1[i].length == 0) {
            printf("  ❌ zero-length entity from markdown: [%s]\n", md_input);
            fail++; free(plain1); return;
        }
    }

    if (!plain1) { printf("  ❌ no plain output: [%s]\n", md_input); fail++; return; }

    /* entities → HTML */
    html_out = usr_entities_to_html(plain1, e1, n1);
    if (!html_out) {
        printf("  ❌ entities_to_html returned NULL: [%s]\n", md_input);
        fail++; free(plain1); return;
    }

    /* HTML → plain + entities */
    size_t n2 = usr_html_parse(html_out, &plain2, e2, MAXE);
    if (n2 == (size_t)-1) {
        printf("  ❌ html_parse OOM: [%s]\n", md_input); fail++;
        free(plain1); free(html_out); return;
    }
    n2 = usr_entities_normalize(e2, n2);

    for (size_t i = 0; i < n2; i++) {
        if (e2[i].length == 0) {
            printf("  ❌ zero-length entity from HTML: [%s] → [%s]\n", md_input, html_out);
            fail++; free(plain1); free(plain2); free(html_out); return;
        }
    }

    /* Plain text must be identical after round-trip */
    if (plain2 && strcmp(plain1, plain2) != 0) {
        printf("  ❌ plain text mismatch:\n     md_input: [%s]\n     plain1:   [%s]\n     html:     [%s]\n     plain2:   [%s]\n",
               md_input, plain1, html_out, plain2);
        fail++;
    } else {
        printf("  ✅ [%s]\n", md_input);
        pass++;
    }

    free(plain1); free(plain2); free(html_out);
    for (size_t i = 0; i < n1; i++) if (e1[i].extra) { /* don't free; owned by entities */ }
    for (size_t i = 0; i < n2; i++) { /* same */ }
}

static void test_entities_to_markdown(const char *plain, usr_entity *ents, size_t n,
                                       const char *expected_md) {
    char *md = usr_entities_to_markdown(plain, ents, n, USR_MD_V2);
    if (!md) { printf("  ❌ entities_to_markdown returned NULL\n"); fail++; return; }
    if (strcmp(md, expected_md) == 0) {
        printf("  ✅ entities_to_markdown: [%s]\n", md); pass++;
    } else {
        printf("  ❌ entities_to_markdown:\n     got:      [%s]\n     expected: [%s]\n", md, expected_md);
        fail++;
    }
    free(md);
}

int main(void) {
    printf("====== USR Round-Trip Tests ======\n\n");

    printf("── Markdown → HTML → plain round-trip ──\n");
    test_md_html_roundtrip("Hello *world*");
    test_md_html_roundtrip("*bold* and _italic_");
    test_md_html_roundtrip("__underline__");
    test_md_html_roundtrip("~strike~");
    test_md_html_roundtrip("||spoiler||");
    test_md_html_roundtrip("`code`");
    test_md_html_roundtrip("*bold _italic_*");
    test_md_html_roundtrip("Hello 🙂 *world*");
    test_md_html_roundtrip("[Google](https://google.com)");
    test_md_html_roundtrip("simple text no formatting");
    test_md_html_roundtrip("*bold* `code` ~strike~");

    printf("\n── Entities → Markdown ──\n");
    {
        const char *plain = "Hello world";
        usr_entity ents[1] = {{USR_ENTITY_BOLD, 6, 5, NULL}};
        test_entities_to_markdown(plain, ents, 1, "Hello *world*");
    }
    {
        const char *plain = "Hello world";
        usr_entity ents[1] = {{USR_ENTITY_ITALIC, 0, 5, NULL}};
        test_entities_to_markdown(plain, ents, 1, "_Hello_ world");
    }

    printf("\n══════════════════════════════\n");
    printf("Results: %d passed, %d failed\n", pass, fail);
    return (fail > 0) ? 1 : 0;
}
