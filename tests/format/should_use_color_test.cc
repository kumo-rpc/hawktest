// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/common/port.h>
#include <hawktest/flags/builtin.h>
#include <hawktest/format/report_sink.h>
#include <hawktest/test/htest.h>

#include <cstdlib>
#include <string>

namespace {

class ProbeSink : public hawk::ReportSink {
public:
    explicit ProbeSink(bool tty)
        : tty_(tty) { }

    bool open() override { return true; }
    void sink(std::string_view) override { }
    void close() override { }
    bool is_tty() const override { return tty_; }
    bool is_ansi_color() const override { return terminal_.is_ansi_color(); }

private:
    bool tty_;
    hawk::StdoutSink terminal_;
};

bool use_color(bool is_tty) {
    const ProbeSink sink(is_tty);
    return hawk::ReportSink::should_use_color(&sink);
}

void set_env(const char* name, const char* value) {
#if defined(HTEST_OS_WINDOWS_MOBILE)
    (void)name;
    (void)value;
#elif defined(HTEST_OS_WINDOWS)
    static std::string stored;
    stored = std::string(name) + "=" + value;
    _putenv(stored.data());
#else
    if (value[0] == '\0') {
        unsetenv(name);
    } else {
        setenv(name, value, 1);
    }
#endif
}

} // namespace

TEST(ColoredOutputTest, UsesColorsWhenHTestColorFlagIsYes) {
    (void)(FLAGS_htest_color = ("yes"));

    set_env("TERM", "xterm");
    EXPECT_TRUE(use_color(true));
    EXPECT_TRUE(use_color(false));

    set_env("TERM", "dumb");
    EXPECT_TRUE(use_color(true));
    EXPECT_TRUE(use_color(false));
}

TEST(ColoredOutputTest, UsesColorsWhenHTestColorFlagIsAliasOfYes) {
    set_env("TERM", "dumb");

    (void)(FLAGS_htest_color = ("True"));
    EXPECT_TRUE(use_color(false));

    (void)(FLAGS_htest_color = ("t"));
    EXPECT_TRUE(use_color(false));

    (void)(FLAGS_htest_color = ("1"));
    EXPECT_TRUE(use_color(false));
}

TEST(ColoredOutputTest, UsesNoColorWhenHTestColorFlagIsNo) {
    (void)(FLAGS_htest_color = ("no"));

    set_env("TERM", "xterm");
    EXPECT_FALSE(use_color(true));
    EXPECT_FALSE(use_color(false));

    set_env("TERM", "dumb");
    EXPECT_FALSE(use_color(true));
    EXPECT_FALSE(use_color(false));
}

TEST(ColoredOutputTest, UsesNoColorWhenHTestColorFlagIsInvalid) {
    set_env("TERM", "xterm");

    (void)(FLAGS_htest_color = ("F"));
    EXPECT_FALSE(use_color(true));

    (void)(FLAGS_htest_color = ("0"));
    EXPECT_FALSE(use_color(true));

    (void)(FLAGS_htest_color = ("unknown"));
    EXPECT_FALSE(use_color(true));
}

TEST(ColoredOutputTest, UsesColorsWhenStdoutIsTty) {
    (void)(FLAGS_htest_color = ("auto"));

    set_env("TERM", "xterm");
    EXPECT_FALSE(use_color(false));
    EXPECT_TRUE(use_color(true));
}

TEST(ColoredOutputTest, UsesColorsWhenTermSupportsColors) {
    (void)(FLAGS_htest_color = ("auto"));

#if defined(HTEST_OS_WINDOWS) && !defined(HTEST_OS_WINDOWS_MINGW)
    set_env("TERM", "dumb");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "xterm");
    EXPECT_TRUE(use_color(true));
#else
    set_env("TERM", "dumb");
    EXPECT_FALSE(use_color(true));

    set_env("TERM", "emacs");
    EXPECT_FALSE(use_color(true));

    set_env("TERM", "vt100");
    EXPECT_FALSE(use_color(true));

    set_env("TERM", "xterm-mono");
    EXPECT_FALSE(use_color(true));

    set_env("TERM", "xterm");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "xterm-color");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "xterm-ghostty");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "xterm-kitty");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "alacritty");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "xterm-256color");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "screen");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "screen-256color");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "tmux");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "tmux-256color");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "rxvt-unicode");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "rxvt-unicode-256color");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "linux");
    EXPECT_TRUE(use_color(true));

    set_env("TERM", "cygwin");
    EXPECT_TRUE(use_color(true));
#endif
}
