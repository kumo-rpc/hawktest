// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/mark/md_types.h>

#include <cctype>
#include <sstream>

namespace md_test {
namespace {

std::string normalize_option_key(std::string_view key) {
    std::string out(key);
    for (char &ch : out) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return out;
}

}  // namespace

std::string MdCase::gtest_name() const {
    std::ostringstream oss;
    oss << md_path.filename().string() << "_L" << title_line << "_" << name;
    std::string out = oss.str();
    for (char &ch : out) {
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
            ch = '_';
        }
    }
    return out;
}

bool MdCase::has_tag(std::string_view tag) const {
    return tags.find(std::string(tag)) != tags.end();
}

const MdTagBlock *MdCase::find_tag(std::string_view tag) const {
    const auto it = tags.find(std::string(tag));
    if (it == tags.end()) {
        return nullptr;
    }
    return &it->second;
}

std::optional<std::string> MdCase::option(std::string_view key) const {
    const auto it = options.find(normalize_option_key(key));
    if (it == options.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool MdCase::option_bool(std::string_view key, bool default_value) const {
    const auto value = option(key);
    if (!value.has_value()) {
        return default_value;
    }
    if (*value == "true" || *value == "1" || *value == "yes") {
        return true;
    }
    if (*value == "false" || *value == "0" || *value == "no") {
        return false;
    }
    return true;
}

std::optional<std::string> MdCase::skip_reason() const {
    return option("skip");
}

std::optional<MdCompareSpec> MdCase::find_compare_spec(std::string_view source_tag,
                                                       std::string_view target_tag) const {
    for (const auto &spec : compare_specs) {
        if (spec.source_tag == source_tag && spec.target_tag == target_tag) {
            return spec;
        }
    }
    return std::nullopt;
}

CompareMode parse_compare_mode(std::string_view text) {
    if (text == "set") {
        return CompareMode::kSet;
    }
    if (text == "multiset") {
        return CompareMode::kMultiset;
    }
    if (text == "subset") {
        return CompareMode::kSubset;
    }
    if (text == "http_message" || text == "message") {
        return CompareMode::kHttpMessage;
    }
    return CompareMode::kOrdered;
}

const char *compare_mode_name(CompareMode mode) {
    switch (mode) {
        case CompareMode::kSet:
            return "set";
        case CompareMode::kMultiset:
            return "multiset";
        case CompareMode::kSubset:
            return "subset";
        case CompareMode::kHttpMessage:
            return "http_message";
        case CompareMode::kOrdered:
        default:
            return "ordered";
    }
}

}  // namespace md_test
