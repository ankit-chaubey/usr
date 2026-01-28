import ctypes
from ._lib import lib, libc

class usr_bytes(ctypes.Structure):
    _fields_ = [
        ("data", ctypes.POINTER(ctypes.c_uint8)),
        ("len", ctypes.c_size_t),
        ("cap", ctypes.c_size_t),
    ]

class usr_bytes_view(ctypes.Structure):
    _fields_ = [
        ("data", ctypes.POINTER(ctypes.c_uint8)),
        ("len", ctypes.c_size_t),
    ]

lib.usr_binary_from_text_out.argtypes = [
    ctypes.c_char_p,
    ctypes.POINTER(usr_bytes),
]
lib.usr_text_from_binary.argtypes = [
    ctypes.POINTER(usr_bytes_view),
]
lib.usr_text_from_binary.restype = ctypes.c_void_p
lib.usr_bytes_free.argtypes = [ctypes.POINTER(usr_bytes)]

def from_text(text: str | bytes) -> bytes:
    if isinstance(text, str):
        text = text.encode("utf-8")

    b = usr_bytes()
    lib.usr_binary_from_text_out(text, ctypes.byref(b))
    try:
        return bytes(b.data[:b.len])
    finally:
        lib.usr_bytes_free(ctypes.byref(b))

def to_text(data: bytes) -> str:
    buf = (ctypes.c_uint8 * len(data)).from_buffer_copy(data)
    view = usr_bytes_view(buf, len(data))
    ptr = lib.usr_text_from_binary(ctypes.byref(view))
    try:
        return ctypes.string_at(ptr).decode("utf-8")
    finally:
        libc.free(ptr)
