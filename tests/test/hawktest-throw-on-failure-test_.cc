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

// Tests Hawk Test's throw-on-failure mode with exceptions disabled.
//
// This program must be compiled with exceptions disabled.  It will be
// invoked by hawktest-throw-on-failure-test.py, and is expected to exit
// with non-zero in the throw-on-failure mode or 0 otherwise.

#include <stdio.h>   // for fflush, fprintf, NULL, etc.
#include <stdlib.h>  // for exit

#include <exception>  // for set_terminate

#include <hawktest/test/htest.h>

// This terminate handler aborts the program using exit() rather than abort().
// This avoids showing pop-ups on Windows systems and core dumps on Unix-like
// ones.
void TerminateHandler() {
  fprintf(stderr, "%s\n", "Unhandled C++ exception terminating the program.");
  fflush(nullptr);
  exit(1);
}

int main(int argc, char** argv) {
#if HTEST_HAS_EXCEPTIONS
  std::set_terminate(&TerminateHandler);
#endif
  testing::init_hawk_test(&argc, argv);

  // We want to ensure that people can use Hawk Test assertions in
  // other testing frameworks, as long as they initialize Hawk Test
  // properly and set the throw-on-failure mode.  Therefore, we don't
  // use Hawk Test's constructs for defining and running tests
  // (e.g. TEST and RUN_ALL_TESTS) here.

  // In the throw-on-failure mode with exceptions disabled, this
  // assertion will cause the program to exit with a non-zero code.
  EXPECT_EQ(2, 3);

  // When not in the throw-on-failure mode, the control will reach
  // here.
  return 0;
}
