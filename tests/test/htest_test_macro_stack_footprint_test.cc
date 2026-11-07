// Copyright 2013, Google Inc.
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

//
// Each TEST() expands to some static registration logic.  GCC puts all
// such static initialization logic for a translation unit in a common,
// internal function.  Since Google's build system restricts how much
// stack space a function can use, there's a limit on how many TEST()s
// one can put in a single C++ test file.  This test ensures that a large
// number of TEST()s can be defined in the same translation unit.

#include <hawktest/test/htest.h>

// This macro defines 10 dummy tests.
#define TEN_TESTS_(test_case_name) \
  TEST(test_case_name, T0) {}      \
  TEST(test_case_name, T1) {}      \
  TEST(test_case_name, T2) {}      \
  TEST(test_case_name, T3) {}      \
  TEST(test_case_name, T4) {}      \
  TEST(test_case_name, T5) {}      \
  TEST(test_case_name, T6) {}      \
  TEST(test_case_name, T7) {}      \
  TEST(test_case_name, T8) {}      \
  TEST(test_case_name, T9) {}

// This macro defines 100 dummy tests.
#define HUNDRED_TESTS_(test_case_name_prefix) \
  TEN_TESTS_(test_case_name_prefix##0)        \
  TEN_TESTS_(test_case_name_prefix##1)        \
  TEN_TESTS_(test_case_name_prefix##2)        \
  TEN_TESTS_(test_case_name_prefix##3)        \
  TEN_TESTS_(test_case_name_prefix##4)        \
  TEN_TESTS_(test_case_name_prefix##5)        \
  TEN_TESTS_(test_case_name_prefix##6)        \
  TEN_TESTS_(test_case_name_prefix##7)        \
  TEN_TESTS_(test_case_name_prefix##8)        \
  TEN_TESTS_(test_case_name_prefix##9)

// This macro defines 1000 dummy tests.
#define THOUSAND_TESTS_(test_case_name_prefix) \
  HUNDRED_TESTS_(test_case_name_prefix##0)     \
  HUNDRED_TESTS_(test_case_name_prefix##1)     \
  HUNDRED_TESTS_(test_case_name_prefix##2)     \
  HUNDRED_TESTS_(test_case_name_prefix##3)     \
  HUNDRED_TESTS_(test_case_name_prefix##4)     \
  HUNDRED_TESTS_(test_case_name_prefix##5)     \
  HUNDRED_TESTS_(test_case_name_prefix##6)     \
  HUNDRED_TESTS_(test_case_name_prefix##7)     \
  HUNDRED_TESTS_(test_case_name_prefix##8)     \
  HUNDRED_TESTS_(test_case_name_prefix##9)

// Ensures that we can define 1000 TEST()s in the same translation
// unit.
THOUSAND_TESTS_(T)

int main(int argc, char **argv) {
  testing::init_hawk_test(&argc, argv);

  // We don't actually need to run the dummy tests - the purpose is to
  // ensure that they compile.
  return 0;
}
