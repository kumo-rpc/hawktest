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

#include <hawktest/flags/builtin.h>

#include <hawktest/common/environment.h>
#include <hawktest/common/internal-inl.h>
#include <hawktest/common/port_internal.h>
#include <hawktest/flags/gflags.h>
#include <hawktest/numeric/parse.h>

#include <cstdio>
#include <cstring>
#include <string>

namespace hawk {

    static const char kUniversalFilter[] = "*";
    static const char kTestShardIndex[] = "HTEST_SHARD_INDEX";
    static const char kTestTotalShards[] = "HTEST_TOTAL_SHARDS";
    static const char kDefaultDeathTestStyle[] = HTEST_DEFAULT_DEATH_TEST_STYLE;

    static std::string GetDefaultFilter() {
        const std::optional<std::string> testbridge_test_only = hawk::get_env("TESTBRIDGE_TEST_ONLY");
        if (testbridge_test_only.has_value()) {
            return *testbridge_test_only;
        }
        return kUniversalFilter;
    }

    static bool GetDefaultFailFast() {
        const std::optional<std::string> testbridge_test_runner_fail_fast = hawk::get_env("TESTBRIDGE_TEST_RUNNER_FAIL_FAST");
        if (testbridge_test_runner_fail_fast.has_value()) {
            return testbridge_test_runner_fail_fast->compare("1") == 0;
        }
        return false;
    }

    namespace internal {

        static std::string FlagToEnvVar(const char* flag) {
            const std::string full_flag = (hawk::Message() << HTEST_FLAG_PREFIX_ << flag).GetString();

            hawk::Message env_var;
            for (size_t i = 0; i != full_flag.length(); i++) {
                env_var << ToUpper(full_flag.c_str()[i]);
            }

            return env_var.GetString();
        }

        static bool BoolFromHTestEnv(const char* flag, bool default_value) {
#if defined(HTEST_GET_BOOL_FROM_ENV_)
            return HTEST_GET_BOOL_FROM_ENV_(flag, default_value);
#else
            const std::string env_var = FlagToEnvVar(flag);
            const std::optional<std::string> string_value = hawk::get_env(env_var);
            return string_value.has_value() ? string_value->compare("0") != 0 : default_value;
#endif
        }

        static int32_t Int32FromHTestEnv(const char* flag, int32_t default_value) {
#if defined(HTEST_GET_INT32_FROM_ENV_)
            return HTEST_GET_INT32_FROM_ENV_(flag, default_value);
#else
            const std::string env_var = FlagToEnvVar(flag);
            const std::optional<std::string> string_value = hawk::get_env(env_var);
            if (!string_value.has_value()) {
                return default_value;
            }

            int32_t result = default_value;
            if (!ParseInt32(hawk::Message() << "Environment variable " << env_var, string_value->c_str(), &result)) {
                printf("The default value %s is used.\n", (hawk::Message() << default_value).GetString().c_str());
                fflush(stdout);
                return default_value;
            }

            return result;
#endif
        }

        static std::string OutputFlagAlsoCheckEnvVar() {
            std::string default_value_for_output_flag = "";
            const std::optional<std::string> xml_output_file_env = hawk::get_env("XML_OUTPUT_FILE");
            if (xml_output_file_env.has_value()) {
                default_value_for_output_flag = std::string("xml:") + *xml_output_file_env;
            }
            return default_value_for_output_flag;
        }

        static std::string StringFromHTestEnv(const char* flag, const char* default_value) {
#if defined(HTEST_GET_STRING_FROM_ENV_)
            return HTEST_GET_STRING_FROM_ENV_(flag, default_value);
#else
            const std::string env_var = FlagToEnvVar(flag);
            const std::optional<std::string> value = hawk::get_env(env_var);
            return value.has_value() ? *value : std::string(default_value);
#endif
        }

    } // namespace internal

} // namespace hawk

DEFINE_bool(htest_fail_fast,
    hawk::internal::BoolFromHTestEnv("fail_fast", hawk::GetDefaultFailFast()),
    "True if and only if a test failure should stop further test execution.");

DEFINE_bool(htest_fail_if_no_test_linked,
    hawk::internal::BoolFromHTestEnv("fail_if_no_test_linked", false),
    "True if and only if the test should fail if no test case (including "
    "disabled test cases) is linked.");

DEFINE_bool(htest_fail_if_no_test_selected,
    hawk::internal::BoolFromHTestEnv("fail_if_no_test_selected", false),
    "True if and only if the test should fail if no test case is selected to "
    "run. A test case is selected to run if it is not disabled and is matched "
    "by the filter flag so that it starts executing.");

DEFINE_bool(htest_also_run_disabled_tests,
    hawk::internal::BoolFromHTestEnv("also_run_disabled_tests", false),
    "Run disabled tests too, in addition to the tests normally being run.");

DEFINE_bool(htest_break_on_failure,
    hawk::internal::BoolFromHTestEnv("break_on_failure", false),
    "True if and only if a failed assertion should be a debugger "
    "break-point.");

DEFINE_bool(htest_catch_exceptions,
    hawk::internal::BoolFromHTestEnv("catch_exceptions", true),
    "True if and only if " HTEST_NAME_ " should catch exceptions and treat them as test failures.");

DEFINE_string(htest_color,
    hawk::internal::StringFromHTestEnv("color", "auto"),
    "Whether to use colors in the output.  Valid values: yes, no, "
    "and auto.  'auto' means to use colors if the output is "
    "being sent to a terminal and the TERM environment variable "
    "is set to a terminal type that supports colors.");

DEFINE_string(htest_filter,
    hawk::internal::StringFromHTestEnv("filter", hawk::GetDefaultFilter().c_str()),
    "A colon-separated list of glob (not regex) patterns "
    "for filtering the tests to run, optionally followed by a "
    "'-' and a : separated list of negative patterns (tests to "
    "exclude).  A test is run if it matches one of the positive "
    "patterns and does not match any of the negative patterns.");

DEFINE_bool(htest_install_failure_signal_handler,
    hawk::internal::BoolFromHTestEnv("install_failure_signal_handler", false),
    "If true and supported on the current platform, " HTEST_NAME_ " should "
    "install a signal handler that dumps debugging information when fatal "
    "signals are raised.");

DEFINE_bool(htest_list_tests, false, "List all tests without running them.");

DEFINE_string(htest_output,
    hawk::internal::StringFromHTestEnv("output", hawk::internal::OutputFlagAlsoCheckEnvVar().c_str()),
    "A format (defaults to \"xml\" but can be specified to be \"json\"), "
    "optionally followed by a colon and an output file name or directory. "
    "A directory is indicated by a trailing pathname separator. "
    "Examples: \"xml:filename.xml\", \"xml::directoryname/\". "
    "If a directory is specified, output files will be created "
    "within that directory, with file-names based on the test "
    "executable's name and, if necessary, made unique by adding "
    "digits.");

DEFINE_bool(htest_brief,
    hawk::internal::BoolFromHTestEnv("brief", false),
    "True if only test failures should be displayed in text output.");

DEFINE_bool(htest_print_time,
    hawk::internal::BoolFromHTestEnv("print_time", true),
    "True if and only if " HTEST_NAME_ " should display elapsed time in text output.");

DEFINE_bool(htest_print_utf8,
    hawk::internal::BoolFromHTestEnv("print_utf8", true),
    "True if and only if " HTEST_NAME_ " prints UTF8 characters as text.");

DEFINE_int32(htest_random_seed,
    hawk::internal::Int32FromHTestEnv("random_seed", 0),
    "Random number seed to use when shuffling test orders.  Must be in range "
    "[1, 99999], or 0 to use a seed based on the current time.");

DEFINE_int32(htest_repeat,
    hawk::internal::Int32FromHTestEnv("repeat", 1),
    "How many times to repeat each test.  Specify a negative number "
    "for repeating forever.  Useful for shaking out flaky tests.");

DEFINE_bool(htest_recreate_environments_when_repeating,
    hawk::internal::BoolFromHTestEnv("recreate_environments_when_repeating", false),
    "Controls whether global test environments are recreated for each repeat "
    "of the tests. If set to false the global test environments are only set "
    "up once, for the first iteration, and only torn down once, for the last. "
    "Useful for shaking out flaky tests with stable, expensive test "
    "environments. If --htest_repeat is set to a negative number, meaning "
    "there is no last run, the environments will always be recreated to avoid "
    "leaks.");

DEFINE_bool(htest_show_internal_stack_frames, false,
    "True if and only if " HTEST_NAME_ " should include internal stack frames when "
    "printing test failure stack traces.");

DEFINE_bool(htest_shuffle,
    hawk::internal::BoolFromHTestEnv("shuffle", false),
    "True if and only if " HTEST_NAME_ " should randomize tests' order on every run.");

DEFINE_int32(htest_stack_trace_depth,
    hawk::internal::Int32FromHTestEnv("stack_trace_depth", hawk::kMaxStackTraceDepth),
    "The maximum number of stack frames to print when an "
    "assertion fails.  The valid range is 0 through 100, inclusive.");

DEFINE_string(htest_stream_result_to,
    hawk::internal::StringFromHTestEnv("stream_result_to", ""),
    "This flag specifies the host name and the port number on which to stream "
    "test results. Example: \"localhost:555\". The flag is effective only on "
    "Linux and macOS.");

DEFINE_bool(htest_throw_on_failure,
    hawk::internal::BoolFromHTestEnv("throw_on_failure", false),
    "When this flag is specified, a failed assertion will throw an exception "
    "if exceptions are enabled or exit the program with a non-zero code "
    "otherwise. For use with an external test framework.");

DEFINE_int32(htest_shard_index,
    hawk::get_int32_from_env_or_die(hawk::kTestShardIndex, -1),
    "The zero-based index of the shard to run. A value of -1 "
    "(the default) indicates that sharding is disabled.");

DEFINE_int32(htest_total_shards,
    hawk::get_int32_from_env_or_die(hawk::kTestTotalShards, -1),
    "The total number of shards to use when running tests in parallel. "
    "A value of -1 (the default) indicates that sharding is disabled.");

DEFINE_string(htest_flagfile,
    hawk::internal::StringFromHTestEnv("flagfile", ""),
    "This flag specifies the flagfile to read command-line flags from.");

DEFINE_string(htest_death_test_style,
    hawk::internal::StringFromHTestEnv("death_test_style", hawk::kDefaultDeathTestStyle),
    "Indicates how to run a death test in a forked child process: "
    "\"threadsafe\" (child process re-executes the test binary "
    "from the beginning, running only the specific death test) or "
    "\"fast\" (child process runs the death test immediately "
    "after forking).");

DEFINE_bool(htest_death_test_use_fork,
    hawk::internal::BoolFromHTestEnv("death_test_use_fork", false),
    "Instructs to use fork()/_Exit() instead of clone() in death tests. "
    "Ignored and always uses fork() on POSIX systems where clone() is not "
    "implemented. Useful when running under valgrind or similar tools if "
    "those do not support clone(). Valgrind 3.3.1 will just fail if "
    "it sees an unsupported combination of clone() flags. "
    "It is not recommended to use this flag w/o valgrind though it will "
    "work in 99% of the cases. Once valgrind is fixed, this flag will "
    "most likely be removed.");

DEFINE_string(htest_internal_run_death_test, "",
    "Indicates the file, line number, temporal index of "
    "the single death test to run, and a file descriptor to "
    "which a success code may be sent, all separated by "
    "the '|' characters.  This flag is specified if and only if the "
    "current process is a sub-process launched for running a thread-safe "
    "death test.  FOR INTERNAL USE ONLY.");
