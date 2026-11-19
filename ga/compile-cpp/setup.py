from pathlib import Path

from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

# This file is located at:
# chessbot/ga/compile-cpp/setup.py
COMPILE_CPP_DIR = Path(__file__).resolve().parent

# Go from compile-cpp -> ga -> chessbot
PROJECT_ROOT = COMPILE_CPP_DIR.parent.parent

ENGINE = PROJECT_ROOT / "engine"

sources = [
    # pybind11 module entry point
    str(COMPILE_CPP_DIR / "python_bindings.cpp"),

    # C++ engine sources
    str(ENGINE / "bot" / "evaluation.cpp"),
    str(ENGINE / "src" / "movegen.cpp"),
    str(ENGINE / "src" / "utils.cpp"),
]

ext_modules = [
    Pybind11Extension(
        "chessbots",
        sources,
        include_dirs=[
            str(ENGINE),               # Supports #include "include/position.h"
            str(ENGINE / "bot"),       # Supports #include "bot.h"
            str(ENGINE / "include"),   # Supports #include "position.h"
            str(ENGINE / "src"),
        ],
        cxx_std=17,
    ),
]

setup(
    name="chessbots",
    version="0.1.0",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)