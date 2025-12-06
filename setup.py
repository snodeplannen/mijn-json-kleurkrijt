from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir
import pybind11
from setuptools import setup
import os

ext_modules = [
    Pybind11Extension(
        "colored_json",
        [
            "src/bindings.cpp",
        ],
        include_dirs=[
            "src",
            pybind11.get_include(),
        ],
        cxx_std=17,
        define_macros=[
            ("VERSION_INFO", "1.0.0"),
        ],
        extra_compile_args=[
            "-O3", "-march=native", "-ffast-math", "-DNDEBUG"
        ] if os.name != "nt" else [
            "/O2", "/Ob2", "/GL", "/Gy", "/DNDEBUG"
        ],
    ),
]

setup(
    name="colored_json",
    version="1.0.0",
    description="Razendsnelle gekleurde JSON/dict printer",
    long_description="C++ module voor gekleurde weergave van Python dicts met preset thema's",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
    package_data={"": ["*.pyi"]},
    include_package_data=True,
)

