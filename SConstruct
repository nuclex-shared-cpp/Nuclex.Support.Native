#!/usr/bin/env python

import sys
import importlib

# Nuclex SCons libraries
sys.path.append('../References/scripts/scons')
nuclex = importlib.import_module('nuclex')

# ----------------------------------------------------------------------------------------------- #

universal_target_name = 'Nuclex.Support.Native'
universal_tests_target_name = universal_target_name + ".Tests"

# ----------------------------------------------------------------------------------------------- #

common_environment = nuclex.create_cplusplus_environment()

library_environment = common_environment.Clone()
compile_library_and_tests = library_environment.build_library_with_tests(
    universal_target_name, universal_tests_target_name
)

unit_test_environment = common_environment.Clone()
run_unit_tests = unit_test_environment.run_unit_tests(
    universal_tests_target_name
)

# ----------------------------------------------------------------------------------------------- #

# Dependencies
Depends(run_unit_tests, compile_library_and_tests)

# ----------------------------------------------------------------------------------------------- #

# Always run the unit tests
AlwaysBuild(run_unit_tests)
