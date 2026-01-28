#ifndef USR_ENTITIES_H
#define USR_ENTITIES_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    USR_ENTITY_BOLD,
    USR_ENTITY_ITALIC,
    USR_ENTITY_UNDERLINE,
    USR_ENTITY_STRIKETHROUGH,
    USR_ENTITY_SPOILER,
    USR_ENTITY_CODE,
    USR_ENTITY_PRE,
    USR_ENTITY_TEXT_LINK,
    USR_ENTITY_CUSTOM_EMOJI
} usr_entity_type;

typedef struct {
    usr_entity_type type;
    uint32_t offset;   /* UTF-16 */
    uint32_t length;   /* UTF-16 */
    const char *extra;
} usr_entity;

/* 🔑 ADD THIS DECLARATION */
size_t usr_entities_normalize(usr_entity *entities, size_t count);

#endif
