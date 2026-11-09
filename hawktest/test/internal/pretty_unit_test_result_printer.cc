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


#include <hawktest/test/internal/pretty_unit_test_result_printer.h>
#include <hawktest/flags/builtin.h>
#include <hawktest/common/internal-inl.h>

namespace testing {
namespace internal {

    // Formats a countable noun.  Depending on its quantity, either the
    // singular form or the plural form is used. e.g.
    //
    // FormatCountableNoun(1, "formula", "formuli") returns "1 formula".
    // FormatCountableNoun(5, "book", "books") returns "5 books".
    static std::string FormatCountableNoun(int count, const char* singular_form,
        const char* plural_form) {
        return internal::StreamableToString(count) + " " + (count == 1 ? singular_form : plural_form);
    }


    // Formats the count of test suites.
    std::string FormatTestSuiteCount(int test_suite_count) {
        return FormatCountableNoun(test_suite_count, "test suite", "test suites");
    }

    // Formats the count of tests.
    std::string FormatTestCount(int test_count) {
        return FormatCountableNoun(test_count, "test", "tests");
    }

    // Fired before each iteration of tests starts.
    void PrettyUnitTestResultPrinter::OnTestIterationStart(
        const UnitTest& unit_test, int iteration) {
        if (FLAGS_htest_repeat != 1)
            printf("\nRepeating all tests (iteration %d) . . .\n\n", iteration + 1);

        std::string f = FLAGS_htest_filter;
        const char* const filter = f.c_str();

        // Prints the filter if it's not *.  This reminds the user that some
        // tests may be skipped.
        if (!String::CStringEquals(filter, kUniversalFilter)) {
            ColoredPrintf(HTestColor::kYellow, "Note: %s filter = %s\n", HTEST_NAME_,
                filter);
        }

        if (internal::ShouldShard(false)) {
            const int32_t shard_index = FLAGS_htest_shard_index;
            ColoredPrintf(HTestColor::kYellow, "Note: This is test shard %d of %d.\n",
                static_cast<int>(shard_index) + 1,
                FLAGS_htest_total_shards);
        }

        if (FLAGS_htest_shuffle) {
            ColoredPrintf(HTestColor::kYellow,
                "Note: Randomizing tests' orders with a seed of %d .\n",
                unit_test.random_seed());
        }

        ColoredPrintf(HTestColor::kGreen, "[==========] ");
        printf("Running %s from %s.\n",
            FormatTestCount(unit_test.test_to_run_count()).c_str(),
            FormatTestSuiteCount(unit_test.test_suite_to_run_count()).c_str());
        fflush(stdout);
    }

    void PrettyUnitTestResultPrinter::OnEnvironmentsSetUpStart(
    const UnitTest& /*unit_test*/) {
        ColoredPrintf(HTestColor::kGreen, "[----------] ");
        printf("Global test environment set-up.\n");
        fflush(stdout);
    }




#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
    void PrettyUnitTestResultPrinter::OnTestCaseStart(const TestCase& test_case) {
        const std::string counts = FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");
        ColoredPrintf(HTestColor::kGreen, "[----------] ");
        printf("%s from %s", counts.c_str(), test_case.name());
        if (test_case.type_param() == nullptr) {
            printf("\n");
        } else {
            printf(", where %s = %s\n", kTypeParamLabel, test_case.type_param());
        }
        fflush(stdout);
    }
#else
    void PrettyUnitTestResultPrinter::OnTestSuiteStart(
        const TestSuite& test_suite) {
        const std::string counts = FormatCountableNoun(test_suite.test_to_run_count(), "test", "tests");
        ColoredPrintf(HTestColor::kGreen, "[----------] ");
        printf("%s from %s", counts.c_str(), test_suite.name());
        if (test_suite.type_param() == nullptr) {
            printf("\n");
        } else {
            printf(", where %s = %s\n", kTypeParamLabel, test_suite.type_param());
        }
        fflush(stdout);
    }
#endif // HTEST_REMOVE_LEGACY_TEST_CASEAPI_

    void PrettyUnitTestResultPrinter::OnTestStart(const TestInfo& test_info) {
            ColoredPrintf(HTestColor::kGreen, "[ RUN      ] ");
            PrintTestName(test_info.test_suite_name(), test_info.name());
            printf("\n");
            fflush(stdout);
        }

        void PrettyUnitTestResultPrinter::OnTestDisabled(const TestInfo& test_info) {
            ColoredPrintf(HTestColor::kYellow, "[ DISABLED ] ");
            PrintTestName(test_info.test_suite_name(), test_info.name());
            printf("\n");
            fflush(stdout);
        }

        // Called after an assertion failure.
        void PrettyUnitTestResultPrinter::OnTestPartResult(
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

        void PrettyUnitTestResultPrinter::OnTestEnd(const TestInfo& test_info) {
            if (test_info.result()->Passed()) {
                ColoredPrintf(HTestColor::kGreen, "[       OK ] ");
            } else if (test_info.result()->Skipped()) {
                ColoredPrintf(HTestColor::kGreen, "[  SKIPPED ] ");
            } else {
                ColoredPrintf(HTestColor::kRed, "[  FAILED  ] ");
            }
            PrintTestName(test_info.test_suite_name(), test_info.name());
            if (test_info.result()->Failed())
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

#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        void PrettyUnitTestResultPrinter::OnTestCaseEnd(const TestCase& test_case) {
            if (!FLAGS_htest_print_time)
                return;

            const std::string counts = FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");
            ColoredPrintf(HTestColor::kGreen, "[----------] ");
            printf("%s from %s (%s ms total)\n\n", counts.c_str(), test_case.name(),
                internal::StreamableToString(test_case.elapsed_time()).c_str());
            fflush(stdout);
        }
#else
        void PrettyUnitTestResultPrinter::OnTestSuiteEnd(const TestSuite& test_suite) {
            if (!FLAGS_htest_print_time)
                return;

            const std::string counts = FormatCountableNoun(test_suite.test_to_run_count(), "test", "tests");
            ColoredPrintf(HTestColor::kGreen, "[----------] ");
            printf("%s from %s (%s ms total)\n\n", counts.c_str(), test_suite.name(),
                internal::StreamableToString(test_suite.elapsed_time()).c_str());
            fflush(stdout);
        }
#endif // HTEST_REMOVE_LEGACY_TEST_CASEAPI_

        void PrettyUnitTestResultPrinter::OnEnvironmentsTearDownStart(
            const UnitTest& /*unit_test*/) {
            ColoredPrintf(HTestColor::kGreen, "[----------] ");
            printf("Global test environment tear-down\n");
            fflush(stdout);
        }

        // Internal helper for printing the list of failed tests.
        void PrettyUnitTestResultPrinter::PrintFailedTests(const UnitTest& unit_test) {
            const int failed_test_count = unit_test.failed_test_count();
            ColoredPrintf(HTestColor::kRed, "[  FAILED  ] ");
            printf("%s, listed below:\n", FormatTestCount(failed_test_count).c_str());

            for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
                const TestSuite& test_suite = *unit_test.GetTestSuite(i);
                if (!test_suite.should_run() || (test_suite.failed_test_count() == 0)) {
                    continue;
                }
                for (int j = 0; j < test_suite.total_test_count(); ++j) {
                    const TestInfo& test_info = *test_suite.GetTestInfo(j);
                    if (!test_info.should_run() || !test_info.result()->Failed()) {
                        continue;
                    }
                    ColoredPrintf(HTestColor::kRed, "[  FAILED  ] ");
                    printf("%s.%s", test_suite.name(), test_info.name());
                    PrintFullTestCommentIfPresent(test_info);
                    printf("\n");
                }
            }
            printf("\n%2d FAILED %s\n", failed_test_count,
                failed_test_count == 1 ? "TEST" : "TESTS");
        }

        // Internal helper for printing the list of test suite failures not covered by
        // PrintFailedTests.
        void PrettyUnitTestResultPrinter::PrintFailedTestSuites(
            const UnitTest& unit_test) {
            int suite_failure_count = 0;
            for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
                const TestSuite& test_suite = *unit_test.GetTestSuite(i);
                if (!test_suite.should_run()) {
                    continue;
                }
                if (test_suite.ad_hoc_test_result().Failed()) {
                    ColoredPrintf(HTestColor::kRed, "[  FAILED  ] ");
                    printf("%s: SetUpTestSuite or TearDownTestSuite\n", test_suite.name());
                    ++suite_failure_count;
                }
            }
            if (suite_failure_count > 0) {
                printf("\n%2d FAILED TEST %s\n", suite_failure_count,
                    suite_failure_count == 1 ? "SUITE" : "SUITES");
            }
        }

        // Internal helper for printing the list of skipped tests.
        void PrettyUnitTestResultPrinter::PrintSkippedTests(const UnitTest& unit_test) {
            const int skipped_test_count = unit_test.skipped_test_count();
            if (skipped_test_count == 0) {
                return;
            }

            for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
                const TestSuite& test_suite = *unit_test.GetTestSuite(i);
                if (!test_suite.should_run() || (test_suite.skipped_test_count() == 0)) {
                    continue;
                }
                for (int j = 0; j < test_suite.total_test_count(); ++j) {
                    const TestInfo& test_info = *test_suite.GetTestInfo(j);
                    if (!test_info.should_run() || !test_info.result()->Skipped()) {
                        continue;
                    }
                    ColoredPrintf(HTestColor::kGreen, "[  SKIPPED ] ");
                    printf("%s.%s", test_suite.name(), test_info.name());
                    printf("\n");
                }
            }
        }

        void PrettyUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
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
                printf("%s, listed below:\n", FormatTestCount(skipped_test_count).c_str());
                PrintSkippedTests(unit_test);
            }

            if (!unit_test.Passed()) {
                PrintFailedTests(unit_test);
                PrintFailedTestSuites(unit_test);
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
} // namespace testing
