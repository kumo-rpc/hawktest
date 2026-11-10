// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Portions of this software are derived from GoogleTest and Google Mock,
// originally distributed under the BSD 3-Clause License.
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

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace hawk {

    enum class ColorPolicy { AUTO, ON, OFF };

    class ReportSink {
    public:
        virtual ~ReportSink() = default;

        /// assign the param by you constructor
        virtual bool open() = 0;

        virtual void sink(std::string_view data) = 0;

        virtual void close() = 0;

        virtual bool detect_color() = 0;

        ColorPolicy color_policy() const {
            return _color_policy;
        }

        bool set_color_policy(ColorPolicy c) {
            switch (c) {
            case ColorPolicy::AUTO: {
                _cache_enabled_color = detect_color();
                break;
            }
            case ColorPolicy::ON: {
                _cache_enabled_color = true;
                break;
            }
            case ColorPolicy::OFF: {
                _cache_enabled_color = false;
                break;
            }
            default:
                return false;
            }
            _color_policy = c;
            return true;
        }

        bool color_enabled() const {
            return _cache_enabled_color;
        }
    protected:
        ReportSink(bool cache_enable) : _cache_enabled_color(cache_enable) {}
        ColorPolicy  _color_policy{};
        bool         _cache_enabled_color{false};
    };

    class StdoutSink : public ReportSink {
    public:
        StdoutSink();

        bool open() override;
        void sink(std::string_view data) override;
        void close() override;
        bool detect_color() override;
    };

    class StderrSink : public ReportSink {
    public:
        StderrSink();

        bool open() override;
        void sink(std::string_view data) override;
        void close() override;
        bool detect_color() override;
    };

    // Writes to a caller-owned stream. Does not close it. Detection is off.
    // `color` sets the policy to ON or OFF.
    class OStreamSink : public ReportSink {
    public:
        explicit OStreamSink(std::ostream& stream, bool color = false);

        bool open() override;
        void sink(std::string_view data) override;
        void close() override;
        bool detect_color() override;

    private:
        std::ostream& stream_;
    };

    // Opens `path` in open(). Detection is off.
    class FileSink : public ReportSink {
    public:
        explicit FileSink(std::string path);

        bool open() override;
        void sink(std::string_view data) override;
        void close() override;
        bool detect_color() override;

    private:
        std::string path_;
        std::ofstream file_;
    };

} // namespace hawk
