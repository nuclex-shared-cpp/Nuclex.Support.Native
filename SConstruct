#!/usr/bin/env python

import sys
import importlib
import os

# Nuclex SCons libraries
sys.path.append('../References/scripts/scons')
nuclex = importlib.import_module('nuclex')

# ----------------------------------------------------------------------------------------------- #

# Standard C/C++ build environment with Nuclex extension methods
common_environment = nuclex.create_cplusplus_environment()
common_environment['ENV'] = os.environ
common_environment['CXX'] = 'clang++'

# Compile the main library
library_environment = common_environment.Clone()
library_artifacts = library_environment.build_library('Nuclex.Support.Native')

# Compile the unit test executable
unit_test_environment = common_environment.Clone()
unit_test_environment.add_preprocessor_constant('NUCLEX_SUPPORT_EXECUTABLE')
unit_test_artifacts = unit_test_environment.build_unit_tests(
    'Nuclex.Support.Native.Tests'
)

# ----------------------------------------------------------------------------------------------- #
