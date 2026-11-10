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
// This header file defines internal utilities needed for implementing
// death tests.  They are subject to change without notice.

#pragma once

#include <stdio.h>

#include <memory>
#include <string>
#include <string_view>

#include <hawktest/common/internal.h>
#include <hawktest/common/port.h>
#include <hawktest/test/matchers.h>
#include <hawktest/flags/builtin.h>

namespace hawk {
    namespace internal {

        // Name of the flag (needed for parsing Hawk Test flag).
        const char kInternalRunDeathTestFlag[] = "internal_run_death_test";

        // A string passed to EXPECT_DEATH (etc.) is caught by one of these overloads
        // and interpreted as a regex (rather than an Eq matcher) for legacy
        // compatibility.
        inline Matcher<const ::std::string&> MakeDeathTestMatcher(
            ::hawk::internal::RE regex) {
            return ContainsRegex(regex.pattern());
        }
        inline Matcher<const ::std::string&> MakeDeathTestMatcher(
            std::string_view regex) {
            return ContainsRegex(regex);
        }
        inline Matcher<const ::std::string&> MakeDeathTestMatcher(const char* regex) {
            return ContainsRegex(regex);
        }
        inline Matcher<const ::std::string&> MakeDeathTestMatcher(
            const ::std::string& regex) {
            return ContainsRegex(regex);
        }

        // If a Matcher<const ::std::string&> is passed to EXPECT_DEATH (etc.), it's
        // used directly.
        inline Matcher<const ::std::string&> MakeDeathTestMatcher(
            Matcher<const ::std::string&> matcher) {
            return matcher;
        }

#ifdef HTEST_HAS_DEATH_TEST

        HTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 /* class A needs to have dll-interface to be used by clients of class B */)

        // DeathTest is a class that hides much of the complexity of the
        // HTEST_DEATH_TEST_ macro.  It is abstract; its static Create method
        // returns a concrete class that depends on the prevailing death test
        // style, as defined by the --htest_death_test_style and/or
        // --htest_internal_run_death_test flags.

        // In describing the results of death tests, these terms are used with
        // the corresponding definitions:
        //
        // exit status:  The integer exit information in the format specified
        //               by wait(2)
        // exit code:    The integer code passed to exit(3), _Exit(2), or
        //               returned from main()
        class HTEST_API_ [[nodiscard]] DeathTest {
        public:
            // Create returns false if there was an error determining the
            // appropriate action to take for the current death test; for example,
            // if the htest_death_test_style flag is set to an invalid value.
            // The LastMessage method will return a more detailed message in that
            // case.  Otherwise, the DeathTest pointer pointed to by the "test"
            // argument is set.  If the death test should be skipped, the pointer
            // is set to NULL; otherwise, it is set to the address of a new concrete
            // DeathTest object that controls the execution of the current test.
            static bool Create(const char* statement, Matcher<const std::string&> matcher,
                const char* file, int line, DeathTest** test);
            DeathTest();
            virtual ~DeathTest() = default;

            // A helper class that aborts a death test when it's deleted.
            class ReturnSentinel {
            public:
                explicit ReturnSentinel(DeathTest* test)
                    : test_(test) { }
                ~ReturnSentinel() { test_->Abort(TEST_ENCOUNTERED_RETURN_STATEMENT); }

            private:
                DeathTest* const test_;
                ReturnSentinel(const ReturnSentinel&) = delete;
                ReturnSentinel& operator=(const ReturnSentinel&) = delete;
            };

            // An enumeration of possible roles that may be taken when a death
            // test is encountered.  EXECUTE means that the death test logic should
            // be executed immediately.  OVERSEE means that the program should prepare
            // the appropriate environment for a child process to execute the death
            // test, then wait for it to complete.
            enum TestRole { OVERSEE_TEST,
                EXECUTE_TEST };

            // An enumeration of the three reasons that a test might be aborted.
            enum AbortReason {
                TEST_ENCOUNTERED_RETURN_STATEMENT,
                TEST_THREW_EXCEPTION,
                TEST_DID_NOT_DIE
            };

            // Assumes one of the above roles.
            virtual TestRole AssumeRole() = 0;

            // Waits for the death test to finish and returns its status.
            virtual int Wait() = 0;

            // Returns true if the death test passed; that is, the test process
            // exited during the test, its exit status matches a user-supplied
            // predicate, and its stderr output matches a user-supplied regular
            // expression.
            // The user-supplied predicate may be a macro expression rather
            // than a function pointer or functor, or else Wait and Passed could
            // be combined.
            virtual bool Passed(bool exit_status_ok) = 0;

            // Signals that the death test did not die as expected.
            virtual void Abort(AbortReason reason) = 0;

            // Returns a human-readable outcome message regarding the outcome of
            // the last death test.
            static const char* LastMessage();

            static void set_last_death_test_message(const std::string& message);

        private:
            // A string containing a description of the outcome of the last death test.
            static std::string last_death_test_message_;

            DeathTest(const DeathTest&) = delete;
            DeathTest& operator=(const DeathTest&) = delete;
        };

        HTEST_DISABLE_MSC_WARNINGS_POP_() //  4251

        // Factory interface for death tests.  May be mocked out for testing.
        class [[nodiscard]] DeathTestFactory {
        public:
            virtual ~DeathTestFactory() = default;
            virtual bool Create(const char* statement,
                Matcher<const std::string&> matcher, const char* file,
                int line, DeathTest** test) = 0;
        };

        // A concrete DeathTestFactory implementation for normal use.
        class [[nodiscard]] DefaultDeathTestFactory : public DeathTestFactory {
        public:
            bool Create(const char* statement, Matcher<const std::string&> matcher,
                const char* file, int line, DeathTest** test) override;
        };

        // Returns true if exit_status describes a process that was terminated
        // by a signal, or exited normally with a nonzero exit code.
        HTEST_API_ bool ExitedUnsuccessfully(int exit_status);

// Traps C++ exceptions escaping statement and reports them as test
// failures. Note that trapping SEH exceptions is not implemented here.
#if HTEST_HAS_EXCEPTIONS
#define HTEST_EXECUTE_DEATH_TEST_STATEMENT_(statement, death_test)               \
    try {                                                                        \
        HTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(statement);               \
    } catch (const ::std::exception& htest_exception) {                          \
        fprintf(                                                                 \
            stderr,                                                              \
            "\n%s: Caught std::exception-derived exception escaping the "        \
            "death test statement. Exception message: %s\n",                     \
            ::hawk::internal::FormatFileLocation(__FILE__, __LINE__).c_str(), \
            htest_exception.what());                                             \
        fflush(stderr);                                                          \
        death_test->Abort(::hawk::internal::DeathTest::TEST_THREW_EXCEPTION); \
    } catch (...) {                                                              \
        death_test->Abort(::hawk::internal::DeathTest::TEST_THREW_EXCEPTION); \
    }

#else
#define HTEST_EXECUTE_DEATH_TEST_STATEMENT_(statement, death_test) \
    HTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(statement)

#endif

// This macro is for implementing ASSERT_DEATH*, EXPECT_DEATH*,
// ASSERT_EXIT*, and EXPECT_EXIT*.
#define HTEST_DEATH_TEST_(statement, predicate, regex_or_matcher, fail)              \
    HTEST_AMBIGUOUS_ELSE_BLOCKER_                                                    \
    if (::hawk::internal::AlwaysTrue()) {                                         \
        ::hawk::internal::DeathTest* htest_dt;                                    \
        if (!::hawk::internal::DeathTest::Create(                                 \
                #statement,                                                          \
                ::hawk::internal::MakeDeathTestMatcher(regex_or_matcher),         \
                __FILE__, __LINE__, &htest_dt)) {                                    \
            goto HTEST_CONCAT_TOKEN_(htest_label_, __LINE__);                        \
        }                                                                            \
        if (htest_dt != nullptr) {                                                   \
            const std::unique_ptr<::hawk::internal::DeathTest> htest_dt_ptr(      \
                htest_dt);                                                           \
            switch (htest_dt->AssumeRole()) {                                        \
            case ::hawk::internal::DeathTest::OVERSEE_TEST:                       \
                if (!htest_dt->Passed(predicate(htest_dt->Wait()))) {                \
                    goto HTEST_CONCAT_TOKEN_(htest_label_, __LINE__);                \
                }                                                                    \
                break;                                                               \
            case ::hawk::internal::DeathTest::EXECUTE_TEST: {                     \
                const ::hawk::internal::DeathTest::ReturnSentinel htest_sentinel( \
                    htest_dt);                                                       \
                HTEST_EXECUTE_DEATH_TEST_STATEMENT_(statement, htest_dt);            \
                htest_dt->Abort(::hawk::internal::DeathTest::TEST_DID_NOT_DIE);   \
                break;                                                               \
            }                                                                        \
            }                                                                        \
        }                                                                            \
    } else                                                                           \
        HTEST_CONCAT_TOKEN_(htest_label_, __LINE__)                                  \
            : fail(::hawk::internal::DeathTest::LastMessage())
// The symbol "fail" here expands to something into which a message
// can be streamed.

// This macro is for implementing ASSERT/EXPECT_DEBUG_DEATH when compiled in
// NDEBUG mode. In this case we need the statements to be executed and the macro
// must accept a streamed message even though the message is never printed.
// The regex object is not evaluated, but it is used to prevent "unused"
// warnings and to avoid an expression that doesn't compile in debug mode.
#define HTEST_EXECUTE_STATEMENT_(statement, regex_or_matcher)              \
    HTEST_AMBIGUOUS_ELSE_BLOCKER_                                          \
    if (::hawk::internal::AlwaysTrue()) {                               \
        HTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(statement);         \
    } else if (!::hawk::internal::AlwaysTrue()) {                       \
        (void)::hawk::internal::MakeDeathTestMatcher(regex_or_matcher); \
    } else                                                                 \
        ::hawk::Message()

        // A class representing the parsed contents of the
        // --htest_internal_run_death_test flag, as it existed when
        // RUN_ALL_TESTS was called.
        class [[nodiscard]] InternalRunDeathTestFlag {
        public:
            InternalRunDeathTestFlag(const std::string& a_file, int a_line, int an_index,
                int a_write_fd)
                : file_(a_file)
                , line_(a_line)
                , index_(an_index)
                , write_fd_(a_write_fd) { }

            ~InternalRunDeathTestFlag() {
                if (write_fd_ >= 0)
                    posix::Close(write_fd_);
            }

            const std::string& file() const { return file_; }
            int line() const { return line_; }
            int index() const { return index_; }
            int write_fd() const { return write_fd_; }

        private:
            std::string file_;
            int line_;
            int index_;
            int write_fd_;

            InternalRunDeathTestFlag(const InternalRunDeathTestFlag&) = delete;
            InternalRunDeathTestFlag& operator=(const InternalRunDeathTestFlag&) = delete;
        };

        // Returns a newly created InternalRunDeathTestFlag object with fields
        // initialized from the FLAGS_htest_internal_run_death_test flag if
        // the flag is specified; otherwise returns NULL.
        InternalRunDeathTestFlag* ParseInternalRunDeathTestFlag();

#endif // HTEST_HAS_DEATH_TEST

    } // namespace internal
} // namespace hawk
