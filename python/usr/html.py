from ._lib import lib
import ctypes

class usr_entity(ctypes.Structure):
    _fields_ = [
        ("type", ctypes.c_int),
        ("offset", ctypes.c_uint32),
        ("length", ctypes.c_uint32),
        ("extra", ctypes.c_char_p),
    ]

lib.usr_entities_to_html_rt.argtypes = [
    ctypes.c_char_p,
    ctypes.POINTER(usr_entity),
    ctypes.c_size_t,
]
lib.usr_entities_to_html_rt.restype = ctypes.c_void_p

def entities_to_html(text: str, entities):
    buf = text.encode("utf-8")
    arr = (usr_entity * len(entities))(*entities)
    ptr = lib.usr_entities_to_html_rt(buf, arr, len(entities))
    return ctypes.string_at(ptr).decode("utf-8")
