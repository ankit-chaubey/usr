"""usr.entities — Telegram MessageEntity handling."""
from __future__ import annotations
from dataclasses import dataclass, field
from typing import Optional, List
import ctypes
from ._lib import lib, libc
from ._structs import usr_entity, EntityType

@dataclass
class Entity:
    type:   int
    offset: int
    length: int
    extra:  Optional[str] = None

    @property
    def type_name(self) -> str:
        return EntityType.name(self.type)

    def to_c(self) -> usr_entity:
        e = usr_entity()
        e.type   = self.type
        e.offset = self.offset
        e.length = self.length
        e.extra  = self.extra.encode() if self.extra else None
        return e

    def __repr__(self) -> str:
        s = f"Entity({self.type_name}, offset={self.offset}, length={self.length}"
        if self.extra: s += f", extra={self.extra!r}"
        return s + ")"

def _c_array(entities: List[Entity]):
    arr = (usr_entity * len(entities))()
    for i, e in enumerate(entities):
        arr[i] = e.to_c()
    return arr

def _from_c(arr, count: int) -> List[Entity]:
    result = []
    for i in range(count):
        e = arr[i]
        result.append(Entity(
            type   = e.type,
            offset = e.offset,
            length = e.length,
            extra  = e.extra.decode() if e.extra else None,
        ))
    return result

# ── Normalize ──────────────────────────────────────────────────────────────
lib.usr_entities_normalize.argtypes = [ctypes.POINTER(usr_entity), ctypes.c_size_t]
lib.usr_entities_normalize.restype  = ctypes.c_size_t

def normalize(entities: List[Entity]) -> List[Entity]:
    """Sort and remove zero-length / crossing spans. Proper nesting is kept."""
    if not entities: return []
    arr = _c_array(entities)
    n   = lib.usr_entities_normalize(arr, len(entities))
    return _from_c(arr, n)

__all__ = ["Entity", "EntityType", "normalize"]
