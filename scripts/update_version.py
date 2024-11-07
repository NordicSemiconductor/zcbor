#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

from sys import argv
from pathlib import Path
from re import sub, match, S
from datetime import datetime

p_root = Path(__file__).absolute().parents[1]
p_VERSION = Path(p_root, 'zcbor', 'VERSION')
p_RELEASE_NOTES = Path(p_root, 'RELEASE_NOTES.md')
p_MIGRATION_GUIDE = Path(p_root, 'MIGRATION_GUIDE.md')
p_common_h = Path(p_root, 'include', 'zcbor_common.h')

RELEASE_NOTES_boilerplate = """
Any new bugs, requests, or missing features should be reported as [Github issues](https://github.com/NordicSemiconductor/zcbor/issues).

## Improvements:

## Bugfixes:
"""


def update_relnotes(p_relnotes, version, boilerplate="", include_date=True):
    relnotes_contents = p_relnotes.read_text(encoding="utf-8")
    relnotes_lines = relnotes_contents.splitlines()
    if version not in relnotes_lines[0]:
        new_date = f" ({datetime.today().strftime('%Y-%m-%d')})" if include_date else ""
        relnotes_new_header = f"# zcbor v. {version}{new_date}\n"
        if ".99" not in relnotes_lines[0]:
            relnotes_contents = relnotes_new_header + boilerplate + '\n\n' + relnotes_contents
        relnotes_contents = sub(r".*?\n", relnotes_new_header, relnotes_contents, count=1)
        p_relnotes.write_text(relnotes_contents, encoding="utf-8")


if __name__ == "__main__":
    if len(argv) != 2 or match(r'\d+\.\d+\.\d+', argv[1]) is None:
        print(f"Usage: {argv[0]} <new zcbor version>")
        exit(1)
    version = argv[1]
    (major, minor, bugfix) = version.split('.')

    p_VERSION.write_text(version, encoding="utf-8")
    update_relnotes(p_RELEASE_NOTES, version, boilerplate=RELEASE_NOTES_boilerplate)
    update_relnotes(p_MIGRATION_GUIDE, version, include_date=False)
    p_common_h_contents = p_common_h.read_text(encoding="utf-8")
    common_h_new_contents = sub(r"(#define ZCBOR_VERSION_MAJOR )\d+", f"\\g<1>{major}", p_common_h_contents)
    common_h_new_contents = sub(r"(#define ZCBOR_VERSION_MINOR )\d+", f"\\g<1>{minor}", common_h_new_contents)
    common_h_new_contents = sub(r"(#define ZCBOR_VERSION_BUGFIX )\d+", f"\\g<1>{bugfix}", common_h_new_contents)
    p_common_h.write_text(common_h_new_contents, encoding="utf-8")
