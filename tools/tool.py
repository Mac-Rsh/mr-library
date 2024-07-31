#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@copyright (c) 2023-2024, MR Development Team

@license SPDX-License-Identifier: Apache-2.0

@date 2024-07-26    MacRsh       First version
"""

import sys
import logging
import argparse
import subprocess
from pathlib import Path
from builder import Builder
from config import kconfig

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] %(message)s',
                    datefmt='%Y-%m-%d %H:%M:%S')


def _check_python_version():
    if sys.version_info < (3, 10):
        logging.error(f'Python version must be >= 3.10(current: {sys.version})')
        exit(1)


def _find_mrlib() -> Path | None:
    for dir in Path(__file__).parents:
        if dir.name == 'mr-library':
            return dir
    return None


def _build(projdir: Path, incdirs: list[Path], srcfiles: list[Path]):
    try:
        builder = Builder(projdir)
        for incdir in incdirs:
            builder.add_include_dir(incdir)
        for srcfile in srcfiles:
            builder.add_source_file(srcfile)
        builder.build()
        logging.info("Build succeeded")
    except Exception as e:
        logging.error(f"Error during build: {e}")
        exit(1)


def _run_menuconfig(configfile: Path):
    try:
        subprocess.run(['menuconfig'], stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
        kconfig.generate_config(configfile)
        logging.info("Menuconfig succeeded")
    except Exception as e:
        logging.error(f"Error during menuconfig: {e}")


def main():
    # Check Python version
    _check_python_version()

    # Find "mr-library"
    mrlib = _find_mrlib()
    if mrlib is None:
        logging.error('mr-library not found')
        return

    # Parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-b', '--build', action='store_true',
                        help='Build the project')
    parser.add_argument('-m', '--menuconfig', action='store_true',
                        help='Run menuconfig')
    args = parser.parse_args()

    # Build the project
    if args.build:
        _build(mrlib.parent, [mrlib], list(mrlib.rglob('*.c')))

    # Run menuconfig
    if args.menuconfig:
        _run_menuconfig(mrlib / 'include' / 'mr_config.h')


if __name__ == '__main__':
    main()
