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

#include <hawktest/test/internal/unit_test_impl.h>
#include <hawktest/test/spi.h>

namespace testing {
    namespace internal {

        // This predicate-formatter checks that 'results' contains a test part
        // failure of the given type and that the failure message contains the
        // given substring.
        static AssertionResult HasOneFailure(const char* /* results_expr */,
            const char* /* type_expr */,
            const char* /* substr_expr */,
            const TestPartResultArray& results,
            TestPartResult::Type type,
            const std::string& substr) {
            const std::string expected(type == TestPartResult::kFatalFailure
                    ? "1 fatal failure"
                    : "1 non-fatal failure");
            Message msg;
            if (results.size() != 1) {
                msg << "Expected: " << expected << "\n"
                    << "  Actual: " << results.size() << " failures";
                for (int i = 0; i < results.size(); i++) {
                    msg << "\n"
                        << results.GetTestPartResult(i);
                }
                return AssertionFailure() << msg;
            }

            const TestPartResult& r = results.GetTestPartResult(0);
            if (r.type() != type) {
                return AssertionFailure() << "Expected: " << expected << "\n"
                                          << "  Actual:\n"
                                          << r;
            }

            if (strstr(r.message(), substr.c_str()) == nullptr) {
                return AssertionFailure()
                    << "Expected: " << expected << " containing \"" << substr << "\"\n"
                    << "  Actual:\n"
                    << r;
            }

            return AssertionSuccess();
        }

        // The constructor of SingleFailureChecker remembers where to look up
        // test part results, what type of failure we expect, and what
        // substring the failure message should contain.
        SingleFailureChecker::SingleFailureChecker(const TestPartResultArray* results,
            TestPartResult::Type type,
            const std::string& substr)
            : results_(results)
            , type_(type)
            , substr_(substr) { }

        // The destructor of SingleFailureChecker verifies that the given
        // TestPartResultArray contains exactly one failure that has the given
        // type and contains the given substring.  If that's not the case, a
        // non-fatal failure will be generated.
        SingleFailureChecker::~SingleFailureChecker() {
            EXPECT_PRED_FORMAT3(HasOneFailure, *results_, type_, substr_);
        }
    } // namespace internal

    void ScopedFakeTestPartResultReporter::Init() {
        internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
        if (intercept_mode_ == INTERCEPT_ALL_THREADS) {
            old_reporter_ = impl->GetGlobalTestPartResultReporter();
            impl->SetGlobalTestPartResultReporter(this);
        } else {
            old_reporter_ = impl->GetTestPartResultReporterForCurrentThread();
            impl->SetTestPartResultReporterForCurrentThread(this);
        }
    }

    // The c'tor sets this object as the test part result reporter used by
    // Hawk Test.  The 'result' parameter specifies where to report the
    // results. Intercepts only failures from the current thread.
    ScopedFakeTestPartResultReporter::ScopedFakeTestPartResultReporter(
        TestPartResultArray* result)
        : intercept_mode_(INTERCEPT_ONLY_CURRENT_THREAD)
        , result_(result) {
        Init();
    }

    // The c'tor sets this object as the test part result reporter used by
    // Hawk Test.  The 'result' parameter specifies where to report the
    // results.
    ScopedFakeTestPartResultReporter::ScopedFakeTestPartResultReporter(
        InterceptMode intercept_mode, TestPartResultArray* result)
        : intercept_mode_(intercept_mode)
        , result_(result) {
        Init();
    }

    // The d'tor restores the test part result reporter used by Hawk Test
    // before.
    ScopedFakeTestPartResultReporter::~ScopedFakeTestPartResultReporter() {
        internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
        if (intercept_mode_ == INTERCEPT_ALL_THREADS) {
            impl->SetGlobalTestPartResultReporter(old_reporter_);
        } else {
            impl->SetTestPartResultReporterForCurrentThread(old_reporter_);
        }
    }

    // Increments the test part result count and remembers the result.
    // This method is from the TestPartResultReporterInterface interface.
    void ScopedFakeTestPartResultReporter::ReportTestPartResult(
        const TestPartResult& result) {
        result_->Append(result);
    }

} // namespace testing
