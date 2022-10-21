#!/usr/bin/env python3
#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0

from unittest import TestCase, main, skipIf, SkipTest
from pathlib import Path
from re import findall, search, S
from urllib import request
from urllib.error import HTTPError
from argparse import ArgumentParser
from subprocess import Popen, check_output, PIPE, run
from pycodestyle import StyleGuide
from shutil import rmtree
from platform import python_version_tuple
from sys import platform
from threading import Thread
import os


p_root = Path(__file__).absolute().parents[2]
p_tests = p_root / 'tests'
p_readme = p_root / "README.md"
p_architecture = p_root / "ARCHITECTURE.md"
p_release_notes = p_root / "RELEASE_NOTES.md"
p_init_py = p_root / 'zcbor' / '__init__.py'
p_zcbor_py = p_root / 'zcbor' / 'zcbor.py'
p_setup_py = p_root / 'setup.py'
p_add_helptext = p_root / 'add_helptext.py'
p_test_zcbor_py = p_tests / 'scripts' / 'test_zcbor.py'
p_test_versions_py = p_tests / 'scripts' / 'test_versions.py'
p_test_repo_files_py = p_tests / 'scripts' / 'test_repo_files.py'


class TestCodestyle(TestCase):
    def do_codestyle(self, files, **kwargs):
        style = StyleGuide(max_line_length=100, **kwargs)
        result = style.check_files([str(f) for f in files])
        result.print_statistics()
        self.assertEqual(result.total_errors, 0,
                         f"Found {result.total_errors} style errors")

    def test_codestyle(self):
        """Run codestyle tests on all Python scripts in the repo."""
        self.do_codestyle([p_init_py, p_setup_py, p_test_versions_py, p_test_repo_files_py])
        self.do_codestyle([p_zcbor_py], ignore=['W191', 'E101', 'W503'])
        self.do_codestyle([p_test_zcbor_py], ignore=['E402', 'E501', 'W503'])


class TestDocs(TestCase):
    @skipIf(list(map(int, python_version_tuple())) < [3, 10, 0],
            "Skip on Python < 3.10 because of different wording in argparse output.")
    @skipIf(platform.startswith("win"), "Skip on Windows because of path/newline issues.")
    def test_cli_doc(self):
        """Check the auto-generated CLI docs in the top level README.md file."""
        add_help = Popen(["python3", p_add_helptext, "--check"])
        add_help.communicate()
        self.assertEqual(0, add_help.returncode)


if __name__ == "__main__":
    main()
