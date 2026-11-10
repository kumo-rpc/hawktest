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

#pragma once

#include <hawktest/format/color.h>
#include <hawktest/format/report_sink.h>
#include <hawktest/reportor/constants.h>
#include <fmt/format.h>
#include <map>
#include <string>
#include <string_view>
#include <utility>

namespace hawk {

    // Writes text to a ReportSink with $var$ substitution and line indent.
    // "$$" emits a single delimiter. Undefined or unclosed names assert.
    class Printer {
    public:
        explicit Printer(ReportSink& sink, char variable_delimiter = '$');
        
        Printer& set_indent_width(int n) {
            indent_width_ = n;
            return *this;
        }

        int indent_width() const {
            return indent_width_;
        }

        ReportSink& sink() {
            return sink_;
        }

        const ReportSink& sink() const {
            return sink_;
        }

        Printer& indent();
        Printer& outdent();
        size_t dent_size() const;

        Printer& print(const char* text, const std::map<std::string, std::string>& variables);

        template <typename... Args>
        Printer& print(const char* text, const Args&... args) {
            std::map<std::string, std::string> vars;
            return print_internal(&vars, text, args...);
        }

        // Does not scan for newlines. Still inserts the current indent when the
        // sink is at the start of a line and the first byte is not '\n'.
        Printer& print_raw(std::string_view data);
        Printer& write_raw(const char* data, int size);
        Printer& new_line();

        template <typename... Args>
        Printer& fmt(fmt::format_string<Args...> format, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_raw(text);
        }

        template <typename... Args>
        Printer& fmt(fmt::format_string<Args...> format, const std::map<std::string, std::string>& variables, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print(text.c_str(), variables);
        }

        Printer& print_color(const char* text);
        Printer& print_color(const char* text, const std::map<std::string, std::string>& variables);
        Printer& print_color(Color color, const char* text);
        Printer& print_color(Color color, const char* text, const std::map<std::string, std::string>& variables);
        Printer& print_no_color(const char* text);
        Printer& print_no_color(const char* text, const std::map<std::string, std::string>& variables);
        Printer& print_no_color(Color color, const char* text);
        Printer& print_no_color(Color color, const char* text, const std::map<std::string, std::string>& variables);
        Printer& print_auto_color(const char* text);
        Printer& print_auto_color(const char* text, const std::map<std::string, std::string>& variables);
        Printer& print_auto_color(Color color, const char* text);
        Printer& print_auto_color(Color color, const char* text, const std::map<std::string, std::string>& variables);

        template <typename... Args>
        Printer& fmt_color(fmt::format_string<Args...> format, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_color(text.c_str());
        }

        template <typename... Args>
        Printer& fmt_color(fmt::format_string<Args...> format, const std::map<std::string, std::string>& variables, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_color(text.c_str(), variables);
        }

        template <typename... Args>
        Printer& fmt_color(Color color, fmt::format_string<Args...> format, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_color(color, text.c_str());
        }

        template <typename... Args>
        Printer& fmt_color(Color color, fmt::format_string<Args...> format, const std::map<std::string, std::string>& variables, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_color(color, text.c_str(), variables);
        }

        template <typename... Args>
        Printer& fmt_no_color(fmt::format_string<Args...> format, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_no_color(text.c_str());
        }

        template <typename... Args>
        Printer& fmt_no_color(fmt::format_string<Args...> format, const std::map<std::string, std::string>& variables, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_no_color(text.c_str(), variables);
        }

        template <typename... Args>
        Printer& fmt_no_color(Color color, fmt::format_string<Args...> format, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_no_color(color, text.c_str());
        }

        template <typename... Args>
        Printer& fmt_no_color(Color color, fmt::format_string<Args...> format, const std::map<std::string, std::string>& variables, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_no_color(color, text.c_str(), variables);
        }

        template <typename... Args>
        Printer& fmt_auto_color(fmt::format_string<Args...> format, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_auto_color(text.c_str());
        }

        template <typename... Args>
        Printer& fmt_auto_color(fmt::format_string<Args...> format, const std::map<std::string, std::string>& variables, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_auto_color(text.c_str(), variables);
        }

        template <typename... Args>
        Printer& fmt_auto_color(Color color, fmt::format_string<Args...> format, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_auto_color(color, text.c_str());
        }

        template <typename... Args>
        Printer& fmt_auto_color(Color color, fmt::format_string<Args...> format, const std::map<std::string, std::string>& variables, Args&&... args) {
            const std::string text = fmt::format(format, std::forward<Args>(args)...);
            return print_auto_color(color, text.c_str(), variables);
        }

#define HAWK_PRINT_ONE_COLOR(method, color)                                                                 \
    Printer& print_##method(const char* text) {                                                             \
        return print_auto_color(Color::color, text);                                                        \
    }                                                                                                        \
    Printer& print_##method(const char* text, const std::map<std::string, std::string>& variables) {        \
        return print_auto_color(Color::color, text, variables);                                             \
    }                                                                                                        \
    template <typename... Args>                                                                              \
    Printer& fmt_##method(fmt::format_string<Args...> format, Args&&... args) {                             \
        return fmt_auto_color(Color::color, format, std::forward<Args>(args)...);                           \
    }                                                                                                        \
    template <typename... Args>                                                                              \
    Printer& fmt_##method(fmt::format_string<Args...> format, const std::map<std::string, std::string>& variables, Args&&... args) { \
        return fmt_auto_color(Color::color, format, variables, std::forward<Args>(args)...);                \
    }

        HAWK_PRINT_ONE_COLOR(default, COLOR_DEFAULT)
        HAWK_PRINT_ONE_COLOR(red, COLOR_RED)
        HAWK_PRINT_ONE_COLOR(green, COLOR_GREEN)
        HAWK_PRINT_ONE_COLOR(yellow, COLOR_YELLOW)
        HAWK_PRINT_ONE_COLOR(blue, COLOR_BLUE)
        HAWK_PRINT_ONE_COLOR(magenta, COLOR_MAGENTA)
        HAWK_PRINT_ONE_COLOR(cyan, COLOR_CYAN)
        HAWK_PRINT_ONE_COLOR(white, COLOR_WHITE)
#undef HAWK_PRINT_ONE_COLOR

    private:
        Printer& print_internal(std::map<std::string, std::string>* vars, const char* text) {
            return print(text, *vars);
        }

        template <typename... Args>
        Printer& print_internal(std::map<std::string, std::string>* vars, const char* text,
            const char* key, const std::string& value, const Args&... args) {
            (*vars)[key] = value;
            return print_internal(vars, text, args...);
        }

        Printer& print_with(const char* text, const std::map<std::string, std::string>& variables, const std::map<std::string, std::string>& colors);
        Printer& print_tint(Color color, const char* text, const std::map<std::string, std::string>& variables, const std::map<std::string, std::string>& colors);

        ReportSink& sink_;
        const char variable_delimiter_;
        std::string indent_;
        bool at_start_of_line_;
        int indent_width_{2};
    };

    Printer& stderr_printer();
    Printer& stdout_printer();
} // namespace hawk
