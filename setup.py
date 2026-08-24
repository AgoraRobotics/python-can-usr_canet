# -*- coding: utf-8 -*-
"""
setup.py

python-can-usr_canet
"""

import pathlib
import sys
import subprocess
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

# The directory containing this file
HERE = pathlib.Path(__file__).parent

# The text of the README file
README = (HERE / "README.md").read_text()

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = pathlib.Path(sourcedir).absolute()

class CMakeBuild(build_ext):
    def run(self):
        # Check if cmake is available
        try:
            subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the C++ extension")
        
        for ext in self.extensions:
            self.build_extension(ext)
    
    def build_extension(self, ext):
        extdir = pathlib.Path(self.get_ext_fullpath(ext.name)).parent.absolute()
        cmake_args = [
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}',
            f'-DPYTHON_EXECUTABLE={sys.executable}',
            '-DCMAKE_BUILD_TYPE=Release',
        ]
        
        build_args = ['--config', 'Release']
        
        # Build directory
        build_temp = pathlib.Path(self.build_temp) / ext.name
        build_temp.mkdir(parents=True, exist_ok=True)
        
        # Run cmake
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=build_temp)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=build_temp)

setup(
    name="python-can-usr_canet",
    version="0.3.1",  # Bump version for C++ support
    author="Viorel Stirbu",
    author_email="vio@agorarobotics.com",
    description="Python-can USR-CANET200 (with optional C++ acceleration)",
    long_description=README,
    long_description_content_type="text/markdown",
    url="https://github.com/AgoraRobotics/python-can-usr_canet",
    py_modules=["usr_canet", "usr_canet_cpp"],
    python_requires=">=3.9",
    install_requires=[
        "python-can",
    ],
    extras_require={
        'cpp': ['pybind11>=2.6.0'],
    },
    ext_modules=[CMakeExtension('_usr_canet_cpp', sourcedir='cpp')],
    cmdclass={'build_ext': CMakeBuild},
    entry_points={
        'can.interface': [
            'usr_canet = usr_canet:UsrCanetBus',
            'usr_canet_cpp = usr_canet_cpp:UsrCanetBusCpp',
        ],
    },
    scripts=["canet_vcan_fwd.py"],
)
