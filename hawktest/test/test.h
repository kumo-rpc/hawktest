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

namespace testing {


    // The abstract class that all tests inherit from.
    //
    // In Hawk Test, a unit test program contains one or many TestSuites, and
    // each TestSuite contains one or many Tests.
    //
    // When you define a test using the TEST macro, you don't need to
    // explicitly derive from Test - the TEST macro automatically does
    // this for you.
    //
    // The only time you derive from Test is when defining a test fixture
    // to be used in a TEST_F.  For example:
    //
    //   class FooTest : public testing::Test {
    //    protected:
    //     void SetUp() override { ... }
    //     void TearDown() override { ... }
    //     ...
    //   };
    //
    //   TEST_F(FooTest, Bar) { ... }
    //   TEST_F(FooTest, Baz) { ... }
    //
    // Test is not copyable.
    class HTEST_API_ [[nodiscard]] Test {
    public:
        friend class TestInfo;

        // The d'tor is virtual as we intend to inherit from Test.
        virtual ~Test();

        // Sets up the stuff shared by all tests in this test suite.
        //
        // Hawk Test will call Foo::SetUpTestSuite() before running the first
        // test in test suite Foo.  Hence a sub-class can define its own
        // SetUpTestSuite() method to shadow the one defined in the super
        // class.
        static void SetUpTestSuite() { }

        // Tears down the stuff shared by all tests in this test suite.
        //
        // Hawk Test will call Foo::TearDownTestSuite() after running the last
        // test in test suite Foo.  Hence a sub-class can define its own
        // TearDownTestSuite() method to shadow the one defined in the super
        // class.
        static void TearDownTestSuite() { }

        // Legacy API is deprecated but still available. Use SetUpTestSuite and
        // TearDownTestSuite instead.
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        static void TearDownTestCase() { }
        static void SetUpTestCase() { }
#endif // HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        // Returns true if and only if the current test has a fatal failure.
        static bool HasFatalFailure();

        // Returns true if and only if the current test has a non-fatal failure.
        static bool HasNonfatalFailure();

        // Returns true if and only if the current test was skipped.
        static bool IsSkipped();

        // Returns true if and only if the current test has a (either fatal or
        // non-fatal) failure.
        static bool HasFailure() { return HasFatalFailure() || HasNonfatalFailure(); }

        // Logs a property for the current test, test suite, or for the entire
        // invocation of the test program when used outside of the context of a
        // test suite.  Only the last value for a given key is remembered.  These
        // are public static so they can be called from utility functions that are
        // not members of the test fixture.  Calls to RecordProperty made during
        // lifespan of the test (from the moment its constructor starts to the
        // moment its destructor finishes) will be output in XML as attributes of
        // the <testcase> element.  Properties recorded from fixture's
        // SetUpTestSuite or TearDownTestSuite are logged as attributes of the
        // corresponding <testsuite> element.  Calls to RecordProperty made in the
        // global context (before or after invocation of RUN_ALL_TESTS and from
        // SetUp/TearDown method of Environment objects registered with Google
        // Test) will be output as attributes of the <testsuites> element.
        static void RecordProperty(const std::string& key, const std::string& value);
        // We do not define a custom serialization except for values that can be
        // converted to int64_t, but other values could be logged in this way.
        template <typename T, std::enable_if_t<std::is_convertible<T, int64_t>::value, bool> = true>
        static void RecordProperty(const std::string& key, const T& value) {
            RecordProperty(key, (Message() << value).GetString());
        }

    protected:
        // Creates a Test object.
        Test();

        // Sets up the test fixture.
        virtual void SetUp();

        // Tears down the test fixture.
        virtual void TearDown();

    private:
        // Returns true if and only if the current test has the same fixture class
        // as the first test in the current test suite.
        static bool HasSameFixtureClass();

        // Runs the test after the test fixture has been set up.
        //
        // A sub-class must implement this to define the test logic.
        //
        // DO NOT OVERRIDE THIS FUNCTION DIRECTLY IN A USER PROGRAM.
        // Instead, use the TEST or TEST_F macro.
        virtual void TestBody() = 0;

        // Sets up, executes, and tears down the test.
        void Run();

        // Deletes self.  We deliberately pick an unusual name for this
        // internal method to avoid clashing with names used in user TESTs.
        void DeleteSelf_() { delete this; }

        const std::unique_ptr<::testing::internal::HTestFlagSaver> htest_flag_saver_;

        // Often a user misspells SetUp() as Setup() and spends a long time
        // wondering why it is never called by Hawk Test.  The declaration of
        // the following method is solely for catching such an error at
        // compile time:
        //
        //   - The return type is deliberately chosen to be not void, so it
        //   will be a conflict if void Setup() is declared in the user's
        //   test fixture.
        //
        //   - This method is private, so it will be another compiler error
        //   if the method is called from the user's test fixture.
        //
        // DO NOT OVERRIDE THIS FUNCTION.
        //
        // If you see an error about overriding the following function or
        // about it being private, you have mis-spelled SetUp() as Setup().
        struct Setup_should_be_spelled_SetUp { };
        virtual Setup_should_be_spelled_SetUp* Setup() { return nullptr; }

        // We disallow copying Tests.
        Test(const Test&) = delete;
        Test& operator=(const Test&) = delete;
    };



    // A copyable object representing a user specified test property which can be
    // output as a key/value string pair.
    //
    // Don't inherit from TestProperty as its destructor is not virtual.
    class [[nodiscard]] TestProperty {
    public:
        // C'tor.  TestProperty does NOT have a default constructor.
        // Always use this constructor (with parameters) to create a
        // TestProperty object.
        TestProperty(const std::string& a_key, const std::string& a_value)
            : key_(a_key)
            , value_(a_value) { }

        // Gets the user supplied key.
        const char* key() const { return key_.c_str(); }

        // Gets the user supplied value.
        const char* value() const { return value_.c_str(); }

        // Sets a new value, overriding the one supplied in the constructor.
        void SetValue(const std::string& new_value) { value_ = new_value; }

    private:
        // The key supplied by the user.
        std::string key_;
        // The value supplied by the user.
        std::string value_;
    };


    // The result of a single Test.  This includes a list of
    // TestPartResults, a list of TestProperties, a count of how many
    // death tests there are in the Test, and how much time it took to run
    // the Test.
    //
    // TestResult is not copyable.
    class HTEST_API_ [[nodiscard]] TestResult {
    public:
        // Creates an empty TestResult.
        TestResult();

        // D'tor.  Do not inherit from TestResult.
        ~TestResult();

        // Gets the number of all test parts.  This is the sum of the number
        // of successful test parts and the number of failed test parts.
        int total_part_count() const;

        // Returns the number of the test properties.
        int test_property_count() const;

        // Returns true if and only if the test passed (i.e. no test part failed).
        bool Passed() const { return !Skipped() && !Failed(); }

        // Returns true if and only if the test was skipped.
        bool Skipped() const;

        // Returns true if and only if the test failed.
        bool Failed() const;

        // Returns true if and only if the test fatally failed.
        bool HasFatalFailure() const;

        // Returns true if and only if the test has a non-fatal failure.
        bool HasNonfatalFailure() const;

        // Returns the elapsed time, in milliseconds.
        TimeInMillis elapsed_time() const { return elapsed_time_; }

        // Gets the time of the test case start, in ms from the start of the
        // UNIX epoch.
        TimeInMillis start_timestamp() const { return start_timestamp_; }

        // Returns the i-th test part result among all the results. i can range from 0
        // to total_part_count() - 1. If i is not in that range, aborts the program.
        const TestPartResult& GetTestPartResult(int i) const;

        // Returns the i-th test property. i can range from 0 to
        // test_property_count() - 1. If i is not in that range, aborts the
        // program.
        const TestProperty& GetTestProperty(int i) const;

    private:
        friend class TestInfo;
        friend class TestSuite;
        friend class UnitTest;
        friend class internal::DefaultGlobalTestPartResultReporter;
        friend class internal::ExecDeathTest;
        friend class internal::TestResultAccessor;
        friend class internal::UnitTestImpl;
        friend class internal::WindowsDeathTest;
        friend class internal::FuchsiaDeathTest;

        // Gets the vector of TestPartResults.
        const std::vector<TestPartResult>& test_part_results() const {
            return test_part_results_;
        }

        // Gets the vector of TestProperties.
        const std::vector<TestProperty>& test_properties() const {
            return test_properties_;
        }

        // Sets the start time.
        void set_start_timestamp(TimeInMillis start) { start_timestamp_ = start; }

        // Sets the elapsed time.
        void set_elapsed_time(TimeInMillis elapsed) { elapsed_time_ = elapsed; }

        // Adds a test property to the list. The property is validated and may add
        // a non-fatal failure if invalid (e.g., if it conflicts with reserved
        // key names). If a property is already recorded for the same key, the
        // value will be updated, rather than storing multiple values for the same
        // key.  xml_element specifies the element for which the property is being
        // recorded and is used for validation.
        void RecordProperty(const std::string& xml_element,
            const TestProperty& test_property);

        // Adds a failure if the key is a reserved attribute of Hawk Test
        // testsuite tags.  Returns true if the property is valid.
        // FIXME: Validate attribute names are legal and human readable.
        static bool ValidateTestProperty(const std::string& xml_element,
            const TestProperty& test_property);

        // Adds a test part result to the list.
        void AddTestPartResult(const TestPartResult& test_part_result);

        // Returns the death test count.
        int death_test_count() const { return death_test_count_; }

        // Increments the death test count, returning the new count.
        int increment_death_test_count() { return ++death_test_count_; }

        // Clears the test part results.
        void ClearTestPartResults();

        // Clears the object.
        void Clear();

        // Protects mutable state of the property vector and of owned
        // properties, whose values may be updated.
        internal::Mutex test_properties_mutex_;

        // The vector of TestPartResults
        std::vector<TestPartResult> test_part_results_;
        // The vector of TestProperties
        std::vector<TestProperty> test_properties_;
        // Running count of death tests.
        int death_test_count_;
        // The start time, in milliseconds since UNIX Epoch.
        TimeInMillis start_timestamp_;
        // The elapsed time, in milliseconds.
        TimeInMillis elapsed_time_;

        // We disallow copying TestResult.
        TestResult(const TestResult&) = delete;
        TestResult& operator=(const TestResult&) = delete;
    }; // class TestResult


}  // namespace testing
