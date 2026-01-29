#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from setuptools import setup
from setuptools.command.build_ext import build_ext
import subprocess
import os

HERE = os.path.abspath(os.path.dirname(__file__))
ROOT = os.path.abspath(os.path.join(HERE, ".."))
BUILD_DIR = os.path.join(ROOT, "build")


class CMakeBuild(build_ext):
    def run(self):
        os.makedirs(BUILD_DIR, exist_ok=True)

        subprocess.check_call([
            "cmake",
            "-S", ROOT,
            "-B", BUILD_DIR,
        ])

        subprocess.check_call([
            "cmake",
            "--build", BUILD_DIR,
        ])

        super().run()


setup(
    name="usr",
    version="0.1.2",

    author="Ankit Chaubey",
    author_email="m.ankitchaubey@gmail.com",

    description="Universal Systems Runtime — experimental C core with Python bindings",
    long_description=(
        "usr is an experimental systems runtime written in C with optional Python bindings. "
        "It is built as a curiosity-driven project to explore low-level systems concepts such as "
        "binary handling, cryptography, text processing, and ABI boundaries.\n\n"
        "This package is a pre-release intended for learning, experimentation, and architectural "
        "exploration. It is not recommended for production use."
    ),
    long_description_content_type="text/plain",

    url="https://github.com/ankit-chaubey/usr",

    packages=["usr"],
    package_dir={"": "."},

    include_package_data=True,
    package_data={
        "usr": ["_lib/*.so"],
    },

    cmdclass={
        "build_ext": CMakeBuild,
    },

    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Topic :: Software Development :: Libraries",
        "Topic :: System :: Operating System Kernels",
        "Programming Language :: Python :: 3",
        "Programming Language :: C",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
    ],

    python_requires=">=3.8",

    project_urls={
        "Source": "https://github.com/ankit-chaubey/usr",
        "Issues": "https://github.com/ankit-chaubey/usr/issues",
    },
)
