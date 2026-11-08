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

#include <map>
#include <string>
#include <string_view>
#include <utility>

namespace hawk {

    /// One labeled blob. `format` is the fence language (`text`, `json`, ...).
    struct Label {
        std::string format;
        std::string bytes;
        /// Markdown line of the first content line. `0` when the bytes came from a file.
        int md_line{0};
    };

    /// Full name is `suite.local` or `suite.sub.local`. Duplicate full names are rejected.
    class LabelRegistry {
    public:
        /// Inserts `full_name`. Returns false and leaves the registry unchanged when it already exists.
        bool add(std::string full_name, Label label) {
            if (full_name.empty()) {
                return false;
            }
            const auto [_, inserted] = entries_.emplace(std::move(full_name), std::move(label));
            return inserted;
        }

        const Label* find(std::string_view full_name) const {
            const auto it = entries_.find(std::string(full_name));
            if (it == entries_.end()) {
                return nullptr;
            }
            return &it->second;
        }

        const std::map<std::string, Label>& entries() const { return entries_; }

    private:
        std::map<std::string, Label> entries_;
    };

    inline LabelRegistry& label_registry() {
        static LabelRegistry registry;
        return registry;
    }

    /// `Identity-pass` + `output1` -> `Identity-pass.output1`.
    inline std::string label_full_name(std::string_view suite_prefix, std::string_view local_name) {
        std::string full;
        full.reserve(suite_prefix.size() + 1 + local_name.size());
        full.append(suite_prefix);
        full.push_back('.');
        full.append(local_name);
        return full;
    }

}  // namespace hawk
