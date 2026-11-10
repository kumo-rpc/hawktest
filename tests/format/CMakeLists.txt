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

kmcmake_cc_test(
        NAME printer_test
        MODULE format
        SOURCES printer_test.cc
        DEFINES HAWK_FORMAT_MARK="${CMAKE_SOURCE_DIR}/tests/format/printer.md"
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${PROJECT_NAME}::hawktest_static hawktest::htest_main_static
)

kmcmake_cc_test(
        NAME should_use_color_test
        MODULE format
        SOURCES should_use_color_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${PROJECT_NAME}::hawktest_static hawktest::htest_main_static
)

test_executable(hawktest-color-test_ ${PROJECT_NAME}::hawktest_static)
py_test(hawktest-color-test)
