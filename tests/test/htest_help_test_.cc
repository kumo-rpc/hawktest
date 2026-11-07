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

// This program is meant to be run by htest_help_test.py.  Do not run
// it directly.

#include <hawktest/test/htest.h>

// When a help flag is specified, this program should skip the tests
// and exit with 0; otherwise the following test will be executed,
// causing this program to exit with a non-zero code.
TEST(HelpFlagTest, ShouldNotBeRun) {
  ASSERT_TRUE(false) << "Tests shouldn't be run when --help is specified.";
}

#ifdef HTEST_HAS_DEATH_TEST
TEST(DeathTest, UsedByPythonScriptToDetectSupportForDeathTestsInThisBinary) {}
#endif
