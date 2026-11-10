// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include <hawktest/flags/parser.h>
#include <hawktest/format/printer.h>
#include <hawktest/common/internal-inl.h>
#include <hawktest/flags/builtin.h>

#include <cstring>
#include <string>
#include <vector>

#ifdef HTEST_OS_MAC
#ifndef HTEST_OS_IOS
#include <crt_externs.h>
#endif
#endif

namespace testing {
namespace internal {

        // If *pstr starts with the given prefix, modifies *pstr to be right
        // past the prefix and returns true; otherwise leaves *pstr unchanged
        // and returns false.  None of pstr, *pstr, and prefix can be NULL.
        bool SkipPrefix(const char* prefix, const char** pstr) {
            const size_t prefix_len = strlen(prefix);
            if (strncmp(*pstr, prefix, prefix_len) == 0) {
                *pstr += prefix_len;
                return true;
            }
            return false;
        }

        // Parses a string as a command line flag.  The string should have
        // the format "--flag=value".  When def_optional is true, the "=value"
        // part can be omitted.
        //
        // Returns the value of the flag, or NULL if the parsing failed.
        static const char* ParseFlagValue(const char* str, const char* flag_name,
            bool def_optional) {
            // str and flag must not be NULL.
            if (str == nullptr || flag_name == nullptr)
                return nullptr;

            // The flag must start with "--" followed by HTEST_FLAG_PREFIX_.
            const std::string flag_str = std::string("--") + HTEST_FLAG_PREFIX_ + flag_name;
            const size_t flag_len = flag_str.length();
            if (strncmp(str, flag_str.c_str(), flag_len) != 0)
                return nullptr;

            // Skips the flag name.
            const char* flag_end = str + flag_len;

            // When def_optional is true, it's OK to not have a "=value" part.
            if (def_optional && (flag_end[0] == '\0')) {
                return flag_end;
            }

            // If def_optional is true and there are more characters after the
            // flag name, or if def_optional is false, there must be a '=' after
            // the flag name.
            if (flag_end[0] != '=')
                return nullptr;

            // Returns the string after "=".
            return flag_end + 1;
        }

        // Parses a string for a bool flag, in the form of either
        // "--flag=value" or "--flag".
        //
        // In the former case, the value is taken as true as long as it does
        // not start with '0', 'f', or 'F'.
        //
        // In the latter case, the value is taken as true.
        //
        // On success, stores the value of the flag in *value, and returns
        // true.  On failure, returns false without changing *value.
        static bool ParseFlag(const char* str, const char* flag_name, bool* value) {
            // Gets the value of the flag as a string.
            const char* const value_str = ParseFlagValue(str, flag_name, true);

            // Aborts if the parsing failed.
            if (value_str == nullptr)
                return false;

            // Converts the string value to a bool.
            *value = !(*value_str == '0' || *value_str == 'f' || *value_str == 'F');
            return true;
        }

        // Parses a string for an int32_t flag, in the form of "--flag=value".
        //
        // On success, stores the value of the flag in *value, and returns
        // true.  On failure, returns false without changing *value.
        bool ParseFlag(const char* str, const char* flag_name, int32_t* value) {
            // Gets the value of the flag as a string.
            const char* const value_str = ParseFlagValue(str, flag_name, false);

            // Aborts if the parsing failed.
            if (value_str == nullptr)
                return false;

            // Sets *value to the value of the flag.
            return ParseInt32(Message() << "The value of flag --" << flag_name, value_str,
                value);
        }

        // Parses a string for a string flag, in the form of "--flag=value".
        //
        // On success, stores the value of the flag in *value, and returns
        // true.  On failure, returns false without changing *value.
        template <typename String>
        static bool ParseFlag(const char* str, const char* flag_name, String* value) {
            // Gets the value of the flag as a string.
            const char* const value_str = ParseFlagValue(str, flag_name, false);

            // Aborts if the parsing failed.
            if (value_str == nullptr)
                return false;

            // Sets *value to the value of the flag.
            *value = value_str;
            return true;
        }

        // Determines whether a string has a prefix that Hawk Test uses for its
        // flags, i.e., starts with HTEST_FLAG_PREFIX_ or HTEST_FLAG_PREFIX_DASH_.
        // If Hawk Test detects that a command line flag has its prefix but is not
        // recognized, it will print its help message. Flags starting with
        // HTEST_INTERNAL_PREFIX_ followed by "internal_" are considered Hawk Test
        // internal flags and do not trigger the help message.
        static bool HasHawkTestFlagPrefix(const char* str) {
            return (SkipPrefix("--", &str) || SkipPrefix("-", &str) || SkipPrefix("/", &str)) && !SkipPrefix(HTEST_FLAG_PREFIX_ "internal_", &str) && (SkipPrefix(HTEST_FLAG_PREFIX_, &str) || SkipPrefix(HTEST_FLAG_PREFIX_DASH_, &str));
        }

        // Prints a string containing code-encoded text.  The following escape
        // sequences can be used in the string to control the text color:
        //
        //   @@    prints a single '@' character.
        //   @R    changes the color to red.
        //   @G    changes the color to green.
        //   @Y    changes the color to yellow.
        //   @D    changes to the default terminal text color.
        //
        static void print_segment(hawk::Color color, const char* text) {
            hawk::Printer& out = hawk::stdout_printer();
            if (color == hawk::Color::COLOR_DEFAULT) {
                out.fmt("{}", text);
                return;
            }
            out.print_auto_color(color, text);
        }

        static void PrintColorEncoded(const char* str) {
            hawk::Color color = hawk::Color::COLOR_DEFAULT;

            // Conceptually, we split the string into segments divided by escape
            // sequences.  Then we print one segment at a time.  At the end of
            // each iteration, the str pointer advances to the beginning of the
            // next segment.
            for (;;) {
                const char* p = strchr(str, '@');
                if (p == nullptr) {
                    print_segment(color, str);
                    return;
                }

                print_segment(color, std::string(str, p).c_str());

                const char ch = p[1];
                str = p + 2;
                if (ch == '@') {
                    print_segment(color, "@");
                } else if (ch == 'D') {
                    color = hawk::Color::COLOR_DEFAULT;
                } else if (ch == 'R') {
                    color = hawk::Color::COLOR_RED;
                } else if (ch == 'G') {
                    color = hawk::Color::COLOR_GREEN;
                } else if (ch == 'Y') {
                    color = hawk::Color::COLOR_YELLOW;
                } else {
                    --str;
                }
            }
        }

        static const char kColorEncodedHelpMessage[] = "This program contains tests written using " HTEST_NAME_
                                                       ". You can use the\n"
                                                       "following command line flags to control its behavior:\n"
                                                       "\n"
                                                       "Test Selection:\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "list_tests@D\n"
                                                       "      List the names of all tests instead of running them. The name of\n"
                                                       "      TEST(Foo, Bar) is \"Foo.Bar\".\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "filter=@YPOSITIVE_PATTERNS"
                                                       "[@G-@YNEGATIVE_PATTERNS]@D\n"
                                                       "      Run only the tests whose name matches one of the positive patterns "
                                                       "but\n"
                                                       "      none of the negative patterns. '?' matches any single character; "
                                                       "'*'\n"
                                                       "      matches any substring; ':' separates two patterns.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "also_run_disabled_tests@D\n"
                                                       "      Run all disabled tests too.\n"
                                                       "\n"
                                                       "Test Execution:\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "repeat=@Y[COUNT]@D\n"
                                                       "      Run the tests repeatedly; use a negative count to repeat forever.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "shuffle@D\n"
                                                       "      Randomize tests' orders on every iteration.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "random_seed=@Y[NUMBER]@D\n"
                                                       "      Random number seed to use for shuffling test orders (between 1 and\n"
                                                       "      99999, or 0 to use a seed based on the current time).\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "recreate_environments_when_repeating@D\n"
                                                       "      Sets up and tears down the global test environment on each repeat\n"
                                                       "      of the test.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "fail_fast@D\n"
                                                       "      Stop running tests after the first failure.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "fail_if_no_test_linked@D\n"
                                                       "      Fail if no test is linked into the test program.\n"
                                                       "\n"
                                                       "Test Output:\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "color=@Y(@Gyes@Y|@Gno@Y|@Gauto@Y)@D\n"
                                                       "      Enable/disable colored output. The default is @Gauto@D.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "brief=1@D\n"
                                                       "      Only print test failures.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "print_time=0@D\n"
                                                       "      Don't print the elapsed time of each test.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "print_utf8=0@D\n"
                                                       "      Don't print UTF-8 characters as text.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "output=@Y(@Gjson@Y|@Gxml@Y)[@G:@YDIRECTORY_PATH@G" HTEST_PATH_SEP_
                                                       "@Y|@G:@YFILE_PATH]@D\n"
                                                       "      Generate a JSON or XML report in the given directory or with the "
                                                       "given\n"
                                                       "      file name. @YFILE_PATH@D defaults to @Gtest_detail.xml@D.\n"
#if HTEST_CAN_STREAM_RESULTS_
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "stream_result_to=@YHOST@G:@YPORT@D\n"
                                                       "      Stream test results to the given server.\n"
#endif // HTEST_CAN_STREAM_RESULTS_
                                                       "\n"
                                                       "Assertion Behavior:\n"
#if defined(HTEST_HAS_DEATH_TEST) && !defined(HTEST_OS_WINDOWS)
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "death_test_style=@Y(@Gfast@Y|@Gthreadsafe@Y)@D\n"
                                                       "      Set the default death test style.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "death_test_use_fork@D\n"
                                                       "      Use fork() instead of clone() to spawn death test child processes.\n"
#endif // HTEST_HAS_DEATH_TEST && !HTEST_OS_WINDOWS
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "break_on_failure@D\n"
                                                       "      Turn assertion failures into debugger break-points.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "throw_on_failure@D\n"
                                                       "      Turn assertion failures into C++ exceptions for use by an external\n"
                                                       "      test framework.\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "catch_exceptions=0@D\n"
                                                       "      Do not report exceptions as test failures. Instead, allow them\n"
                                                       "      to crash the program or throw a pop-up (on Windows).\n"
                                                       "  @G--" HTEST_FLAG_PREFIX_
                                                       "stack_trace_depth=@Y[NUMBER]@D\n"
                                                       "      Maximum number of stack frames to print when an assertion fails.\n"
                                                       "\n"
                                                       "Except for @G--" HTEST_FLAG_PREFIX_
                                                       "list_tests@D, you can alternatively set "
                                                       "the corresponding\n"
                                                       "environment variable of a flag (all letters in upper-case). For example, "
                                                       "to\n"
                                                       "disable colored text output, you can either specify "
                                                       "@G--" HTEST_FLAG_PREFIX_
                                                       "color=no@D or set\n"
                                                       "the @G" HTEST_FLAG_PREFIX_UPPER_
                                                       "COLOR@D environment variable to @Gno@D.\n"
                                                       "\n"
                                                       "For more information, please read the " HTEST_NAME_
                                                       " documentation at\n"
                                                       "@G" HTEST_PROJECT_URL_ "@D. If you find a bug in " HTEST_NAME_
                                                       "\n"
                                                       "(not one in your own code or tests), please report it to\n"
                                                       "@G<" HTEST_DEV_EMAIL_ ">@D.\n";

        static bool parse_hawk_test_flag(const char* const arg) {
#define HTEST_INTERNAL_PARSE_FLAG(flag_name)      \
    do {                                          \
        auto value = FLAGS_htest_##flag_name;   \
        if (ParseFlag(arg, #flag_name, &value)) { \
            FLAGS_htest_##flag_name = value;     \
            return true;                          \
        }                                         \
    } while (false)

            HTEST_INTERNAL_PARSE_FLAG(also_run_disabled_tests);
            HTEST_INTERNAL_PARSE_FLAG(break_on_failure);
            HTEST_INTERNAL_PARSE_FLAG(catch_exceptions);
            HTEST_INTERNAL_PARSE_FLAG(color);
            HTEST_INTERNAL_PARSE_FLAG(death_test_style);
            HTEST_INTERNAL_PARSE_FLAG(death_test_use_fork);
            HTEST_INTERNAL_PARSE_FLAG(fail_fast);
            HTEST_INTERNAL_PARSE_FLAG(fail_if_no_test_linked);
            HTEST_INTERNAL_PARSE_FLAG(fail_if_no_test_selected);
            HTEST_INTERNAL_PARSE_FLAG(filter);
            HTEST_INTERNAL_PARSE_FLAG(internal_run_death_test);
            HTEST_INTERNAL_PARSE_FLAG(list_tests);
            HTEST_INTERNAL_PARSE_FLAG(output);
            HTEST_INTERNAL_PARSE_FLAG(brief);
            HTEST_INTERNAL_PARSE_FLAG(print_time);
            HTEST_INTERNAL_PARSE_FLAG(print_utf8);
            HTEST_INTERNAL_PARSE_FLAG(random_seed);
            HTEST_INTERNAL_PARSE_FLAG(repeat);
            HTEST_INTERNAL_PARSE_FLAG(shard_index);
            HTEST_INTERNAL_PARSE_FLAG(total_shards);
            HTEST_INTERNAL_PARSE_FLAG(recreate_environments_when_repeating);
            HTEST_INTERNAL_PARSE_FLAG(shuffle);
            HTEST_INTERNAL_PARSE_FLAG(stack_trace_depth);
            HTEST_INTERNAL_PARSE_FLAG(stream_result_to);
            HTEST_INTERNAL_PARSE_FLAG(throw_on_failure);
            return false;
        }

#if HTEST_HAS_FILE_SYSTEM
        static void LoadFlagsFromFile(const std::string& path) {
            FILE* flagfile = posix::FOpen(path.c_str(), "r");
            if (!flagfile) {
                HTEST_LOG_(FATAL) << "Unable to open file \"" << FLAGS_htest_flagfile
                                  << "\"";
            }
            std::string contents(ReadEntireFile(flagfile));
            posix::FClose(flagfile);
            std::vector<std::string> lines;
            SplitString(contents, '\n', &lines);
            for (size_t i = 0; i < lines.size(); ++i) {
                if (lines[i].empty())
                    continue;
                if (!parse_hawk_test_flag(lines[i].c_str()))
                    g_help_flag = true;
            }
        }
#endif // HTEST_HAS_FILE_SYSTEM

        // Parses the command line for Hawk Test flags, without initializing
        // other parts of Hawk Test.  The type parameter CharType can be
        // instantiated to either char or wchar_t.
        template <typename CharType>
        void parse_hawk_test_flags_only_impl(int* argc, CharType** argv) {
            std::string flagfile_value;
            for (int i = 1; i < *argc; i++) {
                const std::string arg_string = StreamableToString(argv[i]);
                const char* const arg = arg_string.c_str();

                using internal::ParseFlag;

                bool remove_flag = false;
                if (parse_hawk_test_flag(arg)) {
                    remove_flag = true;
#if HTEST_HAS_FILE_SYSTEM
                } else if (ParseFlag(arg, "flagfile", &flagfile_value)) {
                    FLAGS_htest_flagfile = flagfile_value;
                    LoadFlagsFromFile(flagfile_value);
                    remove_flag = true;
#endif // HTEST_HAS_FILE_SYSTEM
                } else if (arg_string == "--help" || HasHawkTestFlagPrefix(arg)) {
                    // Both help flag and unrecognized Hawk Test flags (excluding
                    // internal ones) trigger help display.
                    g_help_flag = true;
                }

                if (remove_flag) {
                    // Shift the remainder of the argv list left by one.
                    for (int j = i + 1; j < *argc; ++j) {
                        argv[j - 1] = argv[j];
                    }

                    // Decrements the argument count.
                    (*argc)--;

                    // Terminate the array with nullptr.
                    argv[*argc] = nullptr;

                    // We also need to decrement the iterator as we just removed
                    // an element.
                    i--;
                }
            }

            if (g_help_flag) {
                // We print the help here instead of in RUN_ALL_TESTS(), as the
                // latter may not be called at all if the user is using Google
                // Test with another testing framework.
                PrintColorEncoded(kColorEncodedHelpMessage);
            }
        }

        // Parses the command line for Hawk Test flags, without initializing
        // other parts of Hawk Test. This function updates argc and argv by removing
        // flags that are known to HawkTest. Other arguments
        // remain in place. Unrecognized flags are not reported and do not cause the
        // program to exit.
        void parse_hawk_test_flags_only(int* argc, char** argv) {
            parse_hawk_test_flags_only_impl(argc, argv);

            // Fix the value of *_NSGetArgc() on macOS, but if and only if
            // *_NSGetArgv() == argv
            // Only applicable to char** version of argv
#ifdef HTEST_OS_MAC
#ifndef HTEST_OS_IOS
            if (*_NSGetArgv() == argv) {
                *_NSGetArgc() = *argc;
            }
#endif
#endif
        }
        void parse_hawk_test_flags_only(int* argc, wchar_t** argv) {
            parse_hawk_test_flags_only_impl(argc, argv);
        }

} // namespace internal
} // namespace testing
