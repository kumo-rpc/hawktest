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

#pragma once


// This header implements typed tests and type-parameterized tests.

// Typed (aka type-driven) tests repeat the same test for types in a
// list.  You must know which types you want to test with when writing
// typed tests. Here's how you do it:

#if 0

// First, define a fixture class template.  It should be parameterized
// by a type.  Remember to derive it from hawk::Test.
template <typename T>
class [[nodiscard]] FooTest : public hawk::Test {
 public:
  ...
  using List = ::std::list<T>;
  static T shared_;
  T value_;
};

// Next, associate a list of types with the test suite, which will be
// repeated for each type in the list.  The using-declaration is necessary for
// the macro to parse correctly.
using MyTypes = ::hawk::Types<char, int, unsigned int>;
TYPED_TEST_SUITE(FooTest, MyTypes);

// If the type list contains only one type, you can write that type
// directly without Types<...>:
//   TYPED_TEST_SUITE(FooTest, int);

// Then, use TYPED_TEST() instead of TEST_F() to define as many typed
// tests for this test suite as you want.
TYPED_TEST(FooTest, DoesBlah) {
  // Inside a test, refer to the special name TypeParam to get the type
  // parameter.  Since we are inside a derived class template, C++ requires
  // us to visit the members of FooTest via 'this'.
  TypeParam n = this->value_;

  // To visit static members of the fixture, add the TestFixture::
  // prefix.
  n += TestFixture::shared_;

  // To refer to typedefs in the fixture, add the "typename
  // TestFixture::" prefix.
  typename TestFixture::List values;
  values.push_back(n);
  ...
}

TYPED_TEST(FooTest, HasPropertyA) { ... }

// TYPED_TEST_SUITE takes an optional third argument which allows to specify a
// class that generates custom test name suffixes based on the type. This should
// be a class which has a static template function GetName(int index) returning
// a string for each type. The provided integer index equals the index of the
// type in the provided type list. In many cases the index can be ignored.
//
// For example:
//   class MyTypeNames {
//    public:
//     template <typename T>
//     static std::string GetName(int) {
//       if (std::is_same<T, char>()) return "char";
//       if (std::is_same<T, int>()) return "int";
//       if (std::is_same<T, unsigned int>()) return "unsignedInt";
//     }
//   };
//   TYPED_TEST_SUITE(FooTest, MyTypes, MyTypeNames);

#endif // 0

// Type-parameterized tests are abstract test patterns parameterized
// by a type.  Compared with typed tests, type-parameterized tests
// allow you to define the test pattern without knowing what the type
// parameters are.  The defined pattern can be instantiated with
// different types any number of times, in any number of translation
// units.
//
// If you are designing an interface or concept, you can define a
// suite of type-parameterized tests to verify properties that any
// valid implementation of the interface/concept should have.  Then,
// each implementation can easily instantiate the test suite to verify
// that it conforms to the requirements, without having to write
// similar tests repeatedly.  Here's an example:

#if 0

// First, define a fixture class template.  It should be parameterized
// by a type.  Remember to derive it from hawk::Test.
template <typename T>
class [[nodiscard]] FooTest : public hawk::Test {
  ...
};

// Next, declare that you will define a type-parameterized test suite
// (the _P suffix is for "parameterized" or "pattern", whichever you
// prefer):
TYPED_TEST_SUITE_P(FooTest);

// Then, use TYPED_TEST_P() to define as many type-parameterized tests
// for this type-parameterized test suite as you want.
TYPED_TEST_P(FooTest, DoesBlah) {
  // Inside a test, refer to TypeParam to get the type parameter.
  TypeParam n = 0;
  ...
}

TYPED_TEST_P(FooTest, HasPropertyA) { ... }

// Now the tricky part: you need to register all test patterns before
// you can instantiate them.  The first argument of the macro is the
// test suite name; the rest are the names of the tests in this test
// case.
REGISTER_TYPED_TEST_SUITE_P(FooTest,
                            DoesBlah, HasPropertyA);

// Finally, you are free to instantiate the pattern with the types you
// want.  If you put the above code in a header file, you can #include
// it in multiple C++ source files and instantiate it multiple times.
//
// To distinguish different instances of the pattern, the first
// argument to the INSTANTIATE_* macro is a prefix that will be added
// to the actual test suite name.  Remember to pick unique prefixes for
// different instances.
using MyTypes = ::hawk::Types<char, int, unsigned int>;
INSTANTIATE_TYPED_TEST_SUITE_P(My, FooTest, MyTypes);

// If the type list contains only one type, you can write that type
// directly without Types<...>:
//   INSTANTIATE_TYPED_TEST_SUITE_P(My, FooTest, int);
//
// Similar to the optional argument of TYPED_TEST_SUITE above,
// INSTANTIATE_TEST_SUITE_P takes an optional fourth argument which allows to
// generate custom names.
//   INSTANTIATE_TYPED_TEST_SUITE_P(My, FooTest, MyTypes, MyTypeNames);

#endif // 0

#include <hawktest/common/internal.h>
#include <hawktest/common/port.h>
#include <hawktest/test/internal/type-util.h>

// Implements typed tests.

// INTERNAL IMPLEMENTATION - DO NOT USE IN USER CODE.
//
// Expands to the name of the typedef for the type parameters of the
// given test suite.
#define HTEST_TYPE_PARAMS_(TestSuiteName) htest_type_params_##TestSuiteName##_

// Expands to the name of the typedef for the NameGenerator, responsible for
// creating the suffixes of the name.
#define HTEST_NAME_GENERATOR_(TestSuiteName) \
    htest_type_params_##TestSuiteName##_NameGenerator

#define TYPED_TEST_SUITE(CaseName, Types, ...)                            \
    typedef ::hawk::internal::GenerateTypeList<Types>::type            \
        HTEST_TYPE_PARAMS_(CaseName);                                     \
    typedef ::hawk::internal::NameGeneratorSelector<__VA_ARGS__>::type \
    HTEST_NAME_GENERATOR_(CaseName)

#define TYPED_TEST(CaseName, TestName)                                                                                     \
    static_assert(sizeof(HTEST_STRINGIFY_(TestName)) > 1,                                                                  \
        "test-name must not be empty");                                                                                    \
    template <typename htest_TypeParam_>                                                                                   \
    class HTEST_TEST_CLASS_NAME_(CaseName, TestName)                                                                       \
        : public CaseName<htest_TypeParam_> {                                                                              \
    private:                                                                                                               \
        typedef CaseName<htest_TypeParam_> TestFixture;                                                                    \
        typedef htest_TypeParam_ TypeParam;                                                                                \
        void TestBody() override;                                                                                          \
    };                                                                                                                     \
    [[maybe_unused]] static bool htest_##CaseName##_##TestName##_registered_ = ::hawk::internal::TypeParameterizedTest< \
        CaseName,                                                                                                          \
        ::hawk::internal::TemplateSel<HTEST_TEST_CLASS_NAME_(CaseName,                                                  \
            TestName)>,                                                                                                    \
        HTEST_TYPE_PARAMS_(                                                                                                \
            CaseName)>::Register("",                                                                                       \
        ::hawk::internal::CodeLocation(                                                                                 \
            __FILE__, __LINE__),                                                                                           \
        HTEST_STRINGIFY_(CaseName),                                                                                        \
        HTEST_STRINGIFY_(TestName), 0,                                                                                     \
        ::hawk::internal::GenerateNames<                                                                                \
            HTEST_NAME_GENERATOR_(CaseName),                                                                               \
            HTEST_TYPE_PARAMS_(CaseName)>());                                                                              \
    template <typename htest_TypeParam_>                                                                                   \
    void HTEST_TEST_CLASS_NAME_(CaseName,                                                                                  \
        TestName)<htest_TypeParam_>::TestBody()

// Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
#define TYPED_TEST_CASE                                                  \
    static_assert(::hawk::internal::TypedTestCaseIsDeprecated(), ""); \
    TYPED_TEST_SUITE
#endif // HTEST_REMOVE_LEGACY_TEST_CASEAPI_

// Implements type-parameterized tests.

// INTERNAL IMPLEMENTATION - DO NOT USE IN USER CODE.
//
// Expands to the namespace name that the type-parameterized tests for
// the given type-parameterized test suite are defined in.  The exact
// name of the namespace is subject to change without notice.
#define HTEST_SUITE_NAMESPACE_(TestSuiteName) htest_suite_##TestSuiteName##_

// INTERNAL IMPLEMENTATION - DO NOT USE IN USER CODE.
//
// Expands to the name of the variable used to remember the names of
// the defined tests in the given test suite.
#define HTEST_TYPED_TEST_SUITE_P_STATE_(TestSuiteName) \
    htest_typed_test_suite_p_state_##TestSuiteName##_

// INTERNAL IMPLEMENTATION - DO NOT USE IN USER CODE DIRECTLY.
//
// Expands to the name of the variable used to remember the names of
// the registered tests in the given test suite.
#define HTEST_REGISTERED_TEST_NAMES_(TestSuiteName) \
    htest_registered_test_names_##TestSuiteName##_

// The variables defined in the type-parameterized test macros are
// static as typically these macros are used in a .h file that can be
// #included in multiple translation units linked together.
#define TYPED_TEST_SUITE_P(SuiteName)                \
    static ::hawk::internal::TypedTestSuitePState \
    HTEST_TYPED_TEST_SUITE_P_STATE_(SuiteName)

// Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
#define TYPED_TEST_CASE_P                                                   \
    static_assert(::hawk::internal::TypedTestCase_P_IsDeprecated(), ""); \
    TYPED_TEST_SUITE_P
#endif // HTEST_REMOVE_LEGACY_TEST_CASEAPI_

#define TYPED_TEST_P(SuiteName, TestName)                                                                                  \
    namespace HTEST_SUITE_NAMESPACE_(SuiteName) {                                                                          \
        template <typename htest_TypeParam_>                                                                               \
        class TestName : public SuiteName<htest_TypeParam_> {                                                              \
        private:                                                                                                           \
            typedef SuiteName<htest_TypeParam_> TestFixture;                                                               \
            typedef htest_TypeParam_ TypeParam;                                                                            \
            void TestBody() override;                                                                                      \
        };                                                                                                                 \
        [[maybe_unused]] static bool htest_##TestName##_defined_ = HTEST_TYPED_TEST_SUITE_P_STATE_(SuiteName).AddTestName( \
            __FILE__, __LINE__, HTEST_STRINGIFY_(SuiteName),                                                               \
            HTEST_STRINGIFY_(TestName));                                                                                   \
    }                                                                                                                      \
    template <typename htest_TypeParam_>                                                                                   \
    void HTEST_SUITE_NAMESPACE_(                                                                                           \
        SuiteName)::TestName<htest_TypeParam_>::TestBody()

// Note: this won't work correctly if the trailing arguments are macros.
#define REGISTER_TYPED_TEST_SUITE_P(SuiteName, ...)                          \
    namespace HTEST_SUITE_NAMESPACE_(SuiteName) {                            \
        typedef ::hawk::internal::Templates<__VA_ARGS__> htest_AllTests_; \
    }                                                                        \
    [[maybe_unused]] static const char* const HTEST_REGISTERED_TEST_NAMES_(  \
        SuiteName) = HTEST_TYPED_TEST_SUITE_P_STATE_(SuiteName).VerifyRegisteredTestNames(HTEST_STRINGIFY_(SuiteName), __FILE__, __LINE__, #__VA_ARGS__)

// Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
#define REGISTER_TYPED_TEST_CASE_P                                             \
    static_assert(::hawk::internal::RegisterTypedTestCase_P_IsDeprecated(), \
        "");                                                                   \
    REGISTER_TYPED_TEST_SUITE_P
#endif // HTEST_REMOVE_LEGACY_TEST_CASEAPI_

#define INSTANTIATE_TYPED_TEST_SUITE_P(Prefix, SuiteName, Types, ...)                                            \
    static_assert(sizeof(HTEST_STRINGIFY_(Prefix)) > 1,                                                          \
        "test-suit-prefix must not be empty");                                                                   \
    [[maybe_unused]] static bool htest_##Prefix##_##SuiteName = ::hawk::internal::TypeParameterizedTestSuite< \
        SuiteName, HTEST_SUITE_NAMESPACE_(SuiteName)::htest_AllTests_,                                           \
        ::hawk::internal::GenerateTypeList<Types>::type>::                                                    \
        Register(HTEST_STRINGIFY_(Prefix),                                                                       \
            ::hawk::internal::CodeLocation(__FILE__, __LINE__),                                               \
            &HTEST_TYPED_TEST_SUITE_P_STATE_(SuiteName),                                                         \
            HTEST_STRINGIFY_(SuiteName),                                                                         \
            HTEST_REGISTERED_TEST_NAMES_(SuiteName),                                                             \
            ::hawk::internal::GenerateNames<                                                                  \
                ::hawk::internal::NameGeneratorSelector<                                                      \
                    __VA_ARGS__>::type,                                                                          \
                ::hawk::internal::GenerateTypeList<Types>::type>())

// Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
#define INSTANTIATE_TYPED_TEST_CASE_P                                        \
    static_assert(                                                           \
        ::hawk::internal::InstantiateTypedTestCase_P_IsDeprecated(), ""); \
    INSTANTIATE_TYPED_TEST_SUITE_P
#endif // HTEST_REMOVE_LEGACY_TEST_CASEAPI_
