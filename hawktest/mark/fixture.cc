// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/mark/fixture.h>

namespace md_test {

bool FixtureRegistry::add(std::string name, FixtureFactory factory) {
    if (name.empty() || !factory) {
        return false;
    }
    const auto [_, inserted] = factories_.emplace(std::move(name), std::move(factory));
    return inserted;
}

std::unique_ptr<Fixture> FixtureRegistry::create(std::string_view name) const {
    const auto it = factories_.find(std::string(name));
    if (it == factories_.end()) {
        return nullptr;
    }
    return it->second();
}

FixtureRegistry &fixture_registry() {
    static FixtureRegistry registry;
    return registry;
}

}  // namespace md_test
