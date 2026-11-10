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

#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace hawk {

    enum class ColorPolicy { AUTO, FORCE, OFF };

    class ReportSink {
    public:
        virtual ~ReportSink() = default;

        /// assign the param by you constructor
        virtual bool open() = 0;

        virtual void sink(std::string_view data) = 0;

        virtual void close() = 0;

        ColorPolicy color_policy() const {
            return _color_policy;
        }

        bool set_color_policy(ColorPolicy c) {
            switch (c) {
            case ColorPolicy::AUTO:
            case ColorPolicy::FORCE:
            case ColorPolicy::OFF:
                _color_policy = c;
                return true;
            default:
                return false;
            }
        }

        // Reads --htest_color. yes/true/t/1 forces color. auto uses the sink's
        // is_tty() and is_ansi_color(). Anything else is off. A null sink is off
        // except when the flag forces color.
        static bool should_use_color(const ReportSink* sink);

        virtual bool is_tty() const {
            return false;
        }

        virtual const std::string& tty() const {
            static const std::string empty;
            return empty;
        }

        virtual bool is_ansi_color() const {
            return false;
        }
    protected:
        ColorPolicy _color_policy{ColorPolicy::AUTO};
    };

    class StdoutSink : public ReportSink {
    public:
        StdoutSink();

        bool open() override;
        void sink(std::string_view data) override;
        void close() override;
        bool is_tty() const override;
        const std::string& tty() const override;
        bool is_ansi_color() const override;
    };

    class StderrSink : public ReportSink {
    public:
        StderrSink();

        bool open() override;
        void sink(std::string_view data) override;
        void close() override;
        bool is_tty() const override;
        const std::string& tty() const override;
        bool is_ansi_color() const override;
    };

    // Writes to a caller-owned stream. Does not close it.
    // `color` sets the policy to FORCE or OFF.
    class OStreamSink : public ReportSink {
    public:
        explicit OStreamSink(std::ostream& stream, bool color = false);

        bool open() override;
        void sink(std::string_view data) override;
        void close() override;

    private:
        std::ostream& stream_;
    };

    // Opens `path` in open(). Policy is OFF.
    class FileSink : public ReportSink {
    public:
        explicit FileSink(std::string path);

        bool open() override;
        void sink(std::string_view data) override;
        void close() override;

    private:
        std::string path_;
        std::ofstream file_;
    };

} // namespace hawk
