#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@copyright (c) 2023-2024, MR Development Team

@license SPDX-License-Identifier: Apache-2.0

@date 2023-12-17    MacRsh       First version
"""

import re
from pathlib import Path
from kconfiglib import Kconfig


def generate_config(configfile: Path):
    kconf = Kconfig("Kconfig", warn=False, warn_to_stderr=False)

    # Load config file
    kconf.load_config(".config")
    kconf.write_config(".config")
    kconf.write_autoconf(configfile)

    with open(configfile, 'r+') as file:
        content = file.read()
        file.truncate(0)
        file.seek(0)

        # Writes file header
        file.write("#ifndef __MR_CONFIG_H__\n")
        file.write("#define __MR_CONFIG_H__\n\n")

        # Writes cplusplus header
        file.write("#ifdef __cplusplus\n")
        file.write("extern \"C\" {\n")
        file.write("#endif /* __cplusplus */\n\n")

        # Writes the formatted context
        content = content.replace("#define CONFIG_", "#define ")
        content = re.sub(r'#define MR_USE_(\w+) (\d+)', r'#define MR_USE_\1',
                         content)
        file.write(content)

        # Writes cplusplus footer
        file.write("\n#ifdef __cplusplus\n")
        file.write("}\n")
        file.write("#endif /* __cplusplus */\n\n")

        # Writes file footer
        file.write("#endif /* __MR_CONFIG_H__ */\n")
