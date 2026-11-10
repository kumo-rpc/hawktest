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

#include <hawktest/common/environment.h>
#include <hawktest/common/port_internal.h>

namespace hawk {
namespace {

    void write_env(const std::string& key, const std::string& value) {
#if defined(_WIN32)
        _putenv_s(key.c_str(), value.c_str());
#else
        ::setenv(key.c_str(), value.c_str(), 1);
#endif
    }

    void erase_env(const std::string& key) {
#if defined(_WIN32)
        _putenv_s(key.c_str(), "");
#else
        ::unsetenv(key.c_str());
#endif
    }

} // namespace

    void set_env(const std::string& key, const std::string& value) {
        write_env(key, value);
    }

    bool set_env_if_not_exists(const std::string& key, const std::string& value) {
        if (exists_env(key)) {
            return false;
        }
        write_env(key, value);
        return true;
    }

    std::optional<std::string> exchange_env(const std::string& key, const std::string& value) {
        std::optional<std::string> previous = get_env(key);
        write_env(key, value);
        return previous;
    }

    std::optional<std::string> get_env(const std::string& key) {
#if defined(HTEST_OS_WINDOWS_MOBILE) || defined(HTEST_OS_WINDOWS_PHONE) || defined(HTEST_OS_ESP8266) || defined(HTEST_OS_XTENSA) || defined(HTEST_OS_QURT)
        (void)key;
        return std::nullopt;
#else
        const char* value = std::getenv(key.c_str());
#if defined(__BORLANDC__) || defined(__SunOS_5_8) || defined(__SunOS_5_9)
        if (value == nullptr || value[0] == '\0') {
            return std::nullopt;
        }
#else
        if (value == nullptr) {
            return std::nullopt;
        }
#endif
        return std::string(value);
#endif
    }

    bool exists_env(const std::string& key) {
        return get_env(key).has_value();
    }

    std::optional<std::string> unset_env(const std::string& key) {
        std::optional<std::string> previous = get_env(key);
        if (!previous.has_value()) {
            return std::nullopt;
        }
        erase_env(key);
        return previous;
    }

} // namespace hawk
