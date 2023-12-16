#!/usr/bin/env python

import os
import pip
import argparse
import subprocess


def install_package(package):
    pip.main(['install', package])


try:
    from lxml import etree
except ImportError:
    print("Package not installed, installing...")
    install_package('lxml')

try:
    from kconfiglib import Kconfig
except ImportError:
    install_package('kconfiglib')

try:
    import curses
except ImportError:
    install_package('windows-curses')


class MDK5:

    def __init__(self, mdk_path):
        # Get MDK path
        for root, dirs, fs in os.walk(mdk_path):
            for f in fs:
                if f.endswith(".uvprojx"):
                    mdk_path = os.path.join(root, f)
        self.path = os.path.dirname(mdk_path)
        self.file = mdk_path
        self.name = os.path.basename(mdk_path)
        self.tree = etree.parse(mdk_path)
        self.root = self.tree.getroot()

    def add_include_path(self, path):
        # Fix path
        path = os.path.relpath(path, os.path.abspath(self.path)).replace('\\', '/')
        # Add path
        inc_path = self.tree.xpath("//Cads/VariousControls/IncludePath")[0]
        exist_paths = inc_path.text.split(';')
        if path not in exist_paths:
            inc_path.text += f";{path}"
            print("Include %s" % path)

    def add_include_paths(self, paths):
        for path in paths:
            self.add_include_path(path)

    def add_files_new_group(self, name, files):
        # Fix path
        name = name.replace('\\', '/')
        fix_files = []
        for file in files:
            fix_files.append(file.replace('\\', '/'))
        files = fix_files
        # Add group
        groups_node = self.tree.find('//Groups')
        group_node = groups_node.find(f"./Group[GroupName='{name}']")
        if group_node is None:
            group_node = etree.SubElement(groups_node, "Group")
            group_name_node = etree.SubElement(group_node, "GroupName")
            group_name_node.text = name
        # Check files exist
        if files is None:
            return
        # Add files
        files_node = group_node.find("Files")
        if files_node is None:
            files_node = etree.SubElement(group_node, "Files")
        for file in files:
            # Add files
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
            _, file_extension = os.path.splitext(file_name_node.text)
            if file_extension == ".c":
                file_type_node.text = "1"
            elif file_extension == ".s" or file_extension == ".S":
                file_type_node.text = "2"
            elif file_extension == ".o":
                file_type_node.text = "3"
            elif file_extension == ".lib":
                file_type_node.text = "4"
            elif file_extension == ".h":
                file_type_node.text = "5"
            elif file_extension == ".cpp" or file_extension == ".cxx":
                file_type_node.text = "8"
            else:
                file_type_node.text = "9"
        print("Add %s" % name)

    def add_path_files(self, path):
        files = []
        for root, dirs, fs in os.walk(path):
            for f in fs:
                files.append(os.path.relpath(os.path.join(root, f), os.path.abspath(self.path)))
        self.add_files_new_group(path, files)

    def add_path_c_files(self, path):
        files = []
        for root, dirs, fs in os.walk(path):
            for f in fs:
                if f.endswith(".c") or f.endswith(".cpp") or f.endswith(".cxx"):
                    files.append(os.path.relpath(os.path.join(root, f), os.path.abspath(self.path)))
        self.add_files_new_group(path, files)

    def use_gnu(self, enable=True):
        # Check uAC6
        ac6_node = self.tree.find('//Target/uAC6')
        if ac6_node.text == '0':
            # Use uGnu
            gnu_node = self.tree.find('//Cads/uGnu')
            gnu_node.text = '1' if enable else '0'
        else:
            # Use gnu-c99
            gnu_node = self.tree.find('//Cads/v6Lang')
            gnu_node.text = '4' if enable else '3'
        print("Use GNU")

    def save(self):
        self.tree.write(self.file, pretty_print=True, encoding="utf-8", xml_declaration=True)
        print("Build successfully")


def show_logo():
    print(" __  __                  _   _   _                                 ")
    print("|  \/  |  _ __          | | (_) | |__    _ __    __ _   _ __   _   _")
    print("| |\/| | | '__|  _____  | | | | | '_ \  | '__|  / _` | | '__| | | | |")
    print("| |  | | | |    |_____| | | | | | |_) | | |    | (_| | | |    | |_| |")
    print("|_|  |_| |_|            |_| |_| |_.__/  |_|     \__,_| |_|     \__, |")
    print("                                                               |___/")


def build_mdk():
    # Mr-library path
    mr_path = os.path.dirname(__file__)
    mr_c_paths = ['mr-library/device', 'mr-library/driver', 'mr-library/source']
    # MDK path
    mdk_file = os.path.dirname(mr_path)

    # Build
    mdk_proj = MDK5(mdk_file)
    mdk_proj.add_include_path(mr_path)
    for mr_c_path in mr_c_paths:
        mdk_proj.add_path_files(mr_c_path)
    mdk_proj.use_gnu(True)
    mdk_proj.save()


def menuconfig():
    subprocess.run(["menuconfig"], check=True)
    subprocess.run(["python", "kconfig.py"], check=True)


if __name__ == '__main__':
    # Show mr-library logo
    show_logo()

    parser = argparse.ArgumentParser()
    parser.add_argument("-mdk", "--mdk", action="store_true", help="Build with MDK")
    parser.add_argument("-m", "--menuconfig", action="store_true", help="Run menuconfig")

    args = parser.parse_args()
    if args.mdk:
        build_mdk()
    if args.menuconfig:
        menuconfig()
