#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "usr/markdown.h"
#include "usr/html.h"
#include "usr/entities.h"

#define MAXE 128
#define MAXL 256
#define ITERS 3000

static const char *atoms[] = {
    "text", "hello", "world",
    "*b*", "_i_", "~s~",
    "`c`",
    "||sp||",
    "__u__",
    "🙂",
    " "
};

/*
 Best-effort Telegram-style fuzz domain.
 This intentionally avoids pathological grammar.
*/
static int is_reasonable_markdown(const char *s) {
    int star = 0, under = 0, tilde = 0, pipe = 0;
    int backticks = 0;
    int has_format = 0;

    for (size_t i = 0; s[i]; i++) {

        if (s[i] == '`')
            backticks++;

        if (s[i] == '*' || s[i] == '_' || s[i] == '~' || s[i] == '|')
            has_format = 1;

        /* allow only one inline code span */
        if (backticks > 2)
            return 0;

        /* do not mix code with formatting */
        if (backticks > 0 && has_format && s[i] != '`')
            return 0;

        /* reject ambiguous marker soup */
        if ((s[i] == '*' && s[i+1] == '*') ||
            (s[i] == '_' && s[i+1] == '_') ||
            (s[i] == '~' && s[i+1] == '~') ||
            (s[i] == '|' && s[i+1] == '|' && s[i+2] == '|')) {
            return 0;
        }

        if (s[i] == '*') star ^= 1;
        if (s[i] == '_') under ^= 1;
        if (s[i] == '~') tilde ^= 1;

        if (s[i] == '|' && s[i+1] == '|') {
            pipe ^= 1;
            i++;
        }
    }

    return !star && !under && !tilde && !pipe;
}

static void rand_input(char *out) {
    int n = rand() % 10 + 1;
    out[0] = 0;

    for (int i = 0; i < n; i++) {
        strcat(out, atoms[rand() % (sizeof(atoms) / sizeof(atoms[0]))]);
    }
}

int main(void) {
    srand((unsigned)time(NULL));

    for (int i = 0; i < ITERS; i++) {
        char input[MAXL];

        do {
            rand_input(input);
        } while (!is_reasonable_markdown(input));

        usr_entity e1[MAXE], e2[MAXE];

        /* Markdown → entities */
        size_t n1 = usr_markdown_parse(input, USR_MD_V2, e1, MAXE);
        n1 = usr_entities_normalize(e1, n1);

        /* Invariant: entities valid */
        for (size_t j = 0; j < n1; j++) {
            if (e1[j].length == 0) {
                printf("❌ INVALID ENTITY (markdown)\nInput: %s\n", input);
                return 1;
            }
        }

        /* entities → HTML */
        char *html = usr_entities_to_html_rt(input, e1, n1);
        if (!html) {
            printf("❌ HTML NULL\nInput: %s\n", input);
            return 1;
        }

        /* HTML → entities */
        size_t n2 = usr_html_parse(html, e2, MAXE);
        n2 = usr_entities_normalize(e2, n2);

        /* Invariant: entities valid after round */
        for (size_t j = 0; j < n2; j++) {
            if (e2[j].length == 0) {
                printf("❌ INVALID ENTITY (html)\nInput: %s\nHTML: %s\n",
                       input, html);
                free(html);
                return 1;
            }
        }

        free(html);
    }

    printf("✅ Fuzz tests passed (best-effort invariants)\n");
    return 0;
}
