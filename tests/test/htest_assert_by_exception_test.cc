// Copyright 2009, Google Inc.
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

// Tests Hawk Test's assert-by-exception mode with exceptions enabled.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdexcept>

#include <hawktest/test/htest.h>

class ThrowListener : public hawk::EmptyTestEventListener {
  void OnTestPartResult(const hawk::TestPartResult& result) override {
    if (result.type() == hawk::TestPartResult::kFatalFailure) {
      throw hawk::AssertionException(result);
    }
  }
};

// Prints the given failure message and exits the program with
// non-zero.  We use this instead of a Hawk Test assertion to
// indicate a failure, as the latter is been tested and cannot be
// relied on.
void Fail(const char* msg) {
  printf("FAILURE: %s\n", msg);
  fflush(stdout);
  exit(1);
}

static void AssertFalse() { ASSERT_EQ(2, 3) << "Expected failure"; }

// Tests that an assertion failure throws a subclass of
// std::runtime_error.
TEST(Test, Test) {
  // A successful assertion shouldn't throw.
  try {
    EXPECT_EQ(3, 3);
  } catch (...) {
    Fail("A successful assertion wrongfully threw.");
  }

  // A successful assertion shouldn't throw.
  try {
    EXPECT_EQ(3, 4);
  } catch (...) {
    Fail("A failed non-fatal assertion wrongfully threw.");
  }

  // A failed assertion should throw.
  try {
    AssertFalse();
  } catch (const hawk::AssertionException& e) {
    if (strstr(e.what(), "Expected failure") != nullptr) throw;

    printf("%s",
           "A failed assertion did throw an exception of the right type, "
           "but the message is incorrect.  Instead of containing \"Expected "
           "failure\", it is:\n");
    Fail(e.what());
  } catch (...) {
    Fail("A failed assertion threw the wrong type of exception.");
  }
  Fail("A failed assertion should've thrown but didn't.");
}

int kTestForContinuingTest = 0;

TEST(Test, Test2) { kTestForContinuingTest = 1; }

int main(int argc, char** argv) {
  hawk::init_hawk_test(&argc, argv);
  hawk::UnitTest::GetInstance()->listeners().Append(new ThrowListener);

  int result = RUN_ALL_TESTS();
  if (result == 0) {
    printf("RUN_ALL_TESTS returned %d\n", result);
    Fail("Expected failure instead.");
  }

  if (kTestForContinuingTest == 0) {
    Fail("Should have continued with other tests, but did not.");
  }
  return 0;
}
