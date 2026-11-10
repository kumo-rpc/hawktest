// Copyright 2018, Google Inc.
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
// Author: david.schuldenfrei@gmail.com (David Schuldenfrei)

// Unit test for Hawk Test's --htest_list_tests and --htest_output flag.
//
// A user can ask Hawk Test to list all tests that will run,
// and have the output saved in a Json/Xml file.
// The tests will not be run after listing.
//
// This program will be invoked from a Python unit test.
// Don't run it directly.

#include <hawktest/test/htest.h>

TEST(FooTest, Test1) {}

TEST(FooTest, Test2) {}

class FooTestFixture : public ::hawk::Test {};
TEST_F(FooTestFixture, Test3) {}
TEST_F(FooTestFixture, Test4) {}

class ValueParamTest : public ::hawk::TestWithParam<int> {};
TEST_P(ValueParamTest, Test5) {}
TEST_P(ValueParamTest, Test6) {}
INSTANTIATE_TEST_SUITE_P(ValueParam, ValueParamTest, ::hawk::Values(33, 42));

template <typename T>
class TypedTest : public ::hawk::Test {};
typedef hawk::Types<int, bool> TypedTestTypes;
TYPED_TEST_SUITE(TypedTest, TypedTestTypes);
TYPED_TEST(TypedTest, Test7) {}
TYPED_TEST(TypedTest, Test8) {}

template <typename T>
class TypeParameterizedTestSuite : public ::hawk::Test {};
TYPED_TEST_SUITE_P(TypeParameterizedTestSuite);
TYPED_TEST_P(TypeParameterizedTestSuite, Test9) {}
TYPED_TEST_P(TypeParameterizedTestSuite, Test10) {}
REGISTER_TYPED_TEST_SUITE_P(TypeParameterizedTestSuite, Test9, Test10);
typedef hawk::Types<int, bool> TypeParameterizedTestSuiteTypes;  // NOLINT
INSTANTIATE_TYPED_TEST_SUITE_P(Single, TypeParameterizedTestSuite,
                               TypeParameterizedTestSuiteTypes);

int main(int argc, char **argv) {
  ::hawk::init_hawk_test(&argc, argv);

  return RUN_ALL_TESTS();
}
