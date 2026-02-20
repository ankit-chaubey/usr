from setuptools import setup, find_packages
from pathlib import Path

long_description = (Path(__file__).parent.parent / "README.md").read_text(encoding="utf-8")

setup(
    name="usr",
    version="0.1.3",
    description="Universal Systems Runtime — crypto, encoding, Telegram HTML/Markdown",
    long_description=long_description,
    long_description_content_type="text/markdown",
    author="usr contributors",
    python_requires=">=3.8",
    packages=find_packages(),
    package_data={"usr": ["*.so", "*.dylib", "*.dll"]},
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
        "Topic :: Security :: Cryptography",
        "Topic :: Text Processing :: Markup :: HTML",
    ],
)
