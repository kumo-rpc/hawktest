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
// This file implements cardinalities.

#include <hawktest/mock/cardinalities.h>

#include <limits.h>

#include <ostream>  // NOLINT
#include <sstream>
#include <string>

#include <hawktest/mock/internal/internal-utils.h>
#include <hawktest/test/htest.h>

namespace hawk {

namespace {

// Implements the Between(m, n) cardinality.
class BetweenCardinalityImpl : public CardinalityInterface {
 public:
  BetweenCardinalityImpl(int min, int max)
      : min_(min >= 0 ? min : 0), max_(max >= min_ ? max : min_) {
    std::stringstream ss;
    if (min < 0) {
      ss << "The invocation lower bound must be >= 0, " << "but is actually "
         << min << ".";
      internal::Expect(false, __FILE__, __LINE__, ss.str());
    } else if (max < 0) {
      ss << "The invocation upper bound must be >= 0, " << "but is actually "
         << max << ".";
      internal::Expect(false, __FILE__, __LINE__, ss.str());
    } else if (min > max) {
      ss << "The invocation upper bound (" << max
         << ") must be >= the invocation lower bound (" << min << ").";
      internal::Expect(false, __FILE__, __LINE__, ss.str());
    }
  }

  // Conservative estimate on the lower/upper bound of the number of
  // calls allowed.
  int ConservativeLowerBound() const override { return min_; }
  int ConservativeUpperBound() const override { return max_; }

  bool IsSatisfiedByCallCount(int call_count) const override {
    return min_ <= call_count && call_count <= max_;
  }

  bool IsSaturatedByCallCount(int call_count) const override {
    return call_count >= max_;
  }

  void DescribeTo(::std::ostream* os) const override;

 private:
  const int min_;
  const int max_;

  BetweenCardinalityImpl(const BetweenCardinalityImpl&) = delete;
  BetweenCardinalityImpl& operator=(const BetweenCardinalityImpl&) = delete;
};

// Formats "n times" in a human-friendly way.
inline std::string FormatTimes(int n) {
  if (n == 1) {
    return "once";
  } else if (n == 2) {
    return "twice";
  } else {
    std::stringstream ss;
    ss << n << " times";
    return ss.str();
  }
}

// Describes the Between(m, n) cardinality in human-friendly text.
void BetweenCardinalityImpl::DescribeTo(::std::ostream* os) const {
  if (min_ == 0) {
    if (max_ == 0) {
      *os << "never called";
    } else if (max_ == INT_MAX) {
      *os << "called any number of times";
    } else {
      *os << "called at most " << FormatTimes(max_);
    }
  } else if (min_ == max_) {
    *os << "called " << FormatTimes(min_);
  } else if (max_ == INT_MAX) {
    *os << "called at least " << FormatTimes(min_);
  } else {
    // 0 < min_ < max_ < INT_MAX
    *os << "called between " << min_ << " and " << max_ << " times";
  }
}

}  // Unnamed namespace

// Describes the given call count to an ostream.
void Cardinality::DescribeActualCallCountTo(int actual_call_count,
                                            ::std::ostream* os) {
  if (actual_call_count > 0) {
    *os << "called " << FormatTimes(actual_call_count);
  } else {
    *os << "never called";
  }
}

// Creates a cardinality that allows at least n calls.
HTEST_API_ Cardinality AtLeast(int n) { return Between(n, INT_MAX); }

// Creates a cardinality that allows at most n calls.
HTEST_API_ Cardinality AtMost(int n) { return Between(0, n); }

// Creates a cardinality that allows any number of calls.
HTEST_API_ Cardinality AnyNumber() { return AtLeast(0); }

// Creates a cardinality that allows between min and max calls.
HTEST_API_ Cardinality Between(int min, int max) {
  return Cardinality(new BetweenCardinalityImpl(min, max));
}

// Creates a cardinality that allows exactly n calls.
HTEST_API_ Cardinality Exactly(int n) { return Between(n, n); }

}  // namespace hawk
