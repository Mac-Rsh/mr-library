#!/usr/bin/env python

"""
@copyright (c) 2023-2024, MR Development Team

@license SPDX-License-Identifier: Apache-2.0

@date 2023-12-17    MacRsh       First version
"""

import os
import logging
import argparse
import subprocess
from pathlib import Path

try:
    from lxml import etree
except ImportError:
    subprocess.run(['pip', 'install', 'lxml'], check=True)
    from lxml import etree

try:
    from kconfiglib import Kconfig
except ImportError:
    subprocess.run(['pip', 'install', 'kconfiglib'], check=True)

try:
    import curses
except ImportError:
    subprocess.run(['pip', 'install', 'windows-curses'], check=True)

logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')


class MDK5:
    def __init__(self, mdk_path):
        self.path, self.file = self._find_uvprojx_file(mdk_path)
        if self.file:
            self.exist = True
            self.tree = etree.parse(self.file)
            self.root = self.tree.getroot()
        else:
            self.exist = False

    @staticmethod
    def _find_uvprojx_file(mdk_path):
        for root, dirs, files in os.walk(mdk_path):
            for file in files:
                if file.endswith('.uvprojx'):
                    return Path(root), Path(root) / file
        return None, None

    def add_include_path(self, path):
        path = Path(path).relative_to(self.path).as_posix()
        inc_path = self.tree.xpath("//Cads/VariousControls/IncludePath")[0]
        exist_paths = inc_path.text.split(';')
        if path not in exist_paths:
            inc_path.text += f";{path}"
        logging.info(f"include {path}")

    def add_file_to_group(self, name, file):
        file = Path(file).relative_to(self.path).as_posix()

        groups_node = self.tree.find('//Groups')
        group_node = groups_node.find(f"./Group[GroupName='{name}']")
        if group_node is None:
            group_node = etree.SubElement(groups_node, "Group")
            group_name_node = etree.SubElement(group_node, "GroupName")
            group_name_node.text = name

        files_node = group_node.find("Files")
        if files_node is None:
            files_node = etree.SubElement(group_node, "Files")
        # Add file
        file_node = files_node.find(
            f"./File[FileName='{os.path.basename(file)}']")
        if file_node is None:
            file_node = etree.SubElement(files_node, "File")
        file_name_node = file_node.find(f"./FileName")
        if file_name_node is None:
            file_name_node = etree.SubElement(file_node, "FileName")
        file_type_node = file_node.find(f"./FileType")
        if file_type_node is None:
            file_type_node = etree.SubElement(file_node, "FileType")
        file_path_node = file_node.find(f"./FilePath")
        if file_path_node is None:
            file_path_node = etree.SubElement(file_node, "FilePath")
        file_name_node.text = os.path.basename(file)
        file_path_node.text = file
        file_type_map = {
            '.c': '1',
            '.s': '2',
            '.o': '3',
            '.lib': '4',
            '.h': '5',
            '.cpp': '8'
        }
        file_extension = os.path.splitext(file_name_node.text)[1]
        file_type = file_type_map.get(file_extension, '9')
        file_type_node.text = file_type
        logging.info(f"=> {file}")

    def add_file(self, file):
        group_name = Path(file).relative_to(
            self.path).parent.as_posix().replace('../', '')
        file = Path(file).relative_to(self.path).as_posix()
        self.add_file_to_group(group_name, file)

    def add_files(self, files):
        for file in files:
            self.add_file(file)

    def use_gnu(self, enable=True):
        ac6_node = self.tree.find('//Target/uAC6')
        if ac6_node is None:
            logging.error("Config GNU failed")
            return
        if ac6_node.text == '0':
            gnu_node = self.tree.find('//Cads/uGnu')
            gnu_text = '1' if enable else '0'
        else:
            gnu_node = self.tree.find('//Cads/v6Lang')
            gnu_text = '4' if enable else '3'
        if gnu_node is not None:
            if gnu_node.text != gnu_text:
                gnu_node.text = gnu_text
            logging.info("Config GNU successfully")

    def save(self):
        self.tree.write(self.file, pretty_print=True, encoding="utf-8",
                        xml_declaration=True)
        logging.info("Build project successfully")


class Eclipse:
    def __init__(self, eclipse_path):
        self.path, self.file = self._find_cproject_file(eclipse_path)
        if self.file:
            self.exist = True
            self.tree = etree.parse(self.file)
            self.root = self.tree.getroot()
        else:
            self.exist = False

    @staticmethod
    def _find_cproject_file(eclipse_path):
        for root, dirs, files in os.walk(eclipse_path):
            for file in files:
                if file.endswith('.cproject'):
                    return Path(root), Path(root) / file
        return None, None

    def use_auto_init(self):
        ld_file = self._find_ld_file(self.path)
        if ld_file:
            with ld_file.open() as fr:
                content = fr.read()
                pos = content.find('.text :')
                if pos == -1:
                    return
                if '/* mr-library */' not in content:
                    pos_offset = content[pos:].find('}')
                    if pos_offset == -1:
                        return
                    pos += pos_offset
                    link_config = """
                                    /* mr-library */
                                    . = ALIGN(4);
                                    KEEP(*(SORT(mr_auto_init.*)))
                                    KEEP(*(SORT(mr_msh_cmd.*)))
                                    . = ALIGN(4);
                                    """
                    with ld_file.open('w') as fw:
                        fw.write(content[:pos] + link_config + content[pos:])
                    logging.info("Config auto-init successfully")
        else:
            logging.warning("Config auto-init failed, '.ld' not found")

    @staticmethod
    def _find_ld_file(path):
        for root, dirs, files in os.walk(path):
            for file in files:
                if file.endswith(".ld"):
                    return Path(root) / file
        return None

    def add_include_path(self, path):
        path = Path(path).relative_to(self.path).as_posix()
        path_nodes = self.tree.findall(".//option[@valueType='includePath']")
        for path_node in path_nodes:
            path_node_id = path_node.get('id')
            if 'c.compiler' in path_node_id and 'include' in path_node_id:
                option = path_node.find(f".//listOptionValue[@value='{path}']")
                if option is None:
                    etree.SubElement(path_node, "listOptionValue",
                                     builtIn="false", value=path)
                    logging.info(f"=> {path}")
                break

    def save(self):
        self.tree.write(self.file, pretty_print=True, encoding="utf-8",
                        xml_declaration=True)
        logging.info("Build project successfully")


class MrLib:
    def __init__(self):
        self.path = Path.cwd()
        self.c_files = list(self.path.rglob('*.c'))
        self.h_files = list(self.path.rglob('*.h'))
        self.proj_path = self.path.parent


def show_logo():
    logo = """
 __  __                  _   _   _
|  \/  |  _ __          | | (_) | |__    _ __    __ _   _ __   _   _
| |\/| | | '__|  _____  | | | | | '_ \  | '__|  / _` | | '__| | | | |
| |  | | | |    |_____| | | | | | |_) | | |    | (_| | | |    | |_| |
|_|  |_| |_|            |_| |_| |_.__/  |_|     \__,_| |_|     \__, |
                                                               |___/"""
    print(logo)


def show_license():
    license_file = Path(__file__).parent / "LICENSE"
    with license_file.open() as f:
        print(f.read())


def build_mdk(proj_path, include_path, c_files):
    mdk_proj = MDK5(proj_path)
    mdk_proj.add_include_path(include_path)
    mdk_proj.add_files(c_files)
    mdk_proj.use_gnu(True)
    mdk_proj.save()


def build_eclipse(proj_path, path):
    eclipse_proj = Eclipse(proj_path)
    eclipse_proj.add_include_path(path)
    eclipse_proj.use_auto_init()
    eclipse_proj.save()


def menuconfig():
    devnull_path = os.devnull if os.name == 'nt' else '/dev/null'
    try:
        with open(devnull_path, 'w') as devnull:
            subprocess.run(["menuconfig"], stdout=devnull, stderr=devnull,
                           check=True)
    except subprocess.CalledProcessError:
        logging.error("Config menuconfig failed")
        exit(1)
    try:
        subprocess.run(["python", "tools/kconfig.py"], check=True)
    except subprocess.CalledProcessError:
        logging.error("Config menuconfig failed, 'kconfig.py' not found")
        exit(1)


def build():
    mr_lib = MrLib()

    mdk5 = MDK5(mr_lib.proj_path)
    if mdk5.exist:
        build_mdk(mr_lib.proj_path, mr_lib.path, mr_lib.c_files)
    else:
        eclipse = Eclipse(mr_lib.proj_path)
        if eclipse.exist:
            build_eclipse(mr_lib.proj_path, mr_lib.path)
        else:
            logging.error("Found project(MDK5 or Eclipse) failed")
            exit(1)


if __name__ == '__main__':
    show_logo()

    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--menuconfig", action="store_true",
                        help="Run menuconfig")
    parser.add_argument("-b", "--build", action="store_true",
                        help="Build project")
    parser.add_argument("-l", "--license", action="store_true",
                        help="Show license")
    parser.add_argument("-g", "--generate",
                        action="store_true",
                        help="Generate library include file")
    args = parser.parse_args()

    if args.build:
        build()
    if args.menuconfig:
        menuconfig()
    if args.license:
        show_license()
    if args.generate:
        mr = MrLib()
