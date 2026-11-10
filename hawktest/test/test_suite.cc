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


#include <hawktest/test/test_suite.h>

#include "hawktest/format/printer.h"

#include <hawktest/common/internal-inl.h>
#include <hawktest/test/internal/unit_test_impl.h>

namespace testing {

    // class TestInfo

    // Constructs a TestInfo object. It assumes ownership of the test factory
    // object.
    TestInfo::TestInfo(std::string a_test_suite_name, std::string a_name,
        const char* a_type_param, const char* a_value_param,
        internal::CodeLocation a_code_location,
        internal::TypeId fixture_class_id,
        internal::TestFactoryBase* factory)
        : test_suite_name_(std::move(a_test_suite_name))
        , name_(std::move(a_name))
        , type_param_(a_type_param ? new std::string(a_type_param) : nullptr)
        , value_param_(a_value_param ? new std::string(a_value_param) : nullptr)
        , location_(std::move(a_code_location))
        , fixture_class_id_(fixture_class_id)
        , should_run_(false)
        , is_disabled_(false)
        , matches_filter_(false)
        , is_in_another_shard_(false)
        , factory_(factory)
        , result_() { }



    // Destructs a TestInfo object.
    TestInfo::~TestInfo() {
        delete factory_;
    }

    // Skip and records a skipped test result for this object.
    void TestInfo::Skip() {
        if (!should_run_)
            return;

        UnitTest::GetInstance()->set_current_test_info(this);

        TestEventListener* repeater = UnitTest::GetInstance()->listeners().repeater();

        // Notifies the unit test event listeners that a test is about to start.
        repeater->OnTestStart(*this);

        const TestPartResult test_part_result = TestPartResult(TestPartResult::kSkip, this->file(), this->line(), "");
        internal::GetUnitTestImpl()
            ->GetTestPartResultReporterForCurrentThread()
            ->ReportTestPartResult(test_part_result);

        // Notifies the unit test event listener that a test has just finished.
        repeater->OnTestEnd(*this);
        UnitTest::GetInstance()->set_current_test_info(nullptr);
    }


    using internal::CountIf;
    using internal::ForEach;
    using internal::GetElementOr;
    using internal::Shuffle;

    // Gets the number of successful tests in this test suite.
    int TestSuite::successful_test_count() const {
        return CountIf(test_info_list_, TestPassed);
    }

    // Gets the number of successful tests in this test suite.
    int TestSuite::skipped_test_count() const {
        return CountIf(test_info_list_, TestSkipped);
    }



    // Gets the number of failed tests in this test suite.
    int TestSuite::failed_test_count() const {
        return CountIf(test_info_list_, TestFailed);
    }

    // Gets the number of disabled tests that will be reported in the XML report.
    int TestSuite::reportable_disabled_test_count() const {
        return CountIf(test_info_list_, TestReportableDisabled);
    }

    // Gets the number of disabled tests in this test suite.
    int TestSuite::disabled_test_count() const {
        return CountIf(test_info_list_, TestDisabled);
    }

    // Gets the number of tests to be printed in the XML report.
    int TestSuite::reportable_test_count() const {
        return CountIf(test_info_list_, TestReportable);
    }

    // Get the number of tests in this test suite that should run.
    int TestSuite::test_to_run_count() const {
        return CountIf(test_info_list_, ShouldRunTest);
    }

    // Gets the number of all tests.
    int TestSuite::total_test_count() const {
        return static_cast<int>(test_info_list_.size());
    }


    // Creates a TestSuite with the given name.
    //
    // Arguments:
    //
    //   a_name:       name of the test suite
    //   a_type_param: the name of the test suite's type parameter, or NULL if
    //                 this is not a typed or a type-parameterized test suite.
    //   set_up_tc:    pointer to the function that sets up the test suite
    //   tear_down_tc: pointer to the function that tears down the test suite
    TestSuite::TestSuite(const std::string& a_name, const char* a_type_param,
        internal::SetUpTestSuiteFunc set_up_tc,
        internal::TearDownTestSuiteFunc tear_down_tc)
        : name_(a_name)
        , type_param_(a_type_param ? new std::string(a_type_param) : nullptr)
        , set_up_tc_(set_up_tc)
        , tear_down_tc_(tear_down_tc)
        , should_run_(false)
        , start_timestamp_(0)
        , elapsed_time_(0) { }

    // Destructor of TestSuite.
    TestSuite::~TestSuite() {
        // Deletes every Test in the collection.
        ForEach(test_info_list_, internal::Delete<TestInfo>);
    }

    // Returns the i-th test among all the tests. i can range from 0 to
    // total_test_count() - 1. If i is not in that range, returns NULL.
    const TestInfo* TestSuite::GetTestInfo(int i) const {
        const int index = GetElementOr(test_indices_, i, -1);
        return index < 0 ? nullptr : test_info_list_[static_cast<size_t>(index)];
    }

    // Returns the i-th test among all the tests. i can range from 0 to
    // total_test_count() - 1. If i is not in that range, returns NULL.
    TestInfo* TestSuite::GetMutableTestInfo(int i) {
        const int index = GetElementOr(test_indices_, i, -1);
        return index < 0 ? nullptr : test_info_list_[static_cast<size_t>(index)];
    }


    // Adds a test to this test suite.  Will delete the test upon
    // destruction of the TestSuite object.
    void TestSuite::AddTestInfo(TestInfo* test_info) {
        test_info_list_.push_back(test_info);
        test_indices_.push_back(static_cast<int>(test_indices_.size()));
    }

    // Skips all tests under this TestSuite.
    void TestSuite::Skip() {
        if (!should_run_)
            return;

        UnitTest::GetInstance()->set_current_test_suite(this);

        TestEventListener* repeater = UnitTest::GetInstance()->listeners().repeater();

        // Call both legacy and the new API
        repeater->OnTestSuiteStart(*this);
        //  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        repeater->OnTestCaseStart(*this);
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        for (int i = 0; i < total_test_count(); i++) {
            GetMutableTestInfo(i)->Skip();
        }

        // Call both legacy and the new API
        repeater->OnTestSuiteEnd(*this);
        // Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        repeater->OnTestCaseEnd(*this);
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        UnitTest::GetInstance()->set_current_test_suite(nullptr);
    }

    // Clears the results of all tests in this test suite.
    void TestSuite::ClearResult() {
        ad_hoc_test_result_.Clear();
        ForEach(test_info_list_, TestInfo::ClearTestResult);
    }

    // Shuffles the tests in this test suite.
    void TestSuite::ShuffleTests(hawk::Random* random) {
        Shuffle(random, &test_indices_);
    }

    // Restores the test order to before the first shuffle.
    void TestSuite::UnshuffleTests() {
        for (size_t i = 0; i < test_indices_.size(); i++) {
            test_indices_[i] = static_cast<int>(i);
        }
    }

    namespace internal {

        void PrintFullTestCommentIfPresent(const TestInfo& test_info) {
            const char* const type_param = test_info.type_param();
            const char* const value_param = test_info.value_param();

            if (type_param != nullptr || value_param != nullptr) {
                printf(", where ");
                if (type_param != nullptr) {
                    hawk::stdout_printer().fmt("{} = {}", hawk::Constants::kTypeParamLabel, type_param);
                    if (value_param != nullptr)
                        printf(" and ");
                }
                if (value_param != nullptr) {
                    hawk::stdout_printer().fmt("{} = {}", hawk::Constants::kValueParamLabel, value_param);
                }
            }
        }

    }

    // Creates the test object, runs it, records its result, and then
    // deletes it.
    void TestInfo::Run() {
        TestEventListener* repeater = UnitTest::GetInstance()->listeners().repeater();
        if (!should_run_) {
            if (is_disabled_ && matches_filter_)
                repeater->OnTestDisabled(*this);
            return;
        }

        // Tells UnitTest where to store test result.
        UnitTest::GetInstance()->set_current_test_info(this);

        // Notifies the unit test event listeners that a test is about to start.
        repeater->OnTestStart(*this);
        result_.set_start_timestamp(hawk::GetTimeInMillis());
        hawk::Timer timer;
        UnitTest::GetInstance()->UponLeavingHTest();

        // Creates the test object.
        Test* const test = internal::HandleExceptionsInMethodIfSupported(
            factory_, &internal::TestFactoryBase::CreateTest,
            "the test fixture's constructor");

        // Runs the test if the constructor didn't generate a fatal failure or invoke
        // HTEST_SKIP().
        // Note that the object will not be null
        if (!Test::HasFatalFailure() && !Test::IsSkipped()) {
            // This doesn't throw as all user code that can throw are wrapped into
            // exception handling code.
            test->Run();
        }

        if (test != nullptr) {
            // Deletes the test object.
            UnitTest::GetInstance()->UponLeavingHTest();
            internal::HandleExceptionsInMethodIfSupported(
                test, &Test::DeleteSelf_, "the test fixture's destructor");
        }

        result_.set_elapsed_time(timer.Elapsed());

        // Notifies the unit test event listener that a test has just finished.
        repeater->OnTestEnd(*this);

        // Tells UnitTest to stop associating assertion results to this
        // test.
        UnitTest::GetInstance()->set_current_test_info(nullptr);
    }


    // Runs every test in this TestSuite.
    void TestSuite::Run() {
        if (!should_run_)
            return;

        UnitTest::GetInstance()->set_current_test_suite(this);

        TestEventListener* repeater = UnitTest::GetInstance()->listeners().repeater();

        // Ensure our tests are in a deterministic order.
        //
        // We do this by sorting lexicographically on (file, line number), providing
        // an order matching what the user can see in the source code.
        //
        // In the common case the line number comparison shouldn't be necessary,
        // because the registrations made by the TEST macro are executed in order
        // within a translation unit. But this is not true of the manual registration
        // API, and in more exotic scenarios a single file may be part of multiple
        // translation units.
        std::stable_sort(test_info_list_.begin(), test_info_list_.end(),
            [](const TestInfo* const a, const TestInfo* const b) {
                if (const int result = std::strcmp(a->file(), b->file())) {
                    return result < 0;
                }

                return a->line() < b->line();
            });

        // Call both legacy and the new API
        repeater->OnTestSuiteStart(*this);
//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        repeater->OnTestCaseStart(*this);
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        UnitTest::GetInstance()->UponLeavingHTest();
        internal::HandleExceptionsInMethodIfSupported(
            this, &TestSuite::RunSetUpTestSuite, "SetUpTestSuite()");

        const bool skip_all = ad_hoc_test_result().Failed() || ad_hoc_test_result().Skipped();

        start_timestamp_ = hawk::GetTimeInMillis();
        hawk::Timer timer;
        for (int i = 0; i < total_test_count(); i++) {
            if (skip_all) {
                GetMutableTestInfo(i)->Skip();
            } else {
                GetMutableTestInfo(i)->Run();
            }
            if (FLAGS_htest_fail_fast && GetMutableTestInfo(i)->result()->Failed()) {
                for (int j = i + 1; j < total_test_count(); j++) {
                    GetMutableTestInfo(j)->Skip();
                }
                break;
            }
        }
        elapsed_time_ = timer.Elapsed();

        UnitTest::GetInstance()->UponLeavingHTest();
        internal::HandleExceptionsInMethodIfSupported(
            this, &TestSuite::RunTearDownTestSuite, "TearDownTestSuite()");

        // Call both legacy and the new API
        repeater->OnTestSuiteEnd(*this);
//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        repeater->OnTestCaseEnd(*this);
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        UnitTest::GetInstance()->set_current_test_suite(nullptr);
    }
}  // namespace testing
