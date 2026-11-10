// Copyright 2005, Google Inc.
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

// Unit test for Hawk Test test filters.
//
// A user can specify which test(s) in a Hawk Test program to run via
// either the HTEST_FILTER environment variable or the --htest_filter
// flag.  This is used for testing such functionality.
//
// The program will be invoked from a Python unit test.  Don't run it
// directly.

#include <hawktest/test/htest.h>

namespace {

// Test HasFixtureTest.

class HasFixtureTest : public hawk::Test {};

TEST_F(HasFixtureTest, Test0) {}

TEST_F(HasFixtureTest, Test1) { FAIL() << "Expected failure."; }

TEST_F(HasFixtureTest, Test2) { FAIL() << "Expected failure."; }

TEST_F(HasFixtureTest, Test3) { FAIL() << "Expected failure."; }

TEST_F(HasFixtureTest, Test4) { FAIL() << "Expected failure."; }

// Test HasSimpleTest.

TEST(HasSimpleTest, Test0) {}

TEST(HasSimpleTest, Test1) { FAIL() << "Expected failure."; }

TEST(HasSimpleTest, Test2) { FAIL() << "Expected failure."; }

TEST(HasSimpleTest, Test3) { FAIL() << "Expected failure."; }

TEST(HasSimpleTest, Test4) { FAIL() << "Expected failure."; }

// Test HasDisabledTest.

TEST(HasDisabledTest, Test0) {}

TEST(HasDisabledTest, DISABLED_Test1) { FAIL() << "Expected failure."; }

TEST(HasDisabledTest, Test2) { FAIL() << "Expected failure."; }

TEST(HasDisabledTest, Test3) { FAIL() << "Expected failure."; }

TEST(HasDisabledTest, Test4) { FAIL() << "Expected failure."; }

// Test HasDeathTest

TEST(HasDeathTest, Test0) { EXPECT_DEATH_IF_SUPPORTED(exit(1), ".*"); }

TEST(HasDeathTest, Test1) {
  EXPECT_DEATH_IF_SUPPORTED(FAIL() << "Expected failure.", ".*");
}

TEST(HasDeathTest, Test2) {
  EXPECT_DEATH_IF_SUPPORTED(FAIL() << "Expected failure.", ".*");
}

TEST(HasDeathTest, Test3) {
  EXPECT_DEATH_IF_SUPPORTED(FAIL() << "Expected failure.", ".*");
}

TEST(HasDeathTest, Test4) {
  EXPECT_DEATH_IF_SUPPORTED(FAIL() << "Expected failure.", ".*");
}

// Test DISABLED_HasDisabledSuite

TEST(DISABLED_HasDisabledSuite, Test0) {}

TEST(DISABLED_HasDisabledSuite, Test1) { FAIL() << "Expected failure."; }

TEST(DISABLED_HasDisabledSuite, Test2) { FAIL() << "Expected failure."; }

TEST(DISABLED_HasDisabledSuite, Test3) { FAIL() << "Expected failure."; }

TEST(DISABLED_HasDisabledSuite, Test4) { FAIL() << "Expected failure."; }

// Test HasParametersTest

class HasParametersTest : public hawk::TestWithParam<int> {};

TEST_P(HasParametersTest, Test1) { FAIL() << "Expected failure."; }

TEST_P(HasParametersTest, Test2) { FAIL() << "Expected failure."; }

INSTANTIATE_TEST_SUITE_P(HasParametersSuite, HasParametersTest,
                         hawk::Values(1, 2));

class MyTestListener : public ::hawk::EmptyTestEventListener {
  void OnTestSuiteStart(const ::hawk::TestSuite& test_suite) override {
    printf("We are in OnTestSuiteStart of %s.\n", test_suite.name());
  }

  void OnTestStart(const ::hawk::TestInfo& test_info) override {
    printf("We are in OnTestStart of %s.%s.\n", test_info.test_suite_name(),
           test_info.name());
  }

  void OnTestPartResult(
      const ::hawk::TestPartResult& test_part_result) override {
    printf("We are in OnTestPartResult %s:%d.\n", test_part_result.file_name(),
           test_part_result.line_number());
  }

  void OnTestEnd(const ::hawk::TestInfo& test_info) override {
    printf("We are in OnTestEnd of %s.%s.\n", test_info.test_suite_name(),
           test_info.name());
  }

  void OnTestSuiteEnd(const ::hawk::TestSuite& test_suite) override {
    printf("We are in OnTestSuiteEnd of %s.\n", test_suite.name());
  }
};

TEST(HasSkipTest, Test0) { SUCCEED() << "Expected success."; }

TEST(HasSkipTest, Test1) { HTEST_SKIP() << "Expected skip."; }

TEST(HasSkipTest, Test2) { FAIL() << "Expected failure."; }

TEST(HasSkipTest, Test3) { FAIL() << "Expected failure."; }

TEST(HasSkipTest, Test4) { FAIL() << "Expected failure."; }

}  // namespace

int main(int argc, char** argv) {
  ::hawk::init_hawk_test(&argc, argv);
  ::hawk::UnitTest::GetInstance()->listeners().Append(new MyTestListener());
  return RUN_ALL_TESTS();
}
