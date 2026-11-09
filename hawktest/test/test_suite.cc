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
#include <hawktest/common/internal-inl.h>

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
    void TestSuite::ShuffleTests(internal::Random* random) {
        Shuffle(random, &test_indices_);
    }

    // Restores the test order to before the first shuffle.
    void TestSuite::UnshuffleTests() {
        for (size_t i = 0; i < test_indices_.size(); i++) {
            test_indices_[i] = static_cast<int>(i);
        }
    }

}  // namespace testing
