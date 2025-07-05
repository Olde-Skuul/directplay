#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Build rules for the makeprojects suite of build tools.

This file is parsed by the cleanme, buildme, rebuildme and makeprojects
command line tools to clean, build and generate project files.

When any of these tools are invoked, this file is loaded and parsed to
determine special rules on how to handle building the code and / or data.

When adding this folder as a library, add either or both of these attributes to
``configuration`` to change the behavior below

Test for ``directplay_headers_only = True`` to disable adding libraries

Test for ``directplay_classic_lib = True`` to use dplay.lib instead of
dplayx.lib
"""

# pylint: disable=unused-argument
# pylint: disable=consider-using-f-string

from __future__ import absolute_import, print_function, unicode_literals

import sys
import os

from makeprojects import PlatformTypes, IDETypes
from burger import environment_root

# Set the directory this script resides in
THIS_DIRECTORY = os.path.dirname(os.path.abspath(__file__))

# If any IDE file is present, cleanme and buildme will process them.
# Can be overridden above
PROCESS_PROJECT_FILES = False

# ``cleanme`` will clean the listed folders using their rules before cleaning
# this folder. Overrides DEPENDENCIES
# Clean all the samples.
if os.path.isdir(os.path.join(THIS_DIRECTORY, "Samples")):
    # Clean all the samples
    CLEANME_DEPENDENCIES = ["Samples"]
else:
    # If set to True, Don't parse directories in this folder when ``-r``
    # is active.
    # Can be overridden above
    NO_RECURSE = True

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

    # pylint: disable=too-many-branches

    # Get the directory of the project being processed and compare it
    # to either BURGER_SDKS (Olde Skuul's common folder) or if it's
    # a folder from a git repository.
    root_path, env_var = environment_root(
        THIS_DIRECTORY, "BURGER_SDKS", configuration.working_directory)

    # Folders for the libraries and includes
    lib_dir = root_path + os.sep + "Lib" + os.sep
    include_dir = root_path + os.sep + "Include"

    # Check if BURGER_SDKS was uses, and also check if it's not already
    # in the variable list
    if env_var and "BURGER_SDKS" not in configuration.env_variable_list:
        # Add code in the project to test for the existence of BURGER_SDKS
        # and generate an error if not found
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
            lib_dir = lib_dir + "x86"
        else:
            lib_dir = lib_dir + "x64"

        # First, check if only directplay headers are added.
        # If libraries are not requested, skip them
        if not getattr(configuration, "directplay_headers_only", False):

            # Select directplay classic or the latest version
            if getattr(configuration, "directplay_classic_lib", False):
                item = "dplay.lib"
            else:
                item = "dplayx.lib"
            configuration.libraries_list.append(item)

            # Make sure this folder is found first to override older versions
            # on older compilers
            configuration.library_folders_list.insert(0, lib_dir)

        # Include headers, however Codewarrior uses the library folder
        # Insert at the beginning to ensure this folder is scanned first
        if configuration.project.solution.ide.is_codewarrior():
            configuration.library_folders_list.insert(0, include_dir)
        else:
            configuration.include_folders_list.insert(0, include_dir)

        # Enable the higher APIs with IDIRECTPLAY2_OR_GREATER if using
        # dplayx.dll
        if not getattr(configuration, "directplay_classic_lib", False):
            if "IDIRECTPLAY2_OR_GREATER" not in configuration.define_list:
                configuration.define_list.append("IDIRECTPLAY2_OR_GREATER")

        # Done, and allow further processing
        return None

    # All other Windows platforms are not supported by Directplay
    if platform.is_windows():

        # These IDEs only support 32 bit Intel, so don't generate warnings
        # for ARM Windows
        if ide.is_codewarrior() or ide is IDETypes.watcom:
            return None

        print("# Error: Directplay for Windows is Intel only")

        # Stop processing
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
        "When adding this folder as a library, add either or both of these attributes to\n"
        "configuration to change the behavior below:\n\n"
        "configuration.directplay_headers_only = True # Disable adding libraries\n"
        "configuration.directplay_classic_lib = True  # Use dplay.lib instead of\n"
        "\tdplayx.lib\n\n"
        "On ARM Windows platforms, it will generate an error since Directplay is not\n"
        "supported on native ARM executables for Windows."
    )
    return 0

########################################


# If called as a command line and not a class, perform the build
if __name__ == "__main__":
    sys.exit(command_line())
