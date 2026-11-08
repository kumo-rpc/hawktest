// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/mark/suite.h>

#include <cctype>

namespace md_test {

std::string SubSuite::prefix() const {
    std::string out;
    out.reserve(suite_name.size() + 1 + name.size());
    out.append(suite_name);
    out.push_back('.');
    out.append(name);
    return out;
}

SuiteRegistry &suite_registry() {
    static SuiteRegistry registry;
    return registry;
}

bool SuiteRegistry::add(Suite suite) {
    if (suite.name.empty() || suites_.find(suite.name) != suites_.end()) {
        return false;
    }
    std::map<std::string, int> sub_names;
    for (const SubSuite &sub : suite.sub_suites) {
        if (sub.name.empty() || sub.suite_name != suite.name || !sub_names.emplace(sub.name, 0).second) {
            return false;
        }
    }
    suites_.emplace(suite.name, std::move(suite));
    return true;
}

const Suite *SuiteRegistry::find(std::string_view name) const {
    const auto it = suites_.find(std::string(name));
    if (it == suites_.end()) {
        return nullptr;
    }
    return &it->second;
}

std::string RegisteredSuite::gtest_name() const {
    std::string out = prefix;
    for (char &ch : out) {
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
            ch = '_';
        }
    }
    return out;
}

std::vector<RegisteredSuite> registered_suites(const SuiteRegistry &suites) {
    std::vector<RegisteredSuite> out;
    for (const auto &entry : suites.entries()) {
        const Suite &suite = entry.second;
        out.push_back(RegisteredSuite{suite.name, &suite, nullptr});
        for (const SubSuite &sub : suite.sub_suites) {
            out.push_back(RegisteredSuite{sub.prefix(), &suite, &sub});
        }
    }
    return out;
}

const SubSuite *SuiteRegistry::find_sub(std::string_view suite_name, std::string_view sub_name) const {
    const Suite *suite = find(suite_name);
    if (suite == nullptr) {
        return nullptr;
    }
    for (const SubSuite &sub : suite->sub_suites) {
        if (sub.name == sub_name) {
            return &sub;
        }
    }
    return nullptr;
}

}  // namespace md_test
