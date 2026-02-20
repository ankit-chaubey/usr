# usr — Universal Systems Runtime v0.1.3

> ⚠️ **Pre-release / Experimental**
>
> Built to explore low-level systems programming, cryptography, and Telegram text formatting internals.
> Not recommended for production use.

---

## ✨ What's New in v0.1.3

- **All cryptographic bugs fixed** — SHA-256 two-block padding, AES-256 decrypt fully implemented
- **Complete AES suite** — IGE, CBC (PKCS#7), CTR modes
- **SHA-512, HMAC-SHA256, PBKDF2** — full streaming + one-shot APIs
- **Base64, hex, URL, HTML** encoding/decoding
- **Secure random** via `getrandom()` / `/dev/urandom`
- **UTF-8/UTF-16 utilities** — decode, encode, validate, codepoint count, offset conversion
- **Complete Markdown parser + renderer** — V1 and V2, correct UTF-16 offsets
- **Complete HTML parser + renderer** — all Telegram HTML tags
- **Entity normalization** — proper nesting via interval-stack algorithm
- **Python bindings** — all APIs exposed via ctypes, zero external dependencies

---

## 📦 Installation

### Build from Source (Recommended)

```bash
git clone https://github.com/ankit-chaubey/usr
cd usr

# Build static library
mkdir -p build /tmp/objs
for f in $(find src -name '*.c'); do
  gcc -O2 -Iinclude -Isrc/crypto -c "$f" -o /tmp/objs/$(basename ${f%.c}).o
done
ar rcs build/libusr.a /tmp/objs/*.o

# Run tests
gcc -O2 -Iinclude tests/test_crypto.c    build/libusr.a -o build/test_crypto
gcc -O2 -Iinclude tests/test_encoding.c  build/libusr.a -o build/test_encoding
gcc -O2 -Iinclude tests/test_utf8.c      build/libusr.a -o build/test_utf8
gcc -O2 -Iinclude tests/test_roundtrip.c build/libusr.a -o build/test_roundtrip
gcc -O2 -Iinclude tests/fuzz_roundtrip.c build/libusr.a -o build/fuzz_roundtrip
build/test_crypto && build/test_encoding && build/test_utf8
build/test_roundtrip && build/fuzz_roundtrip
```

### Python Bindings

```bash
# Build shared library for Python
SRCS=$(find src -name '*.c' | tr '\n' ' ')
gcc -O2 -shared -fPIC -Iinclude -Isrc/crypto $SRCS -o python/usr/libusr.so

cd python
pip install -e .
```

---

## 🧪 Example Usage

### C

```c
#include "usr/usr.h"

// SHA-256
uint8_t digest[32];
usr_sha256((uint8_t*)"hello", 5, digest);

// AES-256-IGE (Telegram MTProto)
uint8_t key[32] = {0x11};  // fill properly
uint8_t iv[32]  = {0x22};
usr_aes256_ige_encrypt(data, 32, key, iv);

// Markdown parse
usr_entity ents[64]; char *plain;
size_t n = usr_markdown_parse("*bold* _italic_", USR_MD_V2, &plain, ents, 64);
char *html = usr_entities_to_html(plain, ents, n);
```

### Python

```python
import usr

# Hashing
print(usr.sha256(b"test").hex())
print(usr.hmac_sha256(b"key", b"message").hex())

# AES-256-IGE (Telegram)
key = bytes(32); iv = bytes(32)
enc = usr.aes256_ige_encrypt(b"\x00" * 16, key, iv)
dec = usr.aes256_ige_decrypt(enc, key, iv)

# AES-256-CBC
enc = usr.aes256_cbc_encrypt(b"secret", b"\x11" * 32, b"\x22" * 16)
dec = usr.aes256_cbc_decrypt(enc, b"\x11" * 32, b"\x22" * 16)

# Markdown
plain, ents = usr.markdown_parse("*bold* _italic_")
html = usr.entities_to_html(plain, ents)     # <b>bold</b> <i>italic</i>
md   = usr.entities_to_markdown(plain, ents)  # *bold* _italic_

# Encoding
usr.base64_encode(b"hello")    # 'aGVsbG8='
usr.hex_encode(b"\xde\xad")    # 'dead'
usr.url_encode("hello world")  # 'hello%20world'
```

---

## 📐 API Reference

### Cryptography (`usr/crypto.h`)

| Function | Description |
|---|---|
| `usr_sha256(data, len, out)` | One-shot SHA-256 |
| `usr_sha512(data, len, out)` | One-shot SHA-512 |
| `usr_hmac_sha256(key, klen, data, dlen, out)` | HMAC-SHA256 |
| `usr_pbkdf2_sha256(pass, plen, salt, slen, iters, out, olen)` | PBKDF2-HMAC-SHA256 |
| `usr_aes256_ige_encrypt(data, len, key, iv)` | AES-256-IGE (in-place) |
| `usr_aes256_ige_decrypt(data, len, key, iv)` | AES-256-IGE decrypt |
| `usr_aes256_cbc_encrypt(in, ilen, key, iv, out, olen)` | AES-256-CBC + PKCS#7 |
| `usr_aes256_cbc_decrypt(in, ilen, key, iv, out, olen)` | AES-256-CBC + unpad |
| `usr_aes256_ctr_crypt(data, len, key, nonce)` | AES-256-CTR (symmetric) |
| `usr_crc32(data, len)` | CRC-32 (IEEE 802.3) |
| `usr_rand_bytes(out, len)` | Cryptographically secure random |

### Encoding (`usr/encoding.h`)

| Function | Description |
|---|---|
| `usr_base64_encode(data, len, out)` | Standard Base64 |
| `usr_base64url_encode(data, len, out)` | URL-safe Base64 |
| `usr_base64_decode(s, slen, out)` | Decode Base64 |
| `usr_hex_encode(data, len, out)` | Lowercase hex |
| `usr_hex_decode(s, slen, out)` | Hex → bytes |
| `usr_url_encode(s, slen, out)` | RFC 3986 URL encoding |
| `usr_url_decode(s, slen, out)` | URL decode |
| `usr_html_escape(s, slen, out)` | Escape `<>&"'` |
| `usr_html_unescape(s, slen, out)` | Unescape `&amp;` etc. |

### Text / Entities

| Function | Description |
|---|---|
| `usr_markdown_parse(text, version, plain_out, ents, max)` | Markdown → entities |
| `usr_entities_to_markdown(text, ents, n, version)` | Entities → Markdown |
| `usr_html_parse(html, plain_out, ents, max)` | HTML → entities |
| `usr_entities_to_html(text, ents, n)` | Entities → HTML |
| `usr_entities_normalize(ents, n)` | Sort + fix overlaps |

---

## 🗂️ Project Structure

```
usr/
├── include/usr/        # Public headers
│   ├── usr.h           # Umbrella include
│   ├── crypto.h        # SHA-256/512, AES, HMAC, PBKDF2, CRC-32
│   ├── encoding.h      # Base64, hex, URL, HTML
│   ├── entities.h      # MessageEntity types
│   ├── html.h          # HTML ↔ entities
│   ├── markdown.h      # Markdown ↔ entities
│   ├── utf8.h          # UTF-8/16 utilities
│   ├── bytes.h         # Owned byte buffer
│   ├── strbuilder.h    # String builder
│   └── rand.h          # Secure random
├── src/
│   ├── crypto/         # AES, SHA, HMAC, CRC, rand
│   ├── encoding/       # Base64, hex, URL, HTML escaping
│   ├── entities/       # Entity normalization
│   ├── html/           # HTML parser & renderer
│   ├── markdown/       # Markdown parser & renderer
│   └── utf8/           # UTF-8 codec
├── python/usr/         # Python ctypes bindings
│   ├── _lib.py         # Library loader
│   ├── _structs.py     # ctypes structure definitions
│   ├── crypto.py       # Crypto bindings
│   ├── encoding.py     # Encoding bindings
│   ├── entities.py     # Entity class + normalize
│   ├── html.py         # HTML parse/render
│   └── markdown.py     # Markdown parse/render
├── tests/              # C test suite + fuzz
├── examples/full_demo.c
└── benchmarks/bench_crypto.c
```

---

## 👤 Author

**Ankit Chaubey** · [github.com/ankit-chaubey](https://github.com/ankit-chaubey)

Inspired by `cryptg`, `tgcrypto`, and Telegram MTProto internals.

---

## 📄 License

MIT
