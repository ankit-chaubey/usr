# usr — Universal Systems Runtime (pre‑release)

> ⚠️ **Pre‑release / Experimental**
>
> This project is an early, curiosity‑driven build created to explore low‑level systems programming, binary processing, and cryptography.
>
> **It is NOT recommended for production use.**  
> This is the **first real release after placeholders**, published mainly for learning, experimentation, and architectural exploration.

---

## ✨ Overview

**usr** (Universal Systems Runtime) is a low‑level systems library written in **C**, with optional **Python bindings**, designed to experiment with:

* Binary & byte manipulation
* UTF‑8 / UTF‑16 handling
* Cryptographic primitives (SHA‑256, AES‑256‑IGE)
* Telegram‑style text entities & formatting
* Markdown ⇄ HTML ⇄ Entity round‑tripping
* Performance‑oriented, minimal abstractions

This project exists to understand *how real systems libraries are built*, not to replace existing production‑grade tools.

---

## 🧠 Project Philosophy

* Written **from scratch** in C
* Clear separation between **core logic** and **language bindings**
* Focus on **determinism, memory ownership, and correctness**
* Minimal abstractions, explicit APIs
* Inspired by projects such as:

  * `cryptg`
  * `tgcrypto`
  * Telegram MTProto internals

The goal is **understanding systems internals**, not shipping a black‑box dependency.

---

## 🚧 Current Status

* ✔ Core C library: **stable for experimentation**
* ✔ Extensive internal tests (round‑trip, fuzz, stress)
* ✔ Python wrapper available (**experimental**)
* ❌ **Not production‑ready**
* ❌ **Android / Termux has known limitations**

---

## ⚠️ Packaging Status (Important)

The Python package is **not fully ready for universal PyPI usage yet**.

* Current releases are **experimental**
* Native components are expected to be **built locally**
* Prebuilt wheels may be incomplete or platform‑limited

For now, **usr is primarily intended to be built from source** for reliable results.

Full, cross‑platform PyPI support (Linux/macOS/Windows wheels) will be added in a future release once the build system is finalized.

---

## 📦 Installation

### Supported environments

* Linux (native C / Python)
* macOS (native C / Python)
* Android / Termux (**C‑only recommended**)

---

### ✅ Recommended (Current)

Build locally from source:

```bash
git clone https://github.com/ankit-chaubey/usr
cd usr
mkdir build && cd build
cmake ..
make
```

This is the **most reliable and supported way** to use `usr` at the moment.

---

### Python (Experimental)

```bash
pip install usr
```

> Python bindings are provided for exploration only.  
> Native components may still require local compilation depending on platform.

---

## 🧪 Example Usage (Python)

### 1️⃣ Text ⇄ Binary

```python
from usr import from_text, to_text

raw = from_text("Hello usr 🚀")
print(raw)

text = to_text(raw)
print(text)
```

Demonstrates:

* UTF‑8 safety
* Binary ownership handling
* Round‑trip correctness

---

### 2️⃣ Hashing (SHA‑256)

```python
from usr import sha256

print(sha256(b"test").hex())
```

---

### 3️⃣ AES‑256‑IGE (Experimental)

```python
from usr import aes256_ige_encrypt, aes256_ige_decrypt

key = b"\x11" * 32
iv  = b"\x22" * 32
msg = b"0123456789ABCDEF0123456789ABCDEF"

enc = aes256_ige_encrypt(msg, key, iv)
dec = aes256_ige_decrypt(enc, key, iv)
print(dec)
```

⚠️ Notes:

* May abort on Android / Termux via `ctypes`
* Works correctly on Linux/macOS
* Prefer pure C or NDK/JNI on Android

---

## 🗂️ Project Structure

```text
usr/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── examples/
│   └── full_demo.c
├── include/
│   └── usr/
│       ├── binary.h
│       ├── bytes.h
│       ├── crypto.h
│       ├── entities.h
│       ├── html.h
│       ├── markdown.h
│       ├── media.h
│       ├── usr.h
│       ├── utf8.h
│       └── version.h
├── python/
│   ├── MANIFEST.in
│   ├── README.md
│   ├── pyproject.toml
│   ├── setup.py
│   └── usr/
│       ├── __init__.py
│       ├── binary.py
│       ├── crypto.py
│       ├── entities.py
│       ├── html.py
│       └── markdown.py
├── src/
│   ├── binary/
│   │   └── binary.c
│   ├── bytes/
│   │   └── bytes.c
│   ├── crypto/
│   │   ├── aes_block.c
│   │   ├── aes_block_decrypt.c
│   │   ├── aes_ige.c
│   │   ├── aes_tables.c
│   │   ├── aes_tables.h
│   │   ├── crypto_stub.c
│   │   └── sha256.c
│   ├── entities/
│   │   ├── entities_stub.c
│   │   └── normalize.c
│   ├── html/
│   │   ├── entities_to_html.c
│   │   ├── entities_to_html_rt.c
│   │   ├── html.c
│   │   └── html_stub.c
│   ├── markdown/
│   │   ├── markdown.c
│   │   └── markdown_stub.c
│   ├── media/
│   │   └── media.c
│   └── utf8/
│       └── utf8.c
├── tests/
│   ├── entity_eq.h
│   ├── fuzz_roundtrip.c
│   └── test_roundtrip.c
├── tools/
│   ├── test1.py
│   ├── test2.py
│   └── test3_hard.py
└── wrappers/
```

---

## 🧩 Available APIs

### Binary / Text

* `from_text(str) -> bytes`
* `to_text(bytes) -> str`

### Cryptography

* `sha256(data: bytes) -> bytes`
* `aes256_ige_encrypt(data, key, iv)` *(experimental)*
* `aes256_ige_decrypt(data, key, iv)` *(experimental)*

### Text Processing (C‑side)

* UTF‑8 decoding
* Markdown parsing (Telegram‑style)
* HTML ⇄ Entity conversion
* Entity normalization & round‑trip validation

---

## 📱 Android / Termux Note

On Android (Termux):

* `ctypes` + native crypto may abort due to Bionic tagged‑pointer protection
* Binary/text APIs work in pure C
* SHA‑256 works internally

Recommended:

* Use pure‑Python fallbacks
* Or integrate the C core via NDK/JNI

---

## 🧪 Testing

```bash
cd build
ctest
```

Manual Python check:

```bash
python - << 'EOF'
from usr import from_text, to_text, sha256
print(to_text(from_text("test")))
print(sha256(b"test").hex())
EOF
```

---

## 👤 Author & Credits

**Author:** Ankit Chaubey  
**GitHub:** [https://github.com/ankit-chaubey](https://github.com/ankit-chaubey)

Inspired by open‑source systems projects such as `cryptg` and `tgcrypto`.

---

## 🔗 Repository

[https://github.com/ankit-chaubey/usr](https://github.com/ankit-chaubey/usr)

---

## 🧭 Final Note

This repository represents a **learning milestone**, not a finished product.

It documents real engineering trade‑offs: ABI boundaries, memory ownership, platform limitations, and performance considerations.

If you are reading this, you are looking at a **hands‑on exploration of systems programming**, not a polished framework.

Feedback, discussion, and curiosity are always welcome 🤝
