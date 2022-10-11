#!/usr/bin/env python3
#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

from sys import version
from unittest import TestCase, main
from pathlib import Path
from subprocess import Popen, PIPE
from datetime import date

p_script_dir = Path(__file__).absolute().parents[0]
p_root = p_script_dir.parents[1]
p_VERSION = Path(p_root, "zcbor", "VERSION")
p_RELEASE_NOTES = Path(p_root, "RELEASE_NOTES.md")
p_HEAD_REF = Path(p_script_dir, "HEAD_REF")


class ReleaseTest(TestCase):
    def test_version_num(self):
        current_branch = Popen(['git', 'branch', '--show-current'],
                               stdout=PIPE).communicate()[0].decode("utf-8").strip()
        if not current_branch:
            current_branch = p_HEAD_REF.read_text()
        self.assertRegex(
            current_branch, r"release/\d+\.\d+\.\d+",
            "Release tests must be run on a release branch on the form 'release/x.y.z'.")

        version_number = current_branch.replace("release/", "")
        self.assertRegex(
            version_number, r'\d+\.\d+\.(?!99)\d+',
            "Releases cannot have the x.y.99 development bugfix release number.")
        self.assertEqual(
            version_number, p_VERSION.read_text(),
            f"{p_VERSION} has not been updated to the correct version number.")
        self.assertEqual(
            p_RELEASE_NOTES.read_text().splitlines()[0],
            r"# zcbor v. " + version_number + f" ({date.today():%Y-%m-%d})",
            f"{p_RELEASE_NOTES} has not been updated with the correct version number.")

        tags_stdout, _ = Popen(['git', 'tag'], stdout=PIPE).communicate()
        tags = tags_stdout.decode("utf-8").strip().splitlines()
        self.assertNotIn(
            version_number, tags,
            "Version number already exists as a tag. Has the version number been updated?")


if __name__ == "__main__":
    main()
