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
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace md_test {

    /// One env name on a suite. The value lives in `EnvRegistry`.
    /// `source` is another registry key when this name is a reference. `drop` removes this suite's key.
    struct EnvBinding {
        std::string key;
        std::string source;
        bool drop{false};
    };

    /// One input/expect comparison. Both sides are label keys. Bytes live in `LabelRegistry`.
    struct Match {
        std::string mode;
        std::string input;
        std::string expect;
        int md_line{0};
    };

    /// Fields shared by ## and ###. Label bytes and the env map live in their own registries.
    struct SuiteFields {
        std::string fixture_name;
        std::map<std::string, std::string> fixture_params;
        std::map<std::string, std::string> flags;
        std::vector<std::string> labels;
        std::vector<EnvBinding> env;
        bool skip{false};
        std::string skip_reason;
        std::vector<Match> matches;
    };

    struct SubSuite : SuiteFields {
        std::string name;
        std::string suite_name;
        std::filesystem::path md_path;
        int md_line{0};

        std::string prefix() const;
    };

    struct Suite : SuiteFields {
        std::string name;
        std::filesystem::path md_path;
        int md_line{0};
        std::vector<SubSuite> sub_suites;
    };

    /// Suites keyed by ## name. A second insert of the same name is rejected.
    class SuiteRegistry {
    public:
        bool add(Suite suite);

        const Suite *find(std::string_view name) const;

        const SubSuite *find_sub(std::string_view suite_name, std::string_view sub_name) const;

        const std::map<std::string, Suite> &entries() const { return suites_; }

    private:
        std::map<std::string, Suite> suites_;
    };

    SuiteRegistry &suite_registry();

    /// One htest case. `sub` is null when the ## itself is the case.
    struct RegisteredSuite {
        std::string prefix;
        const Suite *suite{nullptr};
        const SubSuite *sub{nullptr};

        std::string gtest_name() const;
    };

    /// Every ## is one case. Each ### under it is another case.
    std::vector<RegisteredSuite> registered_suites(const SuiteRegistry &suites);

}  // namespace md_test
