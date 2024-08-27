# -*- coding: utf-8 -*-
"""
setup.py

python-can-usr_canet
"""

import pathlib
from setuptools import setup

# The directory containing this file
HERE = pathlib.Path(__file__).parent

# The text of the README file
README = (HERE / "README.md").read_text()

setup(
    name="python-can-usr_canet",
    version="0.2.0",
    author="Viorel Stirbu",
    author_email="vio@agorarobotics.com",
    description="Python-can USR-CANET200",
    long_description=README,
    long_description_content_type="text/markdown",
    url="https://github.com/AgoraRobotics/python-can-usr_canet",
    py_modules = ["usr_canet"],
    python_requires=">=3.9",
    install_requires=[
        "python-can",
    ],
    entry_points = {
        'can.interface': [
            'usr_canet = usr_canet:UsrCanetBus'
        ],
    },
    scripts=["canet_vcan_fwd.py"],
)
