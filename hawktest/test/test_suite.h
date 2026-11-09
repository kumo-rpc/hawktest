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
#include <hawktest/test/test.h>
#include <hawktest/test/internal/death-test-internal.h>

namespace testing {


    // A TestInfo object stores the following information about a test:
    //
    //   Test suite name
    //   Test name
    //   Whether the test should be run
    //   A function pointer that creates the test object when invoked
    //   Test result
    //
    // The constructor of TestInfo registers itself with the UnitTest
    // singleton such that the RUN_ALL_TESTS() macro knows which tests to
    // run.
    class HTEST_API_ [[nodiscard]] TestInfo {
    public:
        // Destructs a TestInfo object.  This function is not virtual, so
        // don't inherit from TestInfo.
        ~TestInfo();

        // Returns the test suite name.
        const char* test_suite_name() const { return test_suite_name_.c_str(); }

// Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        const char* test_case_name() const { return test_suite_name(); }
#endif // HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        // Returns the test name.
        const char* name() const { return name_.c_str(); }

        // Returns the name of the parameter type, or NULL if this is not a typed
        // or a type-parameterized test.
        const char* type_param() const {
            if (type_param_ != nullptr)
                return type_param_->c_str();
            return nullptr;
        }

        // Returns the text representation of the value parameter, or NULL if this
        // is not a value-parameterized test.
        const char* value_param() const {
            if (value_param_ != nullptr)
                return value_param_->c_str();
            return nullptr;
        }

        // Returns the file name where this test is defined.
        const char* file() const { return location_.file.c_str(); }

        // Returns the line where this test is defined.
        int line() const { return location_.line; }

        // Return true if this test should not be run because it's in another shard.
        bool is_in_another_shard() const { return is_in_another_shard_; }

        // Returns true if this test should run, that is if the test is not
        // disabled (or it is disabled but the also_run_disabled_tests flag has
        // been specified) and its full name matches the user-specified filter.
        //
        // Hawk Test allows the user to filter the tests by their full names.
        // The full name of a test Bar in test suite Foo is defined as
        // "Foo.Bar".  Only the tests that match the filter will run.
        //
        // A filter is a colon-separated list of glob (not regex) patterns,
        // optionally followed by a '-' and a colon-separated list of
        // negative patterns (tests to exclude).  A test is run if it
        // matches one of the positive patterns and does not match any of
        // the negative patterns.
        //
        // For example, *A*:Foo.* is a filter that matches any string that
        // contains the character 'A' or starts with "Foo.".
        bool should_run() const { return should_run_; }

        // Returns true if and only if this test will appear in the XML report.
        bool is_reportable() const {
            // The XML report includes tests matching the filter, excluding those
            // run in other shards.
            return matches_filter_ && !is_in_another_shard_;
        }

        // Returns the result of the test.
        const TestResult* result() const { return &result_; }

    private:
#ifdef HTEST_HAS_DEATH_TEST
        friend class internal::DefaultDeathTestFactory;
#endif // HTEST_HAS_DEATH_TEST
        friend class Test;
        friend class TestSuite;
        friend class internal::UnitTestImpl;
        friend class internal::StreamingListenerTest;
        friend TestInfo* internal::MakeAndRegisterTestInfo(
            std::string test_suite_name, const char* name, const char* type_param,
            const char* value_param, internal::CodeLocation code_location,
            internal::TypeId fixture_class_id, internal::SetUpTestSuiteFunc set_up_tc,
            internal::TearDownTestSuiteFunc tear_down_tc,
            internal::TestFactoryBase* factory);

        // Constructs a TestInfo object. The newly constructed instance assumes
        // ownership of the factory object.
        TestInfo(std::string test_suite_name, std::string name,
            const char* a_type_param, // NULL if not a type-parameterized test
            const char* a_value_param, // NULL if not a value-parameterized test
            internal::CodeLocation a_code_location,
            internal::TypeId fixture_class_id,
            internal::TestFactoryBase* factory);

        // Increments the number of death tests encountered in this test so
        // far.
        int increment_death_test_count() {
            return result_.increment_death_test_count();
        }

        // Creates the test object, runs it, records its result, and then
        // deletes it.
        void Run();

        // Skip and records the test result for this object.
        void Skip();

        static void ClearTestResult(TestInfo* test_info) {
            test_info->result_.Clear();
        }

        // These fields are immutable properties of the test.
        const std::string test_suite_name_; // test suite name
        const std::string name_; // Test name
        // Name of the parameter type, or NULL if this is not a typed or a
        // type-parameterized test.
        const std::unique_ptr<const ::std::string> type_param_;
        // Text representation of the value parameter, or NULL if this is not a
        // value-parameterized test.
        const std::unique_ptr<const ::std::string> value_param_;
        internal::CodeLocation location_;
        const internal::TypeId fixture_class_id_; // ID of the test fixture class
        bool should_run_; // True if and only if this test should run
        bool is_disabled_; // True if and only if this test is disabled
        bool matches_filter_; // True if this test matches the
                              // user-specified filter.
        bool is_in_another_shard_; // Will be run in another shard.
        internal::TestFactoryBase* const factory_; // The factory that creates
                                                   // the test object

        // This field is mutable and needs to be reset before running the
        // test for the second time.
        TestResult result_;

        TestInfo(const TestInfo&) = delete;
        TestInfo& operator=(const TestInfo&) = delete;
    };


    // A test suite, which consists of a vector of TestInfos.
    //
    // TestSuite is not copyable.
    class HTEST_API_ [[nodiscard]] TestSuite {
    public:
        // Creates a TestSuite with the given name.
        //
        // TestSuite does NOT have a default constructor.  Always use this
        // constructor to create a TestSuite object.
        //
        // Arguments:
        //
        //   name:         name of the test suite
        //   a_type_param: the name of the test's type parameter, or NULL if
        //                 this is not a type-parameterized test.
        //   set_up_tc:    pointer to the function that sets up the test suite
        //   tear_down_tc: pointer to the function that tears down the test suite
        TestSuite(const std::string& name, const char* a_type_param,
            internal::SetUpTestSuiteFunc set_up_tc,
            internal::TearDownTestSuiteFunc tear_down_tc);

        // Destructor of TestSuite.
        virtual ~TestSuite();

        // Gets the name of the TestSuite.
        const char* name() const { return name_.c_str(); }

        // Returns the name of the parameter type, or NULL if this is not a
        // type-parameterized test suite.
        const char* type_param() const {
            if (type_param_ != nullptr)
                return type_param_->c_str();
            return nullptr;
        }

        // Returns true if any test in this test suite should run.
        bool should_run() const { return should_run_; }

        // Gets the number of successful tests in this test suite.
        int successful_test_count() const;

        // Gets the number of skipped tests in this test suite.
        int skipped_test_count() const;

        // Gets the number of failed tests in this test suite.
        int failed_test_count() const;

        // Gets the number of disabled tests that will be reported in the XML report.
        int reportable_disabled_test_count() const;

        // Gets the number of disabled tests in this test suite.
        int disabled_test_count() const;

        // Gets the number of tests to be printed in the XML report.
        int reportable_test_count() const;

        // Get the number of tests in this test suite that should run.
        int test_to_run_count() const;

        // Gets the number of all tests in this test suite.
        int total_test_count() const;

        // Returns true if and only if the test suite passed.
        bool Passed() const { return !Failed(); }

        // Returns true if and only if the test suite failed.
        bool Failed() const {
            return failed_test_count() > 0 || ad_hoc_test_result().Failed();
        }

        // Returns the elapsed time, in milliseconds.
        TimeInMillis elapsed_time() const { return elapsed_time_; }

        // Gets the time of the test suite start, in ms from the start of the
        // UNIX epoch.
        TimeInMillis start_timestamp() const { return start_timestamp_; }

        // Returns the i-th test among all the tests. i can range from 0 to
        // total_test_count() - 1. If i is not in that range, returns NULL.
        const TestInfo* GetTestInfo(int i) const;

        // Returns the TestResult that holds test properties recorded during
        // execution of SetUpTestSuite and TearDownTestSuite.
        const TestResult& ad_hoc_test_result() const { return ad_hoc_test_result_; }

    private:
        friend class Test;
        friend class internal::UnitTestImpl;

        // Gets the (mutable) vector of TestInfos in this TestSuite.
        std::vector<TestInfo*>& test_info_list() { return test_info_list_; }

        // Gets the (immutable) vector of TestInfos in this TestSuite.
        const std::vector<TestInfo*>& test_info_list() const {
            return test_info_list_;
        }

        // Returns the i-th test among all the tests. i can range from 0 to
        // total_test_count() - 1. If i is not in that range, returns NULL.
        TestInfo* GetMutableTestInfo(int i);

        // Sets the should_run member.
        void set_should_run(bool should) { should_run_ = should; }

        // Adds a TestInfo to this test suite.  Will delete the TestInfo upon
        // destruction of the TestSuite object.
        void AddTestInfo(TestInfo* test_info);

        // Clears the results of all tests in this test suite.
        void ClearResult();

        // Clears the results of all tests in the given test suite.
        static void ClearTestSuiteResult(TestSuite* test_suite) {
            test_suite->ClearResult();
        }

        // Runs every test in this TestSuite.
        void Run();

        // Skips the execution of tests under this TestSuite
        void Skip();

        // Runs SetUpTestSuite() for this TestSuite.  This wrapper is needed
        // for catching exceptions thrown from SetUpTestSuite().
        void RunSetUpTestSuite() {
            if (set_up_tc_ != nullptr) {
                (*set_up_tc_)();
            }
        }

        // Runs TearDownTestSuite() for this TestSuite.  This wrapper is
        // needed for catching exceptions thrown from TearDownTestSuite().
        void RunTearDownTestSuite() {
            if (tear_down_tc_ != nullptr) {
                (*tear_down_tc_)();
            }
        }

        // Returns true if and only if test passed.
        static bool TestPassed(const TestInfo* test_info) {
            return test_info->should_run() && test_info->result()->Passed();
        }

        // Returns true if and only if test skipped.
        static bool TestSkipped(const TestInfo* test_info) {
            return test_info->should_run() && test_info->result()->Skipped();
        }

        // Returns true if and only if test failed.
        static bool TestFailed(const TestInfo* test_info) {
            return test_info->should_run() && test_info->result()->Failed();
        }

        // Returns true if and only if the test is disabled and will be reported in
        // the XML report.
        static bool TestReportableDisabled(const TestInfo* test_info) {
            return test_info->is_reportable() && test_info->is_disabled_;
        }

        // Returns true if and only if test is disabled.
        static bool TestDisabled(const TestInfo* test_info) {
            return test_info->is_disabled_;
        }

        // Returns true if and only if this test will appear in the XML report.
        static bool TestReportable(const TestInfo* test_info) {
            return test_info->is_reportable();
        }

        // Returns true if the given test should run.
        static bool ShouldRunTest(const TestInfo* test_info) {
            return test_info->should_run();
        }

        // Shuffles the tests in this test suite.
        void ShuffleTests(internal::Random* random);

        // Restores the test order to before the first shuffle.
        void UnshuffleTests();

        // Name of the test suite.
        std::string name_;
        // Name of the parameter type, or NULL if this is not a typed or a
        // type-parameterized test.
        const std::unique_ptr<const ::std::string> type_param_;
        // The vector of TestInfos in their original order.  It owns the
        // elements in the vector.
        std::vector<TestInfo*> test_info_list_;
        // Provides a level of indirection for the test list to allow easy
        // shuffling and restoring the test order.  The i-th element in this
        // vector is the index of the i-th test in the shuffled test list.
        std::vector<int> test_indices_;
        // Pointer to the function that sets up the test suite.
        internal::SetUpTestSuiteFunc set_up_tc_;
        // Pointer to the function that tears down the test suite.
        internal::TearDownTestSuiteFunc tear_down_tc_;
        // True if and only if any test in this test suite should run.
        bool should_run_;
        // The start time, in milliseconds since UNIX Epoch.
        TimeInMillis start_timestamp_;
        // Elapsed time, in milliseconds.
        TimeInMillis elapsed_time_;
        // Holds test properties recorded during execution of SetUpTestSuite and
        // TearDownTestSuite.
        TestResult ad_hoc_test_result_;

        // We disallow copying TestSuites.
        TestSuite(const TestSuite&) = delete;
        TestSuite& operator=(const TestSuite&) = delete;
    };


}  // namespace testing

