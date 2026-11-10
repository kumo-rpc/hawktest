// Copyright 2005, Google Inc.
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

// The Google C++ Testing and Mocking Framework (Hawk Test)
//
// This header file defines the public API for Hawk Test.  It should be
// included by any test program that uses Hawk Test.
//
// IMPORTANT NOTE: Due to limitation of the C++ language, we have to
// leave some internal implementation details in this header file.
// They are clearly marked by comments like this:
//
//   // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
//
// Such code is NOT meant to be used by a user directly, and is subject
// to CHANGE WITHOUT NOTICE.  Therefore DO NOT DEPEND ON IT in a user
// program!
//
// Acknowledgment: Hawk Test borrowed the idea of automatic test
// registration from Barthelemy Dagenais' (barthelemy@prologique.com)
// easyUnit framework.

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <hawktest/analysis/assertion_result.h> // IWYU pragma: export
#include <hawktest/common/internal.h>
#include <hawktest/common/string.h>
#include <hawktest/format/message.h> // IWYU pragma: export
#include <hawktest/reporter/listener.h>
#include <hawktest/test/death-test.h> // IWYU pragma: export
#include <hawktest/test/environment.h>
#include <hawktest/test/exception.h>
#include <hawktest/test/initialize.h>
#include <hawktest/test/matchers.h> // IWYU pragma: export
#include <hawktest/test/param-test.h> // IWYU pragma: export
#include <hawktest/test/pred_impl.h> // IWYU pragma: export
#include <hawktest/format/printers.h> // IWYU pragma: export
#include <hawktest/test/prod.h> // IWYU pragma: export
#include <hawktest/test/scoped_trace.h>
#include <hawktest/test/test-part.h> // IWYU pragma: export
#include <hawktest/test/test.h>
#include <hawktest/test/test_suite.h>
#include <hawktest/test/typed-test.h> // IWYU pragma: export
#include <hawktest/test/unit_test.h>
#include <hawktest/test/with_param.h>
#include <hawktest/matcher/compare.h>
#include <hawktest/analysis/failure_reason.h>

HTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 /* class A needs to have dll-interface to be used by clients of class B */)

#include <hawktest/flags/builtin.h>

namespace hawk {

    // Silence C4100 (unreferenced formal parameter) and 4805
    // unsafe mix of type 'const int' and type 'const bool'
    HTEST_DISABLE_MSC_WARNINGS_PUSH_(4805 4100)


// Macros for indicating success/failure in test code.

// Skips test in runtime.
// Skipping test aborts current function.
// Skipped tests are neither successful nor failed.
#define HTEST_SKIP() HTEST_SKIP_("")

// ADD_FAILURE unconditionally adds a failure to the current test.
// SUCCEED generates a success - it doesn't automatically make the
// current test successful, as a test is only successful when it has
// no failure.
//
// EXPECT_* verifies that a certain condition is satisfied.  If not,
// it behaves like ADD_FAILURE.  In particular:
//
//   EXPECT_TRUE  verifies that a Boolean condition is true.
//   EXPECT_FALSE verifies that a Boolean condition is false.
//
// FAIL and ASSERT_* are similar to ADD_FAILURE and EXPECT_*, except
// that they will also abort the current function on failure.  People
// usually want the fail-fast behavior of FAIL and ASSERT_*, but those
// writing data-driven tests often find themselves using ADD_FAILURE
// and EXPECT_* more.

// Generates a nonfatal failure with a generic message.
#define ADD_FAILURE() HTEST_NONFATAL_FAILURE_("Failed")

// Generates a nonfatal failure at the given source file location with
// a generic message.
#define ADD_FAILURE_AT(file, line)          \
    HTEST_MESSAGE_AT_(file, line, "Failed", \
        ::hawk::TestPartResult::kNonFatalFailure)

// Generates a fatal failure with a generic message.
#define HTEST_FAIL() HTEST_FATAL_FAILURE_("Failed")

// Like HTEST_FAIL(), but at the given source file location.
#define HTEST_FAIL_AT(file, line)                  \
    return HTEST_MESSAGE_AT_(file, line, "Failed", \
        ::hawk::TestPartResult::kFatalFailure)

// Define this macro to 1 to omit the definition of FAIL(), which is a
// generic name and clashes with some other libraries.
#if !(defined(HTEST_DONT_DEFINE_FAIL) && HTEST_DONT_DEFINE_FAIL)
#define FAIL() HTEST_FAIL()
#define FAIL_AT(file, line) HTEST_FAIL_AT(file, line)
#endif

// Generates a success with a generic message.
#define HTEST_SUCCEED() HTEST_SUCCESS_("Succeeded")

// Define this macro to 1 to omit the definition of SUCCEED(), which
// is a generic name and clashes with some other libraries.
#if !(defined(HTEST_DONT_DEFINE_SUCCEED) && HTEST_DONT_DEFINE_SUCCEED)
#define SUCCEED() HTEST_SUCCEED()
#endif

    // Macros for testing exceptions.
    //
    //    * {ASSERT|EXPECT}_THROW(statement, expected_exception):
    //         Tests that the statement throws the expected exception.
    //    * {ASSERT|EXPECT}_NO_THROW(statement):
    //         Tests that the statement doesn't throw any exception.
    //    * {ASSERT|EXPECT}_ANY_THROW(statement):
    //         Tests that the statement throws an exception.

#define EXPECT_THROW(statement, expected_exception) \
    HTEST_TEST_THROW_(statement, expected_exception, HTEST_NONFATAL_FAILURE_)
#define EXPECT_NO_THROW(statement) \
    HTEST_TEST_NO_THROW_(statement, HTEST_NONFATAL_FAILURE_)
#define EXPECT_ANY_THROW(statement) \
    HTEST_TEST_ANY_THROW_(statement, HTEST_NONFATAL_FAILURE_)
#define ASSERT_THROW(statement, expected_exception) \
    HTEST_TEST_THROW_(statement, expected_exception, HTEST_FATAL_FAILURE_)
#define ASSERT_NO_THROW(statement) \
    HTEST_TEST_NO_THROW_(statement, HTEST_FATAL_FAILURE_)
#define ASSERT_ANY_THROW(statement) \
    HTEST_TEST_ANY_THROW_(statement, HTEST_FATAL_FAILURE_)

// Boolean assertions. Condition can be either a Boolean expression or an
// AssertionResult. For more information on how to use AssertionResult with
// these macros see comments on that class.
#define HTEST_EXPECT_TRUE(condition)                        \
    HTEST_TEST_BOOLEAN_(condition, #condition, false, true, \
        HTEST_NONFATAL_FAILURE_)
#define HTEST_EXPECT_FALSE(condition)                       \
    HTEST_TEST_BOOLEAN_(condition, #condition, true, false, \
        HTEST_NONFATAL_FAILURE_)
#define HTEST_ASSERT_TRUE(condition) \
    HTEST_TEST_BOOLEAN_(condition, #condition, false, true, HTEST_FATAL_FAILURE_)
#define HTEST_ASSERT_FALSE(condition) \
    HTEST_TEST_BOOLEAN_(condition, #condition, true, false, HTEST_FATAL_FAILURE_)

    // Define these macros to 1 to omit the definition of the corresponding
    // EXPECT or ASSERT, which clashes with some users' own code.

#if !(defined(HTEST_DONT_DEFINE_EXPECT_TRUE) && HTEST_DONT_DEFINE_EXPECT_TRUE)
#define EXPECT_TRUE(condition) HTEST_EXPECT_TRUE(condition)
#endif

#if !(defined(HTEST_DONT_DEFINE_EXPECT_FALSE) && HTEST_DONT_DEFINE_EXPECT_FALSE)
#define EXPECT_FALSE(condition) HTEST_EXPECT_FALSE(condition)
#endif

#if !(defined(HTEST_DONT_DEFINE_ASSERT_TRUE) && HTEST_DONT_DEFINE_ASSERT_TRUE)
#define ASSERT_TRUE(condition) HTEST_ASSERT_TRUE(condition)
#endif

#if !(defined(HTEST_DONT_DEFINE_ASSERT_FALSE) && HTEST_DONT_DEFINE_ASSERT_FALSE)
#define ASSERT_FALSE(condition) HTEST_ASSERT_FALSE(condition)
#endif

    // Macros for testing equalities and inequalities.
    //
    //    * {ASSERT|EXPECT}_EQ(v1, v2): Tests that v1 == v2
    //    * {ASSERT|EXPECT}_NE(v1, v2): Tests that v1 != v2
    //    * {ASSERT|EXPECT}_LT(v1, v2): Tests that v1 < v2
    //    * {ASSERT|EXPECT}_LE(v1, v2): Tests that v1 <= v2
    //    * {ASSERT|EXPECT}_GT(v1, v2): Tests that v1 > v2
    //    * {ASSERT|EXPECT}_GE(v1, v2): Tests that v1 >= v2
    //
    // When they are not, Hawk Test prints both the tested expressions and
    // their actual values.  The values must be compatible built-in types,
    // or you will get a compiler error.  By "compatible" we mean that the
    // values can be compared by the respective operator.
    //
    // Note:
    //
    //   1. It is possible to make a user-defined type work with
    //   {ASSERT|EXPECT}_??(), but that requires overloading the
    //   comparison operators and is thus discouraged by the Google C++
    //   Usage Guide.  Therefore, you are advised to use the
    //   {ASSERT|EXPECT}_TRUE() macro to assert that two objects are
    //   equal.
    //
    //   2. The {ASSERT|EXPECT}_??() macros do pointer comparisons on
    //   pointers (in particular, C strings).  Therefore, if you use it
    //   with two C strings, you are testing how their locations in memory
    //   are related, not how their content is related.  To compare two C
    //   strings by content, use {ASSERT|EXPECT}_STR*().
    //
    //   3. {ASSERT|EXPECT}_EQ(v1, v2) is preferred to
    //   {ASSERT|EXPECT}_TRUE(v1 == v2), as the former tells you
    //   what the actual value is when it fails, and similarly for the
    //   other comparisons.
    //
    //   4. Do not depend on the order in which {ASSERT|EXPECT}_??()
    //   evaluate their arguments, which is undefined.
    //
    //   5. These macros evaluate their arguments exactly once.
    //
    // Examples:
    //
    //   EXPECT_NE(Foo(), 5);
    //   EXPECT_EQ(a_pointer, NULL);
    //   ASSERT_LT(i, array_size);
    //   ASSERT_GT(records.size(), 0) << "There is no record left.";

#define EXPECT_EQ(val1, val2) \
    EXPECT_PRED_FORMAT2(::hawk::EqHelper::Compare, val1, val2)
#define EXPECT_NE(val1, val2) \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperNE, val1, val2)
#define EXPECT_LE(val1, val2) \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperLE, val1, val2)
#define EXPECT_LT(val1, val2) \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperLT, val1, val2)
#define EXPECT_GE(val1, val2) \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperGE, val1, val2)
#define EXPECT_GT(val1, val2) \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperGT, val1, val2)

#define HTEST_ASSERT_EQ(val1, val2) \
    ASSERT_PRED_FORMAT2(::hawk::EqHelper::Compare, val1, val2)
#define HTEST_ASSERT_NE(val1, val2) \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperNE, val1, val2)
#define HTEST_ASSERT_LE(val1, val2) \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperLE, val1, val2)
#define HTEST_ASSERT_LT(val1, val2) \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperLT, val1, val2)
#define HTEST_ASSERT_GE(val1, val2) \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperGE, val1, val2)
#define HTEST_ASSERT_GT(val1, val2) \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperGT, val1, val2)

    // Define macro HTEST_DONT_DEFINE_ASSERT_XY to 1 to omit the definition of
    // ASSERT_XY(), which clashes with some users' own code.

#if !(defined(HTEST_DONT_DEFINE_ASSERT_EQ) && HTEST_DONT_DEFINE_ASSERT_EQ)
#define ASSERT_EQ(val1, val2) HTEST_ASSERT_EQ(val1, val2)
#endif

#if !(defined(HTEST_DONT_DEFINE_ASSERT_NE) && HTEST_DONT_DEFINE_ASSERT_NE)
#define ASSERT_NE(val1, val2) HTEST_ASSERT_NE(val1, val2)
#endif

#if !(defined(HTEST_DONT_DEFINE_ASSERT_LE) && HTEST_DONT_DEFINE_ASSERT_LE)
#define ASSERT_LE(val1, val2) HTEST_ASSERT_LE(val1, val2)
#endif

#if !(defined(HTEST_DONT_DEFINE_ASSERT_LT) && HTEST_DONT_DEFINE_ASSERT_LT)
#define ASSERT_LT(val1, val2) HTEST_ASSERT_LT(val1, val2)
#endif

#if !(defined(HTEST_DONT_DEFINE_ASSERT_GE) && HTEST_DONT_DEFINE_ASSERT_GE)
#define ASSERT_GE(val1, val2) HTEST_ASSERT_GE(val1, val2)
#endif

#if !(defined(HTEST_DONT_DEFINE_ASSERT_GT) && HTEST_DONT_DEFINE_ASSERT_GT)
#define ASSERT_GT(val1, val2) HTEST_ASSERT_GT(val1, val2)
#endif

    // C-string Comparisons.  All tests treat NULL and any non-NULL string
    // as different.  Two NULLs are equal.
    //
    //    * {ASSERT|EXPECT}_STREQ(s1, s2):     Tests that s1 == s2
    //    * {ASSERT|EXPECT}_STRNE(s1, s2):     Tests that s1 != s2
    //    * {ASSERT|EXPECT}_STRCASEEQ(s1, s2): Tests that s1 == s2, ignoring case
    //    * {ASSERT|EXPECT}_STRCASENE(s1, s2): Tests that s1 != s2, ignoring case
    //
    // For wide or narrow string objects, you can use the
    // {ASSERT|EXPECT}_??() macros.
    //
    // Don't depend on the order in which the arguments are evaluated,
    // which is undefined.
    //
    // These macros evaluate their arguments exactly once.

#define EXPECT_STREQ(s1, s2) \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperSTREQ, s1, s2)
#define EXPECT_STRNE(s1, s2) \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperSTRNE, s1, s2)
#define EXPECT_STRCASEEQ(s1, s2) \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperSTRCASEEQ, s1, s2)
#define EXPECT_STRCASENE(s1, s2) \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperSTRCASENE, s1, s2)

#define ASSERT_STREQ(s1, s2) \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperSTREQ, s1, s2)
#define ASSERT_STRNE(s1, s2) \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperSTRNE, s1, s2)
#define ASSERT_STRCASEEQ(s1, s2) \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperSTRCASEEQ, s1, s2)
#define ASSERT_STRCASENE(s1, s2) \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperSTRCASENE, s1, s2)

    // Macros for comparing floating-point numbers.
    //
    //    * {ASSERT|EXPECT}_FLOAT_EQ(val1, val2):
    //         Tests that two float values are almost equal.
    //    * {ASSERT|EXPECT}_DOUBLE_EQ(val1, val2):
    //         Tests that two double values are almost equal.
    //    * {ASSERT|EXPECT}_NEAR(v1, v2, abs_error):
    //         Tests that v1 and v2 are within the given distance to each other.
    //
    // Hawk Test uses ULP-based comparison to automatically pick a default
    // error bound that is appropriate for the operands.  See the
    // FloatingPoint template class in htest-internal.h if you are
    // interested in the implementation details.

#define EXPECT_FLOAT_EQ(val1, val2)                                           \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperFloatingPointEQ<float>, \
        val1, val2)

#define EXPECT_DOUBLE_EQ(val1, val2)                                           \
    EXPECT_PRED_FORMAT2(::hawk::CmpHelperFloatingPointEQ<double>, \
        val1, val2)

#define ASSERT_FLOAT_EQ(val1, val2)                                           \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperFloatingPointEQ<float>, \
        val1, val2)

#define ASSERT_DOUBLE_EQ(val1, val2)                                           \
    ASSERT_PRED_FORMAT2(::hawk::CmpHelperFloatingPointEQ<double>, \
        val1, val2)

#define EXPECT_NEAR(val1, val2, abs_error)                                     \
    EXPECT_PRED_FORMAT3(::hawk::DoubleNearPredFormat, val1, val2, \
        abs_error)

#define ASSERT_NEAR(val1, val2, abs_error)                                     \
    ASSERT_PRED_FORMAT3(::hawk::DoubleNearPredFormat, val1, val2, \
        abs_error)


#ifdef HTEST_OS_WINDOWS

// Macros that test for HRESULT failure and success, these are only useful
// on Windows, and rely on Windows SDK macros and APIs to compile.
//
//    * {ASSERT|EXPECT}_HRESULT_{SUCCEEDED|FAILED}(expr)
//
// When expr unexpectedly fails or succeeds, Hawk Test prints the
// expected result and the actual result with both a human-readable
// string representation of the error, if available, as well as the
// hex result code.
#define EXPECT_HRESULT_SUCCEEDED(expr) \
    EXPECT_PRED_FORMAT1(::hawk::IsHRESULTSuccess, (expr))

#define ASSERT_HRESULT_SUCCEEDED(expr) \
    ASSERT_PRED_FORMAT1(::hawk::IsHRESULTSuccess, (expr))

#define EXPECT_HRESULT_FAILED(expr) \
    EXPECT_PRED_FORMAT1(::hawk::IsHRESULTFailure, (expr))

#define ASSERT_HRESULT_FAILED(expr) \
    ASSERT_PRED_FORMAT1(::hawk::IsHRESULTFailure, (expr))

#endif // HTEST_OS_WINDOWS

// Macros that execute statement and check that it doesn't generate new fatal
// failures in the current thread.
//
//   * {ASSERT|EXPECT}_NO_FATAL_FAILURE(statement);
//
// Examples:
//
//   EXPECT_NO_FATAL_FAILURE(Process());
//   ASSERT_NO_FATAL_FAILURE(Process()) << "Process() failed";
//
#define ASSERT_NO_FATAL_FAILURE(statement) \
    HTEST_TEST_NO_FATAL_FAILURE_(statement, HTEST_FATAL_FAILURE_)
#define EXPECT_NO_FATAL_FAILURE(statement) \
    HTEST_TEST_NO_FATAL_FAILURE_(statement, HTEST_NONFATAL_FAILURE_)

// Causes a trace (including the source file path, the current line
// number, and the given message) to be included in every test failure
// message generated by code in the current scope.  The effect is
// undone when the control leaves the current scope.
//
// The message argument can be anything streamable to std::ostream.
//
// In the implementation, we include the current line number as part
// of the dummy variable name, thus allowing multiple SCOPED_TRACE()s
// to appear in the same block - as long as they are on different
// lines.
//
// Assuming that each thread maintains its own stack of traces.
// Therefore, a SCOPED_TRACE() would (correctly) only affect the
// assertions in its own thread.
#define SCOPED_TRACE(message)                                                 \
    const ::hawk::ScopedTrace HTEST_CONCAT_TOKEN_(htest_trace_, __LINE__)( \
        __FILE__, __LINE__, (message))

    // Compile-time assertion for type equality.
    // StaticAssertTypeEq<type1, type2>() compiles if and only if type1 and type2
    // are the same type.  The value it returns is not interesting.
    //
    // Instead of making StaticAssertTypeEq a class template, we make it a
    // function template that invokes a helper class template.  This
    // prevents a user from misusing StaticAssertTypeEq<T1, T2> by
    // defining objects of that type.
    //
    // CAVEAT:
    //
    // When used inside a method of a class template,
    // StaticAssertTypeEq<T1, T2>() is effective ONLY IF the method is
    // instantiated.  For example, given:
    //
    //   template <typename T> class Foo {
    //    public:
    //     void Bar() { hawk::StaticAssertTypeEq<int, T>(); }
    //   };
    //
    // the code:
    //
    //   void Test1() { Foo<bool> foo; }
    //
    // will NOT generate a compiler error, as Foo<bool>::Bar() is never
    // actually instantiated.  Instead, you need:
    //
    //   void Test2() { Foo<bool> foo; foo.Bar(); }
    //
    // to cause a compiler error.
    template <typename T1, typename T2>
    constexpr bool StaticAssertTypeEq() noexcept {
        static_assert(std::is_same_v<T1, T2>, "T1 and T2 are not the same type");
        return true;
    }

// Defines a test.
//
// The first parameter is the name of the test suite, and the second
// parameter is the name of the test within the test suite.
//
// The convention is to end the test suite name with "Test".  For
// example, a test suite for the Foo class can be named FooTest.
//
// Test code should appear between braces after an invocation of
// this macro.  Example:
//
//   TEST(FooTest, InitializesCorrectly) {
//     Foo foo;
//     EXPECT_TRUE(foo.StatusIsOK());
//   }

// Note that we call GetTestTypeId() instead of GetTypeId<
// ::hawk::Test>() here to get the type ID of hawk::Test.  This
// is to work around a suspected linker bug when using Hawk Test as
// a framework on Mac OS X.  The bug causes GetTypeId<
// ::hawk::Test>() to return different values depending on whether
// the call is from the Hawk Test framework itself or from user test
// code.  GetTestTypeId() is guaranteed to always return the same
// value, as it always calls GetTypeId<>() from the Hawk Test
// framework.
#define HTEST_TEST(test_suite_name, test_name)               \
    HTEST_TEST_(test_suite_name, test_name, ::hawk::Test, \
        ::hawk::internal::GetTestTypeId())

// Define this macro to 1 to omit the definition of TEST(), which
// is a generic name and clashes with some other libraries.
#if !(defined(HTEST_DONT_DEFINE_TEST) && HTEST_DONT_DEFINE_TEST)
#define TEST(test_suite_name, test_name) HTEST_TEST(test_suite_name, test_name)
#endif

// Defines a test that uses a test fixture.
//
// The first parameter is the name of the test fixture class, which
// also doubles as the test suite name.  The second parameter is the
// name of the test within the test suite.
//
// A test fixture class must be declared earlier.  The user should put
// the test code between braces after using this macro.  Example:
//
//   class FooTest : public hawk::Test {
//    protected:
//     void SetUp() override { b_.AddElement(3); }
//
//     Foo a_;
//     Foo b_;
//   };
//
//   TEST_F(FooTest, InitializesCorrectly) {
//     EXPECT_TRUE(a_.StatusIsOK());
//   }
//
//   TEST_F(FooTest, ReturnsElementCountCorrectly) {
//     EXPECT_EQ(a_.size(), 0);
//     EXPECT_EQ(b_.size(), 1);
//   }
#define HTEST_TEST_F(test_fixture, test_name)          \
    HTEST_TEST_(test_fixture, test_name, test_fixture, \
        ::hawk::internal::GetTypeId<test_fixture>())
#if !(defined(HTEST_DONT_DEFINE_TEST_F) && HTEST_DONT_DEFINE_TEST_F)
#define TEST_F(test_fixture, test_name) HTEST_TEST_F(test_fixture, test_name)
#endif

    // Returns a path to a temporary directory, which should be writable. It is
    // implementation-dependent whether or not the path is terminated by the
    // directory-separator character.
    HTEST_API_ std::string TempDir();

    // Returns a path to a directory that contains ancillary data files that might
    // be used by tests. It is implementation dependent whether or not the path is
    // terminated by the directory-separator character. The directory and the files
    // in it should be considered read-only.
    HTEST_API_ std::string SrcDir();

    HTEST_DISABLE_MSC_WARNINGS_POP_() // 4805 4100

    // Dynamically registers a test with the framework.
    //
    // This is an advanced API only to be used when the `TEST` macros are
    // insufficient. The macros should be preferred when possible, as they avoid
    // most of the complexity of calling this function.
    //
    // The `factory` argument is a factory callable (move-constructible) object or
    // function pointer that creates a new instance of the Test object. It
    // handles ownership to the caller. The signature of the callable is
    // `Fixture*()`, where `Fixture` is the test fixture class for the test. All
    // tests registered with the same `test_suite_name` must return the same
    // fixture type. This is checked at runtime.
    //
    // The framework will infer the fixture class from the factory and will call
    // the `SetUpTestSuite` and `TearDownTestSuite` for it.
    //
    // Must be called before `RUN_ALL_TESTS()` is invoked, otherwise behavior is
    // undefined.
    //
    // Use case example:
    //
    // class MyFixture : public ::hawk::Test {
    //  public:
    //   // All of these optional, just like in regular macro usage.
    //   static void SetUpTestSuite() { ... }
    //   static void TearDownTestSuite() { ... }
    //   void SetUp() override { ... }
    //   void TearDown() override { ... }
    // };
    //
    // class MyTest : public MyFixture {
    //  public:
    //   explicit MyTest(int data) : data_(data) {}
    //   void TestBody() override { ... }
    //
    //  private:
    //   int data_;
    // };
    //
    // void RegisterMyTests(const std::vector<int>& values) {
    //   for (int v : values) {
    //     ::hawk::RegisterTest(
    //         "MyFixture", ("Test" + std::to_string(v)).c_str(), nullptr,
    //         std::to_string(v).c_str(),
    //         __FILE__, __LINE__,
    //         // Important to use the fixture type as the return type here.
    //         [=]() -> MyFixture* { return new MyTest(v); });
    //   }
    // }
    // ...
    // int main(int argc, char** argv) {
    //   ::hawk::init_hawk_test(&argc, argv);
    //   std::vector<int> values_to_test = LoadValuesFromConfig();
    //   RegisterMyTests(values_to_test);
    //   ...
    //   return RUN_ALL_TESTS();
    // }
    //
    template <int&... ExplicitParameterBarrier, typename Factory>
    TestInfo* RegisterTest(const char* test_suite_name, const char* test_name,
        const char* type_param, const char* value_param,
        const char* file, int line, Factory factory) {
        using TestT = std::remove_pointer_t<decltype(factory())>;

        class FactoryImpl : public internal::TestFactoryBase {
        public:
            explicit FactoryImpl(Factory f)
                : factory_(std::move(f)) { }
            Test* CreateTest() override { return factory_(); }

        private:
            Factory factory_;
        };

        return internal::MakeAndRegisterTestInfo(
            test_suite_name, test_name, type_param, value_param,
            internal::CodeLocation(file, line), internal::GetTypeId<TestT>(),
            internal::SuiteApiResolver<TestT>::GetSetUpCaseOrSuite(file, line),
            internal::SuiteApiResolver<TestT>::GetTearDownCaseOrSuite(file, line),
            new FactoryImpl { std::move(factory) });
    }

} // namespace hawk

// Use this function in main() to run all tests.  It returns 0 if all
// tests are successful, or 1 otherwise.
//
// RUN_ALL_TESTS() should be invoked after the command line has been
// parsed by init_hawk_test(). RUN_ALL_TESTS will tear down and delete any
// installed environments and should only be called once per binary.
//
// This function was formerly a macro; thus, it is in the global
// namespace and has an all-caps name.
[[nodiscard]] int RUN_ALL_TESTS();

inline int RUN_ALL_TESTS() {
    return ::hawk::UnitTest::GetInstance()->Run();
}

HTEST_DISABLE_MSC_WARNINGS_POP_() //  4251
