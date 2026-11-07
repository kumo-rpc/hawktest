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

//
// This test verifies that it's possible to use Hawk Test by including
// the htest.h header file alone.

#include <hawktest/test/htest.h>

namespace {

void Subroutine() { EXPECT_EQ(42, 42); }

TEST(NoFatalFailureTest, ExpectNoFatalFailure) {
  EXPECT_NO_FATAL_FAILURE(;);
  EXPECT_NO_FATAL_FAILURE(SUCCEED());
  EXPECT_NO_FATAL_FAILURE(Subroutine());
  EXPECT_NO_FATAL_FAILURE({ SUCCEED(); });
}

TEST(NoFatalFailureTest, AssertNoFatalFailure) {
  ASSERT_NO_FATAL_FAILURE(;);
  ASSERT_NO_FATAL_FAILURE(SUCCEED());
  ASSERT_NO_FATAL_FAILURE(Subroutine());
  ASSERT_NO_FATAL_FAILURE({ SUCCEED(); });
}

}  // namespace
