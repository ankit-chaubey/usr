#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "usr/markdown.h"
#include "usr/html.h"
#include "usr/entities.h"

#define MAXE 128
#define ITERS 5000

static const char *atoms[] = {
    "text", "hello", "world", "foo", "bar", "baz",
    "*b*", "_i_", "~s~", "`c`", "||sp||", "__u__",
    "[lnk](https://x.com)", "🙂", "café", " ", "\n"
};
static const int natoms = (int)(sizeof(atoms)/sizeof(atoms[0]));

static void rand_input(char *out, size_t max) {
    int n = rand() % 8 + 1;
    out[0] = 0;
    for (int i = 0; i < n; i++) {
        const char *a = atoms[rand() % natoms];
        if (strlen(out) + strlen(a) + 1 < max) strcat(out, a);
    }
}

int main(void) {
    srand((unsigned)time(NULL));
    int pass = 0, fail = 0;

    for (int iter = 0; iter < ITERS; iter++) {
        char input[512];
        rand_input(input, sizeof(input));

        usr_entity e1[MAXE], e2[MAXE];
        char *plain1 = NULL, *plain2 = NULL, *html_out = NULL;

        size_t n1 = usr_markdown_parse(input, USR_MD_V2, &plain1, e1, MAXE);
        if (n1 == (size_t)-1) { fail++; continue; }
        n1 = usr_entities_normalize(e1, n1);

        for (size_t j = 0; j < n1; j++) {
            if (e1[j].length == 0) {
                printf("❌ iter %d: zero-length entity after markdown\n  input:[%s]\n", iter, input);
                fail++; goto cleanup;
            }
        }

        html_out = plain1 ? usr_entities_to_html(plain1, e1, n1) : NULL;
        if (plain1 && !html_out) {
            printf("❌ iter %d: entities_to_html returned NULL\n  input:[%s]\n", iter, input);
            fail++; goto cleanup;
        }

        if (html_out) {
            size_t n2 = usr_html_parse(html_out, &plain2, e2, MAXE);
            if (n2 != (size_t)-1) {
                n2 = usr_entities_normalize(e2, n2);
                for (size_t j = 0; j < n2; j++) {
                    if (e2[j].length == 0) {
                        printf("❌ iter %d: zero-length entity after html\n  input:[%s]\n  html:[%s]\n",
                               iter, input, html_out);
                        fail++; goto cleanup;
                    }
                }
                /* Plain text invariant: parse(html(plain1)) == plain1 */
                if (plain2 && plain1 && strcmp(plain1, plain2) != 0) {
                    printf("❌ iter %d: plain text mismatch\n  input:[%s]\n  plain1:[%s]\n  html:[%s]\n  plain2:[%s]\n",
                           iter, input, plain1, html_out, plain2);
                    fail++; goto cleanup;
                }
            }
        }

        pass++;

cleanup:
        free(plain1); free(plain2); free(html_out);
        plain1 = plain2 = html_out = NULL;
    }

    printf("Fuzz: %d/%d passed (%d failed)\n", pass, ITERS, fail);
    return (fail > 0) ? 1 : 0;
}
