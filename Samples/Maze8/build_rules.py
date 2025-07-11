#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Configuration file on how to build and clean projects in a specific folder.

This file is parsed by the cleanme, buildme, rebuildme and makeprojects
command line tools to clean, build and generate project files.
"""

# pylint: disable=global-statement
# pylint: disable=unused-argument

from __future__ import absolute_import, print_function, unicode_literals

import os

from makeprojects import ProjectTypes

# ``cleanme`` and ``buildme`` will process build_rules.py in the parent folder
# if True. Default is false
# Can be overridden above
CONTINUE = True

# Both ``cleanme`` and ``buildme`` will process any child directory with the
# clean(), prebuild(), build(), and postbuild() functions if True.
# Can be overridden above
GENERIC = True

# ``cleanme`` will clean the listed folders using their rules before cleaning
# this folder. Overrides DEPENDENCIES
CLEANME_DEPENDENCIES = [
    "MazeClient",
    "MazeConsoleClient",
    "MazeServer"
]

########################################


def project_settings(project):
    """
    Set up defines and default libraries.

    Adjust the default settings for the project to generate. Usually it's
    setting the location of source code or perforce support.

    Args:
        project: Project record to update.

    Returns:
        None, to continue processing, zero is no error and stop processing,
        any other number is an error code.
    """

    this_dir = os.path.dirname(os.path.abspath(__file__))
    dir_prefix = os.path.relpath(this_dir, project.working_directory)
    common_dir = dir_prefix + os.sep + "MazeCommon"

    # The server only needs a subset of the common files
    name = project.solution.name.lower()
    if name in ("mazeserver",):
        common_dir = common_dir + os.sep

        project.source_files_list.extend((
            common_dir + "Maze.cpp",
            common_dir + "Maze.h",
            common_dir + "MazeServer.cpp",
            common_dir + "MazeServer.h",
            common_dir + "NetAbstract.h",
            common_dir + "Packets.h",
            common_dir + "Random.h",
            common_dir + "SyncObjects.h",
            common_dir + "Trig.h"))

    else:
        # Makeprojects defaults to the "source" folder, add the
        # windows folder
        project.source_folders_list.append(common_dir)


########################################


def configuration_settings(configuration):
    """
    Set up defines and libraries on a configuration basis.

    For each configation, set all configuration specific seting. Use
    configuration.name to determine which configuration is being processed.

    Args:
        configuration: Configuration class instance to update.

    Returns:
        None, to continue processing, zero is no error and stop processing,
        any other number is an error code.
    """

    # Console apps are tools
    name = configuration.project.solution.name.lower()
    if name in ("mazeserver", "mazeconsoleclient"):
        # These are tools
        configuration.project_type = ProjectTypes.tool

    # Maze used D3D8
    configuration.define_list.append("DIRECT3D_VERSION=0x800")
