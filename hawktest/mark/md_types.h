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

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace md_test {

    /// One line inside a fenced tag block, with its 1-based line number in the .md file.
    struct MdSourceLine {
        std::string text;
        int md_line{0};
    };

    /// A fenced ```tag block. All tags (http, log, expect, resp, ...) are equal.
    struct MdTagBlock {
        std::string tag;
        int fence_line{0};
        std::vector<MdSourceLine> lines;
    };

    /// Per-case comparison override from "- compare: src->dst:mode".
    struct MdCompareSpec {
        std::string source_tag;
        std::string target_tag;
        std::optional<std::string> mode;  // ordered | set | multiset
    };

    /// Parsed markdown test case.
    struct MdCase {
        std::filesystem::path md_path;
        int title_line{0};
        std::string name;
        std::string type;
        std::unordered_map<std::string, std::string> options;
        std::vector<MdCompareSpec> compare_specs;
        std::unordered_map<std::string, MdTagBlock> tags;

        std::string gtest_name() const;

        bool has_tag(std::string_view tag) const;

        const MdTagBlock *find_tag(std::string_view tag) const;

        std::optional<std::string> option(std::string_view key) const;

        bool option_bool(std::string_view key, bool default_value = false) const;

        std::optional<std::string> skip_reason() const;

        std::optional<MdCompareSpec> find_compare_spec(std::string_view source_tag,
                                                         std::string_view target_tag) const;
    };

    enum class CompareMode {
        kOrdered,
        kSet,
        kMultiset,
        kSubset,
        kHttpMessage,
    };

    CompareMode parse_compare_mode(std::string_view text);

    const char *compare_mode_name(CompareMode mode);

}  // namespace md_test
