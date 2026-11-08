// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/mark/runner.h>

#include <hawktest/common/environment.h>
#include <hawktest/mark/matcher.h>
#include <hawktest/mark/md_compare.h>
#include <hawktest/test/death-test.h>
#include <hawktest/test/htest.h>

#include <cstdint>
#include <cstdlib>
#include <sstream>

HTEST_DECLARE_bool_(death_test_use_fork);
HTEST_DECLARE_bool_(fail_if_no_test_linked);
HTEST_DECLARE_bool_(fail_if_no_test_selected);

namespace md_test {
namespace {

const SuiteFields &case_fields(const RegisteredSuite &registered) {
    if (registered.sub != nullptr) {
        return *registered.sub;
    }
    return *registered.suite;
}

bool install_env(const SuiteFields &fields, const std::string &prefix, std::string *error) {
    for (const EnvBinding &binding : fields.env) {
        const std::string full = prefix + "." + binding.key;
        if (binding.drop) {
            hawk::env_registry().drop(full);
            continue;
        }
        if (binding.source.empty() || binding.source == full) {
            continue;
        }
        const auto it = hawk::env_registry().entries().find(binding.source);
        if (it == hawk::env_registry().entries().end()) {
            *error = "missing env " + binding.source;
            return false;
        }
        hawk::env_registry().set(full, it->second);
    }
    return true;
}

bool parse_bool_flag(const std::string &text, bool *value) {
    if (text.empty()) {
        return false;
    }
    *value = !(text[0] == '0' || text[0] == 'f' || text[0] == 'F');
    return true;
}

bool parse_int_flag(const std::string &text, std::int32_t *value) {
    if (text.empty()) {
        return false;
    }
    char *end = nullptr;
    const long parsed = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || end == nullptr || *end != '\0') {
        return false;
    }
    *value = static_cast<std::int32_t>(parsed);
    return true;
}

struct SavedFlag {
    enum class Kind { kBool, kInt, kString };

    Kind kind{Kind::kBool};
    bool bool_value{false};
    std::int32_t int_value{0};
    std::string string_value;
    void (*restore)(const SavedFlag &){nullptr};
};

class FlagGuard {
public:
    explicit FlagGuard(const std::map<std::string, std::string> &flags) {
        for (const auto &flag : flags) {
            if (!apply(flag.first, flag.second)) {
                return;
            }
        }
    }

    ~FlagGuard() {
        for (auto it = saved_.rbegin(); it != saved_.rend(); ++it) {
            it->restore(*it);
        }
    }

    const std::string &error() const { return error_; }

private:
    bool apply(const std::string &name, const std::string &value) {
#define APPLY_BOOL(flag_name)                                                                 \
        if (name == #flag_name) {                                                             \
            bool parsed = false;                                                              \
            if (!parse_bool_flag(value, &parsed)) {                                           \
                error_ = "invalid flag " + name;                                              \
                return false;                                                                 \
            }                                                                                 \
            SavedFlag saved;                                                                  \
            saved.kind = SavedFlag::Kind::kBool;                                              \
            saved.bool_value = HTEST_FLAG_GET(flag_name);                                     \
            saved.restore = [](const SavedFlag &item) { HTEST_FLAG_SET(flag_name, item.bool_value); }; \
            saved_.push_back(std::move(saved));                                               \
            HTEST_FLAG_SET(flag_name, parsed);                                                \
            return true;                                                                      \
        }
#define APPLY_INT(flag_name)                                                                  \
        if (name == #flag_name) {                                                             \
            std::int32_t parsed = 0;                                                          \
            if (!parse_int_flag(value, &parsed)) {                                            \
                error_ = "invalid flag " + name;                                              \
                return false;                                                                 \
            }                                                                                 \
            SavedFlag saved;                                                                  \
            saved.kind = SavedFlag::Kind::kInt;                                               \
            saved.int_value = HTEST_FLAG_GET(flag_name);                                      \
            saved.restore = [](const SavedFlag &item) { HTEST_FLAG_SET(flag_name, item.int_value); }; \
            saved_.push_back(std::move(saved));                                               \
            HTEST_FLAG_SET(flag_name, parsed);                                                \
            return true;                                                                      \
        }
#define APPLY_STRING(flag_name)                                                               \
        if (name == #flag_name) {                                                             \
            SavedFlag saved;                                                                  \
            saved.kind = SavedFlag::Kind::kString;                                            \
            saved.string_value = HTEST_FLAG_GET(flag_name);                                   \
            saved.restore = [](const SavedFlag &item) { HTEST_FLAG_SET(flag_name, item.string_value); }; \
            saved_.push_back(std::move(saved));                                               \
            HTEST_FLAG_SET(flag_name, value);                                                 \
            return true;                                                                      \
        }

        APPLY_BOOL(also_run_disabled_tests);
        APPLY_BOOL(break_on_failure);
        APPLY_BOOL(catch_exceptions);
        APPLY_STRING(color);
        APPLY_STRING(death_test_style);
        APPLY_BOOL(death_test_use_fork);
        APPLY_BOOL(fail_fast);
        APPLY_BOOL(fail_if_no_test_linked);
        APPLY_BOOL(fail_if_no_test_selected);
        APPLY_STRING(output);
        APPLY_BOOL(brief);
        APPLY_BOOL(print_time);
        APPLY_BOOL(print_utf8);
        APPLY_INT(random_seed);
        APPLY_INT(repeat);
        APPLY_INT(shard_index);
        APPLY_INT(total_shards);
        APPLY_BOOL(recreate_environments_when_repeating);
        APPLY_BOOL(shuffle);
        APPLY_INT(stack_trace_depth);
        APPLY_STRING(stream_result_to);
        APPLY_BOOL(throw_on_failure);
#undef APPLY_BOOL
#undef APPLY_INT
#undef APPLY_STRING
        error_ = "unknown flag " + name;
        return false;
    }

    std::vector<SavedFlag> saved_;
    std::string error_;
};

bool resolve_side(const RegisteredSuite &registered, const std::string &ref, const hawk::LabelRegistry &labels, std::string *bytes, int *content_line, std::string *error) {
    const std::string full = ref.find('.') == std::string::npos ? registered.prefix + "." + ref : ref;
    const hawk::Label *label = labels.find(full);
    if (label == nullptr) {
        *error = "missing label " + full;
        return false;
    }
    *bytes = label->bytes;
    *content_line = label->md_line;
    return true;
}

std::vector<std::string> split_lines(const std::string &bytes) {
    std::vector<std::string> lines;
    std::stringstream stream(bytes);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    while (!lines.empty() && lines.back().empty()) {
        lines.pop_back();
    }
    return lines;
}

}  // namespace

MarkRunResult run_registered_suite(const RegisteredSuite &registered, const hawk::LabelRegistry &labels) {
    MarkRunResult result;
    if (registered.suite == nullptr) {
        result.ok = false;
        result.error = "missing suite";
        return result;
    }

    const SuiteFields &fields = case_fields(registered);
    if (registered.suite->skip || fields.skip) {
        result.skipped = true;
        result.skip_reason = fields.skip ? fields.skip_reason : registered.suite->skip_reason;
        return result;
    }

    std::map<std::string, std::string> flags = registered.suite->flags;
    if (registered.sub != nullptr) {
        for (const auto &flag : registered.sub->flags) {
            flags.insert_or_assign(flag.first, flag.second);
        }
    }
    FlagGuard flag_guard(flags);
    if (!flag_guard.error().empty()) {
        result.ok = false;
        result.error = flag_guard.error();
        return result;
    }

    if (!install_env(*registered.suite, registered.suite->name, &result.error) ||
        (registered.sub != nullptr && !install_env(*registered.sub, registered.sub->prefix(), &result.error))) {
        result.ok = false;
        return result;
    }
    hawk::EnvScope env_scope(hawk::env_registry(), registered.prefix);

    const std::vector<Match> &matches = fields.matches;
    for (const Match &match : matches) {
        std::string input;
        std::string expect;
        int expect_line = 0;
        if (!resolve_side(registered, match.input, labels, &input, &expect_line, &result.error) ||
            !resolve_side(registered, match.expect, labels, &expect, &expect_line, &result.error)) {
            result.ok = false;
            return result;
        }
        MdTagBlock expected;
        expected.tag = "expect";
        int offset = 0;
        for (const std::string &text : split_lines(expect)) {
            const int line = expect_line > 0 ? expect_line + offset : match.md_line;
            expected.lines.push_back(MdSourceLine{text, line});
            ++offset;
        }
        const Matcher *matcher = matcher_registry().find(match.mode);
        if (matcher == nullptr) {
            result.ok = false;
            result.error = format_md_location(registered.suite->md_path, match.md_line) + " unknown matcher " + match.mode;
            return result;
        }
        const int fallback_line = match.md_line > 0 ? match.md_line : registered.suite->md_line;
        const LineCompareReport report = (*matcher)(expected, split_lines(input), fallback_line);
        if (!report.ok()) {
            std::ostringstream message;
            message << "match " << match.mode << " failed";
            for (const LineDiff &diff : report.diffs) {
                const int line = diff.md_line > 0 ? diff.md_line : fallback_line;
                message << "\n" << format_md_location(registered.suite->md_path, line) << " " << line_diff_kind_name(diff.kind)
                        << " expected=[" << diff.expected << "] actual=[" << diff.actual << "]";
            }
            result.ok = false;
            result.error = message.str();
            return result;
        }
    }
    return result;
}

}  // namespace md_test
