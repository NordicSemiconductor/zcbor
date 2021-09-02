#!/usr/bin/env python3
#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

from pathlib import Path

from .cddl_gen import (
    CddlValidationError,
    DataTranslator,
)

script_path = Path(__file__).absolute().parents[0]
VERSION_path = Path(script_path, "VERSION")

with open(VERSION_path, 'r') as f:
    __version__ = f.read()
