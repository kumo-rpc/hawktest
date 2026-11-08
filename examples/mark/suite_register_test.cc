// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/common/label.h>
#include <hawktest/mark/loader.h>
#include <hawktest/mark/runner.h>
#include <hawktest/mark/suite.h>
#include <hawktest/test/htest.h>

namespace {

struct GrammarLoad {
    std::string error;
    std::vector<md_test::RegisteredSuite> suites;
};

const GrammarLoad &grammar_load() {
    static const GrammarLoad load = [] {
        GrammarLoad state;
        if (!md_test::load_mark_file(HAWK_MARK_GRAMMAR, md_test::suite_registry(), hawk::label_registry(), &state.error)) {
            return state;
        }
        state.suites = md_test::registered_suites(md_test::suite_registry());
        return state;
    }();
    return load;
}

const md_test::RegisteredSuite *find_suite(std::string_view prefix) {
    for (const md_test::RegisteredSuite &suite : grammar_load().suites) {
        if (suite.prefix == prefix) {
            return &suite;
        }
    }
    return nullptr;
}

void expect_suite_ok(const md_test::RegisteredSuite &suite) {
    const md_test::MarkRunResult result = md_test::run_registered_suite(suite, hawk::label_registry());
    EXPECT_FALSE(result.skipped);
    EXPECT_TRUE(result.ok) << result.error;
}

TEST(MarkSuite, LoadsGrammar) {
    EXPECT_TRUE(grammar_load().error.empty()) << grammar_load().error;
    EXPECT_FALSE(grammar_load().suites.empty());
}

TEST(MarkSuite, Plain) {
    const md_test::RegisteredSuite *suite = find_suite("Identity-pass");
    ASSERT_NE(suite, nullptr);
    expect_suite_ok(*suite);
}

class MarkFile : public testing::Test {
protected:
    void SetUp() override {
        suite_ = find_suite("Identity-pass.subcase1");
        ASSERT_NE(suite_, nullptr);
    }

    const md_test::RegisteredSuite *suite_{nullptr};
};

TEST_F(MarkFile, Subcase) {
    expect_suite_ok(*suite_);
}

class MarkSuiteTest : public testing::TestWithParam<md_test::RegisteredSuite> {};

TEST_P(MarkSuiteTest, Registered) {
    const md_test::MarkRunResult result = md_test::run_registered_suite(GetParam(), hawk::label_registry());
    if (result.skipped) {
        HTEST_SKIP() << result.skip_reason;
    }
    EXPECT_TRUE(result.ok) << result.error;
}

INSTANTIATE_TEST_SUITE_P(Grammar, MarkSuiteTest, testing::ValuesIn(grammar_load().suites),
                         [](const testing::TestParamInfo<md_test::RegisteredSuite> &info) {
                             return info.param.gtest_name();
                         });

}  // namespace
