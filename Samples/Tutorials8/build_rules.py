#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Configuration file on how to build and clean projects in a specific folder.

This file is parsed by the cleanme, buildme, rebuildme and makeprojects
command line tools to clean, build and generate project files.
"""

# pylint: disable=global-statement

from __future__ import absolute_import, print_function, unicode_literals

import os

from makeprojects import ProjectTypes

# Type of the project, default is ProjectTypes.tool
PROJECT_TYPE = ProjectTypes.tool

# Both ``cleanme`` and ``buildme`` will process any child directory with the
# clean(), prebuild(), build(), and postbuild() functions if True.
# Can be overridden above
GENERIC = True

# ``cleanme`` and ``buildme`` will process build_rules.py in the parent folder
# if True. Default is false
# Can be overridden above
CONTINUE = True

# ``cleanme`` will clean the listed folders using their rules before cleaning
# this folder. Overrides DEPENDENCIES
# Clean all the samples
CLEANME_DEPENDENCIES = [
    "Tut01_EnumSP",
    "Tut02_Host",
    "Tut03_EnumHosts",
    "Tut04_Connect",
    "Tut05_Send",
    "Tut06_HostMigration",
    "Tut07_LobbyLaunch",
    "Tut08_Voice",
    "Tut09_ClientServer" + os.sep + "Client",
    "Tut09_ClientServer" + os.sep + "Server",
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

    # Get rid of the "Tut01_" prefix
    name = project.solution.name
    if name.lower().startswith("tut"):
        project.solution.name = name[6:]
        project.name = name[6:]

    # Tutorial applications are tools, since they start with main()
    project.project_type = ProjectTypes.tool
