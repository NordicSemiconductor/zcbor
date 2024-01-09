#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

from subprocess import Popen, PIPE
from os import linesep
from re import sub, S
from pathlib import Path
from sys import argv

p_root = Path(__file__).absolute().parents[1]
p_README = Path(p_root, 'README.md')

pattern = r"""
Command line documentation
==========================

Added via `add_helptext.py`
"""

if __name__ == "__main__":
    commands = [
        ["zcbor", "--help"],
        ["zcbor", "code", "--help"],
        ["zcbor", "validate", "--help"],
        ["zcbor", "convert", "--help"],
    ]

    output = pattern

    for cmd in commands:
        stdout, _ = Popen(cmd, stdout=PIPE).communicate()
        assert b"options:" in stdout, f"Seems like something went wrong: {stdout.decode('utf-8')}"
        output += f"""
{" ".join(cmd)}
{"-" * len(" ".join(cmd))}

```
{stdout.decode('utf-8')}
```
"""

    with open(p_README, 'r', encoding="utf-8") as f:
        readme_contents = f.read()
    new_readme_contents = sub(pattern + r'.*', output, readme_contents, flags=S)
    if len(argv) > 1 and argv[1] == "--check":
        if new_readme_contents != readme_contents:
            print("Check failed")
            exit(9)
    else:
        with open(p_README, 'w', encoding="utf-8") as f:
            f.write(new_readme_contents)
