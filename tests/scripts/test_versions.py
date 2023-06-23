#!/usr/bin/env python3
#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0

from sys import version
from unittest import TestCase, main
from pathlib import Path
from subprocess import Popen, PIPE
from datetime import date

p_script_dir = Path(__file__).absolute().parents[0]
p_root = p_script_dir.parents[1]
p_VERSION = p_root / "zcbor" / "VERSION"
p_release_notes = p_root / "RELEASE_NOTES.md"
p_HEAD_REF = p_script_dir / "HEAD_REF"


class VersionTest(TestCase):
    def test_version_num(self):
        """For release branches - Test that all version numbers have been updated."""
        current_branch = Popen(['git', 'branch', '--show-current'],
                               stdout=PIPE).communicate()[0].decode("utf-8").strip()
        if not current_branch:
            current_branch = p_HEAD_REF.read_text(encoding="utf-8")
        self.assertRegex(
            current_branch, r"release/\d+\.\d+\.\d+",
            "This test is meant to be run on a release branch on the form 'release/x.y.z'.")

        version_number = current_branch.replace("release/", "")
        self.assertRegex(
            version_number, r'\d+\.\d+\.(?!99)\d+',
            "Releases cannot have the x.y.99 development bugfix release number.")
        self.assertEqual(
            version_number, p_VERSION.read_text(encoding="utf-8"),
            f"{p_VERSION} has not been updated to the correct version number.")
        self.assertEqual(
            p_release_notes.read_text(encoding="utf-8").splitlines()[0],
            r"# zcbor v. " + version_number + f" ({date.today():%Y-%m-%d})",
            f"{p_release_notes} has not been updated with the correct version number.")

        tags_stdout, _ = Popen(['git', 'tag'], stdout=PIPE).communicate()
        tags = tags_stdout.decode("utf-8").strip().splitlines()
        self.assertNotIn(
            version_number, tags,
            "Version number already exists as a tag. Has the version number been updated?")


if __name__ == "__main__":
    main()
