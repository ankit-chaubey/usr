#ifndef USR_ENTITIES_H
#define USR_ENTITIES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   Telegram MessageEntity types
   ============================================================ */

typedef enum {
    USR_ENTITY_BOLD          = 0,
    USR_ENTITY_ITALIC        = 1,
    USR_ENTITY_UNDERLINE     = 2,
    USR_ENTITY_STRIKETHROUGH = 3,
    USR_ENTITY_SPOILER       = 4,
    USR_ENTITY_CODE          = 5,   /* inline code */
    USR_ENTITY_PRE           = 6,   /* code block (may have language) */
    USR_ENTITY_TEXT_LINK     = 7,   /* <a href="..."> */
    USR_ENTITY_CUSTOM_EMOJI  = 8,   /* custom emoji by document_id */
    USR_ENTITY_BLOCKQUOTE    = 9,   /* blockquote / expandable */
    USR_ENTITY_MENTION       = 10,  /* @username */
    USR_ENTITY_HASHTAG       = 11,  /* #hashtag */
    USR_ENTITY_CASHTAG       = 12,  /* $USD */
    USR_ENTITY_BOT_COMMAND   = 13,  /* /start */
    USR_ENTITY_URL           = 14,  /* plain URL */
    USR_ENTITY_EMAIL         = 15,  /* email@example.com */
    USR_ENTITY_PHONE_NUMBER  = 16,  /* +1234567890 */
    USR_ENTITY_TEXT_MENTION  = 17,  /* @mention with user ID in extra */
    _USR_ENTITY_TYPE_COUNT   = 18
} usr_entity_type;

/* ============================================================
   usr_entity — a single formatting/semantic span
   ============================================================ */

typedef struct {
    usr_entity_type type;
    uint32_t        offset;  /* UTF-16 code-unit offset in the plain text */
    uint32_t        length;  /* UTF-16 code-unit length */
    const char     *extra;   /* URL for TEXT_LINK, lang for PRE,
                                document_id for CUSTOM_EMOJI, user_id for TEXT_MENTION */
} usr_entity;

/* ============================================================
   Normalization
   ============================================================ */

/*
 * Sort and normalize an entity array.
 *
 * Rules (Telegram-compatible):
 *  1. Sort by offset ascending; ties: longer entity first.
 *  2. Properly-nested entities are KEPT (unlike the original which dropped all overlaps).
 *     e.g., BOLD spans [0,10], ITALIC spans [2,6] — valid nesting.
 *  3. Truly overlapping (crossing) spans: the inner one is dropped.
 *  4. Zero-length entities are dropped.
 *
 * Returns the new count of valid entities in `e`.
 * The array is compacted in-place.
 */
size_t usr_entities_normalize(usr_entity *e, size_t count);

/* ============================================================
   Utility
   ============================================================ */

/* Return a human-readable name for an entity type. */
const char *usr_entity_type_name(usr_entity_type t);

/* Deep-copy `src` entity into `dst`. Duplicates the `extra` string if non-NULL.
   Caller must free dst->extra when done. */
void usr_entity_copy(usr_entity *dst, const usr_entity *src);

/* Free a deep-copied entity's `extra` field. */
void usr_entity_free_extra(usr_entity *e);

#ifdef __cplusplus
}
#endif

#endif /* USR_ENTITIES_H */
