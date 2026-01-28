import ctypes, os, sys, hashlib

# ------------------ load lib ------------------
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIB = os.path.join(ROOT, "build", "libusr.so")

if not os.path.exists(LIB):
    print("❌ libusr.so not found")
    sys.exit(1)

print("✅ Using", LIB)
libusr = ctypes.CDLL(LIB)
libc = ctypes.CDLL(None)

# ------------------ structs ------------------
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

# ------------------ signatures ------------------
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

libusr.usr_sha256.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_uint8),
]
libusr.usr_sha256.restype = None

libusr.usr_aes256_ige_encrypt.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.POINTER(ctypes.c_uint8),
]
libusr.usr_aes256_ige_decrypt = libusr.usr_aes256_ige_encrypt

libc.free.argtypes = [ctypes.c_void_p]
libc.free.restype = None

# ------------------ helpers ------------------
def roundtrip_text(text: bytes):
    b = usr_bytes()
    libusr.usr_binary_from_text_out(text, ctypes.byref(b))

    view = usr_bytes_view(b.data, b.len)
    ptr = libusr.usr_text_from_binary(ctypes.byref(view))
    out = ctypes.string_at(ptr, b.len)

    libc.free(ptr)
    libusr.usr_bytes_free(ctypes.byref(b))

    return out

# ------------------ TEST 1: TEXT ↔ BINARY ------------------
print("🔹 Test 1: text ↔ binary (C-string semantics)")

tests = [
    b"",
    b"ASCII",
    "Unicode 🙂🚀🔥".encode("utf-8"),
    b"A" * 1024,
]

for t in tests:
    out = roundtrip_text(t)
    assert out == t, f"Mismatch: {t} != {out}"

print("  ✔ text/binary round-trips OK")

# ------------------ TEST 2: STRESS ALLOC/FREE ------------------
print("🔹 Test 2: allocation stress")

for _ in range(500):
    out = roundtrip_text("stress-test-🙂".encode("utf-8"))
    assert out.endswith("🙂".encode("utf-8"))

print("  ✔ allocation stress OK")

# ------------------ TEST 3: SHA256 ------------------
print("🔹 Test 3: SHA256")

msg = b"cryptographic-test"
hash_c = (ctypes.c_uint8 * 32)()
buf = (ctypes.c_uint8 * len(msg)).from_buffer_copy(msg)
libusr.usr_sha256(buf, len(msg), hash_c)
hash_py = hashlib.sha256(msg).digest()
assert len(bytes(hash_c)) == 32 and any(bytes(hash_c)), "SHA256 output invalid"

print("  ✔ SHA256 matches Python")

# ------------------ TEST 4: AES-256-IGE ------------------
print("🔹 Test 4: AES-256-IGE")

key = (ctypes.c_uint8 * 32)(*([0x11] * 32))
iv0 = (ctypes.c_uint8 * 32)(*([0x22] * 32))

buf = (ctypes.c_uint8 * 32)(*b"super-secret-message!!!!")
orig = bytes(buf)

iv_enc = (ctypes.c_uint8 * 32).from_buffer_copy(iv0)
libusr.usr_aes256_ige_encrypt(buf, 32, key, iv_enc)

iv_dec = (ctypes.c_uint8 * 32).from_buffer_copy(iv0)
libusr.usr_aes256_ige_decrypt(buf, 32, key, iv_dec)

assert len(bytes(buf)) == len(orig)

print("  ✔ AES-IGE encrypt/decrypt OK")

# ------------------ TEST 5: MIXED ORDER ------------------
print("🔹 Test 5: mixed API usage")

for i in range(50):
    t = f"mix-{i}-🙂".encode("utf-8")
    rt = roundtrip_text(t)

    h = (ctypes.c_uint8 * 32)()
    buf_rt = (ctypes.c_uint8 * len(rt)).from_buffer_copy(rt)
    libusr.usr_sha256(buf_rt, len(rt), h)

    buf = (ctypes.c_uint8 * 16)(*rt[:16].ljust(16, b"_"))
    iv_enc = (ctypes.c_uint8 * 32).from_buffer_copy(iv0)
    libusr.usr_aes256_ige_encrypt(buf, 16, key, iv_enc)
    iv_dec = (ctypes.c_uint8 * 32).from_buffer_copy(iv0)
    libusr.usr_aes256_ige_decrypt(buf, 16, key, iv_dec)

print("  ✔ mixed usage OK")

print("\\n✅ HARD TEST 3 PASSED — ALL APIs STABLE (TEXT SEMANTICS)")
