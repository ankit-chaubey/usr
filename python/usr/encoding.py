"""usr.encoding — Base64, hex, URL encoding/decoding, HTML escaping."""
from __future__ import annotations
import ctypes
from ._lib import lib, libc

def _buf(data: bytes): return (ctypes.c_uint8 * len(data)).from_buffer_copy(data) if data else (ctypes.c_uint8 * 1)()
def _cbuf(n): return ctypes.create_string_buffer(n)

# ── Base64 ──────────────────────────────────────────────────────────────────
lib.usr_base64_encode.argtypes     = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t, ctypes.c_char_p]
lib.usr_base64_encode.restype      = None
lib.usr_base64url_encode.argtypes  = lib.usr_base64_encode.argtypes
lib.usr_base64url_encode.restype   = None
lib.usr_base64_decode.argtypes     = [ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint8)]
lib.usr_base64_decode.restype      = ctypes.c_size_t
lib.usr_base64url_decode.argtypes  = lib.usr_base64_decode.argtypes
lib.usr_base64url_decode.restype   = ctypes.c_size_t

def _encoded_size(n): return ((n + 2) // 3) * 4 + 1

def base64_encode(data: bytes) -> str:
    data = bytes(data); out = _cbuf(_encoded_size(len(data)))
    lib.usr_base64_encode(_buf(data), len(data), out); return out.value.decode()

def base64url_encode(data: bytes) -> str:
    data = bytes(data); out = _cbuf(_encoded_size(len(data)))
    lib.usr_base64url_encode(_buf(data), len(data), out); return out.value.decode()

def base64_decode(s: str) -> bytes:
    enc = s.encode() if isinstance(s, str) else bytes(s)
    out = (ctypes.c_uint8 * (len(enc) * 3 // 4 + 4))()
    n   = lib.usr_base64_decode(enc, len(enc), out)
    if n == ctypes.c_size_t(-1).value: raise ValueError("Invalid base64")
    return bytes(out[:n])

def base64url_decode(s: str) -> bytes:
    enc = s.encode() if isinstance(s, str) else bytes(s)
    out = (ctypes.c_uint8 * (len(enc) * 3 // 4 + 4))()
    n   = lib.usr_base64url_decode(enc, len(enc), out)
    if n == ctypes.c_size_t(-1).value: raise ValueError("Invalid base64url")
    return bytes(out[:n])

# ── Hex ──────────────────────────────────────────────────────────────────────
lib.usr_hex_encode.argtypes       = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t, ctypes.c_char_p]
lib.usr_hex_encode.restype        = None
lib.usr_hex_encode_upper.argtypes = lib.usr_hex_encode.argtypes
lib.usr_hex_encode_upper.restype  = None
lib.usr_hex_decode.argtypes       = [ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint8)]
lib.usr_hex_decode.restype        = ctypes.c_size_t

def hex_encode(data: bytes, upper: bool = False) -> str:
    data = bytes(data); out = _cbuf(len(data) * 2 + 1)
    fn = lib.usr_hex_encode_upper if upper else lib.usr_hex_encode
    fn(_buf(data), len(data), out); return out.value.decode()

def hex_decode(s: str) -> bytes:
    enc = s.encode() if isinstance(s, str) else bytes(s)
    out = (ctypes.c_uint8 * (len(enc) // 2 + 2))()
    n   = lib.usr_hex_decode(enc, len(enc), out)
    if n == ctypes.c_size_t(-1).value: raise ValueError("Invalid hex string")
    return bytes(out[:n])

# ── URL encoding ─────────────────────────────────────────────────────────────
lib.usr_url_encode.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p]
lib.usr_url_encode.restype  = None
lib.usr_url_decode.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p]
lib.usr_url_decode.restype  = None

def url_encode(s: str) -> str:
    data = s.encode() if isinstance(s, str) else bytes(s)
    out  = _cbuf(len(data) * 3 + 1)
    lib.usr_url_encode(data, len(data), out); return out.value.decode()

def url_decode(s: str) -> str:
    data = s.encode() if isinstance(s, str) else bytes(s)
    out  = _cbuf(len(data) + 1)
    lib.usr_url_decode(data, len(data), out); return out.value.decode()

# ── HTML escape ───────────────────────────────────────────────────────────────
lib.usr_html_escape.argtypes   = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p]
lib.usr_html_escape.restype    = None
lib.usr_html_unescape.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p]
lib.usr_html_unescape.restype  = ctypes.c_size_t

def html_escape(s: str) -> str:
    data = s.encode(); out = _cbuf(len(data) * 6 + 1)
    lib.usr_html_escape(data, len(data), out); return out.value.decode()

def html_unescape(s: str) -> str:
    data = s.encode(); out = _cbuf(len(data) * 4 + 1)
    lib.usr_html_unescape(data, len(data), out); return out.value.decode()

__all__ = [
    "base64_encode","base64_decode","base64url_encode","base64url_decode",
    "hex_encode","hex_decode","url_encode","url_decode",
    "html_escape","html_unescape",
]
