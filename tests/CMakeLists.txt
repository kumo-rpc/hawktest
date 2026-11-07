# Copyright (C) Kumo inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

configure_file(${PROJECT_SOURCE_DIR}/tests/config.h.in ${PROJECT_SOURCE_DIR}/tests/config.h @ONLY)

function(hawk_test name libs)
    kmcmake_cc_test(
            NAME ${name}
            MODULE ut
            SOURCES
            ${name}.cc
            ${ARGN}
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            LINKS
            ${libs}
    )
endfunction()


# py_test(name)
#
# Creates a Python test with the given name whose main module is in
# test/name.py. It does nothing if Python is not installed.
find_package(Python3 COMPONENTS Interpreter QUIET)
function(py_test name)
    if (NOT Python3_Interpreter_FOUND)
        message(WARNING "python not found")
        return()
    endif()

    get_cmake_property(is_multi "GENERATOR_IS_MULTI_CONFIG")
    set(build_dir "${CMAKE_CURRENT_BINARY_DIR}")
    if (is_multi)
        set(build_dir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
    endif()

    add_test(NAME ${name}
            COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/${name}.py
            --build_dir=${build_dir} ${ARGN})

    # Make the Python import path consistent between Bazel and CMake.
    set_tests_properties(${name} PROPERTIES ENVIRONMENT PYTHONPATH=${CMAKE_SOURCE_DIR})
endfunction()



function(test_executable name libs)
    kmcmake_cc_binary(
            NAME ${name}
            SOURCES
            ${name}.cc
            ${ARGN}
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            LINKS
            ${libs}
    )
endfunction()

add_subdirectory(test)
add_subdirectory(mock)
add_subdirectory(bench)