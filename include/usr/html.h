#ifndef USR_HTML_H
#define USR_HTML_H

#include <stddef.h>
#include "usr/entities.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   HTML → Entities
   ============================================================ */

/*
 * Parse Telegram-compatible HTML into entities.
 *
 * Supported tags: <b> <strong> <i> <em> <u> <ins> <s> <del> <strike>
 *   <tg-spoiler> <code> <pre> <pre><code class="language-xxx">
 *   <a href="..."> <tg-emoji emoji-id="...">
 *   HTML entities: &amp; &lt; &gt; &quot; &apos; &#NNN; &#xNN;
 *
 * `plain_out`   — if non-NULL, receives the plain-text (malloc'd, caller frees).
 * `entities_out` — receives entities (UTF-16 offsets into plain_out).
 * `max_entities` — capacity of entities_out.
 *
 * Returns number of entities written to entities_out.
 * Returns (size_t)-1 on allocation failure.
 */
size_t usr_html_parse(
    const char  *html,
    char       **plain_out,
    usr_entity  *entities_out,
    size_t       max_entities
);

/* ============================================================
   Entities → HTML
   ============================================================ */

/*
 * Render plain text + entities to Telegram-compatible HTML.
 *
 * Handles:
 *  - Proper tag nesting (closes inner tags before outer at boundary changes)
 *  - HTML-escaping of &, <, > in text
 *  - All entity types
 *  - UTF-8 / UTF-16 offset arithmetic
 *
 * Returns a heap-allocated string (caller must free), or NULL on OOM.
 *
 * `entities` array does not need to be pre-sorted; the renderer sorts internally.
 */
char *usr_entities_to_html(
    const char        *text,
    const usr_entity  *entities,
    size_t             count
);

#ifdef __cplusplus
}
#endif

#endif /* USR_HTML_H */
