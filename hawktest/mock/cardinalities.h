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
// This file implements some commonly used cardinalities.  More
// cardinalities can be defined by the user implementing the
// CardinalityInterface interface if necessary.

// IWYU pragma: private, include "gmock/gmock.h"
// IWYU pragma: friend gmock/.*

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_CARDINALITIES_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_CARDINALITIES_H_

#include <limits.h>

#include <memory>
#include <ostream>  // NOLINT

#include <hawktest/mock/internal/port.h>
#include <hawktest/test/htest.h>

HTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 \
/* class A needs to have dll-interface to be used by clients of class B */)

namespace hawk {

// To implement a cardinality Foo, define:
//   1. a class FooCardinality that implements the
//      CardinalityInterface interface, and
//   2. a factory function that creates a Cardinality object from a
//      const FooCardinality*.
//
// The two-level delegation design follows that of Matcher, providing
// consistency for extension developers.  It also eases ownership
// management as Cardinality objects can now be copied like plain values.

// The implementation of a cardinality.
class [[nodiscard]] CardinalityInterface {
 public:
  virtual ~CardinalityInterface() = default;

  // Conservative estimate on the lower/upper bound of the number of
  // calls allowed.
  virtual int ConservativeLowerBound() const { return 0; }
  virtual int ConservativeUpperBound() const { return INT_MAX; }

  // Returns true if and only if call_count calls will satisfy this
  // cardinality.
  virtual bool IsSatisfiedByCallCount(int call_count) const = 0;

  // Returns true if and only if call_count calls will saturate this
  // cardinality.
  virtual bool IsSaturatedByCallCount(int call_count) const = 0;

  // Describes self to an ostream.
  virtual void DescribeTo(::std::ostream* os) const = 0;
};

// A Cardinality is a copyable and IMMUTABLE (except by assignment)
// object that specifies how many times a mock function is expected to
// be called.  The implementation of Cardinality is just a std::shared_ptr
// to const CardinalityInterface. Don't inherit from Cardinality!
class HTEST_API_ [[nodiscard]] Cardinality {
 public:
  // Constructs a null cardinality.  Needed for storing Cardinality
  // objects in STL containers.
  Cardinality() = default;

  // Constructs a Cardinality from its implementation.
  explicit Cardinality(const CardinalityInterface* impl) : impl_(impl) {}

  // Conservative estimate on the lower/upper bound of the number of
  // calls allowed.
  int ConservativeLowerBound() const { return impl_->ConservativeLowerBound(); }
  int ConservativeUpperBound() const { return impl_->ConservativeUpperBound(); }

  // Returns true if and only if call_count calls will satisfy this
  // cardinality.
  bool IsSatisfiedByCallCount(int call_count) const {
    return impl_->IsSatisfiedByCallCount(call_count);
  }

  // Returns true if and only if call_count calls will saturate this
  // cardinality.
  bool IsSaturatedByCallCount(int call_count) const {
    return impl_->IsSaturatedByCallCount(call_count);
  }

  // Returns true if and only if call_count calls will over-saturate this
  // cardinality, i.e. exceed the maximum number of allowed calls.
  bool IsOverSaturatedByCallCount(int call_count) const {
    return impl_->IsSaturatedByCallCount(call_count) &&
           !impl_->IsSatisfiedByCallCount(call_count);
  }

  // Describes self to an ostream
  void DescribeTo(::std::ostream* os) const { impl_->DescribeTo(os); }

  // Describes the given actual call count to an ostream.
  static void DescribeActualCallCountTo(int actual_call_count,
                                        ::std::ostream* os);

 private:
  std::shared_ptr<const CardinalityInterface> impl_;
};

// Creates a cardinality that allows at least n calls.
HTEST_API_ Cardinality AtLeast(int n);

// Creates a cardinality that allows at most n calls.
HTEST_API_ Cardinality AtMost(int n);

// Creates a cardinality that allows any number of calls.
HTEST_API_ Cardinality AnyNumber();

// Creates a cardinality that allows between min and max calls.
HTEST_API_ Cardinality Between(int min, int max);

// Creates a cardinality that allows exactly n calls.
HTEST_API_ Cardinality Exactly(int n);

// Creates a cardinality from its implementation.
inline Cardinality MakeCardinality(const CardinalityInterface* c) {
  return Cardinality(c);
}

}  // namespace hawk

HTEST_DISABLE_MSC_WARNINGS_POP_()  //  4251

#endif  // GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_CARDINALITIES_H_
