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

// Unit test for Hawk Test global test environments.
//
// The program will be invoked from a Python unit test.  Don't run it
// directly.

#include <hawktest/test/htest.h>

namespace {

// An environment that always fails in its SetUp method.
class FailingEnvironment final : public ::hawk::Environment {
 public:
  void SetUp() override { FAIL() << "Canned environment setup error"; }
};

// Register the environment.
auto* const g_environment_ =
    ::hawk::AddGlobalTestEnvironment(new FailingEnvironment);

// A test that doesn't actually run.
TEST(SomeTest, DoesFoo) { FAIL() << "Unexpected call"; }

}  // namespace

int main(int argc, char** argv) {
  ::hawk::init_hawk_test(&argc, argv);

  return RUN_ALL_TESTS();
}
