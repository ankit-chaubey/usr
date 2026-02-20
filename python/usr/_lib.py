"""
usr._lib — ctypes loader for libusr.so
"""
import ctypes
import ctypes.util
import os
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
_CANDIDATES = [
    os.path.join(_HERE, "libusr.so"),
    os.path.join(_HERE, "libusr.dylib"),
    os.path.join(_HERE, "..", "..", "build", "libusr.so"),
    os.path.join(_HERE, "..", "..", "build", "libusr.dylib"),
]

_lib_path = None
for _c in _CANDIDATES:
    if os.path.exists(_c):
        _lib_path = _c
        break

if _lib_path is None:
    _found = ctypes.util.find_library("usr")
    if _found:
        _lib_path = _found

if _lib_path is None:
    raise ImportError(
        "usr: could not find libusr.so/dylib.\n"
        "Build with: gcc -shared -fPIC -Iinclude -Isrc/crypto src/**/*.c -o python/usr/libusr.so"
    )

lib = ctypes.CDLL(_lib_path)

if sys.platform == "darwin":
    libc = ctypes.CDLL("libc.dylib", use_errno=True)
else:
    libc = ctypes.CDLL("libc.so.6", use_errno=True)

libc.free.argtypes = [ctypes.c_void_p]
libc.free.restype  = None

__all__ = ["lib", "libc"]
