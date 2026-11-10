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

#include <hawktest/common/environment.h>
#include <charconv>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <hawktest/format/message.h>

namespace hawk {

    template<typename T>
    std::optional<T> custom_numeric(std::string_view src, std::string_view* remain, Message& err) {
        static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "T must be a numeric type");
        static_assert(!std::is_same_v<T, bool>, "bool is not parsed as a number");
        T value{};
        const char* const first = src.data();
        const char* const last = first + src.size();
        const std::from_chars_result result = std::from_chars(first, last, value);
        if (remain != nullptr)
            *remain = std::string_view(result.ptr != nullptr ? result.ptr : first, static_cast<std::size_t>(last - (result.ptr != nullptr ? result.ptr : first)));
        if (result.ec != std::errc{}) {
            err << std::make_error_code(result.ec).message();
            return std::nullopt;
        }
        return value;
    }

    template<>
    inline std::optional<std::int8_t> custom_numeric<std::int8_t>(std::string_view src, std::string_view* remain, Message& err) {
        const auto wide = custom_numeric<std::int64_t>(src, remain, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide < std::numeric_limits<std::int8_t>::min() || *wide > std::numeric_limits<std::int8_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::int8_t>(*wide);
    }

    template<>
    inline std::optional<std::int16_t> custom_numeric<std::int16_t>(std::string_view src, std::string_view* remain, Message& err) {
        const auto wide = custom_numeric<std::int64_t>(src, remain, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide < std::numeric_limits<std::int16_t>::min() || *wide > std::numeric_limits<std::int16_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::int16_t>(*wide);
    }

    template<>
    inline std::optional<std::int32_t> custom_numeric<std::int32_t>(std::string_view src, std::string_view* remain, Message& err) {
        const auto wide = custom_numeric<std::int64_t>(src, remain, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide < std::numeric_limits<std::int32_t>::min() || *wide > std::numeric_limits<std::int32_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::int32_t>(*wide);
    }

    template<>
    inline std::optional<std::uint8_t> custom_numeric<std::uint8_t>(std::string_view src, std::string_view* remain, Message& err) {
        const auto wide = custom_numeric<std::uint64_t>(src, remain, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide > std::numeric_limits<std::uint8_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::uint8_t>(*wide);
    }

    template<>
    inline std::optional<std::uint16_t> custom_numeric<std::uint16_t>(std::string_view src, std::string_view* remain, Message& err) {
        const auto wide = custom_numeric<std::uint64_t>(src, remain, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide > std::numeric_limits<std::uint16_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::uint16_t>(*wide);
    }

    template<>
    inline std::optional<std::uint32_t> custom_numeric<std::uint32_t>(std::string_view src, std::string_view* remain, Message& err) {
        const auto wide = custom_numeric<std::uint64_t>(src, remain, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide > std::numeric_limits<std::uint32_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::uint32_t>(*wide);
    }

    template<>
    inline std::optional<float> custom_numeric<float>(std::string_view src, std::string_view* remain, Message& err) {
        const auto wide = custom_numeric<double>(src, remain, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide < std::numeric_limits<float>::lowest() || *wide > std::numeric_limits<float>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<float>(*wide);
    }

    template<typename T>
    std::optional<T> parse_numeric(std::string_view src, Message& err) {
        std::string_view remain;
        const std::optional<T> parsed = custom_numeric<T>(src, &remain, err);
        if (!parsed.has_value()) {
            return std::nullopt;
        }
        if (!remain.empty()) {
            err << "trailing characters";
            return std::nullopt;
        }
        return parsed;
    }

    template<>
    inline std::optional<std::int8_t> parse_numeric<std::int8_t>(std::string_view src, Message& err) {
        const auto wide = parse_numeric<std::int64_t>(src, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide < std::numeric_limits<std::int8_t>::min() || *wide > std::numeric_limits<std::int8_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::int8_t>(*wide);
    }

    template<>
    inline std::optional<std::int16_t> parse_numeric<std::int16_t>(std::string_view src, Message& err) {
        const auto wide = parse_numeric<std::int64_t>(src, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide < std::numeric_limits<std::int16_t>::min() || *wide > std::numeric_limits<std::int16_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::int16_t>(*wide);
    }

    template<>
    inline std::optional<std::int32_t> parse_numeric<std::int32_t>(std::string_view src, Message& err) {
        const auto wide = parse_numeric<std::int64_t>(src, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide < std::numeric_limits<std::int32_t>::min() || *wide > std::numeric_limits<std::int32_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::int32_t>(*wide);
    }

    template<>
    inline std::optional<std::uint8_t> parse_numeric<std::uint8_t>(std::string_view src, Message& err) {
        const auto wide = parse_numeric<std::uint64_t>(src, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide > std::numeric_limits<std::uint8_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::uint8_t>(*wide);
    }

    template<>
    inline std::optional<std::uint16_t> parse_numeric<std::uint16_t>(std::string_view src, Message& err) {
        const auto wide = parse_numeric<std::uint64_t>(src, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide > std::numeric_limits<std::uint16_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::uint16_t>(*wide);
    }

    template<>
    inline std::optional<std::uint32_t> parse_numeric<std::uint32_t>(std::string_view src, Message& err) {
        const auto wide = parse_numeric<std::uint64_t>(src, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide > std::numeric_limits<std::uint32_t>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<std::uint32_t>(*wide);
    }

    template<>
    inline std::optional<float> parse_numeric<float>(std::string_view src, Message& err) {
        const auto wide = parse_numeric<double>(src, err);
        if (!wide.has_value())
            return std::nullopt;
        if (*wide < std::numeric_limits<float>::lowest() || *wide > std::numeric_limits<float>::max()) {
            err << std::make_error_code(std::errc::result_out_of_range).message();
            return std::nullopt;
        }
        return static_cast<float>(*wide);
    }

    template<typename T>
    std::optional<T> get_numeric_form_env(const std::string& key, Message& err) {
        const std::optional<std::string> value = get_env(key);
        if (!value.has_value()) {
            err << "Environment variable " << key << " is not set.\n";
            return std::nullopt;
        }
        return parse_numeric<T>(*value, err);
    }

}  // namespace hawk

