#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@copyright (c) 2023-2024, MR Development Team

@license SPDX-License-Identifier: Apache-2.0

@date 2024-07-26    MacRsh       First version
"""

from pathlib import Path


class BaseParser:
    def __init__(self, projdir: Path):
        self.projdir = projdir

    def can_handle(self, projdir: Path) -> bool:
        """
        Determines whether the parser can handle the given project directory.

        Args:
            projdir (Path): The project directory to be checked.

        Returns:
            bool: True if the parser can handle the project directory, False otherwise.
        """
        pass

    def build(self, incdirs: list[Path], srcfiles: list[Path]):
        """
        Builds the project using the given include directories and source files.

        Args:
            incdirs (list[Path]): The list of include directories.
            srcfiles (list[Path]): The list of source files.
        """
        pass
