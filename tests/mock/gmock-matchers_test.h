// Copyright 2007, Google Inc.
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
// This file tests some commonly used argument matchers.

#ifndef GOOGLEMOCK_TEST_GMOCK_MATCHERS_TEST_H_
#define GOOGLEMOCK_TEST_GMOCK_MATCHERS_TEST_H_

#include <string.h>
#include <time.h>

#include <array>
#include <cstdint>
#include <deque>
#include <forward_list>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <hawktest/mock/matchers.h>
#include <hawktest/mock/more-matchers.h>
#include <hawktest/mock/hmock.h>
#include <hawktest/test/spi.h>
#include <hawktest/test/htest.h>

namespace testing {
namespace gmock_matchers_test {

using std::greater;
using std::less;
using std::list;
using std::make_pair;
using std::map;
using std::multimap;
using std::multiset;
using std::ostream;
using std::pair;
using std::set;
using std::stringstream;
using std::vector;
using testing::internal::DummyMatchResultListener;
using testing::internal::ElementMatcherPair;
using testing::internal::ElementMatcherPairs;
using testing::internal::ElementsAreArrayMatcher;
using testing::internal::ExplainMatchFailureTupleTo;
using testing::internal::FloatingEqMatcher;
using testing::internal::FormatMatcherDescription;
using testing::internal::IsReadableTypeName;
using testing::internal::MatchMatrix;
using testing::internal::PredicateFormatterFromMatcher;
using testing::internal::RE;
using testing::internal::StreamMatchResultListener;
using testing::internal::Strings;

// Helper for testing container-valued matchers in mock method context. It is
// important to test matchers in this context, since it requires additional type
// deduction beyond what EXPECT_THAT does, thus making it more restrictive.
struct ContainerHelper {
  MOCK_METHOD1(Call, void(std::vector<std::unique_ptr<int>>));
};

// For testing ExplainMatchResultTo().
template <typename T>
struct GtestGreaterThanMatcher {
  using is_htest_matcher = void;

  void DescribeTo(ostream* os) const { *os << "is > " << rhs; }
  void DescribeNegationTo(ostream* os) const { *os << "is <= " << rhs; }

  bool MatchAndExplain(T lhs, MatchResultListener* listener) const {
    if (lhs > rhs) {
      *listener << "which is " << (lhs - rhs) << " more than " << rhs;
    } else if (lhs == rhs) {
      *listener << "which is the same as " << rhs;
    } else {
      *listener << "which is " << (rhs - lhs) << " less than " << rhs;
    }

    return lhs > rhs;
  }

  T rhs;
};

template <typename T>
GtestGreaterThanMatcher<std::decay_t<T>> GtestGreaterThan(T&& rhs) {
  return {rhs};
}

// As the matcher above, but using the base class with virtual functions.
template <typename T>
class GreaterThanMatcher : public MatcherInterface<T> {
 public:
  explicit GreaterThanMatcher(T rhs) : impl_{rhs} {}

  void DescribeTo(ostream* os) const override { impl_.DescribeTo(os); }
  void DescribeNegationTo(ostream* os) const override {
    impl_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(T lhs, MatchResultListener* listener) const override {
    return impl_.MatchAndExplain(lhs, listener);
  }

 private:
  const GtestGreaterThanMatcher<T> impl_;
};

// Names and instantiates a new instance of HTestMatcherTestP.
#define INSTANTIATE_HTEST_MATCHER_TEST_P(TestSuite)                        \
  using TestSuite##P = HTestMatcherTestP;                                  \
  INSTANTIATE_TEST_SUITE_P(MatcherInterface, TestSuite##P, Values(false)); \
  INSTANTIATE_TEST_SUITE_P(GtestMatcher, TestSuite##P, Values(true))

class HTestMatcherTestP : public testing::TestWithParam<bool> {
 public:
  template <typename T>
  Matcher<T> GreaterThan(T n) {
    if (use_htest_matcher_) {
      return GtestGreaterThan(n);
    } else {
      return MakeMatcher(new GreaterThanMatcher<T>(n));
    }
  }
  const bool use_htest_matcher_ = GetParam();
};

// Returns the description of the given matcher.
template <typename T>
std::string Describe(const Matcher<T>& m) {
  return DescribeMatcher<T>(m);
}

// Returns the description of the negation of the given matcher.
template <typename T>
std::string DescribeNegation(const Matcher<T>& m) {
  return DescribeMatcher<T>(m, true);
}

// Returns the reason why x matches, or doesn't match, m.
template <typename MatcherType, typename Value>
std::string Explain(const MatcherType& m, const Value& x) {
  StringMatchResultListener listener;
  ExplainMatchResult(m, x, &listener);
  return listener.str();
}

}  // namespace gmock_matchers_test
}  // namespace testing

#endif  // GOOGLEMOCK_TEST_GMOCK_MATCHERS_TEST_H_
