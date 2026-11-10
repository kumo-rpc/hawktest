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

#include <hawktest/common/environment.h>
#include <hawktest/test/htest.h>

using ::hawk::init_hawk_test;
using ::hawk::Test;
using ::hawk::internal::posix::Stat;
using ::hawk::internal::posix::StatStruct;

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
    const std::optional<std::string> path = hawk::get_env("TEST_PREMATURE_EXIT_FILE");
    premature_exit_path_ = path.has_value() ? *path : "";
  }

  // Returns true if and only if the premature-exit file exists.
  bool PrematureExitFileExists() const {
    return FileExists(premature_exit_path_.c_str());
  }

  std::string premature_exit_path_;
};

typedef PrematureExitTest PrematureExitDeathTest;

// Tests that:
//   - the premature-exit file exists during the execution of a
//     death test (EXPECT_DEATH*), and
//   - a death test doesn't interfere with the main test process's
//     handling of the premature-exit file.
TEST_F(PrematureExitDeathTest, FileExistsDuringExecutionOfDeathTest) {
  if (premature_exit_path_.empty()) {
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
  if (premature_exit_path_.empty()) {
    return;
  }

  EXPECT_TRUE(PrematureExitFileExists())
      << " file " << premature_exit_path_
      << " should exist during test execution, but doesn't.";
}

}  // namespace

int main(int argc, char** argv) {
  init_hawk_test(&argc, argv);
  const int exit_code = RUN_ALL_TESTS();

  // Test that the premature-exit file is deleted upon return from
  // RUN_ALL_TESTS().
  const std::optional<std::string> filepath = hawk::get_env("TEST_PREMATURE_EXIT_FILE");
  if (filepath.has_value() && !filepath->empty()) {
    if (PrematureExitTest::FileExists(filepath->c_str())) {
      printf(
          "File %s shouldn't exist after the test program finishes, but does.",
          filepath->c_str());
      return 1;
    }
  }

  return exit_code;
}
