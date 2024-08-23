#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Build rules for the makeprojects suite of build tools.

This file is parsed by the cleanme, buildme, rebuildme and makeprojects
command line tools to clean, build and generate project files.

When any of these tools are invoked, this file is loaded and parsed to
determine special rules on how to handle building the code and / or data.
"""

# pylint: disable=unused-argument
# pylint: disable=consider-using-f-string

from __future__ import absolute_import, print_function, unicode_literals

import sys
import os

from makeprojects import PlatformTypes, IDETypes
from burger import environment_root

# If any IDE file is present, cleanme and buildme will process them.
# Can be overridden above
PROCESS_PROJECT_FILES = False


########################################


def library_settings(configuration):
    """
    Add settings when using this project at a library

    When configuration.library_rules_list[] is set to a list of
    directories, if the directory has a build_rules.py file, it
    will run this function on every configuration to add the
    library this rules file describes.

    Args:
        configuration: Configuration class instance to update.

    Returns:
        None, to continue processing, zero is no error and stop processing,
        any other number is an error code.
    """

    # Where is this SDK located?
    this_dir = os.path.dirname(os.path.abspath(__file__))

    # Determine the root if it's an environment variable
    root_path, env_var = environment_root(
        this_dir, "BURGER_SDKS", configuration.working_directory)

    # Folders for the libraries and includes
    lib_dir = root_path + "\\Lib"
    include_dir = root_path + "\\Include"

    # Since it will need an environment variable, generate rules
    # to test for its existence
    if env_var and "BURGER_SDKS" not in configuration.env_variable_list:
        configuration.env_variable_list.append("BURGER_SDKS")

    # Get the IDE
    ide = configuration.project.solution.ide

    # Get the platform
    platform = configuration.platform

    # Only modify Windows Intel projects
    if platform in (PlatformTypes.win32, PlatformTypes.win64):

        # The libraries are in different folders for the
        # Intel CPU used
        if platform is PlatformTypes.win32:
            lib_dir = lib_dir + "\\x86"
        else:
            lib_dir = lib_dir + "\\x64"

        # Add in the real library file and directory for Windows
        configuration.libraries_list.append("dplayx.lib")
        configuration.library_folders_list.insert(0, lib_dir)

        # Include headers, however Codewarrior uses the library folder
        if configuration.project.solution.ide.is_codewarrior():
            configuration.library_folders_list.insert(0, include_dir)
        else:
            configuration.include_folders_list.insert(0, include_dir)

        # Enable the higher APIs with IDIRECTPLAY2_OR_GREATER
        if "IDIRECTPLAY2_OR_GREATER" not in configuration.define_list:
            configuration.define_list.append("IDIRECTPLAY2_OR_GREATER")
        return None

    # All other Windows platforms are not supported by Directplay
    if platform.is_windows():

        # These IDEs only support 32 bit Intel
        if ide.is_codewarrior() or ide is IDETypes.watcom:
            return None

        print("# Error: Directplay for Windows is Intel only")
        return 15

    # Otherwise, Directplay is not available
    # pylint: disable=line-too-long
    print(
        "# Error: Directplay is only available on Windows Intel platforms")
    return None

########################################


def command_line():
    """
    Called when directly executed.
    """

    # pylint: disable=line-too-long

    # Print usage
    print(
        "This file contains the rules to add Microsoft Directplay to your makeprojects\n"
        "project. On Windows Intel platforms, it will add the Microsoft Directplay\n"
        "library settings.\n\n"
        "On ARM Windows platforms, it will generate an error since Directplay is not\n"
        "supported on native ARM executables for Windows."
    )
    return 0

########################################


# If called as a command line and not a class, perform the build
if __name__ == "__main__":
    sys.exit(command_line())
