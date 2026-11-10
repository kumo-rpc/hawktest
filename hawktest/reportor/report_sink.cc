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

#include <hawktest/reportor/report_sink.h>
#include <hawktest/common/port_internal.h>
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

    StdoutSink::StdoutSink()
        : ReportSink(false) {
        set_color_policy(ColorPolicy::AUTO);
    }

    bool StdoutSink::open() { return true; }

    void StdoutSink::sink(std::string_view data) { write_file(stdout, data); }

    void StdoutSink::close() { std::fflush(stdout); }

    bool StdoutSink::detect_color() { return file_is_tty(stdout); }

    StderrSink::StderrSink()
        : ReportSink(false) {
        set_color_policy(ColorPolicy::AUTO);
    }

    bool StderrSink::open() { return true; }

    void StderrSink::sink(std::string_view data) { write_file(stderr, data); }

    void StderrSink::close() { std::fflush(stderr); }

    bool StderrSink::detect_color() { return file_is_tty(stderr); }

    OStreamSink::OStreamSink(std::ostream& stream, bool color)
        : ReportSink(false)
        , stream_(stream) {
        set_color_policy(color ? ColorPolicy::ON : ColorPolicy::OFF);
    }

    bool OStreamSink::open() { return static_cast<bool>(stream_); }

    void OStreamSink::sink(std::string_view data) {
        stream_.write(data.data(), static_cast<std::streamsize>(data.size()));
        stream_.flush();
    }

    void OStreamSink::close() { stream_.flush(); }

    bool OStreamSink::detect_color() { return false; }

    FileSink::FileSink(std::string path)
        : ReportSink(false)
        , path_(std::move(path)) { }

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

    bool FileSink::detect_color() { return false; }

} // namespace hawk
