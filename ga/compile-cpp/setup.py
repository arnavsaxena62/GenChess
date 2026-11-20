from pathlib import Path
import os

from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

COMPILE_CPP_DIR = Path(__file__).resolve().parent

PROJECT_ROOT = COMPILE_CPP_DIR.parent.parent

ENGINE = PROJECT_ROOT / "engine"
MSVC_COMPAT_HEADER = COMPILE_CPP_DIR / "msvc_compat.h"

sources = [
    str(COMPILE_CPP_DIR / "python_bindings.cpp"),

    str(ENGINE / "bot" / "evaluation.cpp"),
    str(ENGINE / "src" / "movegen.cpp"),
    str(ENGINE / "src" / "utils.cpp"),
]

ext_modules = [
    Pybind11Extension(
        "chessbots",
        sources,
        include_dirs=[
            str(ENGINE),               
            str(ENGINE / "bot"),       
            str(ENGINE / "include"),   
            str(ENGINE / "src"),
        ],
        cxx_std=17,
        # evaluation.cpp uses GCC bit-operation builtins. Force a compatibility
        # header only when the extension is compiled with Microsoft Visual C++.
        extra_compile_args=[f"/FI{MSVC_COMPAT_HEADER}"] if os.name == "nt" else [],
    ),
]

setup(
    name="chessbots",
    version="0.1.0",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)
