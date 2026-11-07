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

// A helper program for testing how Hawk Test determines whether to use
// colors in the output.  It prints "YES" and returns 1 if Hawk Test
// decides to use colors, and prints "NO" and returns 0 otherwise.

#include <stdio.h>

#include <hawktest/test/htest.h>
#include <hawktest/common/internal-inl.h>

using testing::internal::ShouldUseColor;

// The purpose of this is to ensure that the UnitTest singleton is
// created before main() is entered, and thus that ShouldUseColor()
// works the same way as in a real Google-Test-based test.  We don't actual
// run the TEST itself.
TEST(HTestColorTest, Dummy) {}

int main(int argc, char** argv) {
  testing::init_hawk_test(&argc, argv);

  if (ShouldUseColor(true)) {
    // Hawk Test decides to use colors in the output (assuming it
    // goes to a TTY).
    printf("YES\n");
    return 1;
  } else {
    // Hawk Test decides not to use colors in the output.
    printf("NO\n");
    return 0;
  }
}
