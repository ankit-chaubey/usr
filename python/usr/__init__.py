"""
usr — Universal Systems Runtime v0.1.3
Python bindings for the usr C library.
"""
from .crypto   import (sha256, sha512, hmac_sha256, pbkdf2_sha256,
                        aes256_ige_encrypt, aes256_ige_decrypt,
                        aes256_cbc_encrypt, aes256_cbc_decrypt,
                        aes256_ctr_crypt, crc32, random_bytes)
from .encoding import (base64_encode, base64_decode, base64url_encode, base64url_decode,
                        hex_encode, hex_decode, url_encode, url_decode,
                        html_escape, html_unescape)
from .binary   import from_text, to_text
from .entities import Entity, EntityType, normalize as normalize_entities
from .html     import html_parse, entities_to_html
from .markdown import markdown_parse, entities_to_markdown, MarkdownVersion

__version__ = "0.1.3"
__all__ = [
    "__version__",
    # crypto
    "sha256","sha512","hmac_sha256","pbkdf2_sha256",
    "aes256_ige_encrypt","aes256_ige_decrypt",
    "aes256_cbc_encrypt","aes256_cbc_decrypt","aes256_ctr_crypt",
    "crc32","random_bytes",
    # encoding
    "base64_encode","base64_decode","base64url_encode","base64url_decode",
    "hex_encode","hex_decode","url_encode","url_decode",
    "html_escape","html_unescape",
    # binary
    "from_text","to_text",
    # entities
    "Entity","EntityType","normalize_entities",
    # html
    "html_parse","entities_to_html",
    # markdown
    "markdown_parse","entities_to_markdown","MarkdownVersion",
]
