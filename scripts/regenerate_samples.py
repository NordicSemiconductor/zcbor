#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

from subprocess import run
from os import linesep, makedirs
from re import sub, S
from pathlib import Path
from sys import argv
from shutil import rmtree, copy2
from tempfile import mkdtemp

p_root = Path(__file__).absolute().parents[1]
p_build = p_root / 'build'
p_pet_sample = p_root / 'samples' / 'pet'
p_pet_cmake = p_pet_sample / 'pet.cmake'
p_pet_include = p_pet_sample / 'include'
p_pet_src = p_pet_sample / 'src'

def regenerate():
    tmpdir = Path(mkdtemp())
    p = run(['cmake', p_pet_sample, "-DREGENERATE_ZCBOR=Y", "-DCMAKE_MESSAGE_LOG_LEVEL=WARNING"], cwd=tmpdir)
    rmtree(tmpdir)

def check():
    files = (list(p_pet_include.iterdir()) + list(p_pet_src.iterdir()) + [p_pet_cmake])
    contents = "".join(p.read_text(encoding="utf-8") for p in files)
    tmpdir = Path(mkdtemp())
    list(makedirs(tmpdir / f.relative_to(p_pet_sample).parent, exist_ok=True) for f in files)
    list(copy2(f, tmpdir / f.relative_to(p_pet_sample)) for f in files)
    regenerate()
    new_contents = "".join(p.read_text(encoding="utf-8") for p in files)
    list(copy2(tmpdir / f.relative_to(p_pet_sample), f) for f in files)
    rmtree(tmpdir)
    return contents == new_contents

if __name__ == "__main__":
    if len(argv) > 1 and argv[1] == "--check":
        if not check():
            print("Check failed")
            exit(9)
    else:
        regenerate()
