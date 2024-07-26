#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@copyright (c) 2023-2024, MR Development Team

@license SPDX-License-Identifier: Apache-2.0

@date 2024-07-26    MacRsh       First version
"""

from pathlib import Path
from .parsers import BaseParser


class Builder:
    def __init__(self, projdir: Path):
        # Choose suitable parser
        self.__parser = self._get_parser(projdir)
        if self.__parser is None:
            raise ValueError("Suitable parser not found.")
        self.projdir = projdir
        self.incdirs = []
        self.srcfiles = []

    @staticmethod
    def _get_parser(projdir: Path) -> BaseParser | None:
        for parser_cls in BaseParser.__subclasses__():
            parser = parser_cls(projdir)
            if parser.can_handle(projdir):
                return parser
        return None

    def add_include_dir(self, incdir: Path):
        self.incdirs.append(incdir)

    def add_source_file(self, srcfile: Path):
        self.srcfiles.append(srcfile)

    def build(self):
        self.__parser.build(self.incdirs, self.srcfiles)
