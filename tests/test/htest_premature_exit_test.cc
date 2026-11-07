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
// Tests that Hawk Test manipulates the premature-exit-detection
// file correctly.

#include <stdio.h>

#include <hawktest/test/htest.h>

using ::testing::init_hawk_test;
using ::testing::Test;
using ::testing::internal::posix::GetEnv;
using ::testing::internal::posix::Stat;
using ::testing::internal::posix::StatStruct;

namespace {

class PrematureExitTest : public Test {
 public:
  // Returns true if and only if the given file exists.
  static bool FileExists(const char* filepath) {
    StatStruct stat;
    return Stat(filepath, &stat) == 0;
  }

 protected:
  PrematureExitTest() {
    premature_exit_file_path_ = GetEnv("TEST_PREMATURE_EXIT_FILE");

    // Normalize NULL to "" for ease of handling.
    if (premature_exit_file_path_ == nullptr) {
      premature_exit_file_path_ = "";
    }
  }

  // Returns true if and only if the premature-exit file exists.
  bool PrematureExitFileExists() const {
    return FileExists(premature_exit_file_path_);
  }

  const char* premature_exit_file_path_;
};

typedef PrematureExitTest PrematureExitDeathTest;

// Tests that:
//   - the premature-exit file exists during the execution of a
//     death test (EXPECT_DEATH*), and
//   - a death test doesn't interfere with the main test process's
//     handling of the premature-exit file.
TEST_F(PrematureExitDeathTest, FileExistsDuringExecutionOfDeathTest) {
  if (*premature_exit_file_path_ == '\0') {
    return;
  }

  EXPECT_DEATH_IF_SUPPORTED(
      {
        // If the file exists, crash the process such that the main test
        // process will catch the (expected) crash and report a success;
        // otherwise don't crash, which will cause the main test process
        // to report that the death test has failed.
        if (PrematureExitFileExists()) {
          exit(1);
        }
      },
      "");
}

// Tests that the premature-exit file exists during the execution of a
// normal (non-death) test.
TEST_F(PrematureExitTest, PrematureExitFileExistsDuringTestExecution) {
  if (*premature_exit_file_path_ == '\0') {
    return;
  }

  EXPECT_TRUE(PrematureExitFileExists())
      << " file " << premature_exit_file_path_
      << " should exist during test execution, but doesn't.";
}

}  // namespace

int main(int argc, char** argv) {
  init_hawk_test(&argc, argv);
  const int exit_code = RUN_ALL_TESTS();

  // Test that the premature-exit file is deleted upon return from
  // RUN_ALL_TESTS().
  const char* const filepath = GetEnv("TEST_PREMATURE_EXIT_FILE");
  if (filepath != nullptr && *filepath != '\0') {
    if (PrematureExitTest::FileExists(filepath)) {
      printf(
          "File %s shouldn't exist after the test program finishes, but does.",
          filepath);
      return 1;
    }
  }

  return exit_code;
}
