// Copyright 2008 Google Inc.
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
// Author: arseny.aprelev@gmail.com (Arseny Aprelev)
//

#include <hawktest/test/htest.h>

using ::hawk::Test;

TEST(SkipTest, DoesSkip) {
  HTEST_SKIP() << "skipping single test";
  EXPECT_EQ(0, 1);
}

class Fixture : public Test {
 protected:
  void SetUp() override {
    HTEST_SKIP() << "skipping all tests for this fixture";
  }
};

TEST_F(Fixture, SkipsOneTest) { EXPECT_EQ(5, 7); }

TEST_F(Fixture, SkipsAnotherTest) { EXPECT_EQ(99, 100); }
