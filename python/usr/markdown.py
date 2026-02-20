"""usr.markdown — Parse Telegram Markdown V1/V2 ↔ entities, render entities → markdown."""
from __future__ import annotations
from typing import List, Tuple, Optional
import ctypes
from ._lib import lib, libc
from ._structs import usr_entity, MarkdownVersion
from .entities import Entity, _c_array, _from_c

_MAX_ENTITIES = 256

# ── markdown_parse ────────────────────────────────────────────────────────────
lib.usr_markdown_parse.argtypes = [
    ctypes.c_char_p,                    # markdown text
    ctypes.c_int,                       # version (1 or 2)
    ctypes.POINTER(ctypes.c_char_p),    # plain_out (heap, caller frees)
    ctypes.POINTER(usr_entity),         # entities out
    ctypes.c_size_t,                    # max_entities
]
lib.usr_markdown_parse.restype = ctypes.c_size_t

def markdown_parse(
    text:    str,
    version: int = MarkdownVersion.V2,
    max_entities: int = _MAX_ENTITIES,
) -> Tuple[str, List[Entity]]:
    """Parse Telegram MarkdownV1 or MarkdownV2 into (plain_text, entities)."""
    enc     = text.encode()
    plain_p = ctypes.c_char_p(None)
    arr     = (usr_entity * max_entities)()
    n       = lib.usr_markdown_parse(enc, int(version), ctypes.byref(plain_p), arr, max_entities)
    if n == ctypes.c_size_t(-1).value:
        raise MemoryError("markdown_parse: allocation failed")
    plain = plain_p.value.decode() if plain_p.value else ""
    libc.free(plain_p)
    return plain, _from_c(arr, n)

# ── entities_to_markdown ──────────────────────────────────────────────────────
lib.usr_entities_to_markdown.argtypes = [
    ctypes.c_char_p,
    ctypes.POINTER(usr_entity),
    ctypes.c_size_t,
    ctypes.c_int,
]
lib.usr_entities_to_markdown.restype = ctypes.c_void_p

def entities_to_markdown(
    text:     str,
    entities: List[Entity],
    version:  int = MarkdownVersion.V2,
) -> str:
    """Render plain text + entities to Telegram MarkdownV2 (or V1)."""
    enc = text.encode()
    arr = _c_array(entities) if entities else (usr_entity * 1)()
    ptr = lib.usr_entities_to_markdown(enc, arr, len(entities), int(version))
    if not ptr: raise MemoryError("entities_to_markdown: allocation failed")
    result = ctypes.string_at(ptr).decode()
    libc.free(ptr)
    return result

__all__ = ["markdown_parse", "entities_to_markdown", "MarkdownVersion"]
