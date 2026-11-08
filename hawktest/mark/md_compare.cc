// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/mark/md_compare.h>

#include <algorithm>
#include <map>
#include <optional>
#include <sstream>
#include <unordered_map>

namespace md_test {
namespace {

int first_md_line_for_text(const MdTagBlock &expected, std::string_view text, int fallback) {
    for (const auto &line : expected.lines) {
        if (line.text == text) {
            return line.md_line;
        }
    }
    return fallback;
}

}  // namespace

std::string normalize_compare_line(std::string line) {
    for (size_t pos = 0; (pos = line.find("\\t", pos)) != std::string::npos;) {
        line.replace(pos, 2, "\t");
        ++pos;
    }
    for (size_t pos = 0; (pos = line.find("\\f", pos)) != std::string::npos;) {
        line.replace(pos, 2, "\f");
        ++pos;
    }
    for (size_t i = 0; i < line.size();) {
        if (line[i] == '\\' && i + 3 < line.size() && line[i + 1] == 'x') {
            const auto hex = line.substr(i + 2, 2);
            const int byte = std::stoi(hex, nullptr, 16);
            line.replace(i, 4, 1, static_cast<char>(byte));
            ++i;
        } else {
            ++i;
        }
    }
    return line;
}

bool lines_equivalent(std::string_view expected, std::string_view actual) {
    if (expected == "consumed=*" && actual.size() >= 9 && actual.rfind("consumed=", 0) == 0) {
        return true;
    }
    return normalize_compare_line(std::string(expected)) == normalize_compare_line(std::string(actual));
}

const char *line_diff_kind_name(LineDiffKind kind) {
    switch (kind) {
        case LineDiffKind::kMissingInActual:
            return "MISSING_IN_ACTUAL";
        case LineDiffKind::kExtraInActual:
            return "EXTRA_IN_ACTUAL";
        case LineDiffKind::kCountMismatch:
            return "COUNT_MISMATCH";
        case LineDiffKind::kMismatch:
        default:
            return "MISMATCH";
    }
}

std::string format_md_location(const std::filesystem::path &md_path, int md_line) {
    std::ostringstream oss;
    oss << std::filesystem::absolute(md_path).string() << ':' << md_line << ':';
    return oss.str();
}

std::vector<std::string> tag_block_to_strings(const MdTagBlock &block) {
    std::vector<std::string> out;
    out.reserve(block.lines.size());
    for (const auto &line : block.lines) {
        out.push_back(line.text);
    }
    return out;
}

LineCompareReport compare_ordered(const MdTagBlock &expected, const std::vector<std::string> &actual,
                                  int fallback_md_line) {
    LineCompareReport report;
    report.mode = CompareMode::kOrdered;
    report.expected_lines = expected.lines.size();
    report.actual_lines = actual.size();

    const size_t count = std::max(expected.lines.size(), actual.size());
    for (size_t i = 0; i < count; ++i) {
        const bool has_expected = i < expected.lines.size();
        const bool has_actual = i < actual.size();
        if (has_expected && has_actual) {
            if (lines_equivalent(expected.lines[i].text, actual[i])) {
                ++report.match_count;
                continue;
            }
        }

        LineDiff diff;
        if (has_expected && has_actual) {
            diff.kind = LineDiffKind::kMismatch;
            diff.md_line = expected.lines[i].md_line;
            diff.expected = expected.lines[i].text;
            diff.actual = actual[i];
        } else if (has_expected) {
            diff.kind = LineDiffKind::kMissingInActual;
            diff.md_line = expected.lines[i].md_line;
            diff.expected = expected.lines[i].text;
        } else {
            diff.kind = LineDiffKind::kExtraInActual;
            diff.md_line = fallback_md_line;
            diff.actual = actual[i];
        }

        if (report.first_diff_md_line == 0) {
            report.first_diff_md_line = diff.md_line;
        }
        report.diffs.push_back(std::move(diff));
    }
    return report;
}

LineCompareReport compare_multiset(const MdTagBlock &expected, const std::vector<std::string> &actual,
                                   int fallback_md_line, bool unique_only) {
    LineCompareReport report;
    report.mode = unique_only ? CompareMode::kSet : CompareMode::kMultiset;
    report.expected_lines = expected.lines.size();
    report.actual_lines = actual.size();

    std::map<std::string, int> expected_counts;
    for (const auto &line : expected.lines) {
        ++expected_counts[line.text];
    }
    std::map<std::string, int> actual_counts;
    for (const auto &line : actual) {
        ++actual_counts[line];
    }

    if (unique_only) {
        for (auto &[text, count] : expected_counts) {
            if (count > 1) {
                count = 1;
            }
        }
        for (auto &[text, count] : actual_counts) {
            if (count > 1) {
                count = 1;
            }
        }
    }

    for (const auto &[text, expected_count] : expected_counts) {
        const std::string expected_norm = normalize_compare_line(text);
        int actual_count = 0;
        for (const auto &[actual_text, count] : actual_counts) {
            if (normalize_compare_line(actual_text) == expected_norm) {
                actual_count += count;
            }
        }
        if (expected_count == actual_count) {
            report.match_count += static_cast<size_t>(expected_count);
            continue;
        }
        LineDiff diff;
        diff.md_line = first_md_line_for_text(expected, text, fallback_md_line);
        diff.expected = text;
        diff.expected_count = expected_count;
        diff.actual_count = actual_count;
        if (actual_count == 0) {
            diff.kind = LineDiffKind::kMissingInActual;
        } else {
            diff.kind = LineDiffKind::kCountMismatch;
            diff.actual = text;
        }
        if (report.first_diff_md_line == 0) {
            report.first_diff_md_line = diff.md_line;
        }
        report.diffs.push_back(std::move(diff));
    }

    for (const auto &[text, actual_count] : actual_counts) {
        const std::string actual_norm = normalize_compare_line(text);
        bool found = false;
        for (const auto &[expected_text, _] : expected_counts) {
            if (normalize_compare_line(expected_text) == actual_norm) {
                found = true;
                break;
            }
        }
        if (found) {
            continue;
        }
        LineDiff diff;
        diff.kind = LineDiffKind::kExtraInActual;
        diff.md_line = fallback_md_line;
        diff.actual = text;
        diff.actual_count = actual_count;
        if (report.first_diff_md_line == 0) {
            report.first_diff_md_line = diff.md_line;
        }
        report.diffs.push_back(std::move(diff));
    }

    return report;
}

LineCompareReport compare_subset(const MdTagBlock &expected, const std::vector<std::string> &actual,
                                 int fallback_md_line) {
    LineCompareReport report;
    report.mode = CompareMode::kSubset;
    report.expected_lines = expected.lines.size();
    report.actual_lines = actual.size();

    std::map<std::string, int> actual_counts;
    for (const auto &line : actual) {
        ++actual_counts[normalize_compare_line(line)];
    }

    for (const auto &line : expected.lines) {
        const std::string expected_norm = normalize_compare_line(line.text);
        const auto it = actual_counts.find(expected_norm);
        if (it != actual_counts.end() && it->second > 0) {
            --it->second;
            ++report.match_count;
            continue;
        }
        LineDiff diff;
        diff.kind = LineDiffKind::kMissingInActual;
        diff.md_line = line.md_line;
        diff.expected = line.text;
        if (report.first_diff_md_line == 0) {
            report.first_diff_md_line = diff.md_line;
        }
        report.diffs.push_back(std::move(diff));
    }

    return report;
}

namespace {

bool is_http_message_outcome_line(std::string_view line) {
    return line.rfind("result=", 0) == 0 || line.rfind("sub_code=", 0) == 0 ||
           line.rfind("consumed=", 0) == 0;
}

std::optional<std::string> http_message_result_from_actual(const std::vector<std::string> &actual) {
    for (const auto &line : actual) {
        if (line.rfind("result=", 0) == 0) {
            return line.substr(std::string_view("result=").size());
        }
    }
    return std::nullopt;
}

bool http_message_consumed_line_matches(std::string_view expected_line,
                                        const std::vector<std::string> &actual) {
    if (expected_line.rfind("consumed=", 0) != 0) {
        return false;
    }
    const std::string expected_norm = normalize_compare_line(std::string(expected_line));
    if (expected_norm == "consumed=*") {
        for (const auto &line : actual) {
            if (normalize_compare_line(line).rfind("consumed=", 0) == 0) {
                return true;
            }
        }
        return false;
    }

    for (const auto &line : actual) {
        if (normalize_compare_line(line) == expected_norm) {
            return true;
        }
    }
    return false;
}

bool http_message_line_present(std::string_view expected_line, const std::vector<std::string> &actual) {
    if (expected_line.rfind("consumed=", 0) == 0) {
        return http_message_consumed_line_matches(expected_line, actual);
    }
    const std::string expected_norm = normalize_compare_line(std::string(expected_line));
    for (const auto &line : actual) {
        if (normalize_compare_line(line) == expected_norm) {
            return true;
        }
    }
    return false;
}

}  // namespace

LineCompareReport compare_http_message(const MdTagBlock &expected, const std::vector<std::string> &actual,
                                       int fallback_md_line) {
    LineCompareReport report;
    report.mode = CompareMode::kHttpMessage;
    report.actual_lines = actual.size();

    const std::optional<std::string> actual_result = http_message_result_from_actual(actual);
    const bool check_message_fields =
        actual_result.has_value() && *actual_result == "OK";

    for (const auto &line : expected.lines) {
        if (!check_message_fields && !is_http_message_outcome_line(line.text)) {
            continue;
        }
        ++report.expected_lines;
        if (http_message_line_present(line.text, actual)) {
            ++report.match_count;
            continue;
        }

        LineDiff diff;
        diff.kind = LineDiffKind::kMissingInActual;
        diff.md_line = line.md_line != 0 ? line.md_line : fallback_md_line;
        diff.expected = line.text;
        if (report.first_diff_md_line == 0) {
            report.first_diff_md_line = diff.md_line;
        }
        report.diffs.push_back(std::move(diff));
    }

    return report;
}

LineCompareReport compare_lines(CompareMode mode, const MdTagBlock &expected,
                                const std::vector<std::string> &actual, int fallback_md_line) {
    switch (mode) {
        case CompareMode::kSet:
            return compare_multiset(expected, actual, fallback_md_line, true);
        case CompareMode::kMultiset:
            return compare_multiset(expected, actual, fallback_md_line, false);
        case CompareMode::kSubset:
            return compare_subset(expected, actual, fallback_md_line);
        case CompareMode::kHttpMessage:
            return compare_http_message(expected, actual, fallback_md_line);
        case CompareMode::kOrdered:
        default:
            return compare_ordered(expected, actual, fallback_md_line);
    }
}

}  // namespace md_test
