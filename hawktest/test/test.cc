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


}  // namespace testing
