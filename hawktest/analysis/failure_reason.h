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
#include <hawktest/common/port.h>
#include <list>
#include <map>
#include <string>

namespace hawk {

    namespace edit_distance {
        // Returns the optimal edits to go from 'left' to 'right'.
        // All edits cost the same, with replace having lower priority than
        // add/remove.
        // Simple implementation of the Wagner-Fischer algorithm.
        // See https://en.wikipedia.org/wiki/Wagner-Fischer_algorithm
        enum EditType { kMatch,
            kAdd,
            kRemove,
            kReplace };
        HTEST_API_ std::vector<EditType> CalculateOptimalEdits(
            const std::vector<size_t>& left, const std::vector<size_t>& right);

        // Same as above, but the input is represented as strings.
        HTEST_API_ std::vector<EditType> CalculateOptimalEdits(
            const std::vector<std::string>& left,
            const std::vector<std::string>& right);

        // Create a diff of the input strings in Unified diff format.
        HTEST_API_ std::string CreateUnifiedDiff(const std::vector<std::string>& left,
            const std::vector<std::string>& right,
            size_t context = 2);

    } // namespace edit_distance

    // Constructs and returns the message for an equality assertion
    // (e.g. ASSERT_EQ, EXPECT_STREQ, etc) failure.
    //
    // The first four parameters are the expressions used in the assertion
    // and their values, as strings.  For example, for ASSERT_EQ(foo, bar)
    // where foo is 5 and bar is 6, we have:
    //
    //   expected_expression: "foo"
    //   actual_expression:   "bar"
    //   expected_value:      "5"
    //   actual_value:        "6"
    //
    // The ignoring_case parameter is true if and only if the assertion is a
    // *_STRCASEEQ*.  When it's true, the string " (ignoring case)" will
    // be inserted into the message.
    HTEST_API_ AssertionResult EqFailure(const char* expected_expression,
        const char* actual_expression,
        const std::string& expected_value,
        const std::string& actual_value,
        bool ignoring_case);


    // Constructs a failure message for Boolean assertions such as EXPECT_TRUE.
    HTEST_API_ std::string GetBoolAssertionFailureMessage(
        const AssertionResult& assertion_result, const char* expression_text,
        const char* actual_predicate_value, const char* expected_predicate_value);


}  // namespace hawk
