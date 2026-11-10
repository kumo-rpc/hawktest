// Copyright (C) 2005 Google Inc. All rights reserved.
// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Portions of this software are derived from GoogleTest and Google Mock,
// originally distributed under the BSD 3-Clause License.
// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Portions of this software are derived from GoogleTest and Google Mock,
// originally distributed under the BSD 3-Clause License.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// For third-party components and portions derived from GoogleTest or
// Google Mock, the applicable original license terms and notices are
// retained in accordance with their respective licenses.
//
//
// For third-party components and portions derived from GoogleTest or
// Google Mock, the applicable original license terms and notices are
// retained in accordance with their respective licenses.
//

// A helper program for testing that Hawk Test parses the environment
// variables correctly.

#include <iostream>

#include <hawktest/test/htest.h>
#include <hawktest/common/internal-inl.h>

using ::std::cout;

namespace hawk {

// The purpose of this is to make the test more realistic by ensuring
// that the UnitTest singleton is created before main() is entered.
// We don't actual run the TEST itself.
TEST(HTestEnvVarTest, Dummy) {}

void PrintFlag(const char* flag) {
  if (strcmp(flag, "break_on_failure") == 0) {
    cout << FLAGS_htest_break_on_failure;
    return;
  }

  if (strcmp(flag, "catch_exceptions") == 0) {
    cout << FLAGS_htest_catch_exceptions;
    return;
  }

  if (strcmp(flag, "color") == 0) {
    cout << FLAGS_htest_color;
    return;
  }

  if (strcmp(flag, "death_test_style") == 0) {
    cout << FLAGS_htest_death_test_style;
    return;
  }

  if (strcmp(flag, "death_test_use_fork") == 0) {
    cout << FLAGS_htest_death_test_use_fork;
    return;
  }

  if (strcmp(flag, "fail_fast") == 0) {
    cout << FLAGS_htest_fail_fast;
    return;
  }

  if (strcmp(flag, "filter") == 0) {
    cout << FLAGS_htest_filter;
    return;
  }

  if (strcmp(flag, "output") == 0) {
    cout << FLAGS_htest_output;
    return;
  }

  if (strcmp(flag, "brief") == 0) {
    cout << FLAGS_htest_brief;
    return;
  }

  if (strcmp(flag, "print_time") == 0) {
    cout << FLAGS_htest_print_time;
    return;
  }

  if (strcmp(flag, "repeat") == 0) {
    cout << FLAGS_htest_repeat;
    return;
  }

  if (strcmp(flag, "stack_trace_depth") == 0) {
    cout << FLAGS_htest_stack_trace_depth;
    return;
  }

  if (strcmp(flag, "throw_on_failure") == 0) {
    cout << FLAGS_htest_throw_on_failure;
    return;
  }

  cout << "Invalid flag name " << flag
       << ".  Valid names are break_on_failure, color, filter, etc.\n";
  exit(1);
}

}  // namespace hawk

int main(int argc, char** argv) {
  hawk::init_hawk_test(&argc, argv);

  if (argc != 2) {
    cout << "Usage: hawktest-env-var-test_ NAME_OF_FLAG\n";
    return 1;
  }

  hawk::PrintFlag(argv[1]);
  return 0;
}
