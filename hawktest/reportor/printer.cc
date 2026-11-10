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

#include <hawktest/reportor/printer.h>

#include <cassert>
#include <cstring>

namespace hawk {

    Printer::Printer(ReportSink& sink, char variable_delimiter)
        : sink_(sink)
        , variable_delimiter_(variable_delimiter)
        , at_start_of_line_(true) { }

    Printer& Printer::indent() {
        indent_.append(indent_width_, ' ');
        return *this;
    }

    Printer& Printer::outdent() {
        if (indent_.size() < static_cast<size_t>(indent_width_)) {
            assert(false && "outdent() without matching indent()");
            return *this;
        }
        indent_.resize(indent_.size() - static_cast<size_t>(indent_width_));
        return *this;
    }

    size_t Printer::dent_size() const {
        return indent_.size();
    }

    Printer& Printer::print(const char* text, const std::map<std::string, std::string>& variables) {
        const int size = static_cast<int>(std::strlen(text));
        int pos = 0;

        for (int i = 0; i < size; i++) {
            if (text[i] == '\n') {
                write_raw(text + pos, i - pos + 1);
                pos = i + 1;
                at_start_of_line_ = true;
            } else if (text[i] == variable_delimiter_) {
                write_raw(text + pos, i - pos);
                pos = i + 1;

                const char* end = std::strchr(text + pos, variable_delimiter_);
                if (end == nullptr) {
                    assert(false && "unclosed variable name");
                    end = text + pos;
                }
                const int endpos = static_cast<int>(end - text);

                const std::string varname(text + pos, static_cast<size_t>(endpos - pos));
                if (varname.empty()) {
                    write_raw(&variable_delimiter_, 1);
                } else {
                    const auto iter = variables.find(varname);
                    if (iter == variables.end()) {
                        assert(false && "undefined variable");
                    } else {
                        write_raw(iter->second.data(), static_cast<int>(iter->second.size()));
                    }
                }

                i = endpos;
                pos = endpos + 1;
            }
        }

        write_raw(text + pos, size - pos);
        return *this;
    }

    Printer& Printer::print_with(const char* text, const std::map<std::string, std::string>& variables, const std::map<std::string, std::string>& colors) {
        std::map<std::string, std::string> merged = variables;
        for (const auto& entry : colors) {
            merged.insert_or_assign(entry.first, entry.second);
        }
        return print(text, merged);
    }

    Printer& Printer::print_color(const char* text) {
        return print(text, color_map());
    }

    Printer& Printer::print_color(const char* text, const std::map<std::string, std::string>& variables) {
        return print_with(text, variables, color_map());
    }

    Printer& Printer::print_no_color(const char* text) {
        return print(text, color_off_map());
    }

    Printer& Printer::print_no_color(const char* text, const std::map<std::string, std::string>& variables) {
        return print_with(text, variables, color_off_map());
    }

    Printer& Printer::print_auto_color(const char* text) {
        return ReportSink::should_use_color(&sink_) ? print_color(text) : print_no_color(text);
    }

    Printer& Printer::print_auto_color(const char* text, const std::map<std::string, std::string>& variables) {
        return ReportSink::should_use_color(&sink_) ? print_color(text, variables) : print_no_color(text, variables);
    }

    Printer& Printer::print_tint(Color color, const char* text, const std::map<std::string, std::string>& variables, const std::map<std::string, std::string>& colors) {
        const std::pair<std::string, std::string>* named = find_color(color);
        assert(named != nullptr);
        const auto code = colors.find(named->first);
        const auto reset = colors.find(std::string(kResetKey));
        assert(code != colors.end());
        assert(reset != colors.end());
        write_raw(code->second.data(), static_cast<int>(code->second.size()));
        print(text, variables);
        write_raw(reset->second.data(), static_cast<int>(reset->second.size()));
        return *this;
    }

    Printer& Printer::print_color(Color color, const char* text) {
        return print_tint(color, text, {}, color_map());
    }

    Printer& Printer::print_color(Color color, const char* text, const std::map<std::string, std::string>& variables) {
        return print_tint(color, text, variables, color_map());
    }

    Printer& Printer::print_no_color(Color color, const char* text) {
        return print_tint(color, text, {}, color_off_map());
    }

    Printer& Printer::print_no_color(Color color, const char* text, const std::map<std::string, std::string>& variables) {
        return print_tint(color, text, variables, color_off_map());
    }

    Printer& Printer::print_auto_color(Color color, const char* text) {
        return ReportSink::should_use_color(&sink_) ? print_color(color, text) : print_no_color(color, text);
    }

    Printer& Printer::print_auto_color(Color color, const char* text, const std::map<std::string, std::string>& variables) {
        return ReportSink::should_use_color(&sink_) ? print_color(color, text, variables) : print_no_color(color, text, variables);
    }

    Printer& Printer::new_line() {
        write_raw("\n", 1);
        at_start_of_line_ = true;
        return *this;
    }

    Printer& Printer::print_raw(std::string_view data) {
        return write_raw(data.data(), static_cast<int>(data.size()));
    }

    Printer& Printer::write_raw(const char* data, int size) {
        if (size <= 0)
            return *this;

        if (at_start_of_line_ && data[0] != '\n') {
            at_start_of_line_ = false;
            if (!indent_.empty())
                sink_.sink(indent_);
        }

        sink_.sink(std::string_view(data, static_cast<size_t>(size)));
        return *this;
    }

    Printer& stdout_printer() {
        static StdoutSink sink;
        static Printer printer(sink);
        return printer;
    }

    Printer& stderr_printer() {
        static StderrSink sink;
        static Printer printer(sink);
        return printer;
    }

} // namespace hawk
