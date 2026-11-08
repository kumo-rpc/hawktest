// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/mark/matcher.h>

#include <cmath>
#include <cstdlib>
#include <map>
#include <regex>

namespace md_test {
namespace {

bool parse_double(const std::string &text, double *value) {
    if (text.empty()) {
        return false;
    }
    char *end = nullptr;
    *value = std::strtod(text.c_str(), &end);
    return end != text.c_str() && end != nullptr && *end == '\0';
}

bool near_double(double expected, double actual) {
    const double scale = std::max(1.0, std::max(std::fabs(expected), std::fabs(actual)));
    return std::fabs(expected - actual) <= 1e-6 * scale;
}

LineCompareReport compare_regex(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line) {
    LineCompareReport report;
    report.mode = CompareMode::kOrdered;
    report.expected_lines = expected.lines.size();
    report.actual_lines = actual.size();
    const std::size_t count = std::max(expected.lines.size(), actual.size());
    for (std::size_t i = 0; i < count; ++i) {
        if (i >= expected.lines.size() || i >= actual.size()) {
            LineDiff diff;
            diff.kind = i >= actual.size() ? LineDiffKind::kMissingInActual : LineDiffKind::kExtraInActual;
            diff.md_line = i < expected.lines.size() ? expected.lines[i].md_line : fallback_md_line;
            if (i < expected.lines.size()) {
                diff.expected = expected.lines[i].text;
            }
            if (i < actual.size()) {
                diff.actual = actual[i];
            }
            report.diffs.push_back(std::move(diff));
            continue;
        }
        bool matched = false;
        try {
            matched = std::regex_match(actual[i], std::regex(expected.lines[i].text));
        } catch (const std::regex_error &) {
            matched = false;
        }
        if (!matched) {
            LineDiff diff;
            diff.kind = LineDiffKind::kMismatch;
            diff.md_line = expected.lines[i].md_line;
            diff.expected = expected.lines[i].text;
            diff.actual = actual[i];
            report.diffs.push_back(std::move(diff));
        }
    }
    return report;
}

LineCompareReport compare_approx(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line) {
    LineCompareReport report;
    report.mode = CompareMode::kOrdered;
    report.expected_lines = expected.lines.size();
    report.actual_lines = actual.size();
    const std::size_t count = std::max(expected.lines.size(), actual.size());
    for (std::size_t i = 0; i < count; ++i) {
        if (i >= expected.lines.size() || i >= actual.size()) {
            LineDiff diff;
            diff.kind = i >= actual.size() ? LineDiffKind::kMissingInActual : LineDiffKind::kExtraInActual;
            diff.md_line = i < expected.lines.size() ? expected.lines[i].md_line : fallback_md_line;
            if (i < expected.lines.size()) {
                diff.expected = expected.lines[i].text;
            }
            if (i < actual.size()) {
                diff.actual = actual[i];
            }
            report.diffs.push_back(std::move(diff));
            continue;
        }
        double expected_number = 0;
        double actual_number = 0;
        const bool both_numbers = parse_double(expected.lines[i].text, &expected_number) && parse_double(actual[i], &actual_number);
        const bool same = both_numbers ? near_double(expected_number, actual_number) : expected.lines[i].text == actual[i];
        if (!same) {
            LineDiff diff;
            diff.kind = LineDiffKind::kMismatch;
            diff.md_line = expected.lines[i].md_line;
            diff.expected = expected.lines[i].text;
            diff.actual = actual[i];
            report.diffs.push_back(std::move(diff));
        }
    }
    return report;
}

void register_builtin_matchers(MatcherRegistry *registry) {
    registry->add("ordered", match_ordered);
    registry->add("strict", match_strict);
    registry->add("set", match_set);
    registry->add("multiset", match_multiset);
    registry->add("subset", match_subset);
    registry->add("in_set", match_in_set);
    registry->add("regex", compare_regex);
    registry->add("approx", compare_approx);
}

}  // namespace

LineCompareReport match_ordered(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line) {
    return compare_ordered(expected, actual, fallback_md_line);
}

LineCompareReport match_strict(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line) {
    LineCompareReport report;
    report.mode = CompareMode::kOrdered;
    report.expected_lines = expected.lines.size();
    report.actual_lines = actual.size();
    const std::size_t count = std::max(expected.lines.size(), actual.size());
    for (std::size_t i = 0; i < count; ++i) {
        const bool has_expected = i < expected.lines.size();
        const bool has_actual = i < actual.size();
        if (has_expected && has_actual && expected.lines[i].text == actual[i]) {
            ++report.match_count;
            continue;
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

LineCompareReport match_set(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line) {
    return compare_multiset(expected, actual, fallback_md_line, true);
}

LineCompareReport match_multiset(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line) {
    return compare_multiset(expected, actual, fallback_md_line, false);
}

LineCompareReport match_subset(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line) {
    return compare_subset(expected, actual, fallback_md_line);
}

LineCompareReport match_in_set(const MdTagBlock &expected, const std::vector<std::string> &actual, int fallback_md_line) {
    LineCompareReport report;
    report.mode = CompareMode::kSubset;
    report.expected_lines = expected.lines.size();
    report.actual_lines = actual.size();

    std::map<std::string, int> expected_counts;
    for (const auto &line : expected.lines) {
        ++expected_counts[normalize_compare_line(line.text)];
    }
    for (const std::string &line : actual) {
        const std::string key = normalize_compare_line(line);
        const auto it = expected_counts.find(key);
        if (it != expected_counts.end() && it->second > 0) {
            --it->second;
            ++report.match_count;
            continue;
        }
        LineDiff diff;
        diff.kind = LineDiffKind::kExtraInActual;
        diff.md_line = fallback_md_line;
        diff.actual = line;
        if (report.first_diff_md_line == 0) {
            report.first_diff_md_line = diff.md_line;
        }
        report.diffs.push_back(std::move(diff));
    }
    return report;
}

bool MatcherRegistry::add(std::string name, Matcher matcher) {
    if (name.empty() || !matcher) {
        return false;
    }
    const auto [_, inserted] = matchers_.emplace(std::move(name), std::move(matcher));
    return inserted;
}

const Matcher *MatcherRegistry::find(std::string_view name) const {
    const auto it = matchers_.find(std::string(name));
    if (it == matchers_.end()) {
        return nullptr;
    }
    return &it->second;
}

MatcherRegistry &matcher_registry() {
    static MatcherRegistry registry = [] {
        MatcherRegistry builtins;
        register_builtin_matchers(&builtins);
        return builtins;
    }();
    return registry;
}

}  // namespace md_test
