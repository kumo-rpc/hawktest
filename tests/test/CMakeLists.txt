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

hawk_test(hawktest-death-test-test hawktest::htest_main_static)
hawk_test(htest_environment_test hawktest::hawktest_static)
hawk_test(hawktest-listener-test hawktest::htest_main_static)
hawk_test(htest_main_unittest hawktest::htest_main_static)
hawk_test(hawktest-message-test hawktest::htest_main_static)
hawk_test(htest_no_test_unittest hawktest::hawktest_static)
hawk_test(hawktest-options-test hawktest::htest_main_static)
hawk_test(hawktest-param-test-test hawktest::hawktest_static
        hawktest-param-test2-test.cc)
hawk_test(hawktest-port-test hawktest::htest_main_static)
hawk_test(htest_pred_impl_unittest hawktest::htest_main_static)
hawk_test(htest_premature_exit_test hawktest::hawktest_static
        htest_premature_exit_test.cc)
hawk_test(hawktest-printers-test hawktest::htest_main_static)
hawk_test(htest_prod_test hawktest::htest_main_static
        production.cc)
hawk_test(htest_repeat_test hawktest::hawktest_static)
hawk_test(htest_sole_header_test hawktest::htest_main_static)
hawk_test(htest_stress_test hawktest::hawktest_static)
hawk_test(hawktest-test-part-test hawktest::htest_main_static)
hawk_test(htest_throw_on_failure_ex_test hawktest::hawktest_static)
hawk_test(htest-typed-test_test hawktest::htest_main_static
        htest-typed-test2_test.cc)
hawk_test(htest_unittest hawktest::htest_main_static)
hawk_test(htest-unittest-api_test hawktest::hawktest_static)
hawk_test(htest_skip_in_environment_setup_test hawktest::htest_main_static)
hawk_test(htest_skip_test hawktest::htest_main_static)


############################################################
# Python tests.

test_executable(hawktest-break-on-failure-unittest_ hawktest::hawktest_static)
py_test(hawktest-break-on-failure-unittest)

py_test(htest_skip_check_output_test)
py_test(htest_skip_environment_check_output_test)


kmcmake_cc_binary(
        NAME  hawktest-catch-exceptions-ex-test_
        SOURCES
        hawktest-catch-exceptions-test_.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        hawktest::htest_main_static
)


kmcmake_cc_binary(
        NAME  hawktest-catch-exceptions-no-ex-test_
        SOURCES
        hawktest-catch-exceptions-test_.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        -fno-exceptions
        LINKS
        hawktest::htest_main_static
)


py_test(hawktest-catch-exceptions-test)

test_executable(hawktest-env-var-test_ hawktest::hawktest_static)
py_test(hawktest-env-var-test)

test_executable(hawktest-filter-unittest_ hawktest::hawktest_static)
py_test(hawktest-filter-unittest)

test_executable(htest_help_test_ hawktest::htest_main_static)
py_test(htest_help_test)

test_executable(hawktest-list-tests-unittest_ hawktest::hawktest_static)
py_test(hawktest-list-tests-unittest)

test_executable(hawktest-output-test_ hawktest::hawktest_static)
py_test(hawktest-output-test --no_stacktrace_support)

test_executable(hawktest-shuffle-test_ hawktest::hawktest_static)
py_test(hawktest-shuffle-test)


test_executable(hawktest-uninitialized-test_ hawktest::hawktest_static)
py_test(hawktest-uninitialized-test)

test_executable(htest_list_output_unittest_ hawktest::hawktest_static)
py_test(htest_list_output_unittest)

test_executable(htest_xml_outfile1_test_ hawktest::htest_main_static)
test_executable(htest_xml_outfile2_test_ hawktest::htest_main_static)
py_test(htest_xml_outfiles_test)
py_test(hawktest-json-outfiles-test)

test_executable(htest_xml_output_unittest_ hawktest::hawktest_static)
py_test(htest_xml_output_unittest --no_stacktrace_support)
py_test(hawktest-json-output-unittest --no_stacktrace_support)
