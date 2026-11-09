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

#pragma once

#include <hawktest/test/fwd.h>
#include <hawktest/test/environment.h>
#include <hawktest/test/listener.h>
#include <hawktest/test/internal/param-util.h>

namespace testing {


    // A UnitTest consists of a vector of TestSuites.
    //
    // This is a singleton class.  The only instance of UnitTest is
    // created when UnitTest::GetInstance() is first called.  This
    // instance is never deleted.
    //
    // UnitTest is not copyable.
    //
    // This class is thread-safe as long as the methods are called
    // according to their specification.
    class HTEST_API_ [[nodiscard]] UnitTest {
    public:
        // Gets the singleton UnitTest object.  The first time this method
        // is called, a UnitTest object is constructed and returned.
        // Consecutive calls will return the same object.
        static UnitTest* GetInstance();

        // Runs all tests in this UnitTest object and prints the result.
        // Returns 0 if successful, or 1 otherwise.
        //
        // This method can only be called from the main thread.
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        [[nodiscard]] int Run();

        // Returns the working directory when the first TEST() or TEST_F()
        // was executed.  The UnitTest object owns the string.
        const char* original_working_dir() const;

        // Returns the TestSuite object for the test that's currently running,
        // or NULL if no test is running.
        const TestSuite* current_test_suite() const HTEST_LOCK_EXCLUDED_(mutex_);

// Legacy API is still available but deprecated
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        const TestCase* current_test_case() const HTEST_LOCK_EXCLUDED_(mutex_);
#endif

        // Returns the TestInfo object for the test that's currently running,
        // or NULL if no test is running.
        const TestInfo* current_test_info() const HTEST_LOCK_EXCLUDED_(mutex_);

        // Returns the random seed used at the start of the current test run.
        int random_seed() const;

        // Returns the ParameterizedTestSuiteRegistry object used to keep track of
        // value-parameterized tests and instantiate and register them.
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        internal::ParameterizedTestSuiteRegistry& parameterized_test_registry()
            HTEST_LOCK_EXCLUDED_(mutex_);

        // Gets the number of successful test suites.
        int successful_test_suite_count() const;

        // Gets the number of failed test suites.
        int failed_test_suite_count() const;

        // Gets the number of all test suites.
        int total_test_suite_count() const;

        // Gets the number of all test suites that contain at least one test
        // that should run.
        int test_suite_to_run_count() const;

        //  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        int successful_test_case_count() const;
        int failed_test_case_count() const;
        int total_test_case_count() const;
        int test_case_to_run_count() const;
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        // Gets the number of successful tests.
        int successful_test_count() const;

        // Gets the number of skipped tests.
        int skipped_test_count() const;

        // Gets the number of failed tests.
        int failed_test_count() const;

        // Gets the number of disabled tests that will be reported in the XML report.
        int reportable_disabled_test_count() const;

        // Gets the number of disabled tests.
        int disabled_test_count() const;

        // Gets the number of tests to be printed in the XML report.
        int reportable_test_count() const;

        // Gets the number of all tests.
        int total_test_count() const;

        // Gets the number of tests that should run.
        int test_to_run_count() const;

        // Gets the time of the test program start, in ms from the start of the
        // UNIX epoch.
        TimeInMillis start_timestamp() const;

        // Gets the elapsed time, in milliseconds.
        TimeInMillis elapsed_time() const;

        // Returns true if and only if the unit test passed (i.e. all test suites
        // passed).
        bool Passed() const;

        // Returns true if and only if the unit test failed (i.e. some test suite
        // failed or something outside of all tests failed).
        bool Failed() const;

        // Gets the i-th test suite among all the test suites. i can range from 0 to
        // total_test_suite_count() - 1. If i is not in that range, returns NULL.
        const TestSuite* GetTestSuite(int i) const;

//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        const TestCase* GetTestCase(int i) const;
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        // Returns the TestResult containing information on test failures and
        // properties logged outside of individual test suites.
        const TestResult& ad_hoc_test_result() const;

        // Returns the list of event listeners that can be used to track events
        // inside Hawk Test.
        TestEventListeners& listeners();

    private:
        // Registers and returns a global test environment.  When a test
        // program is run, all global test environments will be set-up in
        // the order they were registered.  After all tests in the program
        // have finished, all global test environments will be torn-down in
        // the *reverse* order they were registered.
        //
        // The UnitTest object takes ownership of the given environment.
        //
        // This method can only be called from the main thread.
        Environment* AddEnvironment(Environment* env);

        // Adds a TestPartResult to the current TestResult object.  All
        // Hawk Test assertion macros (e.g. ASSERT_TRUE, EXPECT_EQ, etc)
        // eventually call this to report their results.  The user code
        // should use the assertion macros instead of calling this directly.
        void AddTestPartResult(TestPartResult::Type result_type,
            std::string_view file_name, int line_number,
            const std::string& message,
            const std::string& os_stack_trace)
            HTEST_LOCK_EXCLUDED_(mutex_);

        // Adds a TestProperty to the current TestResult object when invoked from
        // inside a test, to current TestSuite's ad_hoc_test_result_ when invoked
        // from SetUpTestSuite or TearDownTestSuite, or to the global property set
        // when invoked elsewhere.  If the result already contains a property with
        // the same key, the value will be updated.
        void RecordProperty(const std::string& key, const std::string& value);

        // Gets the i-th test suite among all the test suites. i can range from 0 to
        // total_test_suite_count() - 1. If i is not in that range, returns NULL.
        TestSuite* GetMutableTestSuite(int i);

        // Invokes OsStackTrackGetterInterface::UponLeavingHTest. UponLeavingHTest()
        // should be called immediately before Hawk Test calls user code. It saves
        // some information about the current stack that CurrentStackTrace() will use
        // to find and hide Hawk Test stack frames.
        void UponLeavingHTest();

        // Sets the TestSuite object for the test that's currently running.
        void set_current_test_suite(TestSuite* a_current_test_suite)
            HTEST_LOCK_EXCLUDED_(mutex_);

        // Sets the TestInfo object for the test that's currently running.
        void set_current_test_info(TestInfo* a_current_test_info)
            HTEST_LOCK_EXCLUDED_(mutex_);

        // Accessors for the implementation object.
        internal::UnitTestImpl* impl() { return impl_; }
        const internal::UnitTestImpl* impl() const { return impl_; }

        // These classes and functions are friends as they need to access private
        // members of UnitTest.
        friend class ScopedTrace;
        friend class Test;
        friend class TestInfo;
        friend class TestSuite;
        friend class internal::AssertHelper;
        friend class internal::StreamingListenerTest;
        friend class internal::UnitTestRecordPropertyTestHelper;
        friend Environment* AddGlobalTestEnvironment(Environment* env);
        friend std::set<std::string>* internal::GetIgnoredParameterizedTestSuites();
        friend internal::UnitTestImpl* internal::GetUnitTestImpl();
        friend void internal::ReportFailureInUnknownLocation(
            TestPartResult::Type result_type, const std::string& message);

        // Creates an empty UnitTest.
        UnitTest();

        // D'tor
        virtual ~UnitTest();

        // Pushes a trace defined by SCOPED_TRACE() on to the per-thread
        // Hawk Test trace stack.
        void PushHTestTrace(const internal::TraceInfo& trace)
            HTEST_LOCK_EXCLUDED_(mutex_);

        // Pops a trace from the per-thread Hawk Test trace stack.
        void PopHTestTrace() HTEST_LOCK_EXCLUDED_(mutex_);

        // Protects mutable state in *impl_.  This is mutable as some const
        // methods need to lock it too.
        mutable internal::Mutex mutex_;

        // Opaque implementation object.  This field is never changed once
        // the object is constructed.  We don't mark it as const here, as
        // doing so will cause a warning in the constructor of UnitTest.
        // Mutable state in *impl_ is protected by mutex_.
        internal::UnitTestImpl* impl_;

        // We disallow copying UnitTest.
        UnitTest(const UnitTest&) = delete;
        UnitTest& operator=(const UnitTest&) = delete;
    };

    // A convenient wrapper for adding an environment for the test
    // program.
    //
    // You should call this before RUN_ALL_TESTS() is called, probably in
    // main().  If you use htest_main, you need to call this before main()
    // starts for it to take effect.  For example, you can define a global
    // variable like this:
    //
    //   testing::Environment* const foo_env =
    //       testing::AddGlobalTestEnvironment(new FooEnvironment);
    //
    // However, we strongly recommend you to write your own main() and
    // call AddGlobalTestEnvironment() there, as relying on initialization
    // of global variables makes the code harder to read and may cause
    // problems when you register multiple environments from different
    // translation units and the environments have dependencies among them
    // (remember that the compiler doesn't guarantee the order in which
    // global variables from different translation units are initialized).
    inline Environment* AddGlobalTestEnvironment(Environment* env) {
        return UnitTest::GetInstance()->AddEnvironment(env);
    }

}  // namespace testing

