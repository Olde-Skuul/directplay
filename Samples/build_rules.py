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

from burger import clean_directories, is_under_git_control, clean_files, \
    clean_codeblocks, clean_xcode
from makeprojects import ProjectTypes, PlatformTypes

# Type of the project, default is ProjectTypes.tool
PROJECT_TYPE = ProjectTypes.app

# Recommend target platform for the project.
PROJECT_PLATFORM = PlatformTypes.win32

# ``cleanme`` and ``buildme`` will process build_rules.py in the parent folder
# if True. Default is false
# Can be overridden above
CONTINUE = False

# Both ``cleanme`` and ``buildme`` will process any child directory with the
# clean(), prebuild(), build(), and postbuild() functions if True.
# Can be overridden above
GENERIC = True

# ``cleanme`` will assume only the function ``clean()`` is used if False.
# Overrides PROCESS_PROJECT_FILES
CLEANME_PROCESS_PROJECT_FILES = False

# ``cleanme`` will clean the listed folders using their rules before cleaning
# this folder. Overrides DEPENDENCIES
# Clean all the samples
CLEANME_DEPENDENCIES = [
    "bellhop",
    "chatconnect",
    "dpchat",
    "dplaunch",
    "dpslots",
    "duel",
    "override",
    "simpleconnect",
    "stagedconnect"
]

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

# Check if git is around
_GIT_FOUND = None

# List of projects that use common code
# _ADD_COMMON = (
#    "chatconnect",
#    "duel",
#    "simpleconnect"
# )

# Add in dxutil.h and dxutil.cpp for these projects
_ADD_DXUTIL = (
    "addressoverride8",
    "chatpeer8",
    "datarelay8",
    "lobbyclient8",
    "mazeclient",
    "mazeconsoleclient",
    "mazeserver",
    "simpleclient",
    "simpleserver",
    "simplepeer8",
    "stagedpeer8",
    "voiceclient",
    "voiceserver",
    "voiceconnect8",
    "voicegroup8",
    "voiceposition8"
)

# Add in dpdialogs.cpp
_ADD_DPDIALOGS = (
    "addressoverride8",
    "datarelay8",
    "simpleclient",
    "simplepeer8",
    "stagedpeer8"
)

# Add in dputils.cpp
_ADD_DPUTILS = (
    "addressoverride8",
    "lobbyclient8",
)

# Add in netconnect.h
_ADD_NETCONNECT = (
    "chatpeer8",
    "datarelay8",
    "simplepeer8",
    "stagedpeer8",
    "voiceclient",
    "voiceserver",
    "voiceconnect8",
    "voicegroup8",
    "voiceposition8"
)

# Add in netclient.h
_ADD_NETCLIENT = (
    "simpleclient",
    "voiceclient",
    "voiceserver",
)

# Add in netvoice.h
_ADD_NETVOICE = (
    "voiceclient",
    "voiceserver",
    "voiceconnect8",
    "voicegroup8",
    "voiceposition8",
)

# Add in directx.ico
_ADD_ICON = (
    "addressoverride8",
)

########################################


def is_git(working_directory):
    """
    Detect if perforce or git is source control

    Returns:
        True if found, False if not.
    """

    # Cached result
    global _GIT_FOUND

    if _GIT_FOUND is None:
        _GIT_FOUND = is_under_git_control(working_directory)
    return _GIT_FOUND

########################################


def clean(working_directory):
    """
    Delete temporary files.

    This function is called by ``cleanme`` to remove temporary files.

    On exit, return 0 for no error, or a non zero error code if there was an
    error to report.

    Args:
        working_directory
            Directory this script resides in.

    Returns:
        None if not implemented, otherwise an integer error code.
    """

    clean_directories(
        working_directory,
        (".vscode", "temp", "ipch", "bin", ".vs", "*_Data",
         "* Data", "__pycache__"))

    clean_files(
        working_directory,
        (".DS_Store", "*.suo", "*.user", "*.ncb", "*.err",
         "*.sdf", "*.layout.cbTemp", "*.VC.db", "*.pyc", "*.pyo"))

    # If Doxygen was found using this directory, clean up
    if os.path.isfile(os.path.join(working_directory, "Doxyfile")):
        clean_files(
            working_directory,
            ("doxygenerrors.txt", "*.chm", "*.chw", "*.tmp"),
            recursive=True)

    # Check if the directory has a codeblocks project file and clean
    # codeblocks extra files
    clean_codeblocks(working_directory)

    # Allow purging user data in XCode project directories
    clean_xcode(working_directory)

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

    # Makeprojects defaults to perforce on
    project.solution.perforce = not is_git(project.working_directory)

    # Makeprojects defaults to the "source" folder, add the
    # windows folder
    project.source_folders_list.append("source\\windows")

    # Determine the common folder's position relative to the project
    this_dir = os.path.dirname(os.path.abspath(__file__))
    dir_prefix = os.path.relpath(this_dir, project.working_directory)
    common_dir = dir_prefix + os.sep + "common" + os.sep

    # Convert to lower case for case insensive matching
    project_name = project.name.lower()

    # Every project includes this header
    project.source_files_list.append(
        common_dir + "dpmacros.h"
    )

    # Add in the folder with the common code
    # if project_name in _ADD_COMMON:
    #    project.source_folders_list.append(common_dir)

    # Add in dxutil
    if project_name in _ADD_DXUTIL:
        project.source_files_list.extend((
            common_dir + "dxutil.cpp",
            common_dir + "dxutil.h"))

    # Add in dpdialogs
    if project_name in _ADD_DPDIALOGS:
        project.source_files_list.extend((
            common_dir + "dpdialogs.cpp",
            common_dir + "dpdialogs.h"))

    # Add in dputils
    if project_name in _ADD_DPUTILS:
        project.source_files_list.extend((
            common_dir + "dputils.cpp",
            common_dir + "dputils.h"))

    # Add in netconnect
    if project_name in _ADD_NETCONNECT:
        project.source_files_list.extend((
            common_dir + "netconnect.cpp",
            common_dir + "netconnect.h",
            common_dir + "netconnectres.h"))

    # Add in netconnect
    if project_name in _ADD_NETCLIENT:
        project.source_files_list.extend((
            common_dir + "netclient.cpp",
            common_dir + "netclient.h",
            common_dir + "netclientres.h"))

    # Add in netvoice
    if project_name in _ADD_NETVOICE:
        project.source_files_list.extend((
            common_dir + "netvoice.cpp",
            common_dir + "netvoice.h"))
        project.libraries_list.append("dsound.lib")

    # Add in the directx.ico file
    if project_name in _ADD_ICON:
        project.source_files_list.append(
            common_dir + "directx.ico")

    # Disable Visual Studio warnings
    if project.platform.is_windows():
        project.define_list.extend((
            "_CRT_NONSTDC_NO_WARNINGS",
            "_CRT_SECURE_NO_WARNINGS"))

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

    # Where is this file?
    this_dir = os.path.dirname(os.path.abspath(__file__))
    this_dir = os.path.dirname(this_dir)

    # Add the root folder to link in directplay library
    configuration.library_rules_list.append(this_dir)
