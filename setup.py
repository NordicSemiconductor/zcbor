#!/usr/bin/env python3
#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

import setuptools
from pathlib import Path
from re import sub


P_REPO_ROOT = Path(__file__).absolute().parents[0]
ZCBOR_URL = 'https://github.com/NordicSemiconductor/zcbor'


def absolute_links(text):
    def new_link(match):
        match_text = match.group(0)
        path = Path(P_REPO_ROOT, match_text)
        try:
            path_exists = path.exists()
        except OSError:
            path_exists = False
        if path_exists:
            path_comp = "tree" if path.is_dir() else "blob"
            url_prefix = f"{ZCBOR_URL}/{path_comp}/{get_version()}/"
            return url_prefix + match_text
        else:
            return match_text
    new_text = sub(r"(?<=\]\().*?(?=\))", new_link, text)
    return new_text


def get_description(title_only=False):
    """Use the README as the long description that appears on PyPI."""
    p_readme = Path(P_REPO_ROOT, 'README.md').absolute()

    if p_readme.is_file():
        with p_readme.open(encoding='utf-8') as f:
            return f.readline().strip() if title_only else absolute_links(f.read())
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
    p_version = Path(P_REPO_ROOT, 'zcbor', 'VERSION').absolute()
    return p_version.read_text(encoding='utf-8').strip()


setuptools.setup(
    name='zcbor',
    version=get_version(),
    description=get_description(title_only=True),
    long_description=get_description(),
    long_description_content_type='text/markdown',
    url=ZCBOR_URL,
    author='Nordic Semiconductor ASA',
    license='Apache Software License',
    classifiers=[
        "Development Status :: 4 - Beta",
        "License :: OSI Approved :: Apache Software License",
        "Programming Language :: Python :: 3",
        "Topic :: Software Development :: Build Tools",
    ],
    python_requires='>=3.7',
    packages=setuptools.find_packages(),
    install_requires=get_dependencies(),
    entry_points={
        'console_scripts': ['zcbor=zcbor.zcbor:main'],
    },
    include_package_data=True,
    package_data={'': ['VERSION', 'cddl/prelude.cddl']},
    data_files=[
        ("lib/zcbor/include",
            ("include/zcbor_decode.h", "include/zcbor_encode.h", "include/zcbor_common.h",
                "include/zcbor_tags.h", "include/zcbor_debug.h")),
        ("lib/zcbor/src", ("src/zcbor_decode.c", "src/zcbor_encode.c", "src/zcbor_common.c"))],
)
