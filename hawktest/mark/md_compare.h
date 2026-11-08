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

#include <hawktest/mark/md_types.h>

#include <string>
#include <vector>

namespace md_test {

    enum class LineDiffKind {
        kMismatch,
        kMissingInActual,
        kExtraInActual,
        kCountMismatch,
    };

    const char *line_diff_kind_name(LineDiffKind kind);

    struct LineDiff {
        LineDiffKind kind{LineDiffKind::kMismatch};
        int md_line{0};
        std::string expected;
        std::string actual;
        int expected_count{0};
        int actual_count{0};
    };

    struct LineCompareReport {
        CompareMode mode{CompareMode::kOrdered};
        size_t expected_lines{0};
        size_t actual_lines{0};
        size_t match_count{0};
        int first_diff_md_line{0};
        std::vector<LineDiff> diffs;
        bool ok() const { return diffs.empty(); }
    };

    std::vector<std::string> tag_block_to_strings(const MdTagBlock &block);

    LineCompareReport compare_ordered(const MdTagBlock &expected, const std::vector<std::string> &actual,
                                      int fallback_md_line);

    LineCompareReport compare_multiset(const MdTagBlock &expected, const std::vector<std::string> &actual,
                                       int fallback_md_line, bool unique_only);

    LineCompareReport compare_subset(const MdTagBlock &expected, const std::vector<std::string> &actual,
                                     int fallback_md_line);

    /// When actual result!=OK, only expect lines for result/sub_code/consumed are compared.
    LineCompareReport compare_http_message(const MdTagBlock &expected, const std::vector<std::string> &actual,
                                           int fallback_md_line);

    std::string normalize_compare_line(std::string line);

    LineCompareReport compare_lines(CompareMode mode, const MdTagBlock &expected,
                                    const std::vector<std::string> &actual, int fallback_md_line);

    /// Format a gtest/IDE-friendly location prefix: /abs/path/file.md:42:
    std::string format_md_location(const std::filesystem::path &md_path, int md_line);

}  // namespace md_test
