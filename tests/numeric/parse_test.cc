// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/common/environment.h>
#include <hawktest/numeric/parse.h>
#include <hawktest/test/htest.h>

// Environment variables are not supported on Windows CE.
#ifndef HTEST_OS_WINDOWS_MOBILE
TEST(GetInt32FromEnvOrDieTest, ParsesAndReturnsValidValue) {
    hawk::unset_env("HAWKTEST_NUMERIC_UNSET");
    EXPECT_EQ(333, hawk::get_int32_from_env_or_die("HAWKTEST_NUMERIC_UNSET", 333));
    hawk::set_env("HAWKTEST_NUMERIC_UNSET", "123");
    EXPECT_EQ(123, hawk::get_int32_from_env_or_die("HAWKTEST_NUMERIC_UNSET", 333));
    hawk::set_env("HAWKTEST_NUMERIC_UNSET", "-123");
    EXPECT_EQ(-123, hawk::get_int32_from_env_or_die("HAWKTEST_NUMERIC_UNSET", 333));
    hawk::unset_env("HAWKTEST_NUMERIC_UNSET");
}
#endif // !HTEST_OS_WINDOWS_MOBILE

TEST(GetInt32FromEnvOrDieDeathTest, AbortsOnFailure) {
    hawk::set_env("HAWKTEST_NUMERIC_VAR", "xxx");
    EXPECT_DEATH_IF_SUPPORTED(
        hawk::get_int32_from_env_or_die("HAWKTEST_NUMERIC_VAR", 123), ".*");
    hawk::unset_env("HAWKTEST_NUMERIC_VAR");
}

TEST(GetInt32FromEnvOrDieDeathTest, AbortsOnInt32Overflow) {
    hawk::set_env("HAWKTEST_NUMERIC_VAR", "1234567891234567891234");
    EXPECT_DEATH_IF_SUPPORTED(
        hawk::get_int32_from_env_or_die("HAWKTEST_NUMERIC_VAR", 123), ".*");
    hawk::unset_env("HAWKTEST_NUMERIC_VAR");
}
