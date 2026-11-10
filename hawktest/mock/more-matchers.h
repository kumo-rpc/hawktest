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

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements some matchers that depend on gmock-matchers.h.
//
// Note that tests are implemented in gmock-matchers_test.cc rather than
// gmock-more-matchers-test.cc.

// IWYU pragma: private, include "gmock/gmock.h"
// IWYU pragma: friend gmock/.*

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MORE_MATCHERS_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MORE_MATCHERS_H_

#include <ostream>
#include <string>

#include <hawktest/mock/matchers.h>

namespace hawk {

// Silence C4100 (unreferenced formal
// parameter) for MSVC
HTEST_DISABLE_MSC_WARNINGS_PUSH_(4100)
#if defined(_MSC_VER) && (_MSC_VER == 1900)
// and silence C4800 (C4800: 'int *const ': forcing value
// to bool 'true' or 'false') for MSVC 14
HTEST_DISABLE_MSC_WARNINGS_PUSH_(4800)
#endif

namespace internal {

// Implements the polymorphic IsEmpty matcher, which
// can be used as a Matcher<T> as long as T is either a container that defines
// empty() and size() (e.g. std::vector or std::string), or a C-style string.
class [[nodiscard]] IsEmptyMatcher {
 public:
  // Matches anything that defines empty() and size().
  template <typename MatcheeContainerType>
  bool MatchAndExplain(const MatcheeContainerType& c,
                       MatchResultListener* listener) const {
    if (c.empty()) {
      return true;
    }
    *listener << "whose size is " << c.size();
    return false;
  }

  // Matches C-style strings.
  bool MatchAndExplain(const char* s, MatchResultListener* listener) const {
    if (s == nullptr) {
      return false;
    }
    return MatchAndExplain(std::string(s), listener);
  }

#if HTEST_HAS_STD_WSTRING
  // Matches C-style wide strings.
  bool MatchAndExplain(const wchar_t* s, MatchResultListener* listener) const {
    if (s == nullptr) {
      return false;
    }
    return MatchAndExplain(std::wstring(s), listener);
  }
#endif  // HTEST_HAS_STD_WSTRING

  // Describes what this matcher matches.
  void DescribeTo(std::ostream* os) const { *os << "is empty"; }

  void DescribeNegationTo(std::ostream* os) const { *os << "isn't empty"; }
};

}  // namespace internal

// Creates a polymorphic matcher that matches an empty container or C-style
// string. The container must support both size() and empty(), which all
// STL-like containers provide.
inline PolymorphicMatcher<internal::IsEmptyMatcher> IsEmpty() {
  return MakePolymorphicMatcher(internal::IsEmptyMatcher());
}

// Define a matcher that matches a value that evaluates in boolean
// context to true.  Useful for types that define "explicit operator
// bool" operators and so can't be compared for equality with true
// and false.
MATCHER(IsTrue, negation ? "is false" : "is true") {
  return static_cast<bool>(arg);
}

// Define a matcher that matches a value that evaluates in boolean
// context to false.  Useful for types that define "explicit operator
// bool" operators and so can't be compared for equality with true
// and false.
MATCHER(IsFalse, negation ? "is true" : "is false") {
  return !static_cast<bool>(arg);
}

#if defined(_MSC_VER) && (_MSC_VER == 1900)
HTEST_DISABLE_MSC_WARNINGS_POP_()  // 4800
#endif
HTEST_DISABLE_MSC_WARNINGS_POP_()  // 4100

}  // namespace hawk

#endif  // GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MORE_MATCHERS_H_
