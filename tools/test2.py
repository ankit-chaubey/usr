import ctypes, os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIB = os.path.join(ROOT, "build", "libusr.so")

if not os.path.exists(LIB):
    print("libusr.so not found")
    sys.exit(1)

print("✅ Using", LIB)
libusr = ctypes.CDLL(LIB)
libc = ctypes.CDLL(None)

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

libusr.usr_binary_from_text_out.argtypes = [
    ctypes.c_char_p,
    ctypes.POINTER(usr_bytes),
]
libusr.usr_binary_from_text_out.restype = None

libusr.usr_text_from_binary.argtypes = [
    ctypes.POINTER(usr_bytes_view),
]
libusr.usr_text_from_binary.restype = ctypes.c_void_p

libusr.usr_bytes_free.argtypes = [ctypes.POINTER(usr_bytes)]
libusr.usr_bytes_free.restype = None

libc.free.argtypes = [ctypes.c_void_p]
libc.free.restype = None

# ---- TEST 2 (different content) ----
text = "Unicode 🙂 test2 payload".encode("utf-8")
b = usr_bytes()
libusr.usr_binary_from_text_out(text, ctypes.byref(b))

print("Length:", b.len)
print("Bytes :", bytes(b.data[:b.len]))

view = usr_bytes_view(b.data, b.len)
ptr = libusr.usr_text_from_binary(ctypes.byref(view))
out = ctypes.string_at(ptr).decode("utf-8")

print("Back :", out)

libc.free(ptr)
libusr.usr_bytes_free(ctypes.byref(b))
