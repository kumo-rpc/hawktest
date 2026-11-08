// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace md_test {

    /// C++ fixture named by `@htest.fixture@`. `SetUp` runs before matches, `TearDown` after.
    /// An empty return is success.
    class Fixture {
    public:
        virtual ~Fixture() = default;

        virtual std::string SetUp() { return {}; }

        virtual std::string TearDown() { return {}; }

        void set_params(std::map<std::string, std::string> params) { params_ = std::move(params); }

        const std::map<std::string, std::string> &params() const { return params_; }

    private:
        std::map<std::string, std::string> params_;
    };

    using FixtureFactory = std::function<std::unique_ptr<Fixture>()>;

    class FixtureRegistry {
    public:
        /// Returns false when the name is empty or already registered.
        bool add(std::string name, FixtureFactory factory);

        std::unique_ptr<Fixture> create(std::string_view name) const;

    private:
        std::map<std::string, FixtureFactory> factories_;
    };

    FixtureRegistry &fixture_registry();

}  // namespace md_test
