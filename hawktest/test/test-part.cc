// Copyright (C) 2005 Google Inc. All rights reserved.
// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Portions of this software are derived from GoogleTest and Google Mock,
// originally distributed under the BSD 3-Clause License.
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
// For third-party components and portions derived from GoogleTest or
// Google Mock, the applicable original license terms and notices are
// retained in accordance with their respective licenses.
//
//
// For third-party components and portions derived from GoogleTest or
// Google Mock, the applicable original license terms and notices are
// retained in accordance with their respective licenses.
//

//
// The Google C++ Testing and Mocking Framework (Hawk Test)

#include <hawktest/test/test-part.h>

#include <ostream>
#include <string>
#include <string_view>

#include <hawktest/common/internal-inl.h>
#include <hawktest/common/internal.h>
#include <hawktest/common/port.h>

namespace testing {

    // Gets the summary of the failure message by omitting the stack trace
    // in it.
    std::string TestPartResult::ExtractSummary(const std::string_view message) {
        auto stack_trace = message.find(internal::kStackTraceMarker);
        return std::string(message.substr(0, stack_trace));
    }

    // Prints a TestPartResult object.
    std::ostream& operator<<(std::ostream& os, const TestPartResult& result) {
        return os << internal::FormatFileLocation(result.file_name(),
                   result.line_number())
                  << " "
                  << (result.type() == TestPartResult::kSuccess       ? "Success"
                             : result.type() == TestPartResult::kSkip ? "Skipped"
                             : result.type() == TestPartResult::kFatalFailure
                             ? "Fatal failure"
                             : "Non-fatal failure")
                  << ":\n"
                  << result.message() << std::endl;
    }

    // Appends a TestPartResult to the array.
    void TestPartResultArray::Append(const TestPartResult& result) {
        array_.push_back(result);
    }

    // Returns the TestPartResult at the given index (0-based).
    const TestPartResult& TestPartResultArray::GetTestPartResult(int index) const {
        if (index < 0 || index >= size()) {
            printf("\nInvalid index (%d) into TestPartResultArray.\n", index);
            internal::posix::Abort();
        }

        return array_[static_cast<size_t>(index)];
    }

    // Returns the number of TestPartResult objects in the array.
    int TestPartResultArray::size() const {
        return static_cast<int>(array_.size());
    }

    namespace internal {

        HasNewFatalFailureHelper::HasNewFatalFailureHelper()
            : has_new_fatal_failure_(false)
            , original_reporter_(
                  GetUnitTestImpl()->GetTestPartResultReporterForCurrentThread()) {
            GetUnitTestImpl()->SetTestPartResultReporterForCurrentThread(this);
        }

        HasNewFatalFailureHelper::~HasNewFatalFailureHelper() {
            GetUnitTestImpl()->SetTestPartResultReporterForCurrentThread(
                original_reporter_);
        }

        void HasNewFatalFailureHelper::ReportTestPartResult(
            const TestPartResult& result) {
            if (result.fatally_failed())
                has_new_fatal_failure_ = true;
            original_reporter_->ReportTestPartResult(result);
        }

    } // namespace internal

} // namespace testing
