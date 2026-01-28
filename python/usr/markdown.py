from ._lib import lib
import ctypes

class usr_entity(ctypes.Structure):
    _fields_ = [
        ("type", ctypes.c_int),
        ("offset", ctypes.c_uint32),
        ("length", ctypes.c_uint32),
        ("extra", ctypes.c_char_p),
    ]

lib.usr_markdown_parse.argtypes = [
    ctypes.c_char_p,
    ctypes.c_int,
    ctypes.POINTER(usr_entity),
    ctypes.c_size_t,
]

def parse_markdown(text: str):
    buf = text.encode("utf-8")
    ents = (usr_entity * 64)()
    n = lib.usr_markdown_parse(buf, 2, ents, 64)
    return ents[:n]
