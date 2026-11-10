// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/common/environment.h>
#include <hawktest/common/label.h>
#include <hawktest/mark/loader.h>
#include <hawktest/mark/suite.h>
#include <hawktest/reportor/color.h>
#include <hawktest/reportor/printer.h>
#include <hawktest/reportor/report_sink.h>
#include <hawktest/test/htest.h>

#include <map>
#include <string>

namespace {

class MemorySink : public hawk::ReportSink {
public:
    explicit MemorySink(bool color)
        : hawk::ReportSink(false) {
        set_color_policy(color ? hawk::ColorPolicy::ON : hawk::ColorPolicy::OFF);
    }

    bool open() override { return true; }
    void sink(std::string_view data) override { bytes_.append(data); }
    void close() override { }
    bool detect_color() override { return false; }

    const std::string& bytes() const { return bytes_; }

private:
    std::string bytes_;
};

struct MarkLoad {
    std::string error;
    std::vector<md_test::RegisteredSuite> suites;
};

const MarkLoad& mark_load() {
    static const MarkLoad load = [] {
        MarkLoad state;
        if (!md_test::load_mark_file(HAWK_REPORTOR_MARK, md_test::suite_registry(), hawk::label_registry(), &state.error)) {
            return state;
        }
        state.suites = md_test::registered_suites(md_test::suite_registry());
        return state;
    }();
    return load;
}

const md_test::SuiteFields& case_fields(const md_test::RegisteredSuite& registered) {
    if (registered.sub != nullptr) {
        return *registered.sub;
    }
    return *registered.suite;
}

std::string label_bytes(const md_test::RegisteredSuite& registered, const std::string& ref) {
    const std::string full = ref.find('.') == std::string::npos ? registered.prefix + "." + ref : ref;
    const hawk::Label* label = hawk::label_registry().find(full);
    return label == nullptr ? std::string() : label->bytes;
}

std::map<std::string, std::string> vars_for(const std::string& prefix) {
    std::map<std::string, std::string> vars;
    const std::string head = prefix + ".";
    for (const auto& entry : hawk::env_registry().entries()) {
        if (entry.first.compare(0, head.size(), head) != 0) {
            continue;
        }
        const std::string local = entry.first.substr(head.size());
        if (local.find('.') != std::string::npos) {
            continue;
        }
        vars.emplace(local, entry.second);
    }
    return vars;
}

void run_printer(const md_test::RegisteredSuite& registered) {
    const md_test::SuiteFields& fields = case_fields(registered);
    ASSERT_EQ(fields.matches.size(), 1u);
    const md_test::Match& match = fields.matches[0];

    std::map<std::string, std::string> vars = vars_for(registered.prefix);
    int dents = 0;
    bool color = false;
    const auto indent = vars.find("indent");
    if (indent != vars.end()) {
        dents = std::stoi(indent->second);
        vars.erase(indent);
    }
    const auto color_flag = vars.find("color");
    std::string color_mode;
    if (color_flag != vars.end()) {
        color_mode = color_flag->second;
        color = color_mode == "on" || color_mode == "mock";
        vars.erase(color_flag);
    }
    if (color_mode == "on") {
        vars.insert(hawk::color_map().begin(), hawk::color_map().end());
    } else if (color_mode == "mock") {
        vars.insert(hawk::mock_color_map().begin(), hawk::mock_color_map().end());
    } else if (color_mode == "off") {
        vars.insert(hawk::color_off_map().begin(), hawk::color_off_map().end());
    }

    const std::string input = label_bytes(registered, match.input);
    const std::string expect = label_bytes(registered, match.expect);
    ASSERT_FALSE(input.empty());
    ASSERT_FALSE(expect.empty());

    MemorySink sink(color);
    hawk::Printer printer(sink);
    for (int i = 0; i < dents; ++i) {
        printer.indent();
    }
    printer.print(input.c_str(), vars);
    EXPECT_EQ(sink.bytes(), expect);
}

TEST(ReportorMark, Loads) {
    EXPECT_TRUE(mark_load().error.empty()) << mark_load().error;
    EXPECT_FALSE(mark_load().suites.empty());
}

class PrinterMark : public testing::TestWithParam<md_test::RegisteredSuite> { };

TEST_P(PrinterMark, Case) {
    run_printer(GetParam());
}

INSTANTIATE_TEST_SUITE_P(Reportor, PrinterMark, testing::ValuesIn(mark_load().suites),
    [](const testing::TestParamInfo<md_test::RegisteredSuite>& info) {
        return info.param.gtest_name();
    });

const std::string& color_code(const std::string& key) {
    return hawk::color_map().at(key);
}

std::string tint(const std::string& key, const std::string& text) {
    return color_code(key) + text + color_code(std::string(hawk::kResetKey));
}

TEST(PrinterApi, Fmt) {
    MemorySink sink(false);
    hawk::Printer printer(sink);
    printer.fmt("n={}", 1);
    EXPECT_EQ(sink.bytes(), "n=1");
}

TEST(PrinterApi, FmtKeepsDollar) {
    MemorySink sink(false);
    hawk::Printer printer(sink);
    printer.fmt("n=$1");
    EXPECT_EQ(sink.bytes(), "n=$1");
}

TEST(PrinterApi, FmtThenVariables) {
    MemorySink sink(false);
    hawk::Printer printer(sink);
    const std::map<std::string, std::string> variables = { { "name", "Bob" } };
    printer.fmt("{} $name$", variables, "hi");
    EXPECT_EQ(sink.bytes(), "hi Bob");
}

TEST(PrinterApi, PrintColorMaps) {
    MemorySink on(false);
    hawk::Printer colored(on);
    colored.print_color("$RED$x$RESET$");
    EXPECT_EQ(on.bytes(), color_code("RED") + "x" + color_code("RESET"));

    MemorySink off(false);
    hawk::Printer plain(off);
    plain.print_no_color("$RED$x$RESET$");
    EXPECT_EQ(off.bytes(), "x");
}

TEST(PrinterApi, PrintAutoColor) {
    MemorySink on(true);
    hawk::Printer colored(on);
    colored.print_auto_color("$RED$x$RESET$");
    EXPECT_EQ(on.bytes(), color_code("RED") + "x" + color_code("RESET"));

    MemorySink off(false);
    hawk::Printer plain(off);
    plain.print_auto_color("$RED$x$RESET$");
    EXPECT_EQ(off.bytes(), "x");
}

TEST(PrinterApi, PrintOneColor) {
    MemorySink on(false);
    hawk::Printer colored(on);
    colored.print_color(hawk::Color::COLOR_RED, "bad");
    EXPECT_EQ(on.bytes(), tint("RED", "bad"));

    MemorySink named(false);
    hawk::Printer with_name(named);
    const std::map<std::string, std::string> variables = { { "name", "Bob" } };
    with_name.print_color(hawk::Color::COLOR_RED, "hi $name$", variables);
    EXPECT_EQ(named.bytes(), tint("RED", "hi Bob"));

    MemorySink off(false);
    hawk::Printer plain(off);
    plain.print_no_color(hawk::Color::COLOR_RED, "bad");
    EXPECT_EQ(off.bytes(), "bad");
}

TEST(PrinterApi, ColorWrappers) {
    MemorySink red_sink(false);
    hawk::Printer red(red_sink);
    red.print_red("bad");
    EXPECT_EQ(red_sink.bytes(), tint("RED", "bad"));

    MemorySink fmt_sink(false);
    hawk::Printer formatted(fmt_sink);
    formatted.fmt_red("{}", "bad");
    EXPECT_EQ(fmt_sink.bytes(), tint("RED", "bad"));

    MemorySink green_sink(false);
    hawk::Printer green(green_sink);
    green.print_green("bad");
    EXPECT_EQ(green_sink.bytes(), tint("GREEN", "bad"));
}

TEST(PrinterApi, NewLineAndIndent) {
    MemorySink sink(false);
    hawk::Printer printer(sink);
    printer.print_red("x").new_line().indent().print_raw("y");
    EXPECT_EQ(sink.bytes(), tint("RED", "x") + "\n  y");
}

} // namespace
