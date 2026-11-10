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

namespace testing {
    namespace internal {

        // This class implements the TestEventListener interface.
        //
        // Class BriefUnitTestResultPrinter is copyable.
        class BriefUnitTestResultPrinter : public TestEventListener {
        public:
            BriefUnitTestResultPrinter() = default;
            static void PrintTestName(const char* test_suite, const char* test) {
                printf("%s.%s", test_suite, test);
            }

            // The following methods override what's in the TestEventListener class.
            void OnTestProgramStart(const UnitTest& /*unit_test*/) override { }
            void OnTestIterationStart(const UnitTest& /*unit_test*/,
                int /*iteration*/) override { }
            void OnEnvironmentsSetUpStart(const UnitTest& /*unit_test*/) override { }
            void OnEnvironmentsSetUpEnd(const UnitTest& /*unit_test*/) override { }
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
            void OnTestCaseStart(const TestCase& /*test_case*/) override { }
#else
            void OnTestSuiteStart(const TestSuite& /*test_suite*/) override { }
#endif // OnTestCaseStart

            void OnTestStart(const TestInfo& /*test_info*/) override { }
            void OnTestDisabled(const TestInfo& /*test_info*/) override { }

            void OnTestPartResult(const TestPartResult& result) override;
            void OnTestEnd(const TestInfo& test_info) override;
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
            void OnTestCaseEnd(const TestCase& /*test_case*/) override { }
#else
            void OnTestSuiteEnd(const TestSuite& /*test_suite*/) override { }
#endif // HTEST_REMOVE_LEGACY_TEST_CASEAPI_

            void OnEnvironmentsTearDownStart(const UnitTest& /*unit_test*/) override { }
            void OnEnvironmentsTearDownEnd(const UnitTest& /*unit_test*/) override { }
            void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override;
            void OnTestProgramEnd(const UnitTest& /*unit_test*/) override { }
        };
    }
}
