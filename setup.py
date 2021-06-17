#!/usr/bin/env python3
#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

import setuptools
from pathlib import Path


def get_long_description():
    """Use the README as the long description that appears on PyPI."""
    p_readme = Path(
        Path(__file__).absolute().parents[0], 'README.md').absolute()

    if p_readme.is_file():
        with p_readme.open(encoding='utf-8') as f:
            return f.read()
    return ''


setuptools.setup(
    name='cddl-gen',
    version='0.1.0',
    description='Generate code from CDDL description',
    long_description=get_long_description(),
    long_description_content_type='text/markdown',
    url='https://github.com/NordicSemiconductor/cddl-gen',
    author='Nordic Semiconductor ASA',
    license='Apache Software License',
    classifiers=[
        "Development Status :: 4 - Beta",
        "License :: OSI Approved :: Apache Software License",
        "Programming Language :: Python :: 3",
        "Topic :: Software Development :: Build Tools",
    ],
    python_requires='>=3.6',
    packages=setuptools.find_packages(),
    install_requires=[
        'cbor2>=5.4.0',
        'pyyaml~=5.4.1',
        'regex>=2020.11.13'
    ],
    entry_points={
        'console_scripts': [ 'cddl_gen=cddl_gen.cddl_gen:main' ],
    }
)
