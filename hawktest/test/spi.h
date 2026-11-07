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

#pragma once


#include <string>

#include <hawktest/test/htest.h>

HTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 /* class A needs to have dll-interface to be used by clients of class B */)

namespace testing {

    // This helper class can be used to mock out Hawk Test failure reporting
    // so that we can test Hawk Test or code that builds on Hawk Test.
    //
    // An object of this class appends a TestPartResult object to the
    // TestPartResultArray object given in the constructor whenever a Hawk Test
    // failure is reported. It can either intercept only failures that are
    // generated in the same thread that created this object or it can intercept
    // all generated failures. The scope of this mock object can be controlled with
    // the second argument to the two arguments constructor.
    class HTEST_API_ [[nodiscard]] ScopedFakeTestPartResultReporter
        : public TestPartResultReporterInterface {
    public:
        // The two possible mocking modes of this object.
        enum InterceptMode {
            INTERCEPT_ONLY_CURRENT_THREAD, // Intercepts only thread local failures.
            INTERCEPT_ALL_THREADS // Intercepts all failures.
        };

        // The c'tor sets this object as the test part result reporter used
        // by Hawk Test.  The 'result' parameter specifies where to report the
        // results. This reporter will only catch failures generated in the current
        // thread. DEPRECATED
        explicit ScopedFakeTestPartResultReporter(TestPartResultArray* result);

        // Same as above, but you can choose the interception scope of this object.
        ScopedFakeTestPartResultReporter(InterceptMode intercept_mode,
            TestPartResultArray* result);

        // The d'tor restores the previous test part result reporter.
        ~ScopedFakeTestPartResultReporter() override;

        // Appends the TestPartResult object to the TestPartResultArray
        // received in the constructor.
        //
        // This method is from the TestPartResultReporterInterface
        // interface.
        void ReportTestPartResult(const TestPartResult& result) override;

    private:
        void Init();

        const InterceptMode intercept_mode_;
        TestPartResultReporterInterface* old_reporter_;
        TestPartResultArray* const result_;

        ScopedFakeTestPartResultReporter(const ScopedFakeTestPartResultReporter&) = delete;
        ScopedFakeTestPartResultReporter& operator=(
            const ScopedFakeTestPartResultReporter&) = delete;
    };

    namespace internal {

        // A helper class for implementing EXPECT_FATAL_FAILURE() and
        // EXPECT_NONFATAL_FAILURE().  Its destructor verifies that the given
        // TestPartResultArray contains exactly one failure that has the given
        // type and contains the given substring.  If that's not the case, a
        // non-fatal failure will be generated.
        class HTEST_API_ [[nodiscard]] SingleFailureChecker {
        public:
            // The constructor remembers the arguments.
            SingleFailureChecker(const TestPartResultArray* results,
                TestPartResult::Type type, const std::string& substr);
            ~SingleFailureChecker();

        private:
            const TestPartResultArray* const results_;
            const TestPartResult::Type type_;
            const std::string substr_;

            SingleFailureChecker(const SingleFailureChecker&) = delete;
            SingleFailureChecker& operator=(const SingleFailureChecker&) = delete;
        };

    } // namespace internal

} // namespace testing

HTEST_DISABLE_MSC_WARNINGS_POP_() //  4251

// A set of macros for testing Hawk Test assertions or code that's expected
// to generate Hawk Test fatal failures (e.g. a failure from an ASSERT_EQ, but
// not a non-fatal failure, as from EXPECT_EQ).  It verifies that the given
// statement will cause exactly one fatal Hawk Test failure with 'substr'
// being part of the failure message.
//
// There are two different versions of this macro. EXPECT_FATAL_FAILURE only
// affects and considers failures generated in the current thread and
// EXPECT_FATAL_FAILURE_ON_ALL_THREADS does the same but for all threads.
//
// The verification of the assertion is done correctly even when the statement
// throws an exception or aborts the current function.
//
// Known restrictions:
//   - 'statement' cannot reference local non-static variables or
//     non-static members of the current object.
//   - 'statement' cannot return a value.
//   - You cannot stream a failure message to this macro.
//
// Note that even though the implementations of the following two
// macros are much alike, we cannot refactor them to use a common
// helper macro, due to some peculiarity in how the preprocessor
// works.  The AcceptsMacroThatExpandsToUnprotectedComma test in
// htest_unittest.cc will fail to compile if we do that.
#define EXPECT_FATAL_FAILURE(statement, substr)                                   \
    do {                                                                          \
        class HTestExpectFatalFailureHelper {                                     \
        public:                                                                   \
            static void Execute() { statement; }                                  \
        };                                                                        \
        ::testing::TestPartResultArray htest_failures;                            \
        ::testing::internal::SingleFailureChecker htest_checker(                  \
            &htest_failures, ::testing::TestPartResult::kFatalFailure, (substr)); \
        {                                                                         \
            ::testing::ScopedFakeTestPartResultReporter htest_reporter(           \
                ::testing::ScopedFakeTestPartResultReporter::                     \
                    INTERCEPT_ONLY_CURRENT_THREAD,                                \
                &htest_failures);                                                 \
            HTestExpectFatalFailureHelper::Execute();                             \
        }                                                                         \
    } while (::testing::internal::AlwaysFalse())

#define EXPECT_FATAL_FAILURE_ON_ALL_THREADS(statement, substr)                      \
    do {                                                                            \
        class HTestExpectFatalFailureHelper {                                       \
        public:                                                                     \
            static void Execute() { statement; }                                    \
        };                                                                          \
        ::testing::TestPartResultArray htest_failures;                              \
        ::testing::internal::SingleFailureChecker htest_checker(                    \
            &htest_failures, ::testing::TestPartResult::kFatalFailure, (substr));   \
        {                                                                           \
            ::testing::ScopedFakeTestPartResultReporter htest_reporter(             \
                ::testing::ScopedFakeTestPartResultReporter::INTERCEPT_ALL_THREADS, \
                &htest_failures);                                                   \
            HTestExpectFatalFailureHelper::Execute();                               \
        }                                                                           \
    } while (::testing::internal::AlwaysFalse())

// A macro for testing Hawk Test assertions or code that's expected to
// generate Hawk Test non-fatal failures (e.g. a failure from an EXPECT_EQ,
// but not from an ASSERT_EQ). It asserts that the given statement will cause
// exactly one non-fatal Hawk Test failure with 'substr' being part of the
// failure message.
//
// There are two different versions of this macro. EXPECT_NONFATAL_FAILURE only
// affects and considers failures generated in the current thread and
// EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS does the same but for all threads.
//
// 'statement' is allowed to reference local variables and members of
// the current object.
//
// The verification of the assertion is done correctly even when the statement
// throws an exception or aborts the current function.
//
// Known restrictions:
//   - You cannot stream a failure message to this macro.
//
// Note that even though the implementations of the following two
// macros are much alike, we cannot refactor them to use a common
// helper macro, due to some peculiarity in how the preprocessor
// works.  If we do that, the code won't compile when the user gives
// EXPECT_NONFATAL_FAILURE() a statement that contains a macro that
// expands to code containing an unprotected comma.  The
// AcceptsMacroThatExpandsToUnprotectedComma test in htest_unittest.cc
// catches that.
//
// For the same reason, we have to write
//   if (::testing::internal::AlwaysTrue()) { statement; }
// instead of
//   HTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(statement)
// to avoid an MSVC warning on unreachable code.
#define EXPECT_NONFATAL_FAILURE(statement, substr)                        \
    do {                                                                  \
        ::testing::TestPartResultArray htest_failures;                    \
        ::testing::internal::SingleFailureChecker htest_checker(          \
            &htest_failures, ::testing::TestPartResult::kNonFatalFailure, \
            (substr));                                                    \
        {                                                                 \
            ::testing::ScopedFakeTestPartResultReporter htest_reporter(   \
                ::testing::ScopedFakeTestPartResultReporter::             \
                    INTERCEPT_ONLY_CURRENT_THREAD,                        \
                &htest_failures);                                         \
            if (::testing::internal::AlwaysTrue()) {                      \
                statement;                                                \
            }                                                             \
        }                                                                 \
    } while (::testing::internal::AlwaysFalse())

#define EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS(statement, substr)                   \
    do {                                                                            \
        ::testing::TestPartResultArray htest_failures;                              \
        ::testing::internal::SingleFailureChecker htest_checker(                    \
            &htest_failures, ::testing::TestPartResult::kNonFatalFailure,           \
            (substr));                                                              \
        {                                                                           \
            ::testing::ScopedFakeTestPartResultReporter htest_reporter(             \
                ::testing::ScopedFakeTestPartResultReporter::INTERCEPT_ALL_THREADS, \
                &htest_failures);                                                   \
            if (::testing::internal::AlwaysTrue()) {                                \
                statement;                                                          \
            }                                                                       \
        }                                                                           \
    } while (::testing::internal::AlwaysFalse())
