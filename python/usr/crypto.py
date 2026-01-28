from ._lib import lib
import ctypes

import sys
import warnings

_IS_ANDROID = (
    sys.platform.startswith("linux")
    and "android" in sys.platform.lower()
)

if _IS_ANDROID:
    warnings.warn(
        "AES-IGE via ctypes is not supported on Android / Termux. "
        "Calling AES functions may abort the process.",
        RuntimeWarning,
        stacklevel=2,
    )

# ---------------- SHA256 ----------------

lib.usr_sha256.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_uint8),
]
lib.usr_sha256.restype = None

def sha256(data: bytes) -> bytes:
    buf = (ctypes.c_uint8 * len(data)).from_buffer_copy(data)
    out = (ctypes.c_uint8 * 32)()
    lib.usr_sha256(buf, len(data), out)
    return bytes(out)

# ---------------- AES-256-IGE ----------------

lib.usr_aes256_ige_encrypt.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.POINTER(ctypes.c_uint8),
]
lib.usr_aes256_ige_encrypt.restype = None

lib.usr_aes256_ige_decrypt.argtypes = lib.usr_aes256_ige_encrypt.argtypes
lib.usr_aes256_ige_decrypt.restype = None

def aes256_ige_encrypt(data: bytes, key: bytes, iv: bytes) -> bytes:
    if len(key) != 32 or len(iv) != 32:
        raise ValueError("key and iv must be 32 bytes")

    buf = (ctypes.c_uint8 * len(data)).from_buffer_copy(data)
    key_buf = (ctypes.c_uint8 * 32).from_buffer_copy(key)
    iv_buf  = (ctypes.c_uint8 * 32).from_buffer_copy(iv)

    lib.usr_aes256_ige_encrypt(buf, len(data), key_buf, iv_buf)
    return bytes(buf)

def aes256_ige_decrypt(data: bytes, key: bytes, iv: bytes) -> bytes:
    if len(key) != 32 or len(iv) != 32:
        raise ValueError("key and iv must be 32 bytes")

    buf = (ctypes.c_uint8 * len(data)).from_buffer_copy(data)
    key_buf = (ctypes.c_uint8 * 32).from_buffer_copy(key)
    iv_buf  = (ctypes.c_uint8 * 32).from_buffer_copy(iv)

    lib.usr_aes256_ige_decrypt(buf, len(data), key_buf, iv_buf)
    return bytes(buf)
