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

// Prints YES and returns 1 when color is on for a TTY, otherwise NO and 0.
// The TTY bit is fixed true so the driver can vary TERM and --htest_color.

#include <hawktest/format/report_sink.h>
#include <hawktest/test/htest.h>

#include <cstdio>

namespace {

class ForcedTtySink : public hawk::ReportSink {
public:
    bool open() override { return true; }
    void sink(std::string_view) override { }
    void close() override { }
    bool is_tty() const override { return true; }
    bool is_ansi_color() const override { return terminal_.is_ansi_color(); }

private:
    hawk::StdoutSink terminal_;
};

} // namespace

int main(int argc, char** argv) {
    hawk::init_hawk_test(&argc, argv);

    const ForcedTtySink sink;
    if (hawk::ReportSink::should_use_color(&sink)) {
        std::printf("YES\n");
        return 1;
    }
    std::printf("NO\n");
    return 0;
}
