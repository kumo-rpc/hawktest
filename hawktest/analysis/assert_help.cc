// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
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

#include <hawktest/analysis/assert_help.h>
#include <hawktest/test/unit_test.h>
#include <hawktest/test/internal/unit_test_impl.h>

namespace hawk::internal {


    // AssertHelper constructor.
    AssertHelper::AssertHelper(TestPartResult::Type type, const char* file,
        int line, const char* message)
        : AssertHelper(
              type, file == nullptr ? std::string_view() : std::string_view(file),
              line,
              message == nullptr ? std::string_view() : std::string_view(message)) {
    }


    AssertHelper::AssertHelper(TestPartResult::Type type,
        const std::string_view file, int line,
        const std::string_view message)
        : data_(new AssertHelperData(type, file, line, message)) { }



    AssertHelper::~AssertHelper() {
        delete data_;
    }

    // Returns true if test part results of type `type` should include a stack
    // trace.
    bool ShouldEmitStackTraceForResultType(TestPartResult::Type type) {
        // Suppress emission of the stack trace for SUCCEED() since it likely never
        // requires investigation, and HTEST_SKIP() since skipping is an intentional
        // act by the developer rather than a failure requiring investigation.
        return type != TestPartResult::kSuccess && type != TestPartResult::kSkip;
    }

    // Message assignment, for assertion streaming support.
    void AssertHelper::operator=(const hawk::Message& message) const {
        UnitTest::GetInstance()->AddTestPartResult(
            data_->type, data_->file, data_->line,
            hawk::internal::AppendUserMessage(data_->message, message),
            ShouldEmitStackTraceForResultType(data_->type)
                ? UnitTest::GetInstance()->impl()->CurrentOsStackTraceExceptTop(1)
                : ""
            // Skips the stack frame for this function itself.
        ); // NOLINT
    }

}  // namespace hawk
