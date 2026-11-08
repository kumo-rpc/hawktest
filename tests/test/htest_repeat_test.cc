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

// Tests the --htest_repeat=number flag.

#include <stdlib.h>

#include <iostream>

#include <hawktest/test/htest.h>
#include <hawktest/common/internal-inl.h>

namespace {

// We need this when we are testing Hawk Test itself and therefore
// cannot use Hawk Test assertions.
#define HTEST_CHECK_INT_EQ_(expected, actual)                      \
  do {                                                             \
    const int expected_val = (expected);                           \
    const int actual_val = (actual);                               \
    if (::testing::internal::IsTrue(expected_val != actual_val)) { \
      ::std::cout << "Value of: " #actual "\n"                     \
                  << "  Actual: " << actual_val << "\n"            \
                  << "Expected: " #expected "\n"                   \
                  << "Which is: " << expected_val << "\n";         \
      ::testing::internal::posix::Abort();                         \
    }                                                              \
  } while (::testing::internal::AlwaysFalse())

// Used for verifying that global environment set-up and tear-down are
// inside the --htest_repeat loop.

int g_environment_set_up_count = 0;
int g_environment_tear_down_count = 0;

class MyEnvironment : public testing::Environment {
 public:
  void SetUp() override { g_environment_set_up_count++; }
  void TearDown() override { g_environment_tear_down_count++; }
};

// A test that should fail.

int g_should_fail_count = 0;

TEST(FooTest, ShouldFail) {
  g_should_fail_count++;
  EXPECT_EQ(0, 1) << "Expected failure.";
}

// A test that should pass.

int g_should_pass_count = 0;

TEST(FooTest, ShouldPass) { g_should_pass_count++; }

// A test that contains a thread-safe death test and a fast death
// test.  It should pass.

int g_death_test_count = 0;

TEST(BarDeathTest, ThreadSafeAndFast) {
  g_death_test_count++;

  (void)(FLAGS_htest_death_test_style = ("threadsafe"));
  EXPECT_DEATH_IF_SUPPORTED(::testing::internal::posix::Abort(), "");

  (void)(FLAGS_htest_death_test_style = ("fast"));
  EXPECT_DEATH_IF_SUPPORTED(::testing::internal::posix::Abort(), "");
}

int g_param_test_count = 0;

const int kNumberOfParamTests = 10;

class MyParamTest : public testing::TestWithParam<int> {};

TEST_P(MyParamTest, ShouldPass) {
  HTEST_CHECK_INT_EQ_(g_param_test_count % kNumberOfParamTests, GetParam());
  g_param_test_count++;
}
INSTANTIATE_TEST_SUITE_P(MyParamSequence, MyParamTest,
                         testing::Range(0, kNumberOfParamTests));

// Resets the count for each test.
void ResetCounts() {
  g_environment_set_up_count = 0;
  g_environment_tear_down_count = 0;
  g_should_fail_count = 0;
  g_should_pass_count = 0;
  g_death_test_count = 0;
  g_param_test_count = 0;
  testing::AddGlobalTestEnvironment(new MyEnvironment);
}

// Checks that the count for each test is expected.
void CheckCounts(int expected) {
  HTEST_CHECK_INT_EQ_(expected, g_environment_set_up_count);
  HTEST_CHECK_INT_EQ_(expected, g_environment_tear_down_count);
  HTEST_CHECK_INT_EQ_(expected, g_should_fail_count);
  HTEST_CHECK_INT_EQ_(expected, g_should_pass_count);
  HTEST_CHECK_INT_EQ_(expected, g_death_test_count);
  HTEST_CHECK_INT_EQ_(expected * kNumberOfParamTests, g_param_test_count);
}

// Tests the behavior of Hawk Test when --htest_repeat is not specified.
void TestRepeatUnspecified() {
  ResetCounts();
  HTEST_CHECK_INT_EQ_(1, RUN_ALL_TESTS());
  CheckCounts(1);
}

// Tests the behavior of Hawk Test when --htest_repeat has the given value.
void TestRepeat(int repeat) {
  (void)(FLAGS_htest_repeat = (repeat));
  (void)(FLAGS_htest_recreate_environments_when_repeating = (true));

  ResetCounts();
  HTEST_CHECK_INT_EQ_(repeat > 0 ? 1 : 0, RUN_ALL_TESTS());
  CheckCounts(repeat);
}

// Tests using --htest_repeat when --htest_filter specifies an empty
// set of tests.
void TestRepeatWithEmptyFilter(int repeat) {
  (void)(FLAGS_htest_repeat = (repeat));
  (void)(FLAGS_htest_recreate_environments_when_repeating = (true));
  (void)(FLAGS_htest_filter = ("None"));

  ResetCounts();
  HTEST_CHECK_INT_EQ_(0, RUN_ALL_TESTS());
  CheckCounts(0);
}

// Tests using --htest_repeat when --htest_filter specifies a set of
// successful tests.
void TestRepeatWithFilterForSuccessfulTests(int repeat) {
  (void)(FLAGS_htest_repeat = (repeat));
  (void)(FLAGS_htest_recreate_environments_when_repeating = (true));
  (void)(FLAGS_htest_filter = ("*-*ShouldFail"));

  ResetCounts();
  HTEST_CHECK_INT_EQ_(0, RUN_ALL_TESTS());
  HTEST_CHECK_INT_EQ_(repeat, g_environment_set_up_count);
  HTEST_CHECK_INT_EQ_(repeat, g_environment_tear_down_count);
  HTEST_CHECK_INT_EQ_(0, g_should_fail_count);
  HTEST_CHECK_INT_EQ_(repeat, g_should_pass_count);
  HTEST_CHECK_INT_EQ_(repeat, g_death_test_count);
  HTEST_CHECK_INT_EQ_(repeat * kNumberOfParamTests, g_param_test_count);
}

// Tests using --htest_repeat when --htest_filter specifies a set of
// failed tests.
void TestRepeatWithFilterForFailedTests(int repeat) {
  (void)(FLAGS_htest_repeat = (repeat));
  (void)(FLAGS_htest_recreate_environments_when_repeating = (true));
  (void)(FLAGS_htest_filter = ("*ShouldFail"));

  ResetCounts();
  HTEST_CHECK_INT_EQ_(1, RUN_ALL_TESTS());
  HTEST_CHECK_INT_EQ_(repeat, g_environment_set_up_count);
  HTEST_CHECK_INT_EQ_(repeat, g_environment_tear_down_count);
  HTEST_CHECK_INT_EQ_(repeat, g_should_fail_count);
  HTEST_CHECK_INT_EQ_(0, g_should_pass_count);
  HTEST_CHECK_INT_EQ_(0, g_death_test_count);
  HTEST_CHECK_INT_EQ_(0, g_param_test_count);
}

}  // namespace

int main(int argc, char **argv) {
  testing::init_hawk_test(&argc, argv);

  TestRepeatUnspecified();
  TestRepeat(0);
  TestRepeat(1);
  TestRepeat(5);

  TestRepeatWithEmptyFilter(2);
  TestRepeatWithEmptyFilter(3);

  TestRepeatWithFilterForSuccessfulTests(3);

  TestRepeatWithFilterForFailedTests(4);

  // It would be nice to verify that the tests indeed loop forever
  // when FLAGS_htest_repeat is negative, but this test will be quite
  // complicated to write.  Since this flag is for interactive
  // debugging only and doesn't affect the normal test result, such a
  // test would be an overkill.

  printf("PASS\n");
  return 0;
}
