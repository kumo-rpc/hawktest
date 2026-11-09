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

#include <hawktest/test/internal/brief_unit_test_result_printer.h>
#include <hawktest/test/internal/pretty_unit_test_result_printer.h>
#include <hawktest/test/listener.h>
#include <hawktest/flags/builtin.h>
#include <hawktest/test/test_suite.h>
#include <hawktest/common/internal-inl.h>

namespace testing {
    namespace internal {


        // Called after an assertion failure.
        void BriefUnitTestResultPrinter::OnTestPartResult(
            const TestPartResult& result) {
            switch (result.type()) {
            // If the test part succeeded, we don't need to do anything.
            case TestPartResult::kSuccess:
                return;
            default:
                // Print failure message from the assertion
                // (e.g. expected this and got that).
                PrintTestPartResult(result);
                fflush(stdout);
            }
        }

        void BriefUnitTestResultPrinter::OnTestEnd(const TestInfo& test_info) {
            if (test_info.result()->Failed()) {
                ColoredPrintf(HTestColor::kRed, "[  FAILED  ] ");
                PrintTestName(test_info.test_suite_name(), test_info.name());
                PrintFullTestCommentIfPresent(test_info);

                if (FLAGS_htest_print_time) {
                    printf(" (%s ms)\n",
                        internal::StreamableToString(test_info.result()->elapsed_time())
                            .c_str());
                } else {
                    printf("\n");
                }
                fflush(stdout);
            }
        }

        void BriefUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
            int /*iteration*/) {
            ColoredPrintf(HTestColor::kGreen, "[==========] ");
            printf("%s from %s ran.",
                FormatTestCount(unit_test.test_to_run_count()).c_str(),
                FormatTestSuiteCount(unit_test.test_suite_to_run_count()).c_str());
            if (FLAGS_htest_print_time) {
                printf(" (%s ms total)",
                    internal::StreamableToString(unit_test.elapsed_time()).c_str());
            }
            printf("\n");
            ColoredPrintf(HTestColor::kGreen, "[  PASSED  ] ");
            printf("%s.\n", FormatTestCount(unit_test.successful_test_count()).c_str());

            const int skipped_test_count = unit_test.skipped_test_count();
            if (skipped_test_count > 0) {
                ColoredPrintf(HTestColor::kGreen, "[  SKIPPED ] ");
                printf("%s.\n", FormatTestCount(skipped_test_count).c_str());
            }

            int num_disabled = unit_test.reportable_disabled_test_count();
            if (num_disabled && !FLAGS_htest_also_run_disabled_tests) {
                if (unit_test.Passed()) {
                    printf("\n"); // Add a spacer if no FAILURE banner is displayed.
                }
                ColoredPrintf(HTestColor::kYellow, "  YOU HAVE %d DISABLED %s\n\n",
                    num_disabled, num_disabled == 1 ? "TEST" : "TESTS");
            }
            // Ensure that Hawk Test output is printed before, e.g., heapchecker output.
            fflush(stdout);
        }

    }
}
