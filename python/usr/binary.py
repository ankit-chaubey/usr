"""usr.binary — Text ↔ binary conversion (ABI-safe wrappers)."""
from __future__ import annotations
import ctypes
from ._lib import lib, libc
from ._structs import usr_bytes, usr_bytes_view

lib.usr_binary_from_text_out.argtypes = [ctypes.c_char_p, ctypes.POINTER(usr_bytes)]
lib.usr_binary_from_text_out.restype  = None
lib.usr_bytes_free.argtypes           = [ctypes.POINTER(usr_bytes)]
lib.usr_bytes_free.restype            = None
lib.usr_text_from_binary.argtypes     = [ctypes.POINTER(usr_bytes_view)]
lib.usr_text_from_binary.restype      = ctypes.c_void_p

def from_text(text: str | bytes) -> bytes:
    """Encode text (str or UTF-8 bytes) to its binary representation."""
    if isinstance(text, str): text = text.encode("utf-8")
    b = usr_bytes()
    lib.usr_binary_from_text_out(text, ctypes.byref(b))
    try:
        return bytes(b.data[:b.len]) if b.len else b""
    finally:
        lib.usr_bytes_free(ctypes.byref(b))

def to_text(data: bytes) -> str:
    """Decode binary data back to text (UTF-8 string)."""
    data = bytes(data)
    buf  = (ctypes.c_uint8 * len(data)).from_buffer_copy(data) if data else (ctypes.c_uint8 * 1)()
    view = usr_bytes_view(buf, len(data))
    ptr  = lib.usr_text_from_binary(ctypes.byref(view))
    try:
        return ctypes.string_at(ptr).decode("utf-8") if ptr else ""
    finally:
        if ptr: libc.free(ptr)

__all__ = ["from_text", "to_text"]
