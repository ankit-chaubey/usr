"""
python/tests/test_python.py
Standalone integration test for usr Python bindings.
Run without pytest: python tests/test_python.py
Run with pytest:    pytest tests/ -v
"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import hashlib
import hmac as _hmac

import usr

pass_count = 0
fail_count = 0

def check(name, got, expected=True):
    global pass_count, fail_count
    ok = (got == expected) if expected is not True else bool(got)
    if ok:
        print(f"  \u2705 {name}")
        pass_count += 1
    else:
        print(f"  \u274c {name}")
        print(f"       got:      {got!r}")
        if expected is not True:
            print(f"       expected: {expected!r}")
        fail_count += 1

def section(name):
    print(f"\n\u2500\u2500 {name} \u2500\u2500")

# SHA-256 / SHA-512
section("SHA-256 / SHA-512")
check("sha256(abc)",    usr.sha256(b"abc"),  hashlib.sha256(b"abc").digest())
check("sha256(empty)",  usr.sha256(b""),     hashlib.sha256(b"").digest())
check("sha512(abc)",    usr.sha512(b"abc"),  hashlib.sha512(b"abc").digest())

# HMAC
section("HMAC-SHA256")
key = b"\x0b" * 20; data = b"Hi There"
check("hmac RFC4231 #1",
      usr.hmac_sha256(key, data),
      _hmac.new(key, data, "sha256").digest())

# PBKDF2
section("PBKDF2-HMAC-SHA256")
dk = usr.pbkdf2_sha256(b"password", b"salt", 1, 32)
check("length 32", len(dk), 32)
check("value", dk, hashlib.pbkdf2_hmac("sha256", b"password", b"salt", 1, 32))

# AES-256-IGE
section("AES-256-IGE")
k32 = b"\x11"*32; iv32 = b"\x22"*32; d16 = b"A"*16
enc = usr.aes256_ige_encrypt(d16, k32, iv32)
check("ige roundtrip", usr.aes256_ige_decrypt(enc, k32, iv32), d16)
check("ige != plaintext", enc != d16)
try:
    usr.aes256_ige_encrypt(b"short", k32, iv32)
    check("ige rejects non-block size", False, True)
except ValueError:
    check("ige rejects non-block size", True)

# AES-256-CBC
section("AES-256-CBC")
k32 = b"\x33"*32; iv16 = b"\x44"*16; msg = b"Hello, AES-CBC!"
enc = usr.aes256_cbc_encrypt(msg, k32, iv16)
check("cbc roundtrip", usr.aes256_cbc_decrypt(enc, k32, iv16), msg)
check("cbc output is block-aligned", len(enc) % 16 == 0)

# AES-256-CTR
section("AES-256-CTR")
k32 = b"\x55"*32; nonce = b"\x00"*16; msg = b"CTR mode test!!"
ct = usr.aes256_ctr_crypt(msg, k32, nonce)
check("ctr symmetric", usr.aes256_ctr_crypt(ct, k32, nonce), msg)

# CRC-32
section("CRC-32")
check("crc32(123456789)", usr.crc32(b"123456789"), 0xCBF43926)

# Base64
section("Base64")
check("encode foobar",   usr.base64_encode(b"foobar"),  "Zm9vYmFy")
check("encode f",        usr.base64_encode(b"f"),       "Zg==")
check("decode Zm9vYmFy", usr.base64_decode("Zm9vYmFy"), b"foobar")
check("decode Zg==",     usr.base64_decode("Zg=="),     b"f")
raw = bytes([0xFB, 0xFF, 0xFE])
check("base64url encode", usr.base64url_encode(raw), "-__-")
check("base64url decode", usr.base64url_decode("-__-"), raw)

# Hex
section("Hex")
check("hex lower",  usr.hex_encode(b"\xde\xad"),       "dead")
check("hex upper",  usr.hex_encode(b"\xde\xad", True), "DEAD")
check("hex decode", usr.hex_decode("dead"),             b"\xde\xad")

# URL
section("URL Encoding")
check("url_encode", usr.url_encode("hello world"), "hello%20world")
check("url_decode", usr.url_decode("hello%20world"), "hello world")

# HTML
section("HTML Escape")
check("html_escape",   usr.html_escape("<b>X & Y</b>"),          "&lt;b&gt;X &amp; Y&lt;/b&gt;")
check("html_unescape", usr.html_unescape("&lt;b&gt;X &amp; Y&lt;/b&gt;"), "<b>X & Y</b>")

# Binary
section("Binary from_text / to_text")
for s in ["Hello", "Hello \U0001F642", "\u4e2d\u6587", ""]:
    check(f"roundtrip {s!r}", usr.to_text(usr.from_text(s)), s)

# Markdown
section("Markdown parse / render")
plain, ents = usr.markdown_parse("*bold* _italic_ ~strike~ `code`")
check("plain",         plain, "bold italic strike code")
check("count",         len(ents), 4)
check("bold type",     ents[0].type, usr.EntityType.BOLD)
html = usr.entities_to_html(plain, ents)
check("html has <b>",     "<b>" in html)
check("html has <i>",     "<i>" in html)
check("html has <s>",     "<s>" in html)
check("html has <code>",  "<code>" in html)
md_out = usr.entities_to_markdown(plain, ents)
plain2, ents2 = usr.markdown_parse(md_out)
check("md roundtrip plain", plain2, plain)
check("md roundtrip count", len(ents2), len(ents))

# HTML parse
section("HTML parse")
plain_h, ents_h = usr.html_parse('<b>bold</b> <i>italic</i> <a href="https://x.com">link</a>')
check("plain",        plain_h, "bold italic link")
check("bold type",    ents_h[0].type, usr.EntityType.BOLD)
check("italic type",  ents_h[1].type, usr.EntityType.ITALIC)
check("link type",    ents_h[2].type, usr.EntityType.TEXT_LINK)
check("link extra",   ents_h[2].extra, "https://x.com")

# Normalize
section("Entity normalize")
raw_e = [
    usr.Entity(usr.EntityType.BOLD, 0, 5),
    usr.Entity(usr.EntityType.ITALIC, 2, 3),  # nested inside bold — kept
    usr.Entity(usr.EntityType.ITALIC, 0, 0),  # zero-length — dropped
]
normed = usr.normalize_entities(raw_e)
check("zero-length dropped", len(normed), 2)
check("bold kept",   normed[0].type, usr.EntityType.BOLD)
check("italic kept", normed[1].type, usr.EntityType.ITALIC)

# Random
section("Secure random")
r1, r2 = usr.random_bytes(32), usr.random_bytes(32)
check("length",        len(r1), 32)
check("not all zeros", r1 != b"\x00"*32)
check("unique",        r1 != r2)

# Summary
total = pass_count + fail_count
print(f"\n{'='*40}")
print(f"Results: {pass_count}/{total} passed, {fail_count} failed")
if fail_count > 0:
    sys.exit(1)
else:
    print("All tests passed \u2705")
