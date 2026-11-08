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

#pragma once

#include <hawktest/mark/md_compare.h>

#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace md_test {

    using Matcher = std::function<LineCompareReport(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line)>;

    /// Line order, after `\t` / `\f` / `\xNN` decode. `consumed=*` matches any consumed line.
    LineCompareReport match_ordered(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line);

    /// Line order, raw strings. No decode and no wildcard.
    LineCompareReport match_strict(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line);

    /// Same lines, order ignored, duplicates collapsed.
    LineCompareReport match_set(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line);

    /// Same lines, order ignored, duplicate counts must match.
    LineCompareReport match_multiset(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line);

    /// Every expected line occurs in actual. Extra actual lines are allowed.
    LineCompareReport match_subset(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line);

    /// Every actual line occurs in expected. Missing expected lines are allowed.
    LineCompareReport match_in_set(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line);

    class MatcherRegistry {
    public:
        /// Returns false when the name is empty or already registered.
        bool add(std::string name, Matcher matcher);

        const Matcher *find(std::string_view name) const;

    private:
        std::map<std::string, Matcher> matchers_;
    };

    MatcherRegistry &matcher_registry();

}  // namespace md_test
