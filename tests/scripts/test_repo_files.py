#!/usr/bin/env python3
#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0

from unittest import TestCase, main, skipIf, SkipTest
from pathlib import Path
from re import search, S, compile
from urllib import request
from urllib.error import HTTPError
from subprocess import Popen, check_output, PIPE, run
from shutil import rmtree
from platform import python_version_tuple
from sys import platform
from threading import Thread
import os
from time import sleep
from http import HTTPStatus


p_root = Path(__file__).absolute().parents[2]
p_tests = p_root / "tests"
p_readme = p_root / "README.md"
p_pypi_readme = p_root / "pypi_README.md"
p_architecture = p_root / "ARCHITECTURE.md"
p_release_notes = p_root / "RELEASE_NOTES.md"
p_add_helptext = p_root / "scripts" / "add_helptext.py"
p_regenerate_samples = p_root / "scripts" / "regenerate_samples.py"
p_hello_world_sample = p_root / "samples" / "hello_world"
p_hello_world_build = p_hello_world_sample / "build"
p_pet_sample = p_root / "samples" / "pet"
p_pet_cmake = p_pet_sample / "pet.cmake"
p_pet_include = p_pet_sample / "include"
p_pet_src = p_pet_sample / "src"
p_pet_build = p_pet_sample / "build"


class TestCodestyle(TestCase):
    def test_codestyle(self):
        black_res = Popen(["black", "--check", p_root, "-l", "100"], stdout=PIPE, stderr=PIPE)
        _, stderr = black_res.communicate()
        self.assertEqual(0, black_res.returncode, "black failed:\n" + stderr.decode("utf-8"))


def version_int(in_str):
    return int(search(r"\A\d+", in_str)[0])  # e.g. '0rc' -> '0'


class TestSamples(TestCase):
    def popen_test(self, args, input="", exp_retcode=0, **kwargs):
        call0 = Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE, **kwargs)
        stdout0, stderr0 = call0.communicate(input)
        self.assertEqual(exp_retcode, call0.returncode, stderr0.decode("utf-8"))
        return stdout0, stderr0

    def cmake_build_run(self, path, build_path):
        if build_path.exists():
            rmtree(build_path)
        with open(path / "README.md", "r", encoding="utf-8") as f:
            contents = f.read()

        to_build_patt = r"### To build:.*?```(?P<to_build>.*?)```"
        to_run_patt = r"### To run:.*?```(?P<to_run>.*?)```"
        exp_out_patt = r"### Expected output:.*?(?P<exp_out>(\n>[^\n]*)+)"
        to_build = search(to_build_patt, contents, flags=S)["to_build"].strip()
        to_run = search(to_run_patt, contents, flags=S)["to_run"].strip()
        exp_out = search(exp_out_patt, contents, flags=S)["exp_out"].replace("\n> ", "\n").strip()

        os.chdir(path)
        commands_build = [(line.split(" ")) for line in to_build.split("\n")]
        assert "\n" not in to_run, "The 'to run' section should only have one command."
        commands_run = to_run.split(" ")
        for c in commands_build:
            self.popen_test(c)
        output_run = ""
        for c in commands_run:
            output, _ = self.popen_test(c)
            output_run += output.decode("utf-8")
        self.assertEqual(exp_out, output_run.strip())

    @skipIf(platform.startswith("win"), "Skip on Windows because requires a Unix shell.")
    def test_hello_world(self):
        output = self.cmake_build_run(p_hello_world_sample, p_hello_world_build)

    @skipIf(platform.startswith("win"), "Skip on Windows because requires a Unix shell.")
    def test_pet(self):
        output = self.cmake_build_run(p_pet_sample, p_pet_build)

    def test_pet_regenerate(self):
        """Check the zcbor-generated code for the "pet" sample"""
        regenerate = Popen(["python3", p_regenerate_samples, "--check"])
        regenerate.communicate()
        self.assertEqual(0, regenerate.returncode)

    def test_pet_file_header(self):
        files = list(p_pet_include.iterdir()) + list(p_pet_src.iterdir()) + [p_pet_cmake]
        for p in [f for f in files if "pet" in f.name]:
            with p.open("r", encoding="utf-8") as f:
                f.readline()  # discard
                self.assertEqual(
                    f.readline().strip(" *#\n"),
                    "Copyright (c) 2022 Nordic Semiconductor ASA",
                )
                f.readline()  # discard
                self.assertEqual(f.readline().strip(" *#\n"), "SPDX-License-Identifier: Apache-2.0")
                f.readline()  # discard
                self.assertIn("Generated using zcbor version", f.readline())
                self.assertIn("https://github.com/NordicSemiconductor/zcbor", f.readline())
                self.assertIn("Generated with a --default-max-qty of", f.readline())


class TestDocs(TestCase):
    """Test the links in the documentation files."""

    def __init__(self, *args, **kwargs):
        """Overridden to get base URL for relative links from remote tracking branch."""
        super(TestDocs, self).__init__(*args, **kwargs)
        remote_tr_args = [
            "git",
            "rev-parse",
            "--abbrev-ref",
            "--symbolic-full-name",
            "@{u}",
        ]
        remote_tracking = run(remote_tr_args, capture_output=True).stdout.decode("utf-8").strip()

        if remote_tracking:
            remote, remote_branch = remote_tracking.split("/", 1)  # '1' to only split one time.
            repo_url_args = ["git", "remote", "get-url", remote]
            repo_url = check_output(repo_url_args).decode("utf-8").strip().strip(".git")
            if "github.com" in repo_url:
                self.base_url = repo_url + "/tree/" + remote_branch + "/"
            else:
                # The URL is not in github.com, so we are not sure it is constructed correctly.
                self.base_url = None
        elif "GITHUB_SHA" in os.environ and "GITHUB_REPOSITORY" in os.environ:
            repo = os.environ["GITHUB_REPOSITORY"]
            sha = os.environ["GITHUB_SHA"]
            self.base_url = f"https://github.com/{repo}/blob/{sha}/"
        else:
            # There is no remote tracking branch.
            self.base_url = None

        target_ref = os.environ["GITHUB_BASE_REF"] if "GITHUB_BASE_REF" in os.environ else "main"
        local_target_ref = "_zcbor_doc_test_target_ref"
        target_repo = (
            os.environ["GITHUB_REPOSITORY"]
            if "GITHUB_REPOSITORY" in os.environ
            else "NordicSemiconductor/zcbor"
        )
        target_repo_url = f"https://github.com/{target_repo}.git"
        check_output(["git", "fetch", f"{target_repo_url}", f"{target_ref}:{local_target_ref}"])

        diff_args = [
            "git",
            "diff",
            "--name-only",
            f"{local_target_ref}..HEAD",
        ]
        diff = check_output(diff_args).decode("utf-8").strip()
        self.affected_files = [Path(p_root, p).absolute() for p in diff.split("\n")]

        self.current_branch = (
            check_output(["git", "rev-parse", "--abbrev-ref", "HEAD"]).decode("utf-8").strip()
        )

        self.link_regex = compile(r"\[.*?\]\((?P<link>.*?)\)")

    def check_code(self, link, backoff_time=0, attempts=5):
        """Check the status code of a URL link. Assert if not 200 (OK)."""
        try:
            call = request.urlopen(link)
            code = call.getcode()
        except HTTPError as e:
            code = e.code

        if code == HTTPStatus.TOO_MANY_REQUESTS and backoff_time > 0 and attempts > 0:
            # Github has instituted a rate limit for requests, so we need a backoff timer
            sleep(backoff_time)
            return self.check_code(link, backoff_time, attempts - 1)
        return code

    def check_code_async(self, link, codes, backoff_time=0, attempts=5):
        codes.append((link, self.check_code(link, backoff_time, attempts)))

    def get_links(self, path, allow_local=True):
        text = path.read_text(encoding="utf-8")

        if allow_local:
            # Use .parent to test relative links (links to repo files):
            relative_path = str(path.relative_to(p_root).parent)
            relative_path = "" if relative_path == "." else relative_path + "/"

        matches = self.link_regex.findall(text)
        links = list()
        local_links = list()

        for m in matches:
            link = m
            if allow_local:
                if link.startswith("#"):
                    # Github sometimes need the filename for anchor (#) links to work, so add it:
                    link = path.name + m
                if not link.startswith("https://"):
                    if "#" in link:
                        self.assertTrue(
                            self.base_url is not None, "This test needs the branch to be pushed."
                        )
                        link = self.base_url + relative_path + link

                    else:
                        local_links.append(relative_path + link)
                        continue
            else:
                self.assertTrue(link.startswith("https://"), "Link is not a URL")
            links.append(link)
        return links, local_links

    def check_links(self, links):
        codes = list()
        threads = list()

        for link in links:
            threads.append(
                t := Thread(
                    target=self.check_code_async,
                    args=(link, codes),
                    kwargs={"backoff_time": 20},
                    daemon=True,
                )
            )
            t.start()
        for t in threads:
            t.join()
        for link, code in codes:
            self.assertEqual(code, HTTPStatus.OK, f"'{link}' gives code {code}")

    def do_test_doc_links(self, only_changed=True):
        files_to_parse = [
            (p_readme, True),
            (p_architecture, True),
            (p_release_notes, True),
            (p_hello_world_sample / "README.md", True),
            (p_pet_sample / "README.md", True),
            (p_pypi_readme, False),
        ]
        links = set()

        for path, allow_local in files_to_parse:
            if only_changed is False or path.absolute() in self.affected_files:
                non_local_links, local_links = self.get_links(path, allow_local)

                for link in local_links:
                    self.assertTrue((p_root / link).exists(), f"Local file '{link}' does not exist")

                links.update(non_local_links)
            else:
                print(f"Skipping link checking of {path} because it is not changed in this PR.")

        # Check all links at once to avoid duplicates.
        self.check_links(links)

    def test_doc_links(self):
        check_all = self.current_branch == "main" or "release/" in self.current_branch
        self.do_test_doc_links(only_changed=not check_all)

    @skipIf(
        list(map(version_int, python_version_tuple())) < [3, 10, 0],
        "Skip on Python < 3.10 because of different wording in argparse output.",
    )
    @skipIf(platform.startswith("win"), "Skip on Windows because of path/newline issues.")
    def test_cli_doc(self):
        """Check the auto-generated CLI docs in the top level README.md file."""
        add_help = Popen(["python3", p_add_helptext, "--check"])
        add_help.communicate()
        self.assertEqual(0, add_help.returncode)


if __name__ == "__main__":
    main()
