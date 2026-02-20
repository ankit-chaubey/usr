"""
usr._structs — Shared ctypes structure definitions matching include/usr/*.h
"""
import ctypes

class usr_bytes(ctypes.Structure):
    _fields_ = [
        ("data", ctypes.POINTER(ctypes.c_uint8)),
        ("len",  ctypes.c_size_t),
        ("cap",  ctypes.c_size_t),
    ]

class usr_bytes_view(ctypes.Structure):
    _fields_ = [
        ("data", ctypes.POINTER(ctypes.c_uint8)),
        ("len",  ctypes.c_size_t),
    ]

class usr_entity(ctypes.Structure):
    _fields_ = [
        ("type",   ctypes.c_int),
        ("offset", ctypes.c_uint32),
        ("length", ctypes.c_uint32),
        ("extra",  ctypes.c_char_p),
    ]

# Entity type enum values — must match entities.h
class EntityType:
    MENTION         = 0
    HASHTAG         = 1
    CASHTAG         = 2
    BOT_COMMAND     = 3
    URL             = 4
    EMAIL           = 5
    PHONE_NUMBER    = 6
    BOLD            = 7
    ITALIC          = 8
    UNDERLINE       = 9
    STRIKETHROUGH   = 10
    SPOILER         = 11
    CODE            = 12
    PRE             = 13
    TEXT_LINK       = 14
    TEXT_MENTION    = 15
    CUSTOM_EMOJI    = 16
    BLOCKQUOTE      = 17

    _names = {
        0: "mention", 1: "hashtag", 2: "cashtag", 3: "bot_command",
        4: "url", 5: "email", 6: "phone_number", 7: "bold", 8: "italic",
        9: "underline", 10: "strikethrough", 11: "spoiler", 12: "code",
        13: "pre", 14: "text_link", 15: "text_mention", 16: "custom_emoji",
        17: "blockquote",
    }

    @classmethod
    def name(cls, t: int) -> str:
        return cls._names.get(t, f"unknown({t})")

# Markdown version enum
class MarkdownVersion:
    V1 = 1
    V2 = 2

__all__ = ["usr_bytes", "usr_bytes_view", "usr_entity", "EntityType", "MarkdownVersion"]
