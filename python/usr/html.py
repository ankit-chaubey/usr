"""usr.html — Parse Telegram HTML ↔ entities, render entities → HTML."""
from __future__ import annotations
from typing import List, Tuple, Optional
import ctypes
from ._lib import lib, libc
from ._structs import usr_entity
from .entities import Entity, _c_array, _from_c

_MAX_ENTITIES = 256

# ── html_parse ────────────────────────────────────────────────────────────────
lib.usr_html_parse.argtypes = [
    ctypes.c_char_p,                    # html
    ctypes.POINTER(ctypes.c_char_p),    # plain_out (heap, caller frees with free())
    ctypes.POINTER(usr_entity),         # entities out
    ctypes.c_size_t,                    # max_entities
]
lib.usr_html_parse.restype = ctypes.c_size_t

def html_parse(html: str, max_entities: int = _MAX_ENTITIES) -> Tuple[str, List[Entity]]:
    """Parse Telegram-compatible HTML into (plain_text, entities)."""
    enc      = html.encode()
    plain_p  = ctypes.c_char_p(None)
    arr      = (usr_entity * max_entities)()
    n        = lib.usr_html_parse(enc, ctypes.byref(plain_p), arr, max_entities)
    if n == ctypes.c_size_t(-1).value:
        raise MemoryError("html_parse: allocation failed")
    plain = plain_p.value.decode() if plain_p.value else ""
    libc.free(plain_p)
    return plain, _from_c(arr, n)

# ── entities_to_html ──────────────────────────────────────────────────────────
lib.usr_entities_to_html.argtypes = [
    ctypes.c_char_p,
    ctypes.POINTER(usr_entity),
    ctypes.c_size_t,
]
lib.usr_entities_to_html.restype = ctypes.c_void_p

def entities_to_html(text: str, entities: List[Entity]) -> str:
    """Render plain text + entities to Telegram-compatible HTML."""
    enc = text.encode()
    arr = _c_array(entities) if entities else (usr_entity * 1)()
    ptr = lib.usr_entities_to_html(enc, arr, len(entities))
    if not ptr: raise MemoryError("entities_to_html: allocation failed")
    result = ctypes.string_at(ptr).decode()
    libc.free(ptr)
    return result

__all__ = ["html_parse", "entities_to_html"]
