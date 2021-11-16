#!/usr/bin/env python3
#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

import setuptools
from pathlib import Path


P_REPO_ROOT = Path(__file__).absolute().parents[0]


def get_description(title_only=False):
    """Use the README as the long description that appears on PyPI."""
    p_readme = Path(P_REPO_ROOT, 'README.md').absolute()

    if p_readme.is_file():
        with p_readme.open(encoding='utf-8') as f:
            return f.readline().strip() if title_only else f.read()
    return ''


def get_dependencies():
    """Extract base requirement packages from file."""
    p_base_reqs = Path(
        P_REPO_ROOT, 'scripts', 'requirements-base.txt').absolute()

    l_dependencies = list()
    if p_base_reqs.is_file():
        with p_base_reqs.open(encoding='utf-8') as f:
            for line in f.readlines():
                l_dependencies.append(line.strip())
    return l_dependencies


def get_version():
    p_version = Path(P_REPO_ROOT, 'cddl_gen', 'VERSION').absolute()
    return p_version.read_text(encoding='utf-8').strip()


setuptools.setup(
    name='cddl-gen',
    version=get_version(),
    description=get_description(title_only=True),
    long_description=get_description(),
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
    install_requires=get_dependencies(),
    entry_points={
        'console_scripts': ['cddl-gen=cddl_gen.cddl_gen:main'],
    },
    include_package_data=True,
    package_data={'': ['VERSION']},
    data_files=[
        ("lib/cddl-gen/include",
            ("include/cbor_decode.h", "include/cbor_encode.h", "include/cbor_common.h",
                "include/cbor_debug.h")),
        ("lib/cddl-gen/src", ("src/cbor_decode.c", "src/cbor_encode.c", "src/cbor_common.c"))],
)
