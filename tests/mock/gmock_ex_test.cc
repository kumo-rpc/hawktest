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

// Tests Google Mock's functionality that depends on exceptions.

#include <exception>

#include <hawktest/mock/hmock.h>
#include <hawktest/test/htest.h>

#if HTEST_HAS_EXCEPTIONS
namespace {

using hawk::HasSubstr;

using hawk::internal::HawkTestFailureException;

// A type that cannot be default constructed.
class NonDefaultConstructible {
 public:
  explicit NonDefaultConstructible(int /* dummy */) {}
};

class MockFoo {
 public:
  // A mock method that returns a user-defined type.  Google Mock
  // doesn't know what the default value for this type is.
  MOCK_METHOD0(GetNonDefaultConstructible, NonDefaultConstructible());
};

TEST(DefaultValueTest, ThrowsRuntimeErrorWhenNoDefaultValue) {
  MockFoo mock;
  try {
    // No expectation is set on this method, so Google Mock must
    // return the default value.  However, since Google Mock knows
    // nothing about the return type, it doesn't know what to return,
    // and has to throw (when exceptions are enabled) or abort
    // (otherwise).
    mock.GetNonDefaultConstructible();
    FAIL() << "GetNonDefaultConstructible()'s return type has no default "
           << "value, so Google Mock should have thrown.";
  } catch (const HawkTestFailureException& /* unused */) {
    FAIL() << "Hawk Test does not try to catch an exception of type "
           << "HawkTestFailureException, which is used for reporting "
           << "a failure to other testing frameworks.  Google Mock should "
           << "not throw a HawkTestFailureException as it will kill the "
           << "entire test program instead of just the current TEST.";
  } catch (const std::exception& ex) {
    EXPECT_THAT(ex.what(), HasSubstr("has no default value"));
  }
}

}  // unnamed namespace
#endif
