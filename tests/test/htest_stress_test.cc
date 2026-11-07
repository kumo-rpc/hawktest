// Copyright 2007, Google Inc.
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

// Tests that SCOPED_TRACE() and various Hawk Test assertions can be
// used in a large number of threads concurrently.

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <hawktest/test/htest.h>
#include <hawktest/common/internal-inl.h>

#ifdef HTEST_IS_THREADSAFE

namespace testing {
namespace {

using internal::Notification;
using internal::TestPropertyKeyIs;
using internal::ThreadWithParam;

// In order to run tests in this file, for platforms where Hawk Test is
// thread safe, implement ThreadWithParam. See the description of its API
// in htest-port.h, where it is defined for already supported platforms.

// How many threads to create?
const int kThreadCount = 50;

std::string IdToKey(int id, const char* suffix) {
  Message key;
  key << "key_" << id << "_" << suffix;
  return key.GetString();
}

std::string IdToString(int id) {
  Message id_message;
  id_message << id;
  return id_message.GetString();
}

void ExpectKeyAndValueWereRecordedForId(
    const std::vector<TestProperty>& properties, int id, const char* suffix) {
  TestPropertyKeyIs matches_key(IdToKey(id, suffix).c_str());
  const std::vector<TestProperty>::const_iterator property =
      std::find_if(properties.begin(), properties.end(), matches_key);
  ASSERT_TRUE(property != properties.end())
      << "expecting " << suffix << " value for id " << id;
  EXPECT_STREQ(IdToString(id).c_str(), property->value());
}

// Calls a large number of Hawk Test assertions, where exactly one of them
// will fail.
void ManyAsserts(int id) {
  HTEST_LOG_(INFO) << "Thread #" << id << " running...";

  SCOPED_TRACE(Message() << "Thread #" << id);

  for (int i = 0; i < kThreadCount; i++) {
    SCOPED_TRACE(Message() << "Iteration #" << i);

    // A bunch of assertions that should succeed.
    EXPECT_TRUE(true);
    ASSERT_FALSE(false) << "This shouldn't fail.";
    EXPECT_STREQ("a", "a");
    ASSERT_LE(5, 6);
    EXPECT_EQ(i, i) << "This shouldn't fail.";

    // RecordProperty() should interact safely with other threads as well.
    // The shared_key forces property updates.
    Test::RecordProperty(IdToKey(id, "string"), IdToString(id));
    Test::RecordProperty(IdToKey(id, "int").c_str(), id);
    Test::RecordProperty("shared_key", IdToString(id));

    // This assertion should fail kThreadCount times per thread.  It
    // is for testing whether Hawk Test can handle failed assertions in a
    // multi-threaded context.
    EXPECT_LT(i, 0) << "This should always fail.";
  }
}

void CheckTestFailureCount(int expected_failures) {
  const TestInfo* const info = UnitTest::GetInstance()->current_test_info();
  const TestResult* const result = info->result();
  HTEST_CHECK_(expected_failures == result->total_part_count())
      << "Logged " << result->total_part_count() << " failures "
      << " vs. " << expected_failures << " expected";
}

// Tests using SCOPED_TRACE() and Hawk Test assertions in many threads
// concurrently.
TEST(StressTest, CanUseScopedTraceAndAssertionsInManyThreads) {
  {
    std::unique_ptr<ThreadWithParam<int> > threads[kThreadCount];
    Notification threads_can_start;
    for (int i = 0; i != kThreadCount; i++)
      threads[i] = std::make_unique<ThreadWithParam<int>>(&ManyAsserts, i,
                                                          &threads_can_start);

    threads_can_start.Notify();

    // Blocks until all the threads are done.
    for (int i = 0; i != kThreadCount; i++) threads[i]->Join();
  }

  // Ensures that kThreadCount*kThreadCount failures have been reported.
  const TestInfo* const info = UnitTest::GetInstance()->current_test_info();
  const TestResult* const result = info->result();

  std::vector<TestProperty> properties;
  // We have no access to the TestResult's list of properties but we can
  // copy them one by one.
  for (int i = 0; i < result->test_property_count(); ++i)
    properties.push_back(result->GetTestProperty(i));

  EXPECT_EQ(kThreadCount * 2 + 1, result->test_property_count())
      << "String and int values recorded on each thread, "
      << "as well as one shared_key";
  for (int i = 0; i < kThreadCount; ++i) {
    ExpectKeyAndValueWereRecordedForId(properties, i, "string");
    ExpectKeyAndValueWereRecordedForId(properties, i, "int");
  }
  CheckTestFailureCount(kThreadCount * kThreadCount);
}

void FailingThread(bool is_fatal) {
  if (is_fatal)
    FAIL() << "Fatal failure in some other thread. "
           << "(This failure is expected.)";
  else
    ADD_FAILURE() << "Non-fatal failure in some other thread. "
                  << "(This failure is expected.)";
}

void GenerateFatalFailureInAnotherThread(bool is_fatal) {
  ThreadWithParam<bool> thread(&FailingThread, is_fatal, nullptr);
  thread.Join();
}

TEST(NoFatalFailureTest, ExpectNoFatalFailureIgnoresFailuresInOtherThreads) {
  EXPECT_NO_FATAL_FAILURE(GenerateFatalFailureInAnotherThread(true));
  // We should only have one failure (the one from
  // GenerateFatalFailureInAnotherThread()), since the EXPECT_NO_FATAL_FAILURE
  // should succeed.
  CheckTestFailureCount(1);
}

void AssertNoFatalFailureIgnoresFailuresInOtherThreads() {
  ASSERT_NO_FATAL_FAILURE(GenerateFatalFailureInAnotherThread(true));
}
TEST(NoFatalFailureTest, AssertNoFatalFailureIgnoresFailuresInOtherThreads) {
  // Using a subroutine, to make sure, that the test continues.
  AssertNoFatalFailureIgnoresFailuresInOtherThreads();
  // We should only have one failure (the one from
  // GenerateFatalFailureInAnotherThread()), since the EXPECT_NO_FATAL_FAILURE
  // should succeed.
  CheckTestFailureCount(1);
}

TEST(FatalFailureTest, ExpectFatalFailureIgnoresFailuresInOtherThreads) {
  // This statement should fail, since the current thread doesn't generate a
  // fatal failure, only another one does.
  EXPECT_FATAL_FAILURE(GenerateFatalFailureInAnotherThread(true), "expected");
  CheckTestFailureCount(2);
}

TEST(FatalFailureOnAllThreadsTest, ExpectFatalFailureOnAllThreads) {
  // This statement should succeed, because failures in all threads are
  // considered.
  EXPECT_FATAL_FAILURE_ON_ALL_THREADS(GenerateFatalFailureInAnotherThread(true),
                                      "expected");
  CheckTestFailureCount(0);
  // We need to add a failure, because main() checks that there are failures.
  // But when only this test is run, we shouldn't have any failures.
  ADD_FAILURE() << "This is an expected non-fatal failure.";
}

TEST(NonFatalFailureTest, ExpectNonFatalFailureIgnoresFailuresInOtherThreads) {
  // This statement should fail, since the current thread doesn't generate a
  // fatal failure, only another one does.
  EXPECT_NONFATAL_FAILURE(GenerateFatalFailureInAnotherThread(false),
                          "expected");
  CheckTestFailureCount(2);
}

TEST(NonFatalFailureOnAllThreadsTest, ExpectNonFatalFailureOnAllThreads) {
  // This statement should succeed, because failures in all threads are
  // considered.
  EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS(
      GenerateFatalFailureInAnotherThread(false), "expected");
  CheckTestFailureCount(0);
  // We need to add a failure, because main() checks that there are failures,
  // But when only this test is run, we shouldn't have any failures.
  ADD_FAILURE() << "This is an expected non-fatal failure.";
}

}  // namespace
}  // namespace testing

int main(int argc, char** argv) {
  testing::init_hawk_test(&argc, argv);

  const int result = RUN_ALL_TESTS();  // Expected to fail.
  HTEST_CHECK_(result == 1) << "RUN_ALL_TESTS() did not fail as expected";

  printf("\nPASS\n");
  return 0;
}

#else
TEST(StressTest,
     DISABLED_ThreadSafetyTestsAreSkippedWhenHawkTestIsNotThreadSafe) {}

int main(int argc, char **argv) {
  testing::init_hawk_test(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif  // HTEST_IS_THREADSAFE
