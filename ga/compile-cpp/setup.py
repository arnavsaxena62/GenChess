from pathlib import Path

from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

COMPILE_CPP_DIR = Path(__file__).resolve().parent

PROJECT_ROOT = COMPILE_CPP_DIR.parent.parent

ENGINE = PROJECT_ROOT / "engine"

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
    ),
]

setup(
    name="chessbots",
    version="0.1.0",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)