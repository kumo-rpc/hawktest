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

#include <hawktest/common/port.h>
#include <hawktest/common/string.h>
#include <hawktest/reporter/constants.h>
#include <unordered_set>
#include <hawktest/reporter/pretty_unit_test_result_printer.h>

namespace hawk::internal {


    // Class UnitTestOptions.
    //
    // This class contains functions for processing options the user
    // specifies when running the tests.  It has only static members.
    //
    // In most cases, the user can specify an option using either an
    // environment variable or a command line flag.  E.g. you can set the
    // test filter using either HTEST_FILTER or --htest_filter.  If both
    // the variable and the flag are present, the latter overrides the
    // former.
    class HTEST_API_ UnitTestOptions {
    public:
        // Functions for processing the htest_output flag.

        // Returns the output format, or "" for normal printed output.
        static std::string GetOutputFormat();

        // Returns the absolute path of the requested output file, or the
        // default (test_detail.xml in the original working directory) if
        // none was explicitly specified.
        static std::string GetAbsolutePathToOutputFile();

        // Functions for processing the htest_filter flag.

        // Returns true if and only if the user-specified filter matches the test
        // suite name and the test name.
        static bool FilterMatchesTest(const std::string& test_suite_name,
            const std::string& test_name);

#ifdef HTEST_OS_WINDOWS
        // Function for supporting the htest_catch_exception flag.

        // Returns EXCEPTION_EXECUTE_HANDLER if given SEH exception was handled, or
        // EXCEPTION_CONTINUE_SEARCH otherwise.
        // This function is useful as an __except condition.
        static int HTestProcessSEH(DWORD seh_code, const char* location);
#endif // HTEST_OS_WINDOWS

        // Returns true if "name" matches the ':' separated list of glob-style
        // filters in "filter".
        static bool MatchesFilter(const std::string& name, const char* filter);
    };

    namespace {
        bool IsGlobPattern(const std::string& pattern) {
            return std::any_of(pattern.begin(), pattern.end(),
                [](const char c) { return c == '?' || c == '*'; });
        }

    }
    class UnitTestFilter {
    public:
        UnitTestFilter() = default;

        // Constructs a filter from a string of patterns separated by `:`.
        explicit UnitTestFilter(std::string filter) {
            // By design "" filter matches "" string.
            std::vector<std::string> all_patterns;
            SplitString(filter, ':', &all_patterns);
            const auto exact_match_patterns_begin = std::partition(
                all_patterns.begin(), all_patterns.end(), &IsGlobPattern);

            glob_patterns_.reserve(static_cast<size_t>(
                std::distance(all_patterns.begin(), exact_match_patterns_begin)));
            std::move(all_patterns.begin(), exact_match_patterns_begin,
                std::inserter(glob_patterns_, glob_patterns_.begin()));
            std::move(
                exact_match_patterns_begin, all_patterns.end(),
                std::inserter(exact_match_patterns_, exact_match_patterns_.begin()));
        }

        // Returns true if and only if name matches at least one of the patterns in
        // the filter.
        bool MatchesName(const std::string& name) const {
            return exact_match_patterns_.find(name) != exact_match_patterns_.end() || std::any_of(glob_patterns_.begin(), glob_patterns_.end(), [&name](const std::string& pattern) {
                return PatternMatchesString(
                    name, pattern.c_str(),
                    pattern.c_str() + pattern.size());
            });
        }

    private:
        std::vector<std::string> glob_patterns_;
        std::unordered_set<std::string> exact_match_patterns_;
    };

    class PositiveAndNegativeUnitTestFilter {
            public:
                // Constructs a positive and a negative filter from a string. The string
                // contains a positive filter optionally followed by a '-' character and a
                // negative filter. In case only a negative filter is provided the positive
                // filter will be assumed "*".
                // A filter is a list of patterns separated by ':'.
                explicit PositiveAndNegativeUnitTestFilter(const std::string& filter) {
                    std::vector<std::string> positive_and_negative_filters;

                    // NOTE: `SplitString` always returns a non-empty container.
                    SplitString(filter, '-', &positive_and_negative_filters);
                    const auto& positive_filter = positive_and_negative_filters.front();

                    if (positive_and_negative_filters.size() > 1) {
                        positive_filter_ = UnitTestFilter(
                            positive_filter.empty() ? hawk::Constants::kUniversalFilter : positive_filter);

                        // TODO(b/214626361): Fail on multiple '-' characters
                        // For the moment to preserve old behavior we concatenate the rest of the
                        // string parts with `-` as separator to generate the negative filter.
                        auto negative_filter_string = positive_and_negative_filters[1];
                        for (std::size_t i = 2; i < positive_and_negative_filters.size(); i++)
                            negative_filter_string = negative_filter_string + '-' + positive_and_negative_filters[i];
                        negative_filter_ = UnitTestFilter(negative_filter_string);
                    } else {
                        // In case we don't have a negative filter and positive filter is ""
                        // we do not use kUniversalFilter by design as opposed to when we have a
                        // negative filter.
                        positive_filter_ = UnitTestFilter(positive_filter);
                    }
                }

                // Returns true if and only if test name (this is generated by appending test
                // suit name and test name via a '.' character) matches the positive filter
                // and does not match the negative filter.
                bool MatchesTest(const std::string& test_suite_name,
                    const std::string& test_name) const {

                    return MatchesName(test_suite_name + "." + test_name);
                }

                // Returns true if and only if name matches the positive filter and does not
                // match the negative filter.
                bool MatchesName(const std::string& name) const {
                    return positive_filter_.MatchesName(name) && !negative_filter_.MatchesName(name);
                }

            private:
                UnitTestFilter positive_filter_;
                UnitTestFilter negative_filter_;
            };
}  // namespace hawk::internal
