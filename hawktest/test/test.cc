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


#include <hawktest/test/test.h>
#include <hawktest/common/internal-inl.h>
#include <hawktest/test/internal/unit_test_impl.h>
#include <hawktest/test/internal/json_unit_test_result_printer.h>
#include <hawktest/reportor/constants.h>

namespace testing {

    // class Test

    // Creates a Test object.

    // The c'tor saves the states of all flags.
    Test::Test()
        : htest_flag_saver_(new ::testing::internal::HTestFlagSaver) { }

    // The d'tor restores the states of all flags.  The actual work is
    // done by the d'tor of the htest_flag_saver_ field, and thus not
    // visible here.
    Test::~Test() = default;

    // Sets up the test fixture.
    //
    // A sub-class may override this.
    void Test::SetUp() { }

    // Tears down the test fixture.
    //
    // A sub-class may override this.
    void Test::TearDown() { }

    // Allows user supplied key value pairs to be recorded for later output.
    void Test::RecordProperty(const std::string& key, const std::string& value) {
        UnitTest::GetInstance()->RecordProperty(key, value);
    }

        // Hawk Test requires all tests in the same test suite to use the same test
    // fixture class.  This function checks if the current test has the
    // same fixture class as the first test in the current test suite.  If
    // yes, it returns true; otherwise it generates a Hawk Test failure and
    // returns false.
    bool Test::HasSameFixtureClass() {
        internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
        const TestSuite* const test_suite = impl->current_test_suite();

        // Info about the first test in the current test suite.
        const TestInfo* const first_test_info = test_suite->test_info_list()[0];
        const internal::TypeId first_fixture_id = first_test_info->fixture_class_id_;
        const char* const first_test_name = first_test_info->name();

        // Info about the current test.
        const TestInfo* const this_test_info = impl->current_test_info();
        const internal::TypeId this_fixture_id = this_test_info->fixture_class_id_;
        const char* const this_test_name = this_test_info->name();

        if (this_fixture_id != first_fixture_id) {
            // Is the first test defined using TEST?
            const bool first_is_TEST = first_fixture_id == internal::GetTestTypeId();
            // Is this test defined using TEST?
            const bool this_is_TEST = this_fixture_id == internal::GetTestTypeId();

            if (first_is_TEST || this_is_TEST) {
                // Both TEST and TEST_F appear in same test suite, which is incorrect.
                // Tell the user how to fix this.

                // Gets the name of the TEST and the name of the TEST_F.  Note
                // that first_is_TEST and this_is_TEST cannot both be true, as
                // the fixture IDs are different for the two tests.
                const char* const TEST_name = first_is_TEST ? first_test_name : this_test_name;
                const char* const TEST_F_name = first_is_TEST ? this_test_name : first_test_name;

                ADD_FAILURE()
                    << "All tests in the same test suite must use the same test fixture\n"
                    << "class, so mixing TEST_F and TEST in the same test suite is\n"
                    << "illegal.  In test suite " << this_test_info->test_suite_name()
                    << ",\n"
                    << "test " << TEST_F_name << " is defined using TEST_F but\n"
                    << "test " << TEST_name << " is defined using TEST.  You probably\n"
                    << "want to change the TEST to TEST_F or move it to another test\n"
                    << "case.";
            } else {
                // Two fixture classes with the same name appear in two different
                // namespaces, which is not allowed. Tell the user how to fix this.
                ADD_FAILURE()
                    << "All tests in the same test suite must use the same test fixture\n"
                    << "class.  However, in test suite "
                    << this_test_info->test_suite_name() << ",\n"
                    << "you defined test " << first_test_name << " and test "
                    << this_test_name << "\n"
                    << "using two different test fixture classes.  This can happen if\n"
                    << "the two classes are from different namespaces or translation\n"
                    << "units and have the same name.  You should probably rename one\n"
                    << "of the classes to put the tests into different test suites.";
            }
            return false;
        }

        return true;
    }

    // Returns true if and only if the current test has a fatal failure.
    bool Test::HasFatalFailure() {
        return internal::GetUnitTestImpl()->current_test_result()->HasFatalFailure();
    }

    // Returns true if and only if the current test has a non-fatal failure.
    bool Test::HasNonfatalFailure() {
        return internal::GetUnitTestImpl()
            ->current_test_result()
            ->HasNonfatalFailure();
    }
    // Returns true if and only if the current test was skipped.
    bool Test::IsSkipped() {
        return internal::GetUnitTestImpl()->current_test_result()->Skipped();
    }


    // Creates an empty TestResult.
    TestResult::TestResult()
        : death_test_count_(0)
        , start_timestamp_(0)
        , elapsed_time_(0) { }

    // D'tor.
    TestResult::~TestResult() = default;

    // Returns the i-th test part result among all the results. i can
    // range from 0 to total_part_count() - 1. If i is not in that range,
    // aborts the program.
    const TestPartResult& TestResult::GetTestPartResult(int i) const {
        if (i < 0 || i >= total_part_count())
            internal::posix::Abort();
        return test_part_results_.at(static_cast<size_t>(i));
    }

    // Returns the i-th test property. i can range from 0 to
    // test_property_count() - 1. If i is not in that range, aborts the
    // program.
    const TestProperty& TestResult::GetTestProperty(int i) const {
        if (i < 0 || i >= test_property_count())
            internal::posix::Abort();
        return test_properties_.at(static_cast<size_t>(i));
    }

    // Clears the test part results.
    void TestResult::ClearTestPartResults() {
        test_part_results_.clear();
    }


    // Adds a test part result to the list.
    void TestResult::AddTestPartResult(const TestPartResult& test_part_result) {
        test_part_results_.push_back(test_part_result);
    }


    // Adds a test property to the list. If a property with the same key as the
    // supplied property is already represented, the value of this test_property
    // replaces the old value for that key.
    void TestResult::RecordProperty(const std::string& xml_element,
        const TestProperty& test_property) {
        if (!ValidateTestProperty(xml_element, test_property)) {
            return;
        }
        internal::MutexLock lock(test_properties_mutex_);
        const std::vector<TestProperty>::iterator property_with_matching_key = std::find_if(test_properties_.begin(), test_properties_.end(),
            internal::TestPropertyKeyIs(test_property.key()));
        if (property_with_matching_key == test_properties_.end()) {
            test_properties_.push_back(test_property);
            return;
        }
        property_with_matching_key->SetValue(test_property.value());
    }

    // Clears the object.
    void TestResult::Clear() {
        test_part_results_.clear();
        test_properties_.clear();
        death_test_count_ = 0;
        elapsed_time_ = 0;
    }

    // Returns true off the test part was skipped.
    static bool TestPartSkipped(const TestPartResult& result) {
        return result.skipped();
    }

    // Returns true if and only if the test was skipped.
    bool TestResult::Skipped() const {
        return !Failed() && internal::CountIf(test_part_results_, TestPartSkipped) > 0;
    }


    // Returns true if and only if the test failed.
    bool TestResult::Failed() const {
        for (int i = 0; i < total_part_count(); ++i) {
            if (GetTestPartResult(i).failed())
                return true;
        }
        return false;
    }


    // Returns true if and only if the test part fatally failed.
    static bool TestPartFatallyFailed(const TestPartResult& result) {
        return result.fatally_failed();
    }

    // Returns true if and only if the test fatally failed.
    bool TestResult::HasFatalFailure() const {
        return internal::CountIf(test_part_results_, TestPartFatallyFailed) > 0;
    }



    // Returns true if and only if the test part non-fatally failed.
    static bool TestPartNonfatallyFailed(const TestPartResult& result) {
        return result.nonfatally_failed();
    }

    // Returns true if and only if the test has a non-fatal failure.
    bool TestResult::HasNonfatalFailure() const {
        return internal::CountIf(test_part_results_, TestPartNonfatallyFailed) > 0;
    }

    // Gets the number of all test parts.  This is the sum of the number
    // of successful test parts and the number of failed test parts.
    int TestResult::total_part_count() const {
        return static_cast<int>(test_part_results_.size());
    }

    // Returns the number of the test properties.
    int TestResult::test_property_count() const {
        return static_cast<int>(test_properties_.size());
    }


    static std::string FormatWordList(const std::vector<std::string>& words) {
        Message word_list;
        for (size_t i = 0; i < words.size(); ++i) {
            if (i > 0 && words.size() > 2) {
                word_list << ", ";
            }
            if (i == words.size() - 1) {
                word_list << "and ";
            }
            word_list << "'" << words[i] << "'";
        }
        return word_list.GetString();
    }


    static bool ValidateTestPropertyName(
        const std::string& property_name,
        const std::vector<std::string>& reserved_names) {
        if (std::find(reserved_names.begin(), reserved_names.end(), property_name) != reserved_names.end()) {
            ADD_FAILURE() << "Reserved key used in RecordProperty(): " << property_name
                          << " (" << FormatWordList(reserved_names)
                          << " are reserved by " << HTEST_NAME_ << ")";
            return false;
        }
        return true;
    }


    static std::vector<std::string> GetReservedAttributesForElement(
        const std::string& xml_element) {
        if (xml_element == "testsuites") {
            return hawk::Constants::kReservedTestSuitesAttributes;
        } else if (xml_element == "testsuite") {
            return hawk::Constants::kReservedTestSuiteAttributes;
        } else if (xml_element == "testcase") {
            return hawk::Constants::kReservedTestCaseAttributes;
        } else {
            HTEST_CHECK_(false) << "Unrecognized xml_element provided: " << xml_element;
        }
        // This code is unreachable but some compilers may not realizes that.
        return std::vector<std::string>{};
    }

    // Adds a failure if the key is a reserved attribute of the element named
    // xml_element.  Returns true if the property is valid.
    bool TestResult::ValidateTestProperty(const std::string& xml_element,
        const TestProperty& test_property) {
        return ValidateTestPropertyName(test_property.key(),
            GetReservedAttributesForElement(xml_element));
    }

    // Runs the test and updates the test result.
    void Test::Run() {
        if (!HasSameFixtureClass())
            return;

        internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
        impl->os_stack_trace_getter()->UponLeavingHTest();
        internal::HandleExceptionsInMethodIfSupported(this, &Test::SetUp, "SetUp()");
        // We will run the test only if SetUp() was successful and didn't call
        // HTEST_SKIP().
        if (!HasFatalFailure() && !IsSkipped()) {
            impl->os_stack_trace_getter()->UponLeavingHTest();
            internal::HandleExceptionsInMethodIfSupported(this, &Test::TestBody,
                "the test body");
        }

        // However, we want to clean up as much as possible.  Hence we will
        // always call TearDown(), even if SetUp() or the test body has
        // failed.
        impl->os_stack_trace_getter()->UponLeavingHTest();
        internal::HandleExceptionsInMethodIfSupported(this, &Test::TearDown,
            "TearDown()");
    }

}  // namespace testing
