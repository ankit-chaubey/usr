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

# Entity type enum values — must match entities.h exactly
class EntityType:
    BOLD            = 0
    ITALIC          = 1
    UNDERLINE       = 2
    STRIKETHROUGH   = 3
    SPOILER         = 4
    CODE            = 5
    PRE             = 6
    TEXT_LINK       = 7
    CUSTOM_EMOJI    = 8
    BLOCKQUOTE      = 9
    MENTION         = 10
    HASHTAG         = 11
    CASHTAG         = 12
    BOT_COMMAND     = 13
    URL             = 14
    EMAIL           = 15
    PHONE_NUMBER    = 16
    TEXT_MENTION    = 17

    _names = {
        0: "bold", 1: "italic", 2: "underline", 3: "strikethrough",
        4: "spoiler", 5: "code", 6: "pre", 7: "text_link",
        8: "custom_emoji", 9: "blockquote", 10: "mention", 11: "hashtag",
        12: "cashtag", 13: "bot_command", 14: "url", 15: "email",
        16: "phone_number", 17: "text_mention",
    }

    @classmethod
    def name(cls, t: int) -> str:
        return cls._names.get(t, f"unknown({t})")

# Markdown version enum
class MarkdownVersion:
    V1 = 1
    V2 = 2

__all__ = ["usr_bytes", "usr_bytes_view", "usr_entity", "EntityType", "MarkdownVersion"]
