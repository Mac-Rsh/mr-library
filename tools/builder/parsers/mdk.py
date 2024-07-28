#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@copyright (c) 2023-2024, MR Development Team

@license SPDX-License-Identifier: Apache-2.0

@date 2024-07-25    MacRsh       First version
"""

import os
from lxml import etree
from pathlib import Path
from .base import BaseParser


class MdkParser(BaseParser):
    def __init__(self, projdir: Path):
        super().__init__(projdir)
        self.projfile: Path | None = None
        self.tree: etree.ElementTree = None

    def can_handle(self, projdir: Path) -> bool:
        # Look for ".uvprojx" files that can be parsed properly
        for file in projdir.rglob('*.uvprojx'):
            try:
                self.projfile = file
                self.tree = etree.parse(file)
                return True
            except:
                continue
        return False

    def build(self, incdirs: list[Path], srcfiles: list[Path]):
        self._add_incdirs(incdirs)
        self._add_srcfiles(srcfiles)
        self._save()

    def _add_incdirs(self, incdirs: list[Path]):
        projdir = self.projfile.parent
        mdk_incdirs = self.tree.xpath("//Cads/VariousControls/IncludePath")
        for incdir in incdirs:
            incdir = Path(os.path.relpath(incdir, projdir)).as_posix()
            if incdir not in mdk_incdirs[0].text.split(';'):
                mdk_incdirs[0].text += f";{incdir}"

    def _add_srcfiles(self, srcfiles: list[Path]):
        projdir = self.projfile.parent
        for srcfile in srcfiles:
            group = Path(
                os.path.relpath(Path(srcfile).parent, projdir)).relative_to(
                "..").as_posix()
            file = Path(os.path.relpath(srcfile, projdir))

            # Add group if it doesn't exist
            groups_node = self.tree.find('.//Groups')
            group_node = groups_node.find(f"./Group[GroupName='{group}']")
            if group_node is None:
                group_node = etree.SubElement(groups_node, "Group")
                group_name_node = etree.SubElement(group_node, "GroupName")
                group_name_node.text = group
                etree.SubElement(group_node, "Files")

            # Add file if it doesn't exist
            files_node = group_node.find("Files")
            file_node = files_node.find(f"./File[FilePath='{file.as_posix()}']")
            if file_node is None:
                file_node = etree.SubElement(files_node, "File")
            file_path_node = file_node.find("FilePath")
            if file_path_node is None:
                file_path_node = etree.SubElement(file_node, "FilePath")
            file_name_node = file_node.find("FileName")
            if file_name_node is None:
                file_name_node = etree.SubElement(file_node, "FileName")
            file_type_node = file_node.find("FileType")
            if file_type_node is None:
                file_type_node = etree.SubElement(file_node, "FileType")
            file_path_node.text = file.as_posix()
            file_name_node.text = file.name
            file_type_node.text = '1'

    def _save(self):
        self.tree.write(self.projfile, pretty_print=True, encoding="utf-8",
                        xml_declaration=True)
