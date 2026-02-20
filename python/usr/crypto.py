"""usr.crypto — SHA-256/512, HMAC, PBKDF2, AES-256-IGE/CBC/CTR, CRC-32, secure random."""
from __future__ import annotations
import ctypes
from ._lib import lib, libc

def _buf(data: bytes) -> ctypes.Array:
    if not data:
        return (ctypes.c_uint8 * 1)()
    return (ctypes.c_uint8 * len(data)).from_buffer_copy(data)

def _check(v, name: str) -> bytes:
    return bytes(v) if isinstance(v, (bytes, bytearray, memoryview)) else (_ for _ in ()).throw(TypeError(f"{name} must be bytes"))

# SHA-256
lib.usr_sha256.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint8)]
lib.usr_sha256.restype = None
def sha256(data: bytes) -> bytes:
    data = bytes(data); out = (ctypes.c_uint8 * 32)()
    lib.usr_sha256(_buf(data), len(data), out); return bytes(out)

# SHA-512
lib.usr_sha512.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint8)]
lib.usr_sha512.restype = None
def sha512(data: bytes) -> bytes:
    data = bytes(data); out = (ctypes.c_uint8 * 64)()
    lib.usr_sha512(_buf(data), len(data), out); return bytes(out)

# HMAC-SHA256
lib.usr_hmac_sha256.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t,
                                  ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t,
                                  ctypes.POINTER(ctypes.c_uint8)]
lib.usr_hmac_sha256.restype = None
def hmac_sha256(key: bytes, data: bytes) -> bytes:
    key, data = bytes(key), bytes(data); out = (ctypes.c_uint8 * 32)()
    lib.usr_hmac_sha256(_buf(key), len(key), _buf(data), len(data), out); return bytes(out)

# PBKDF2
lib.usr_pbkdf2_sha256.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t,
                                    ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t,
                                    ctypes.c_uint32, ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t]
lib.usr_pbkdf2_sha256.restype = None
def pbkdf2_sha256(password: bytes, salt: bytes, iterations: int, length: int = 32) -> bytes:
    p, s = bytes(password), bytes(salt); out = (ctypes.c_uint8 * length)()
    lib.usr_pbkdf2_sha256(_buf(p), len(p), _buf(s), len(s), int(iterations), out, length)
    return bytes(out)

# AES-256-IGE
lib.usr_aes256_ige_encrypt.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t,
                                         ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8)]
lib.usr_aes256_ige_encrypt.restype  = ctypes.c_int
lib.usr_aes256_ige_decrypt.argtypes = lib.usr_aes256_ige_encrypt.argtypes
lib.usr_aes256_ige_decrypt.restype  = ctypes.c_int

def _ige(data, key, iv, enc):
    data, key, iv = bytes(data), bytes(key), bytes(iv)
    if len(key) != 32: raise ValueError("key must be 32 bytes")
    if len(iv) != 32: raise ValueError("IGE iv must be 32 bytes")
    if len(data) % 16: raise ValueError("data must be 16-byte aligned")
    buf = _buf(data)
    fn = lib.usr_aes256_ige_encrypt if enc else lib.usr_aes256_ige_decrypt
    if fn(buf, len(data), _buf(key), _buf(iv)) != 0: raise RuntimeError("AES-IGE failed")
    return bytes(buf)

def aes256_ige_encrypt(data: bytes, key: bytes, iv: bytes) -> bytes: return _ige(data, key, iv, True)
def aes256_ige_decrypt(data: bytes, key: bytes, iv: bytes) -> bytes: return _ige(data, key, iv, False)

# AES-256-CBC
lib.usr_aes256_cbc_encrypt.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t,
                                         ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8),
                                         ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_size_t)]
lib.usr_aes256_cbc_encrypt.restype  = ctypes.c_int
lib.usr_aes256_cbc_decrypt.argtypes = lib.usr_aes256_cbc_encrypt.argtypes
lib.usr_aes256_cbc_decrypt.restype  = ctypes.c_int

def aes256_cbc_encrypt(data: bytes, key: bytes, iv: bytes) -> bytes:
    data, key, iv = bytes(data), bytes(key), bytes(iv)
    if len(key) != 32: raise ValueError("key must be 32 bytes")
    if len(iv)  != 16: raise ValueError("iv must be 16 bytes")
    enc_len = len(data) + 16; out = (ctypes.c_uint8 * enc_len)(); ol = ctypes.c_size_t(enc_len)
    if lib.usr_aes256_cbc_encrypt(_buf(data), len(data), _buf(key), _buf(iv), out, ctypes.byref(ol)) != 0:
        raise RuntimeError("AES-CBC encrypt failed")
    return bytes(out[:ol.value])

def aes256_cbc_decrypt(data: bytes, key: bytes, iv: bytes) -> bytes:
    data, key, iv = bytes(data), bytes(key), bytes(iv)
    if len(key) != 32: raise ValueError("key must be 32 bytes")
    if len(iv)  != 16: raise ValueError("iv must be 16 bytes")
    out = (ctypes.c_uint8 * len(data))(); ol = ctypes.c_size_t(len(data))
    if lib.usr_aes256_cbc_decrypt(_buf(data), len(data), _buf(key), _buf(iv), out, ctypes.byref(ol)) != 0:
        raise RuntimeError("AES-CBC decrypt failed")
    return bytes(out[:ol.value])

# AES-256-CTR
lib.usr_aes256_ctr_crypt.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t,
                                       ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8)]
lib.usr_aes256_ctr_crypt.restype = None
def aes256_ctr_crypt(data: bytes, key: bytes, nonce: bytes) -> bytes:
    data, key, nonce = bytes(data), bytes(key), bytes(nonce)
    if len(key)   != 32: raise ValueError("key must be 32 bytes")
    if len(nonce) != 16: raise ValueError("nonce must be 16 bytes")
    buf = _buf(data); nb = _buf(nonce)
    lib.usr_aes256_ctr_crypt(buf, len(data), _buf(key), nb); return bytes(buf)

# CRC-32
lib.usr_crc32.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t]
lib.usr_crc32.restype  = ctypes.c_uint32
def crc32(data: bytes) -> int:
    data = bytes(data); return lib.usr_crc32(_buf(data), len(data))

# Secure random
lib.usr_rand_bytes.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t]
lib.usr_rand_bytes.restype  = ctypes.c_int
def random_bytes(n: int) -> bytes:
    buf = (ctypes.c_uint8 * n)()
    if lib.usr_rand_bytes(buf, n) != 0: raise RuntimeError("random_bytes failed")
    return bytes(buf)

__all__ = ["sha256","sha512","hmac_sha256","pbkdf2_sha256",
           "aes256_ige_encrypt","aes256_ige_decrypt",
           "aes256_cbc_encrypt","aes256_cbc_decrypt","aes256_ctr_crypt",
           "crc32","random_bytes"]
