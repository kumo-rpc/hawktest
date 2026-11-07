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



############################################################
# C++ tests built with standard compiler flags.

hawk_test(googletest-death-test-test htest_main_static)
hawk_test(htest_environment_test htest_static)
hawk_test(googletest-filepath-test htest_main_static)
hawk_test(googletest-listener-test htest_main_static)
hawk_test(htest_main_unittest htest_main_static)
hawk_test(googletest-message-test htest_main_static)
hawk_test(htest_no_test_unittest htest_static)
hawk_test(googletest-options-test htest_main_static)
hawk_test(googletest-param-test-test htest_static
        googletest-param-test2-test.cc)
hawk_test(googletest-port-test htest_main_static)
hawk_test(htest_pred_impl_unittest htest_main_static)
hawk_test(htest_premature_exit_test htest_static
        htest_premature_exit_test.cc)
hawk_test(googletest-printers-test htest_main_static)
hawk_test(htest_prod_test htest_main_static
        production.cc)
hawk_test(htest_repeat_test htest_static)
hawk_test(htest_sole_header_test htest_main_static)
hawk_test(htest_stress_test htest_static)
hawk_test(googletest-test-part-test htest_main_static)
hawk_test(htest_throw_on_failure_ex_test htest_static)
hawk_test(htest-typed-test_test htest_main_static
        htest-typed-test2_test.cc)
hawk_test(htest_unittest htest_main_static)
hawk_test(htest-unittest-api_test htest_static)
hawk_test(htest_skip_in_environment_setup_test htest_main_static)
hawk_test(htest_skip_test htest_main_static)


############################################################
# Python tests.

test_executable(googletest-break-on-failure-unittest_ hawktest::htest_static)
py_test(googletest-break-on-failure-unittest)

py_test(htest_skip_check_output_test)
py_test(htest_skip_environment_check_output_test)


kmcmake_cc_binary(
        NAME  googletest-catch-exceptions-ex-test_
        SOURCES
        googletest-catch-exceptions-test_.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        hawktest::htest_main_static
)


kmcmake_cc_binary(
        NAME  googletest-catch-exceptions-no-ex-test_
        SOURCES
        googletest-catch-exceptions-test_.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        -fno-exceptions
        LINKS
        hawktest::htest_main_static
)


py_test(googletest-catch-exceptions-test)

test_executable(googletest-color-test_ hawktest::htest_static)
py_test(googletest-color-test)

test_executable(googletest-env-var-test_ hawktest::htest_static)
py_test(googletest-env-var-test)

test_executable(googletest-filter-unittest_ hawktest::htest_static)
py_test(googletest-filter-unittest)

test_executable(htest_help_test_ hawktest::htest_main_static)
py_test(htest_help_test)

test_executable(googletest-list-tests-unittest_ hawktest::htest_static)
py_test(googletest-list-tests-unittest)

test_executable(googletest-output-test_ hawktest::htest_static)
py_test(googletest-output-test --no_stacktrace_support)

test_executable(googletest-shuffle-test_ hawktest::htest_static)
py_test(googletest-shuffle-test)


test_executable(googletest-uninitialized-test_ hawktest::htest_static)
py_test(googletest-uninitialized-test)

test_executable(htest_list_output_unittest_ hawktest::htest_static)
py_test(htest_list_output_unittest)

test_executable(htest_xml_outfile1_test_ hawktest::htest_main_static)
test_executable(htest_xml_outfile2_test_ hawktest::htest_main_static)
py_test(htest_xml_outfiles_test)
py_test(googletest-json-outfiles-test)

test_executable(htest_xml_output_unittest_ hawktest::htest_static)
py_test(htest_xml_output_unittest --no_stacktrace_support)
py_test(googletest-json-output-unittest --no_stacktrace_support)
