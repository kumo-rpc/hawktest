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
#include <hawktest/reporter/listener.h>
#include <hawktest/test/test.h>

namespace testing {
    namespace internal {

            // class TestEventRepeater
        //
        // This class forwards events to other event listeners.
        class TestEventRepeater : public TestEventListener {
        public:
            TestEventRepeater()
                : forwarding_enabled_(true) { }
            ~TestEventRepeater() override;
            void Append(TestEventListener* listener);
            TestEventListener* Release(TestEventListener* listener);

            // Controls whether events will be forwarded to listeners_. Set to false
            // in death test child processes.
            bool forwarding_enabled() const { return forwarding_enabled_; }
            void set_forwarding_enabled(bool enable) { forwarding_enabled_ = enable; }

            void OnTestProgramStart(const UnitTest& parameter) override;
            void OnTestIterationStart(const UnitTest& unit_test, int iteration) override;
            void OnEnvironmentsSetUpStart(const UnitTest& parameter) override;
            void OnEnvironmentsSetUpEnd(const UnitTest& parameter) override;
//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
            void OnTestCaseStart(const TestSuite& parameter) override;
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_
            void OnTestSuiteStart(const TestSuite& parameter) override;
            void OnTestStart(const TestInfo& parameter) override;
            void OnTestDisabled(const TestInfo& parameter) override;
            void OnTestPartResult(const TestPartResult& parameter) override;
            void OnTestEnd(const TestInfo& parameter) override;
//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
            void OnTestCaseEnd(const TestCase& parameter) override;
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_
            void OnTestSuiteEnd(const TestSuite& parameter) override;
            void OnEnvironmentsTearDownStart(const UnitTest& parameter) override;
            void OnEnvironmentsTearDownEnd(const UnitTest& parameter) override;
            void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override;
            void OnTestProgramEnd(const UnitTest& parameter) override;

        private:
            // Controls whether events will be forwarded to listeners_. Set to false
            // in death test child processes.
            bool forwarding_enabled_;
            // The list of listeners that receive events.
            std::vector<TestEventListener*> listeners_;

            TestEventRepeater(const TestEventRepeater&) = delete;
            TestEventRepeater& operator=(const TestEventRepeater&) = delete;
        };
    }
}