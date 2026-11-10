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

// Implements class templates NiceMock, NaggyMock, and StrictMock.
//
// Given a mock class MockFoo that is created using Google Mock,
// NiceMock<MockFoo> is a subclass of MockFoo that allows
// uninteresting calls (i.e. calls to mock methods that have no
// EXPECT_CALL specs), NaggyMock<MockFoo> is a subclass of MockFoo
// that prints a warning when an uninteresting call occurs, and
// StrictMock<MockFoo> is a subclass of MockFoo that treats all
// uninteresting calls as errors.
//
// Currently a mock is naggy by default, so MockFoo and
// NaggyMock<MockFoo> behave like the same.  However, we will soon
// switch the default behavior of mocks to be nice, as that in general
// leads to more maintainable tests.  When that happens, MockFoo will
// stop behaving like NaggyMock<MockFoo> and start behaving like
// NiceMock<MockFoo>.
//
// NiceMock, NaggyMock, and StrictMock "inherit" the constructors of
// their respective base class.  Therefore you can write
// NiceMock<MockFoo>(5, "a") to construct a nice mock where MockFoo
// has a constructor that accepts (int, const char*), for example.
//
// A known limitation is that NiceMock<MockFoo>, NaggyMock<MockFoo>,
// and StrictMock<MockFoo> only works for mock methods defined using
// the MOCK_METHOD* family of macros DIRECTLY in the MockFoo class.
// If a mock method is defined in a base class of MockFoo, the "nice"
// or "strict" modifier may not affect it, depending on the compiler.
// In particular, nesting NiceMock, NaggyMock, and StrictMock is NOT
// supported.

// IWYU pragma: private, include "gmock/gmock.h"
// IWYU pragma: friend gmock/.*

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_NICE_STRICT_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_NICE_STRICT_H_

#include <cstdint>
#include <type_traits>

#include <hawktest/mock/spec-builders.h>
#include <hawktest/mock/internal/port.h>

namespace hawk {
template <class MockClass>
class [[nodiscard]] NiceMock;
template <class MockClass>
class [[nodiscard]] NaggyMock;
template <class MockClass>
class [[nodiscard]] StrictMock;

namespace internal {
template <typename T>
std::true_type StrictnessModifierProbe(const NiceMock<T>&);
template <typename T>
std::true_type StrictnessModifierProbe(const NaggyMock<T>&);
template <typename T>
std::true_type StrictnessModifierProbe(const StrictMock<T>&);
std::false_type StrictnessModifierProbe(...);

template <typename T>
constexpr bool HasStrictnessModifier() {
  return decltype(StrictnessModifierProbe(std::declval<const T&>()))::value;
}

// Base classes that register and deregister with hawk::Mock to alter the
// default behavior around uninteresting calls. Inheriting from one of these
// classes first and then MockClass ensures the MockClass constructor is run
// after registration, and that the MockClass destructor runs before
// deregistration. This guarantees that MockClass's constructor and destructor
// run with the same level of strictness as its instance methods.

#if defined(HTEST_OS_WINDOWS) && !defined(HTEST_OS_WINDOWS_MINGW) && \
    (defined(_MSC_VER) || defined(__clang__))
// We need to mark these classes with this declspec to ensure that
// the empty base class optimization is performed.
#define HTEST_INTERNAL_EMPTY_BASE_CLASS __declspec(empty_bases)
#else
#define HTEST_INTERNAL_EMPTY_BASE_CLASS
#endif

template <typename Base>
class [[nodiscard]] NiceMockImpl {
 public:
  NiceMockImpl() {
    ::hawk::Mock::AllowUninterestingCalls(reinterpret_cast<uintptr_t>(this));
  }

  ~NiceMockImpl() {
    ::hawk::Mock::UnregisterCallReaction(reinterpret_cast<uintptr_t>(this));
  }
};

template <typename Base>
class [[nodiscard]] NaggyMockImpl {
 public:
  NaggyMockImpl() {
    ::hawk::Mock::WarnUninterestingCalls(reinterpret_cast<uintptr_t>(this));
  }

  ~NaggyMockImpl() {
    ::hawk::Mock::UnregisterCallReaction(reinterpret_cast<uintptr_t>(this));
  }
};

template <typename Base>
class [[nodiscard]] StrictMockImpl {
 public:
  StrictMockImpl() {
    ::hawk::Mock::FailUninterestingCalls(reinterpret_cast<uintptr_t>(this));
  }

  ~StrictMockImpl() {
    ::hawk::Mock::UnregisterCallReaction(reinterpret_cast<uintptr_t>(this));
  }
};

}  // namespace internal

template <class MockClass>
class [[nodiscard]] HTEST_INTERNAL_EMPTY_BASE_CLASS NiceMock
    : private internal::NiceMockImpl<MockClass>,
      public MockClass {
 public:
  static_assert(!internal::HasStrictnessModifier<MockClass>(),
                "Can't apply NiceMock to a class hierarchy that already has a "
                "strictness modifier. See "
                "https://google.github.io/googletest/"
                "gmock_cook_book.html#NiceStrictNaggy");
  NiceMock() : MockClass() {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  // Ideally, we would inherit base class's constructors through a using
  // declaration, which would preserve their visibility. However, many existing
  // tests rely on the fact that current implementation reexports protected
  // constructors as public. These tests would need to be cleaned up first.

  // Single argument constructor is special-cased so that it can be
  // made explicit.
  template <typename A>
  explicit NiceMock(A&& arg) : MockClass(std::forward<A>(arg)) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  template <typename TArg1, typename TArg2, typename... An>
  NiceMock(TArg1&& arg1, TArg2&& arg2, An&&... args)
      : MockClass(std::forward<TArg1>(arg1), std::forward<TArg2>(arg2),
                  std::forward<An>(args)...) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

 private:
  NiceMock(const NiceMock&) = delete;
  NiceMock& operator=(const NiceMock&) = delete;
};

template <class MockClass>
class [[nodiscard]] HTEST_INTERNAL_EMPTY_BASE_CLASS NaggyMock
    : private internal::NaggyMockImpl<MockClass>,
      public MockClass {
  static_assert(!internal::HasStrictnessModifier<MockClass>(),
                "Can't apply NaggyMock to a class hierarchy that already has a "
                "strictness modifier. See "
                "https://google.github.io/googletest/"
                "gmock_cook_book.html#NiceStrictNaggy");

 public:
  NaggyMock() : MockClass() {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  // Ideally, we would inherit base class's constructors through a using
  // declaration, which would preserve their visibility. However, many existing
  // tests rely on the fact that current implementation reexports protected
  // constructors as public. These tests would need to be cleaned up first.

  // Single argument constructor is special-cased so that it can be
  // made explicit.
  template <typename A>
  explicit NaggyMock(A&& arg) : MockClass(std::forward<A>(arg)) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  template <typename TArg1, typename TArg2, typename... An>
  NaggyMock(TArg1&& arg1, TArg2&& arg2, An&&... args)
      : MockClass(std::forward<TArg1>(arg1), std::forward<TArg2>(arg2),
                  std::forward<An>(args)...) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

 private:
  NaggyMock(const NaggyMock&) = delete;
  NaggyMock& operator=(const NaggyMock&) = delete;
};

template <class MockClass>
class [[nodiscard]] HTEST_INTERNAL_EMPTY_BASE_CLASS StrictMock
    : private internal::StrictMockImpl<MockClass>,
      public MockClass {
 public:
  static_assert(
      !internal::HasStrictnessModifier<MockClass>(),
      "Can't apply StrictMock to a class hierarchy that already has a "
      "strictness modifier. See "
      "https://google.github.io/googletest/"
      "gmock_cook_book.html#NiceStrictNaggy");
  StrictMock() : MockClass() {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  // Ideally, we would inherit base class's constructors through a using
  // declaration, which would preserve their visibility. However, many existing
  // tests rely on the fact that current implementation reexports protected
  // constructors as public. These tests would need to be cleaned up first.

  // Single argument constructor is special-cased so that it can be
  // made explicit.
  template <typename A>
  explicit StrictMock(A&& arg) : MockClass(std::forward<A>(arg)) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  template <typename TArg1, typename TArg2, typename... An>
  StrictMock(TArg1&& arg1, TArg2&& arg2, An&&... args)
      : MockClass(std::forward<TArg1>(arg1), std::forward<TArg2>(arg2),
                  std::forward<An>(args)...) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

 private:
  StrictMock(const StrictMock&) = delete;
  StrictMock& operator=(const StrictMock&) = delete;
};

#undef HTEST_INTERNAL_EMPTY_BASE_CLASS

}  // namespace hawk

#endif  // GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_NICE_STRICT_H_
