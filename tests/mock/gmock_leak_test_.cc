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

// Google Mock - a framework for writing C++ mock classes.
//
// This program is for verifying that a leaked mock object can be
// caught by Google Mock's leak detector.

#include <hawktest/mock/hmock.h>

namespace {

using ::testing::Return;

class FooInterface {
 public:
  virtual ~FooInterface() = default;
  virtual void DoThis() = 0;
};

class MockFoo : public FooInterface {
 public:
  MockFoo() = default;

  MOCK_METHOD0(DoThis, void());

 private:
  MockFoo(const MockFoo&) = delete;
  MockFoo& operator=(const MockFoo&) = delete;
};

TEST(LeakTest, LeakedMockWithExpectCallCausesFailureWhenLeakCheckingIsEnabled) {
  MockFoo* foo = new MockFoo;

  EXPECT_CALL(*foo, DoThis());
  foo->DoThis();

  // In order to test the leak detector, we deliberately leak foo.

  // Makes sure Google Mock's leak detector can change the exit code
  // to 1 even when the code is already exiting with 0.
  exit(0);
}

TEST(LeakTest, LeakedMockWithOnCallCausesFailureWhenLeakCheckingIsEnabled) {
  MockFoo* foo = new MockFoo;

  ON_CALL(*foo, DoThis()).WillByDefault(Return());

  // In order to test the leak detector, we deliberately leak foo.

  // Makes sure Google Mock's leak detector can change the exit code
  // to 1 even when the code is already exiting with 0.
  exit(0);
}

TEST(LeakTest, CatchesMultipleLeakedMockObjects) {
  MockFoo* foo1 = new MockFoo;
  MockFoo* foo2 = new MockFoo;

  ON_CALL(*foo1, DoThis()).WillByDefault(Return());
  EXPECT_CALL(*foo2, DoThis());
  foo2->DoThis();

  // In order to test the leak detector, we deliberately leak foo1 and
  // foo2.

  // Makes sure Google Mock's leak detector can change the exit code
  // to 1 even when the code is already exiting with 0.
  exit(0);
}

}  // namespace
