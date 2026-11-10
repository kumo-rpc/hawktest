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

namespace hawk {


    // The interface for tracing execution of tests. The methods are organized in
    // the order the corresponding events are fired.
    class [[nodiscard]] TestEventListener {
    public:
        virtual ~TestEventListener() = default;

        // Fired before any test activity starts.
        virtual void OnTestProgramStart(const UnitTest& unit_test) = 0;

        // Fired before each iteration of tests starts.  There may be more than
        // one iteration if FLAGS_htest_repeat is set. iteration is the iteration
        // index, starting from 0.
        virtual void OnTestIterationStart(const UnitTest& unit_test,
            int iteration) = 0;

        // Fired before environment set-up for each iteration of tests starts.
        virtual void OnEnvironmentsSetUpStart(const UnitTest& unit_test) = 0;

        // Fired after environment set-up for each iteration of tests ends.
        virtual void OnEnvironmentsSetUpEnd(const UnitTest& unit_test) = 0;

        // Fired before the test suite starts.
        virtual void OnTestSuiteStart(const TestSuite& /*test_suite*/) { }

        //  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        virtual void OnTestCaseStart(const TestCase& /*test_case*/) { }
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        // Fired before the test starts.
        virtual void OnTestStart(const TestInfo& test_info) = 0;

        // Fired when a test is disabled
        virtual void OnTestDisabled(const TestInfo& /*test_info*/) { }

        // Fired after a failed assertion or a SUCCEED() invocation.
        // If you want to throw an exception from this function to skip to the next
        // TEST, it must be AssertionException defined above, or inherited from it.
        virtual void OnTestPartResult(const TestPartResult& test_part_result) = 0;

        // Fired after the test ends.
        virtual void OnTestEnd(const TestInfo& test_info) = 0;

        // Fired after the test suite ends.
        virtual void OnTestSuiteEnd(const TestSuite& /*test_suite*/) { }

//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        virtual void OnTestCaseEnd(const TestCase& /*test_case*/) { }
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        // Fired before environment tear-down for each iteration of tests starts.
        virtual void OnEnvironmentsTearDownStart(const UnitTest& unit_test) = 0;

        // Fired after environment tear-down for each iteration of tests ends.
        virtual void OnEnvironmentsTearDownEnd(const UnitTest& unit_test) = 0;

        // Fired after each iteration of tests finishes.
        virtual void OnTestIterationEnd(const UnitTest& unit_test, int iteration) = 0;

        // Fired after all test activities have ended.
        virtual void OnTestProgramEnd(const UnitTest& unit_test) = 0;
    };

    // The convenience class for users who need to override just one or two
    // methods and are not concerned that a possible change to a signature of
    // the methods they override will not be caught during the build.  For
    // comments about each method please see the definition of TestEventListener
    // above.
    class [[nodiscard]] EmptyTestEventListener : public TestEventListener {
    public:
        void OnTestProgramStart(const UnitTest& /*unit_test*/) override { }
        void OnTestIterationStart(const UnitTest& /*unit_test*/,
            int /*iteration*/) override { }
        void OnEnvironmentsSetUpStart(const UnitTest& /*unit_test*/) override { }
        void OnEnvironmentsSetUpEnd(const UnitTest& /*unit_test*/) override { }
        void OnTestSuiteStart(const TestSuite& /*test_suite*/) override { }
//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        void OnTestCaseStart(const TestCase& /*test_case*/) override { }
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        void OnTestStart(const TestInfo& /*test_info*/) override { }
        void OnTestDisabled(const TestInfo& /*test_info*/) override { }
        void OnTestPartResult(const TestPartResult& /*test_part_result*/) override { }
        void OnTestEnd(const TestInfo& /*test_info*/) override { }
        void OnTestSuiteEnd(const TestSuite& /*test_suite*/) override { }
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        void OnTestCaseEnd(const TestCase& /*test_case*/) override { }
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        void OnEnvironmentsTearDownStart(const UnitTest& /*unit_test*/) override { }
        void OnEnvironmentsTearDownEnd(const UnitTest& /*unit_test*/) override { }
        void OnTestIterationEnd(const UnitTest& /*unit_test*/,
            int /*iteration*/) override { }
        void OnTestProgramEnd(const UnitTest& /*unit_test*/) override { }
    };

    // TestEventListeners lets users add listeners to track events in Hawk Test.
    class HTEST_API_ [[nodiscard]] TestEventListeners {
    public:
        TestEventListeners();
        ~TestEventListeners();

        // Appends an event listener to the end of the list. Hawk Test assumes
        // the ownership of the listener (i.e. it will delete the listener when
        // the test program finishes).
        void Append(TestEventListener* listener);

        // Removes the given event listener from the list and returns it.  It then
        // becomes the caller's responsibility to delete the listener. Returns
        // NULL if the listener is not found in the list.
        TestEventListener* Release(TestEventListener* listener);

        // Returns the standard listener responsible for the default console
        // output.  Can be removed from the listeners list to shut down default
        // console output.  Note that removing this object from the listener list
        // with Release transfers its ownership to the caller and makes this
        // function return NULL the next time.
        TestEventListener* default_result_printer() const {
            return default_result_printer_;
        }

        // Returns the standard listener responsible for the default XML output
        // controlled by the --htest_output=xml flag.  Can be removed from the
        // listeners list by users who want to shut down the default XML output
        // controlled by this flag and substitute it with custom one.  Note that
        // removing this object from the listener list with Release transfers its
        // ownership to the caller and makes this function return NULL the next
        // time.
        TestEventListener* default_xml_generator() const {
            return default_xml_generator_;
        }

        // Controls whether events will be forwarded by the repeater to the
        // listeners in the list.
        void SuppressEventForwarding(bool);

    private:
        friend class TestSuite;
        friend class TestInfo;
        friend class internal::DefaultGlobalTestPartResultReporter;
        friend class internal::NoExecDeathTest;
        friend class internal::TestEventListenersAccessor;
        friend class internal::UnitTestImpl;

        // Returns repeater that broadcasts the TestEventListener events to all
        // subscribers.
        TestEventListener* repeater();

        // Sets the default_result_printer attribute to the provided listener.
        // The listener is also added to the listener list and previous
        // default_result_printer is removed from it and deleted. The listener can
        // also be NULL in which case it will not be added to the list. Does
        // nothing if the previous and the current listener objects are the same.
        void SetDefaultResultPrinter(TestEventListener* listener);

        // Sets the default_xml_generator attribute to the provided listener.  The
        // listener is also added to the listener list and previous
        // default_xml_generator is removed from it and deleted. The listener can
        // also be NULL in which case it will not be added to the list. Does
        // nothing if the previous and the current listener objects are the same.
        void SetDefaultXmlGenerator(TestEventListener* listener);

        // Controls whether events will be forwarded by the repeater to the
        // listeners in the list.
        bool EventForwardingEnabled() const;

        // The actual list of listeners.
        internal::TestEventRepeater* repeater_;
        // Listener responsible for the standard result output.
        TestEventListener* default_result_printer_;
        // Listener responsible for the creation of the XML output file.
        TestEventListener* default_xml_generator_;

        // We disallow copying TestEventListeners.
        TestEventListeners(const TestEventListeners&) = delete;
        TestEventListeners& operator=(const TestEventListeners&) = delete;
    };

}  // namespace hawk

