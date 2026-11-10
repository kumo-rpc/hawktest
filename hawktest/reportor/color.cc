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
//

#include <hawktest/reportor/color.h>

namespace hawk {

    const std::map<std::string, std::string>& color_map() {
        static const std::map<std::string, std::string> colors = {
            { std::string(kDefaultKey), "\033[0m" },
            { std::string(kRedKey), "\033[31m" },
            { std::string(kGreenKey), "\033[32m" },
            { std::string(kYellowKey), "\033[33m" },
            { std::string(kBlueKey), "\033[34m" },
            { std::string(kMagentaKey), "\033[35m" },
            { std::string(kCyanKey), "\033[36m" },
            { std::string(kWhiteKey), "\033[37m" },
            { std::string(kResetKey), "\033[0m" },
        };
        return colors;
    }

    const std::map<std::string, std::string>& mock_color_map() {
        static const std::map<std::string, std::string> colors = {
            { std::string(kDefaultKey), "<DEFAULT>" },
            { std::string(kRedKey), "<RED>" },
            { std::string(kGreenKey), "<GREEN>" },
            { std::string(kYellowKey), "<YELLOW>" },
            { std::string(kBlueKey), "<BLUE>" },
            { std::string(kMagentaKey), "<MAGENTA>" },
            { std::string(kCyanKey), "<CYAN>" },
            { std::string(kWhiteKey), "<WHITE>" },
            { std::string(kResetKey), "<RESET>" },
        };
        return colors;
    }

    const std::map<std::string, std::string>& color_off_map() {
        static const std::map<std::string, std::string> colors = {
            { std::string(kDefaultKey), "" },
            { std::string(kRedKey), "" },
            { std::string(kGreenKey), "" },
            { std::string(kYellowKey), "" },
            { std::string(kBlueKey), "" },
            { std::string(kMagentaKey), "" },
            { std::string(kCyanKey), "" },
            { std::string(kWhiteKey), "" },
            { std::string(kResetKey), "" },
        };
        return colors;
    }

    static const char* color_name(Color c) {
        switch (c) {
        case Color::COLOR_DEFAULT:
            return kDefaultKey.data();
        case Color::COLOR_RED:
            return kRedKey.data();
        case Color::COLOR_GREEN:
            return kGreenKey.data();
        case Color::COLOR_YELLOW:
            return kYellowKey.data();
        case Color::COLOR_BLUE:
            return kBlueKey.data();
        case Color::COLOR_MAGENTA:
            return kMagentaKey.data();
        case Color::COLOR_CYAN:
            return kCyanKey.data();
        case Color::COLOR_WHITE:
            return kWhiteKey.data();
        }
        return nullptr;
    }

    const std::string* find_color(const std::string& color_var) {
        const auto it = color_map().find(color_var);
        return it == color_map().end() ? nullptr : &it->second;
    }

    const std::string* find_color_mock(const std::string& color_var) {
        const auto it = mock_color_map().find(color_var);
        return it == mock_color_map().end() ? nullptr : &it->second;
    }

    const std::pair<std::string, std::string>* find_color(Color c) {
        const char* name = color_name(c);
        const std::string* code = name == nullptr ? nullptr : find_color(name);
        if (code == nullptr) {
            return nullptr;
        }
        static std::map<Color, std::pair<std::string, std::string>> found;
        return &found.try_emplace(c, name, *code).first->second;
    }

    const std::pair<std::string, std::string>* find_color_mock(Color c) {
        const char* name = color_name(c);
        const std::string* code = name == nullptr ? nullptr : find_color_mock(name);
        if (code == nullptr) {
            return nullptr;
        }
        static std::map<Color, std::pair<std::string, std::string>> found;
        return &found.try_emplace(c, name, *code).first->second;
    }

} // namespace hawk
