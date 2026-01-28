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

The goal is **understanding systems internals**, not shipping a black-box dependency.

---

## 🚧 Current Status

* ✔ Core C library: **stable for experimentation**
* ✔ Extensive internal tests (round‑trip, fuzz, stress)
* ✔ Python wrapper available (experimental)
* ❌ **Not production‑ready**
* ❌ **Android / Termux has known limitations** (documented)

Current version: **v0.1.1 (pre‑release)**

---

## 📦 Installation

### Supported environments

* Linux (native C / Python)
* macOS (native C / Python)
* Android / Termux (**C‑only recommended**)

### Python (experimental)

```bash
pip install usr
```

> The wheel bundles a native `libusr.so` for convenience.

### From source (C library)

```bash
git clone https://github.com/ankit-chaubey/usr
cd usr
mkdir build && cd build
cmake ..
make
```

---

## 🧪 Example Usage (Python)

Below are **basic, safe examples** intended for learning and experimentation.

### 1️⃣ Text ⇄ Binary

```python
from usr import from_text, to_text

raw = from_text("Hello usr 🚀")
print(raw)              # b"Hello usr ð"

text = to_text(raw)
print(text)             # Hello usr 🚀
```

This API demonstrates:

* UTF-8 safety
* Binary ownership handling
* Round-trip correctness

---

### 2️⃣ Hashing (SHA-256)

```python
from usr import sha256

digest = sha256(b"test")
print(digest.hex())
```

Use this to:

* Verify integrity
* Compare against Python's `hashlib`

---

### 3️⃣ AES-256-IGE (Experimental)

```python
from usr import aes256_ige_encrypt, aes256_ige_decrypt

key = b"" * 32
iv  = b""" * 32
msg = b"0123456789ABCDEF0123456789ABCDEF"

enc = aes256_ige_encrypt(msg, key, iv)
dec = aes256_ige_decrypt(enc, key, iv)

print(dec)
```

⚠️ **Important:**

* AES via `ctypes` may abort on **Android / Termux**
* Works correctly on Linux/macOS
* Prefer pure C or NDK/JNI on Android

---

## 🗂️ Project Structure

```text
usr/
├── src/                 # C core implementation
│   ├── binary/          # text ⇄ binary
│   ├── bytes/           # buffer utilities
│   ├── crypto/          # SHA256, AES‑256‑IGE
│   ├── entities/        # Telegram‑style entities
│   ├── html/            # HTML ⇄ entity
│   ├── markdown/        # Markdown parser
│   └── utf8/            # UTF‑8 utilities
│
├── include/usr/          # Public C headers
├── python/usr/           # Python bindings (ctypes)
├── tests/                # Round‑trip & fuzz tests
├── examples/             # Usage examples
└── CMakeLists.txt        # Build configuration
```

---

## 🧩 Available APIs

### 🔤 Binary / Text

* `from_text(str) -> bytes`
* `to_text(bytes) -> str`

### 🔐 Cryptography

* `sha256(data: bytes) -> bytes`
* `aes256_ige_encrypt(data, key, iv)` *(experimental)*
* `aes256_ige_decrypt(data, key, iv)` *(experimental)*

### 📝 Text Processing (C‑side)

* UTF‑8 decoding
* Markdown parsing (Telegram‑style)
* HTML ⇄ Entity conversion
* Entity normalization & round‑trip validation

---

## 📱 Android / Termux Note (Important)

On **Android (Termux)**:

* ❌ `ctypes` + native crypto may **abort** due to Bionic tagged‑pointer protection
* ✔ Binary/text APIs work in pure C
* ✔ SHA‑256 works internally

This is a **platform limitation**, not a logic bug.

**Recommended approaches on Android:**

* Use pure‑Python fallbacks
* Or integrate the C core via **NDK / JNI**

This is how mature projects responsibly handle Android.

---

## 🧪 Testing

This project includes multiple layers of testing designed to validate **correctness**, not formal security guarantees.

### Included tests

* **Round-trip tests**
  Validate Markdown → HTML → Entity → HTML stability

* **Fuzz tests**
  Randomized Telegram-style inputs with constrained grammar

* **Stress tests**
  Allocation, deallocation, and repeated API usage

### Running tests (C core)

```bash
cd build
ctest
```

### Running Python tests (manual)

```bash
python - << 'EOF'
from usr import from_text, to_text, sha256
print(to_text(from_text("test")))
print(sha256(b"test").hex())
EOF
```

Tests are intentionally **conservative** and focus on learning behavior rather than adversarial security.

---

## 👤 Author & Credits

**Author:** Ankit Chaubey
**GitHub:** [https://github.com/ankit-chaubey](https://github.com/ankit-chaubey)

This project is:

* Built with curiosity, patience, and care
* Not affiliated with Telegram or any organization

Special thanks to the open-source community and projects like **cryptg** and **tgcrypto** for architectural inspiration and reference material.

---

## 🔗 Repository

This repository is the single source of truth for development, issues, experiments, and documentation.

👉 [**https://github.com/ankit-chaubey/usr**](https://github.com/ankit-chaubey/usr)

This repository contains:

* Full source code
* Build scripts
* Tests & experiments
* Documentation and notes

---

## 🧭 Final Note

> This repository represents a **learning milestone**, not a finished product.
>
> It documents real engineering trade‑offs: ABI boundaries, memory ownership, platform limitations, and performance considerations.
>
> If you are reading this, you are looking at a **hands‑on exploration of systems programming**, not a polished framework.

Feedback, discussion, and curiosity are always welcome 🤝

> This repository represents a **learning milestone**, not a finished product.
>
> It is intentionally transparent about limitations and trade-offs. If you are reading this, you are looking at a hands-on exploration of systems engineering.

Feedback, discussion, and curiosity are always welcome 🤝
