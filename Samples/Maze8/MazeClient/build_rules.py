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

from makeprojects import ProjectTypes

# Type of the project, default is ProjectTypes.tool
PROJECT_TYPE = ProjectTypes.app

# ``cleanme`` and ``buildme`` will process build_rules.py in the parent folder
# if True. Default is false
# Can be overridden above
CONTINUE = True

# Create windows projects for Watcom, VS 2022, and Codewarrior
MAKEPROJECTS = [
    {"platform": "win32",
     "ide": "vs2003",
     "type": "app",
     "configuration": "Release_LTCG"}
]

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

    # MazeClient need d3d8
    configuration.libraries_list.append("d3d8.lib")
    configuration.libraries_list.append("d3dx8.lib")
    configuration.libraries_list.append("d3dxof.lib")
