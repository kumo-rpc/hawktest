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

#include <cstdlib>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace hawk {

    /// Stores env entries by full name (`suite.KEY` or `suite.sub.KEY`).
    /// Does not touch the process environment.
    class EnvRegistry {
    public:
        void set(std::string full_key, std::string value) {
            entries_.insert_or_assign(std::move(full_key), std::move(value));
        }

        void drop(std::string_view full_key) {
            entries_.erase(std::string(full_key));
        }

        const std::map<std::string, std::string>& entries() const { return entries_; }

    private:
        std::map<std::string, std::string> entries_;
    };

    inline EnvRegistry& env_registry() {
        static EnvRegistry registry;
        return registry;
    }

    /// Applies one suite's env for the lifetime of a runner scope.
    /// Parent prefixes are overlaid first; the current suite wins on the same local key.
    /// The process sees the local key only. Previous values are restored on destruction.
    class EnvScope {
    public:
        EnvScope(const EnvRegistry& registry, std::string_view suite_prefix) {
            std::map<std::string, std::string> flat;
            for (const std::string& prefix : ancestor_prefixes(suite_prefix)) {
                overlay(registry.entries(), prefix, &flat);
            }
            for (const auto& entry : flat) {
                const char* previous = std::getenv(entry.first.c_str());
                saved_.push_back(Saved{entry.first, previous != nullptr ? std::optional<std::string>(previous) : std::nullopt});
                set_process_env(entry.first, entry.second);
            }
        }

        EnvScope(const EnvScope&) = delete;
        EnvScope& operator=(const EnvScope&) = delete;

        EnvScope(EnvScope&& other) noexcept
            : saved_(std::move(other.saved_)) {
            other.saved_.clear();
        }

        EnvScope& operator=(EnvScope&&) = delete;

        ~EnvScope() {
            for (auto it = saved_.rbegin(); it != saved_.rend(); ++it) {
                if (it->previous.has_value()) {
                    set_process_env(it->key, *it->previous);
                } else {
                    unset_process_env(it->key);
                }
            }
        }

    private:
        struct Saved {
            std::string key;
            std::optional<std::string> previous;
        };

        static std::vector<std::string> ancestor_prefixes(std::string_view suite_prefix) {
            std::vector<std::string> prefixes;
            std::string current;
            for (std::size_t i = 0; i <= suite_prefix.size(); ++i) {
                if (i == suite_prefix.size() || suite_prefix[i] == '.') {
                    if (!current.empty()) {
                        prefixes.push_back(current);
                    }
                    if (i == suite_prefix.size()) {
                        break;
                    }
                }
                current.push_back(suite_prefix[i]);
            }
            return prefixes;
        }

        static void overlay(const std::map<std::string, std::string>& entries, const std::string& suite, std::map<std::string, std::string>* flat) {
            const std::string prefix = suite + ".";
            for (auto it = entries.lower_bound(prefix); it != entries.end() && it->first.compare(0, prefix.size(), prefix) == 0; ++it) {
                const std::string_view local(it->first.data() + prefix.size(), it->first.size() - prefix.size());
                if (local.find('.') != std::string_view::npos) {
                    continue;
                }
                flat->insert_or_assign(std::string(local), it->second);
            }
        }

        static void set_process_env(const std::string& key, const std::string& value) {
#if defined(_WIN32)
            _putenv_s(key.c_str(), value.c_str());
#else
            ::setenv(key.c_str(), value.c_str(), 1);
#endif
        }

        static void unset_process_env(const std::string& key) {
#if defined(_WIN32)
            _putenv_s(key.c_str(), "");
#else
            ::unsetenv(key.c_str());
#endif
        }

        std::vector<Saved> saved_;
    };

}  // namespace hawk
