#!/usr/bin/env python

"""
@copyright (c) 2023, MR Development Team

@license SPDX-License-Identifier: Apache-2.0

@date 2023-12-17    MacRsh       First version
"""

import re

try:
    from build import install_package
except ImportError:
    exit(1)

try:
    from kconfiglib import Kconfig
except ImportError:
    install_package('kconfiglib')
    from kconfiglib import Kconfig

try:
    import curses
except ImportError:
    install_package('windows-curses')
    import curses

try:
    from build import log_print
except ImportError:
    exit(1)


def generate_config(kconfig_file, config_in, config_out, header_out):
    kconf = Kconfig(kconfig_file, warn=False, warn_to_stderr=False)

    # Load config
    kconf.load_config(config_in)
    kconf.write_config(config_out)
    kconf.write_autoconf(header_out)

    with open(header_out, 'r+') as header_file:
        content = header_file.read()
        header_file.truncate(0)
        header_file.seek(0)

        # Remove CONFIG_ and MR_USING_XXX following number
        content = content.replace("#define CONFIG_", "#define ")
        content = re.sub(r'#define MR_USING_(\w+) (\d+)', r'#define MR_USING_\1', content)

        # Add the micro
        header_file.write("#ifndef _MR_CONFIG_H_\n")
        header_file.write("#define _MR_CONFIG_H_\n\n")

        header_file.write("#ifdef __cplusplus\n")
        header_file.write("extern \"C\" {\n")
        header_file.write("#endif /* __cplusplus */\n\n")

        # Write back the original data
        header_file.write(content)

        # Add the micro
        header_file.write("\n#ifdef __cplusplus\n")
        header_file.write("}\n")
        header_file.write("#endif /* __cplusplus */\n\n")
        header_file.write("#endif /* _MR_CONFIG_H_ */\n")

        header_file.close()
        log_print('success', "menuconfig %s make success" % header_out)


def main():
    kconfig_file = 'Kconfig'
    config_in = '.config'
    config_out = '.config'
    header_out = 'include/mr_config.h'
    generate_config(kconfig_file, config_in, config_out, header_out)


if __name__ == "__main__":
    main()
