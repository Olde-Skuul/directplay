#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Configuration file on how to build and clean projects in a specific folder.

This file is parsed by the cleanme, buildme, rebuildme and makeprojects
command line tools to clean, build and generate project files.
"""

# pylint: disable=global-statement
# pylint: disable=unused-argument
# pylint: disable=consider-using-f-string

from __future__ import absolute_import, print_function, unicode_literals

import sys
import os

from burger import create_folder_if_needed, save_text_file

# ``cleanme`` and ``buildme`` will process build_rules.py in the parent folder
# if True. Default is false
# Can be overridden above
CONTINUE = True

# List of projects to generate if makeprojects is invoked
# without any parameters, default create recommended
# project for the host machine

# Create windows projects for Watcom, VS 2022, and Codewarrior
MAKEPROJECTS = (
    {"platform": "win32",
     "ide": ("vs2003", "vs2022"),
     "type": "app",
     "configuration": "Release_LTCG"},
    {"platform": "win32",
     "ide": ("watcom", "codewarrior"),
     "type": "app",
     "configuration": "Release"
     }
)

########################################


def prebuild(working_directory, configuration):
    """
    Perform actions before building any IDE based projects.

    This function is called before any IDE or other script is invoked. This is
    perfect for creating headers or other data that the other build projects
    need before being invoked.

    On exit, return 0 for no error, or a non zero error code if there was an
    error to report.

    Args:
        working_directory
            Directory this script resides in.

        configuration
            Configuration to build, ``all`` if no configuration was requested.

    Returns:
        None if not implemented, otherwise an integer error code.
    """

    # Create the output folder if needed
    destfolder = os.path.join(working_directory, "bin")
    create_folder_if_needed(destfolder)

    # If there is a database file?
    database_file = os.path.join(destfolder, "slotsdb.txt")
    if not os.path.isfile(database_file):

        # Get the currently logged in player
        player = os.getlogin()
        if not player:
            player = os.environ["USERNAME"]

        # If there was someone logged in...
        if player:
            # Create the database file with 10000 credits
            lines = ["{}, 10000".format(player.upper())]
            save_text_file(database_file, lines)
    return 0


# If called as a command line and not a class, perform the build

if __name__ == "__main__":
    sys.exit(prebuild(os.path.dirname(os.path.abspath(__file__)), None))
