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


def version_int(in_str):
    return int(search(r'\A\d+', in_str)[0])  # e.g. '0rc' -> '0'


class TestDocs(TestCase):
    def __init__(self, *args, **kwargs):
        """Overridden to get base URL for relative links from remote tracking branch."""
        super(TestDocs, self).__init__(*args, **kwargs)
        remote_tracking = run(['git', 'rev-parse', '--abbrev-ref', '--symbolic-full-name', '@{u}'],
                              capture_output=True).stdout.strip()
        if remote_tracking:
            remote, remote_branch = tuple(remote_tracking.split(b'/'))
            repo_url = check_output(['git', 'remote', 'get-url', remote]).strip().strip(b'.git')
            if b"github.com" in repo_url:
                self.base_url = (repo_url + b'/tree/' + remote_branch + b'/').decode('utf-8')
            else:
                # The URL is not in github.com, so we are not sure it is constructed correctly.
                self.base_url = None
        else:
            # There is no remote tracking branch.
            self.base_url = None

    def check_code(self, link):
        """Check the status code of a URL link. Assert if not 200 (OK)."""
        try:
            call = request.urlopen(link)
            code = call.getcode()
        except HTTPError as e:
            code = e.code
        self.assertEqual(code, 200, f"'{link}' gives code {code}")

    def do_test_links(self, path):
        """Get all Markdown links in the file at <path> and check that they work."""
        if self.base_url is None:
            raise SkipTest('This test requires the current branch to be pushed to Github.')

        text = path.read_text()
        relative_path = str(path.relative_to(p_root).parent)
        relative_path = "" if relative_path == "." else relative_path + "/"

        matches = findall(r'\[.*?\]\((?P<link>.*?)\)', text)
        codes = list()
        threads = list()
        for m in matches:
            link = self.base_url + relative_path + m if "http" not in m else m
            threads.append(t := Thread(target=self.check_code, args=(link,), daemon=True))
            t.start()
        for t in threads:
            t.join()

    def test_readme_links(self):
        self.do_test_links(p_readme)

    def test_architecture(self):
        self.do_test_links(p_architecture)

    def test_release_notes(self):
        self.do_test_links(p_release_notes)

    @skipIf(list(map(version_int, python_version_tuple())) < [3, 10, 0],
            "Skip on Python < 3.10 because of different wording in argparse output.")
    @skipIf(platform.startswith("win"), "Skip on Windows because of path/newline issues.")
    def test_cli_doc(self):
        """Check the auto-generated CLI docs in the top level README.md file."""
        add_help = Popen(["python3", p_add_helptext, "--check"])
        add_help.communicate()
        self.assertEqual(0, add_help.returncode)


if __name__ == "__main__":
    main()
