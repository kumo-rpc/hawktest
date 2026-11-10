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

#pragma once

#include <hawktest/analysis/assertion_result.h>
#include <hawktest/test/test-part.h>

namespace hawk::internal {

    using HawkTest_NotSupported_OnFunctionReturningNonVoid = void;

        // INTERNAL IMPLEMENTATION - DO NOT USE IN USER CODE.
        // A class that enables one to stream messages to assertion macros
        class HTEST_API_ [[nodiscard]] AssertHelper {
        public:
            // Constructor.
            AssertHelper(TestPartResult::Type type, const char* file, int line,
                const char* message);
            AssertHelper(TestPartResult::Type type, std::string_view file, int line,
                std::string_view message);
            ~AssertHelper();

            // Message assignment is a semantic trick to enable assertion
            // streaming; see the HTEST_MESSAGE_ macro below.
            HawkTest_NotSupported_OnFunctionReturningNonVoid operator=(
                const hawk::Message& message) const;

        private:
            // We put our data in a struct so that the size of the AssertHelper class can
            // be as small as possible.  This is important because gcc is incapable of
            // re-using stack space even for temporary variables, so every EXPECT_EQ
            // reserves stack space for another AssertHelper.
            struct AssertHelperData {
                AssertHelperData(TestPartResult::Type t, std::string_view srcfile,
                    int line_num, std::string_view msg)
                    : type(t)
                    , file(srcfile)
                    , line(line_num)
                    , message(msg) { }

                TestPartResult::Type const type;
                const std::string_view file;
                int const line;
                std::string const message;

            private:
                AssertHelperData(const AssertHelperData&) = delete;
                AssertHelperData& operator=(const AssertHelperData&) = delete;
            };

            AssertHelperData* const data_;

            AssertHelper(const AssertHelper&) = delete;
            AssertHelper& operator=(const AssertHelper&) = delete;
        };

}  // namespace hawk
