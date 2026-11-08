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

if (WIN32 AND NOT CYGWIN)
    set(SLASH "\\\\")
else ()
    set(SLASH "/")
endif ()

set(_flags_test_libs hawktest::hawktest_static)

kmcmake_cc_binary(
        NAME gflags_unittest
        SOURCES gflags_unittest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${_flags_test_libs}
)

configure_file(gflags_unittest.cc ${CMAKE_CURRENT_SOURCE_DIR}/gflags_unittest-main.cc COPYONLY)
configure_file(gflags_unittest.cc ${CMAKE_CURRENT_SOURCE_DIR}/gflags_unittest_main.cc COPYONLY)

kmcmake_cc_binary(
        NAME gflags_unittest-main
        SOURCES gflags_unittest-main.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${_flags_test_libs}
)

kmcmake_cc_binary(
        NAME gflags_unittest_main
        SOURCES gflags_unittest_main.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${_flags_test_libs}
)

kmcmake_cc_binary(
        NAME gflags_declare_test
        SOURCES gflags_declare_test.cc gflags_declare_flags.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${_flags_test_libs}
)

kmcmake_cc_binary(
        NAME gflags_strip_flags_test
        SOURCES gflags_strip_flags_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${_flags_test_libs}
)

macro(add_flags_test name expected_rc expected_output unexpected_output cmd)
    set(args "--test_tmpdir=${CMAKE_BINARY_DIR}/Testing/Temporary"
            "--srcdir=${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(
            NAME flags_${name}
            COMMAND "${CMAKE_COMMAND}" "-DCOMMAND:STRING=$<TARGET_FILE:${cmd}>;${args};${ARGN}"
            "-DEXPECTED_RC:STRING=${expected_rc}"
            "-DEXPECTED_OUTPUT:STRING=${expected_output}"
            "-DUNEXPECTED_OUTPUT:STRING=${unexpected_output}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/execute_test.cmake"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endmacro()

add_flags_test(strip_flags_help 1 "" "This text should be stripped out" gflags_strip_flags_test --help)
add_test(
        NAME flags_strip_flags_binary
        COMMAND "${CMAKE_COMMAND}" "-DBINARY=$<TARGET_FILE:gflags_strip_flags_test>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/gflags_strip_flags_test.cmake"
        CONFIGURATIONS Release MinSizeRel
)

add_flags_test(unittest 0 "" "" gflags_unittest)
add_flags_test(help-reporting 1 "${SLASH}gflags_reporting.cc:" "" gflags_unittest --help)
add_flags_test(long-helpstring 1 "end of a long helpstring" "" gflags_unittest --help)
add_flags_test(changed_bool1 1 "-changed_bool1 (changed) type: bool default: true" "" gflags_unittest --help)
add_flags_test(changed_bool2 1 "-changed_bool2 (changed) type: bool default: false currently: true" "" gflags_unittest --help)
add_flags_test(changeable_string_var 1 "-changeable_string_var () type: string default: \"1\" currently: \"2\"" "" gflags_unittest --changeable_string_var 2 --help)
add_flags_test(nohelp 0 "PASS" "" gflags_unittest --nohelp)
add_flags_test(help=false 0 "PASS" "" gflags_unittest --help=false)
add_flags_test(helpfull 1 "${SLASH}gflags_reporting.cc:" "" gflags_unittest --helpfull)
add_flags_test(helpshort 1 "${SLASH}gflags_unittest.cc:" "${SLASH}gflags_reporting.cc:" gflags_unittest --helpshort)
add_flags_test(helpshort-tldflag1 1 "tldflag1" "${SLASH}google.cc:" gflags_unittest --helpshort)
add_flags_test(helpshort-tldflag2 1 "tldflag2" "${SLASH}google.cc:" gflags_unittest --helpshort)
add_flags_test(helpshort-main 1 "${SLASH}gflags_unittest-main.cc:" "${SLASH}gflags_reporting.cc:" gflags_unittest-main --helpshort)
add_flags_test(helpshort_main 1 "${SLASH}gflags_unittest_main.cc:" "${SLASH}gflags_reporting.cc:" gflags_unittest_main --helpshort)
add_flags_test(helpon 1 "'--helpon' is missing its argument; flag description: show help on" "" gflags_unittest --helpon)
add_flags_test(helpon=gflags 1 "${SLASH}gflags.cc:" "${SLASH}gflags_unittest.cc:" gflags_unittest --helpon=gflags)
add_flags_test(helpon_gflags 1 "${SLASH}gflags.cc:" "${SLASH}gflags_unittest.cc:" gflags_unittest --helpon gflags)
add_flags_test(helpon=gflags_unittest 1 "${SLASH}gflags_unittest.cc:" "${SLASH}gflags.cc:" gflags_unittest --helpon=gflags_unittest)
add_flags_test(helpmatch_reporting 1 "${SLASH}gflags_reporting.cc:" "${SLASH}gflags_unittest.cc:" gflags_unittest -helpmatch reporting)
add_flags_test(helpmatch=unittest 1 "${SLASH}gflags_unittest.cc:" "${SLASH}gflags.cc:" gflags_unittest -helpmatch=unittest)
add_flags_test(helpmatch=nosuchsubstring 1 "No modules matched" "${SLASH}gflags_unittest.cc:" gflags_unittest -helpmatch=nosuchsubstring)
add_flags_test(helpon=nosuchmodule 1 "No modules matched" "${SLASH}gflags_unittest.cc:" gflags_unittest -helpon=nosuchmodule)
add_flags_test(helppackage 1 "${SLASH}gflags_reporting.cc:" "" gflags_unittest --helppackage)
add_flags_test(helpxml 1 "${SLASH}gflags_unittest.cc</file>" "${SLASH}gflags_unittest.cc:" gflags_unittest --helpxml)
add_flags_test(version-1 0 "gflags_unittest" "${SLASH}gflags_unittest.cc:" gflags_unittest --version)
add_flags_test(version-2 0 "version test_version" "${SLASH}gflags_unittest.cc:" gflags_unittest --version)
add_flags_test(undefok-1 1 "unknown command line flag 'foo'" "" gflags_unittest --undefok= --foo --unused_bool)
add_flags_test(undefok-2 0 "PASS" "" gflags_unittest --undefok=foo --foo --unused_bool)
add_flags_test(undefok-3 0 "PASS" "" gflags_unittest --undefok=foo --nofoo --unused_bool)
add_flags_test(undefok-4 0 "PASS" "" gflags_unittest --undefok=fee,fi,foo,fum --foo --unused_bool)
add_flags_test(undefok-5 1 "unknown command line flag 'foo'" "" gflags_unittest --undefok=fo --foo --unused_bool)
add_flags_test(undefok-6 1 "unknown command line flag 'foo'" "" gflags_unittest --undefok=foot --foo --unused_bool)
add_flags_test(flagfile.1 0 "gflags_unittest" "${SLASH}gflags_unittest.cc:" gflags_unittest "--flagfile=flagfile.1")
add_flags_test(flagfile.2 0 "PASS" "" gflags_unittest "--flagfile=flagfile.2")
add_flags_test(flagfile.3 0 "PASS" "" gflags_unittest "--flagfile=flagfile.3")
add_flags_test(fromenv=version 0 "gflags_unittest" "${SLASH}gflags_unittest.cc:" gflags_unittest --fromenv=version)
add_flags_test(tryfromenv=version 0 "gflags_unittest" "${SLASH}gflags_unittest.cc:" gflags_unittest --tryfromenv=version)
add_flags_test(fromenv=help 0 "PASS" "" gflags_unittest --fromenv=help)
add_flags_test(tryfromenv=help 0 "PASS" "" gflags_unittest --tryfromenv=help)
add_flags_test(fromenv=helpfull 1 "helpfull not found in environment" "" gflags_unittest --fromenv=helpfull)
add_flags_test(tryfromenv=helpfull 0 "PASS" "" gflags_unittest --tryfromenv=helpfull)
add_flags_test(tryfromenv=undefok 0 "PASS" "" gflags_unittest --tryfromenv=undefok --foo)
add_flags_test(tryfromenv=weirdo 1 "unknown command line flag" "" gflags_unittest --tryfromenv=weirdo)
add_flags_test(tryfromenv-multiple 0 "gflags_unittest" "${SLASH}gflags_unittest.cc:" gflags_unittest --tryfromenv=test_bool,version,unused_bool)
add_flags_test(fromenv=test_bool 1 "not found in environment" "" gflags_unittest --fromenv=test_bool)
add_flags_test(fromenv=test_bool-ok 1 "unknown command line flag" "" gflags_unittest --fromenv=test_bool,ok)
add_flags_test(version-overrides-fromenv 0 "gflags_unittest" "${SLASH}gflags_unittest.cc:" gflags_unittest --fromenv=test_bool,version,ok)
add_flags_test(dashdash 0 "PASS" "" gflags_unittest -- --help)
add_flags_test(always_fail 1 "ERROR: failed validation of new value 'true' for flag 'always_fail'" "" gflags_unittest --always_fail)

add_test(NAME flags_declare COMMAND gflags_declare_test --message "Hello gflags!")
set_tests_properties(flags_declare PROPERTIES
        PASS_REGULAR_EXPRESSION "Hello gflags!"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
