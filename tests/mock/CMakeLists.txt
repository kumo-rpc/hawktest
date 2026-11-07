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



hawk_test(gmock-actions_test hawktest::hmock_main_static)
hawk_test(gmock-cardinalities_test hawktest::hmock_main_static)
hawk_test(gmock_ex_test hawktest::hmock_main_static)
hawk_test(gmock-function-mocker_test hawktest::hmock_main_static)
hawk_test(gmock-internal-utils_test hawktest::hmock_main_static)
hawk_test(gmock-matchers-arithmetic_test hawktest::hmock_main_static)
hawk_test(gmock-matchers-comparisons_test hawktest::hmock_main_static)
hawk_test(gmock-matchers-containers_test hawktest::hmock_main_static)
hawk_test(gmock-matchers-misc_test hawktest::hmock_main_static)
hawk_test(gmock-more-actions_test hawktest::hmock_main_static)
hawk_test(gmock-nice-strict_test hawktest::hmock_main_static)
hawk_test(gmock-port_test hawktest::hmock_main_static)
hawk_test(gmock-spec-builders_test hawktest::hmock_main_static)
hawk_test(gmock_link_test hawktest::hmock_main_static gmock_link2_test.cc)
hawk_test(gmock_test hawktest::hmock_main_static)

if (NOT WIN32)
    hawk_test(gmock_stress_test hawktest::hmock_static)
endif()

test_executable(gmock_leak_test_ hawktest::hmock_main_static)
py_test(gmock_leak_test)

test_executable(gmock_output_test_ hawktest::hmock_main_static)
py_test(gmock_output_test)