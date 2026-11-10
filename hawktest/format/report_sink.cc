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

#include <hawktest/format/report_sink.h>
#include <hawktest/common/port_internal.h>
#include <hawktest/common/string.h>
#include <hawktest/flags/builtin.h>
#include <cstdio>

namespace hawk {
namespace {

    void write_file(std::FILE* file, std::string_view data) {
        if (data.empty()) {
            return;
        }
        std::fwrite(data.data(), 1, data.size(), file);
        std::fflush(file);
    }

    bool term_supports_color(const char* term) {
        if (term == nullptr) {
            return false;
        }
        return testing::internal::String::CStringEquals(term, "xterm")
            || testing::internal::String::CStringEquals(term, "xterm-color")
            || testing::internal::String::CStringEquals(term, "xterm-ghostty")
            || testing::internal::String::CStringEquals(term, "xterm-kitty")
            || testing::internal::String::CStringEquals(term, "alacritty")
            || testing::internal::String::CStringEquals(term, "screen")
            || testing::internal::String::CStringEquals(term, "tmux")
            || testing::internal::String::CStringEquals(term, "rxvt-unicode")
            || testing::internal::String::CStringEquals(term, "linux")
            || testing::internal::String::CStringEquals(term, "cygwin")
            || testing::internal::String::EndsWithCaseInsensitive(term, std::string("-256color"));
    }

    const std::string& term_name() {
        static std::string value;
        const char* term = testing::internal::posix::GetEnv("TERM");
        value = term != nullptr ? term : "";
        return value;
    }

    bool file_supports_ansi() {
#if defined(HTEST_OS_WINDOWS) && !defined(HTEST_OS_WINDOWS_MINGW)
        return true;
#else
        return term_supports_color(term_name().c_str());
#endif
    }

    bool file_is_tty(std::FILE* file) {
#if defined(HTEST_OS_WINDOWS_MOBILE) || defined(HTEST_OS_WINDOWS_PHONE) || defined(HTEST_OS_WINDOWS_RT)
        (void)file;
        return false;
#elif defined(HTEST_OS_WINDOWS)
        const int fd = _fileno(file);
        return _isatty(fd) != 0;
#else
        const int fd = fileno(file);
        return isatty(fd) != 0;
#endif
    }

} // namespace

    bool ReportSink::should_use_color(const ReportSink* sink) {
        const char* const htest_color = FLAGS_htest_color.c_str();
        if (testing::internal::String::CaseInsensitiveCStringEquals(htest_color, "auto")) {
            return sink != nullptr && sink->is_tty() && sink->is_ansi_color();
        }
        return testing::internal::String::CaseInsensitiveCStringEquals(htest_color, "yes")
            || testing::internal::String::CaseInsensitiveCStringEquals(htest_color, "true")
            || testing::internal::String::CaseInsensitiveCStringEquals(htest_color, "t")
            || testing::internal::String::CStringEquals(htest_color, "1");
    }

    StdoutSink::StdoutSink() {
        set_color_policy(ColorPolicy::AUTO);
    }

    bool StdoutSink::open() { return true; }

    void StdoutSink::sink(std::string_view data) { write_file(stdout, data); }

    void StdoutSink::close() { std::fflush(stdout); }

    bool StdoutSink::is_tty() const { return file_is_tty(stdout); }

    const std::string& StdoutSink::tty() const { return term_name(); }

    bool StdoutSink::is_ansi_color() const { return file_supports_ansi(); }

    StderrSink::StderrSink() {
        set_color_policy(ColorPolicy::AUTO);
    }

    bool StderrSink::open() { return true; }

    void StderrSink::sink(std::string_view data) { write_file(stderr, data); }

    void StderrSink::close() { std::fflush(stderr); }

    bool StderrSink::is_tty() const { return file_is_tty(stderr); }

    const std::string& StderrSink::tty() const { return term_name(); }

    bool StderrSink::is_ansi_color() const { return file_supports_ansi(); }

    OStreamSink::OStreamSink(std::ostream& stream, bool color)
        : stream_(stream) {
        set_color_policy(color ? ColorPolicy::FORCE : ColorPolicy::OFF);
    }

    bool OStreamSink::open() { return static_cast<bool>(stream_); }

    void OStreamSink::sink(std::string_view data) {
        stream_.write(data.data(), static_cast<std::streamsize>(data.size()));
        stream_.flush();
    }

    void OStreamSink::close() { stream_.flush(); }

    FileSink::FileSink(std::string path)
        : path_(std::move(path)) {
        set_color_policy(ColorPolicy::OFF);
    }

    bool FileSink::open() {
        if (file_.is_open()) {
            return true;
        }
        file_.open(path_, std::ios::out | std::ios::binary);
        return file_.is_open();
    }

    void FileSink::sink(std::string_view data) {
        if (!file_.is_open()) {
            return;
        }
        file_.write(data.data(), static_cast<std::streamsize>(data.size()));
    }

    void FileSink::close() {
        if (file_.is_open()) {
            file_.flush();
            file_.close();
        }
    }

} // namespace hawk
