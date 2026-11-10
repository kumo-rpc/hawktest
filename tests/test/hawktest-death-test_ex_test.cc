// Copyright 2010, Google Inc.
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
// Tests that verify interaction of exceptions and death tests.

#include <hawktest/test/death-test.h>
#include <hawktest/test/htest.h>

#ifdef HTEST_HAS_DEATH_TEST

#if HTEST_HAS_SEH
#include <windows.h>  // For RaiseException().
#endif

#include <hawktest/test/spi.h>

#if HTEST_HAS_EXCEPTIONS

#include <exception>  // For std::exception.

// Tests that death tests report thrown exceptions as failures and that the
// exceptions do not escape death test macros.
TEST(CxxExceptionDeathTest, ExceptionIsFailure) {
  try {
    EXPECT_NONFATAL_FAILURE(EXPECT_DEATH(throw 1, ""), "threw an exception");
  } catch (...) {  // NOLINT
    FAIL() << "An exception escaped a death test macro invocation "
           << "with catch_exceptions "
           << (FLAGS_htest_catch_exceptions ? "enabled" : "disabled");
  }
}

class TestException : public std::exception {
 public:
  const char* what() const noexcept override { return "exceptional message"; }
};

TEST(CxxExceptionDeathTest, PrintsMessageForStdExceptions) {
  // Verifies that the exception message is quoted in the failure text.
  EXPECT_NONFATAL_FAILURE(EXPECT_DEATH(throw TestException(), ""),
                          "exceptional message");
  // Verifies that the location is mentioned in the failure text.
  EXPECT_NONFATAL_FAILURE(EXPECT_DEATH(throw TestException(), ""), __FILE__);
}
#endif  // HTEST_HAS_EXCEPTIONS

#if HTEST_HAS_SEH
// Tests that enabling interception of SEH exceptions with the
// catch_exceptions flag does not interfere with SEH exceptions being
// treated as death by death tests.
TEST(SehExceptionDeasTest, CatchExceptionsDoesNotInterfere) {
  EXPECT_DEATH(RaiseException(42, 0x0, 0, NULL), "")
      << "with catch_exceptions "
      << (FLAGS_htest_catch_exceptions ? "enabled" : "disabled");
}
#endif

#endif  // HTEST_HAS_DEATH_TEST

int main(int argc, char** argv) {
  hawk::init_hawk_test(&argc, argv);
  (void)(FLAGS_htest_catch_exceptions = (HTEST_ENABLE_CATCH_EXCEPTIONS_ != 0));
  return RUN_ALL_TESTS();
}
