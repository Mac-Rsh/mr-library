#!/usr/bin/env python

"""
@copyright (c) 2023, MR Development Team

@license SPDX-License-Identifier: Apache-2.0

@date 2023-12-17    MacRsh       First version
"""

import os
import pip
import argparse
import subprocess


def log_print(level, text):
    # Log level colors
    LEVEL_COLORS = {
        'error': '\033[31m',
        'success': '\033[32m',
        'warning': '\033[33m',
        'info': '\033[34m',
    }
    RESET_COLOR = '\033[0m'
    # Log level name
    LEVEL_NAME = {
        'error': 'ERROR',
        'success': 'SUCCESS',
        'warning': 'WARNING',
        'info': 'INFO',
    }
    print(LEVEL_COLORS[level] + LEVEL_NAME[level] + ': ' + text + RESET_COLOR)


def install_package(package):
    log_print('info', "%s package installing..." % package)
    pip.main(['install', package])


try:
    from lxml import etree
except ImportError:
    install_package('lxml')
    from lxml import etree

try:
    from kconfiglib import Kconfig
except ImportError:
    install_package('kconfiglib')
    from kconfiglib import Kconfig

try:
    import curses
except ImportError:
    install_package('windows-curses')


class MDK5:

    def __init__(self, mdk_path):
        # Get MDK project file
        mdk_file = []
        for root, dirs, fs in os.walk(mdk_path):
            for f in fs:
                if f.endswith('.uvprojx'):
                    mdk_file = os.path.join(root, f)
                    break
            if mdk_file:
                break
        # Check mdk file, init self
        if mdk_file:
            self.path = os.path.dirname(mdk_file)
            self.file = mdk_file
            self.tree = etree.parse(mdk_file)
            self.root = self.tree.getroot()
        else:
            log_print('error', "MDK build failed, '.uvprojx' file not found")
            exit(1)

    def add_include_path(self, path):
        # Fix path
        path = os.path.relpath(path, self.path).replace('\\', '/')
        # Add path
        inc_path = self.tree.xpath("//Cads/VariousControls/IncludePath")[0]
        exist_paths = inc_path.text.split(';')
        if path not in exist_paths:
            inc_path.text += f";{path}"
        log_print('info', "include %s" % path)

    def add_include_paths(self, paths):
        for path in paths:
            self.add_include_path(path)

    def add_files_new_group(self, name, files):
        # Fix name and files
        name = name.replace('\\', '/')
        fix_files = []
        for file in files:
            file = os.path.relpath(file, self.path).replace('\\', '/')
            fix_files.append(file)
        files = fix_files
        # Add group
        groups_node = self.tree.find('//Groups')
        group_node = groups_node.find(f"./Group[GroupName='{name}']")
        if group_node is None:
            group_node = etree.SubElement(groups_node, "Group")
            group_name_node = etree.SubElement(group_node, "GroupName")
            group_name_node.text = name
        # Check files
        if files is None:
            return
        # Add files
        files_node = group_node.find("Files")
        if files_node is None:
            files_node = etree.SubElement(group_node, "Files")
        for file in files:
            # Add file
            file_node = files_node.find(f"./File[FileName='{os.path.basename(file)}']")
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
        log_print('info', "add %s" % name)

    def add_path_files(self, path):
        files = []
        for root, dirs, fs in os.walk(path):
            for f in fs:
                files.append(os.path.relpath(os.path.join(root, f), self.path))
        self.add_files_new_group(path, files)

    def add_path_c_files(self, path):
        # Get c files
        files = []
        for root, dirs, fs in os.walk(path):
            if root == path:
                for f in fs:
                    if f.endswith(".c") or f.endswith(".cpp") or f.endswith(".cxx"):
                        file = os.path.relpath(os.path.join(root, f), self.path)
                        files.append(file)
        # Fix name
        name = os.path.relpath(path, self.path).replace('\\', '/').replace("../", "")
        # Add group
        if files:
            self.add_files_new_group(name, files)

    def use_gnu(self, enable=True):
        # Check uAC6
        ac6_node = self.tree.find('//Target/uAC6')
        # Use GNU
        if ac6_node.text == '0':
            # Get uGnu
            gnu_node = self.tree.find('//Cads/uGnu')
            gnu_text = '1' if enable else '0'
        else:
            # Get gnu-c99
            gnu_node = self.tree.find('//Cads/v6Lang')
            gnu_text = '4' if enable else '3'
        # Set gnu
        if gnu_node is not None:
            if gnu_node.text != gnu_text:
                gnu_node.text = gnu_text
            log_print('info', "use GNU")

    def save(self):
        self.tree.write(self.file, pretty_print=True, encoding="utf-8", xml_declaration=True)
        log_print('success', "MDK %s build success" % self.file)


class Eclipse:
    def __init__(self, eclipse_path):
        # Get eclipse project file
        eclipse_file = []
        for root, dirs, fs in os.walk(eclipse_path):
            for f in fs:
                if f.endswith('.cproject'):
                    eclipse_file = os.path.join(root, f)
                    break
            if eclipse_file:
                break
        # Check eclipse file, init self
        if eclipse_file:
            self.path = os.path.dirname(eclipse_file)
            self.file = eclipse_file
            self.tree = etree.parse(eclipse_file)
            self.root = self.tree.getroot()
        else:
            log_print('error', "eclipse build failed, '.cproject' not found")
            exit(1)

    def add_include_path(self, path):
        # Fix path
        path = os.path.relpath(path, self.file).replace('\\', '/')
        # Find all include path node
        inc_path_nodes = self.tree.findall(".//option[@valueType='includePath']")
        for inc_path_node in inc_path_nodes:
            inc_path_node_id = inc_path_node.get('id')
            # Check path node
            if 'c.compiler' in inc_path_node_id and 'include' in inc_path_node_id:
                # Add path
                list_option = inc_path_node.find(f".//listOptionValue[@value='{path}']")
                if list_option is None:
                    list_option = etree.SubElement(inc_path_node, "listOptionValue")
                    list_option.set('builtIn', "false")
                    list_option.set('value', path)
                    log_print('info', "include %s" % path)
                break

    def use_auto_init(self):
        # Find ld file
        ld_file = []
        for root, dirs, files in os.walk(self.path):
            for file in files:
                if file.endswith(".ld"):
                    ld_file = os.path.join(root, file)
                    break
            if ld_file:
                break
        # Check ld file
        if ld_file:
            # Use auto init
            with open(ld_file) as fr:
                content = fr.read()
                pos = content.find('.text :')
                # Check pos
                if pos == -1:
                    log_print('warning', "use auto init failed, '.text' not found")
                    return
                # Check auto init is existed
                if content.find('/* mr-library auto init */') == -1:
                    # Find pos offset
                    pos_offset = content[pos:].find('}')
                    # Check pos offset
                    if pos_offset == -1:
                        log_print('warning', "use auto init failed, '.text' not found")
                        return
                    pos = pos + pos_offset
                    # Use auto init
                    with open(ld_file, 'w') as fw:
                        front = content[:pos]
                        auto_init = """
        /* mr-library auto init */
        . = ALIGN(4);
        _mr_auto_init_start = .;
        KEEP(*(SORT(.auto_init*)))
        _mr_auto_init_end = .;
    """
                        back = content[pos:]
                        fw.write(front + auto_init + back)
                        fw.close()
                    fr.close()
                    log_print('info', "use auto init")
        else:
            log_print('warning', "use auto init failed, '.ld' not found")

    def save(self):
        self.tree.write(self.file, pretty_print=True, encoding="utf-8", xml_declaration=True)
        log_print('success', "eclipse %s build success" % self.file)


class MR:

    def __init__(self):
        self.path = os.path.dirname(__file__)
        self.files_paths = []
        for root, dirs, files in os.walk(self.path):
            if root == self.path:
                for path in dirs:
                    file_path = os.path.join(root, path)
                    self.files_paths.append(file_path)
            break
        self.project_path = os.path.dirname(self.path)


def show_logo():
    print(" __  __                  _   _   _                                 ")
    print("|  \/  |  _ __          | | (_) | |__    _ __    __ _   _ __   _   _")
    print("| |\/| | | '__|  _____  | | | | | '_ \  | '__|  / _` | | '__| | | | |")
    print("| |  | | | |    |_____| | | | | | |_) | | |    | (_| | | |    | |_| |")
    print("|_|  |_| |_|            |_| |_| |_.__/  |_|     \__,_| |_|     \__, |")
    print("                                                               |___/")


def show_license():
    license_file = os.path.join(os.path.dirname(__file__), "LICENSE")
    try:
        with open(license_file) as fr:
            print(fr.read())
    except OSError:
        log_print('warning',
                  "This software is provided subject to the terms of the Apache License 2.0, the full text of which "
                  "is not currently available due to missing license documentation. By continuing to use the "
                  "Software, you agree to be bound by the terms of the Apache License 2.0. The full license text is "
                  "available at http://www.apache.org/licenses/LICENSE-2.0. We advise you to review the license terms "
                  "in full before use to ensure you understand and agree to be bound by all provisions contained "
                  "therein.")
        log_print('warning',
                  "本软件根据Apache许可证2.0版本条款提供,由于许可证文件缺失,当前无法获取完整许可内容。继续使用本软件, "
                  "代表您同意接受并遵守Apache许可证2.0版本的所有条款。完整许可证可在http://www.apache.org/licenses/LICENSE-2.0"
                  "查看。建议您在使用前全面复核许可证内容, 以确保完全理解并同意接受其中的所有规定。")


def build_mdk():
    mr = MR()
    # MDK project
    mdk_proj = MDK5(mr.project_path)
    # Include path
    mdk_proj.add_include_path(mr.path)
    # Add all c files
    for files_path in mr.files_paths:
        mdk_proj.add_path_c_files(files_path)
    # Use gnu
    mdk_proj.use_gnu(True)
    # Save
    mdk_proj.save()


def build_eclipse():
    mr = MR()
    # Eclipse project
    eclipse_proj = Eclipse(mr.project_path)
    # Include path
    eclipse_proj.add_include_path(mr.path)
    # Use auto init
    eclipse_proj.use_auto_init()
    # Save
    eclipse_proj.save()


def menuconfig():
    try:
        if os.name == 'nt':
            devnull = open(os.devnull, 'w')
        else:
            devnull = open('/dev/null', 'w')
        subprocess.run(["menuconfig"], stdout=devnull, stderr=devnull, check=True)
        devnull.close()
    except subprocess.CalledProcessError:
        log_print('error', "menuconfig failed")
        exit(1)
    try:
        subprocess.run(["python", "kconfig.py"], check=True)
    except subprocess.CalledProcessError:
        log_print('error', "menuconfig failed, 'kconfig.py' not found")
        exit(1)


if __name__ == '__main__':
    # Show logo
    show_logo()

    # Parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--menuconfig", action="store_true", help="Run menuconfig")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-mdk", "--mdk", action="store_true", help="Build with MDK")
    group.add_argument("-ecl", "--eclipse", action="store_true", help="Build with Eclipse")
    parser.add_argument("-lic", "--license", action="store_true", help="Show license")
    args = parser.parse_args()

    # Build
    if args.mdk:
        build_mdk()
    elif args.eclipse:
        build_eclipse()
    # Menuconfig
    if args.menuconfig:
        menuconfig()
    # Show license
    if args.license:
        show_license()
