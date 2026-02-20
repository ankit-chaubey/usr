#ifndef USR_MARKDOWN_H
#define USR_MARKDOWN_H

#include <stddef.h>
#include <stdint.h>
#include "usr/entities.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   Markdown Version
   ============================================================ */

typedef enum {
    USR_MD_V1 = 1,  /* Legacy Telegram Markdown (limited) */
    USR_MD_V2 = 2   /* MarkdownV2 (current, recommended) */
} usr_markdown_version;

/* ============================================================
   Markdown → Entities
   ============================================================ */

/*
 * Parse Telegram Markdown into plain text + entities.
 *
 * MarkdownV2 syntax:
 *   *bold*   _italic_   __underline__   ~strikethrough~   ||spoiler||
 *   `code`   ```pre block```
 *   [text](url)  (inline link)
 *   ![alt](url)  (inline image — stored as URL entity)
 *   \<char>  (escape any special character)
 *
 * V1 syntax (subset, preserved for compatibility):
 *   *bold*  `code`  [text](url)
 *
 * `plain_out`    — if non-NULL, receives the plain text (malloc'd, caller frees).
 *                  Markers and escape backslashes are stripped.
 * `entities_out` — array to receive parsed entities (UTF-16 offsets into plain_out).
 * `max_entities` — capacity of entities_out.
 *
 * Returns number of entities written.
 * Returns (size_t)-1 on allocation failure.
 */
size_t usr_markdown_parse(
    const char           *text,
    usr_markdown_version  version,
    char                **plain_out,
    usr_entity           *entities_out,
    size_t                max_entities
);

/* ============================================================
   Entities → Markdown
   ============================================================ */

/*
 * Render plain text + entities back to Telegram MarkdownV2.
 *
 * Special characters in the plain text are properly escaped.
 * Nested entities produce correctly-ordered open/close markers.
 *
 * Returns a heap-allocated string (caller must free), or NULL on OOM.
 */
char *usr_entities_to_markdown(
    const char       *text,
    const usr_entity *entities,
    size_t            count,
    usr_markdown_version version
);

#ifdef __cplusplus
}
#endif

#endif /* USR_MARKDOWN_H */
