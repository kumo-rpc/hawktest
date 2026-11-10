// Copyright 2005, Google Inc.
// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Portions of this software are derived from GoogleTest and Google Mock,
// originally distributed under the BSD 3-Clause License.
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
// For third-party components and portions derived from GoogleTest or
// Google Mock, the applicable original license terms and notices are
// retained in accordance with their respective licenses.
//
//
// For third-party components and portions derived from GoogleTest or
// Google Mock, the applicable original license terms and notices are
// retained in accordance with their respective licenses.
//

//
// The Google C++ Testing and Mocking Framework (Hawk Test)

#include <hawktest/common/environment.h>
#include <hawktest/test/htest.h>
#include <hawktest/format/printer.h>
#include <hawktest/flags/parser.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

#include <algorithm>
#include <chrono> // NOLINT
#include <cmath>
#include <csignal> // NOLINT: raise(3) is used on some platforms
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <ostream> // NOLINT
#include <set>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include <hawktest/test/internal/unit_test_impl.h>
#include <hawktest/test/internal/unit_test_option.h>
#include <hawktest/common/port.h>
#include <hawktest/analysis/assertion_result.h>
#include <hawktest/test/internal/custom/htest.h>
#include <hawktest/test/spi.h>
#include <hawktest/reporter/pretty_unit_test_result_printer.h>
#include <hawktest/reporter/brief_unit_test_result_printer.h>
#include <hawktest/reporter/json_unit_test_result_printer.h>
#include <hawktest/reporter/xml_unit_test_result_printer.h>
#include <hawktest/reporter/test_event_repeater.h>
#include <hawktest/test/initialize.h>

#ifdef HTEST_OS_LINUX

#include <fcntl.h> // NOLINT
#include <limits.h> // NOLINT
#include <sched.h> // NOLINT
// Declares vsnprintf().  This header is not available on Windows.
#include <strings.h> // NOLINT
#include <sys/mman.h> // NOLINT
#include <sys/time.h> // NOLINT
#include <unistd.h> // NOLINT

#include <string>

#elif defined(HTEST_OS_ZOS)
#include <sys/time.h> // NOLINT

// On z/OS we additionally need strings.h for strcasecmp.
#include <strings.h> // NOLINT

#elif defined(HTEST_OS_WINDOWS_MOBILE) // We are on Windows CE.

#include <windows.h> // NOLINT
#undef min

#elif defined(HTEST_OS_WINDOWS) // We are on Windows proper.

#include <windows.h> // NOLINT
#undef min

#ifdef _MSC_VER
#include <crtdbg.h> // NOLINT
#endif

#include <io.h> // NOLINT
#include <sys/stat.h> // NOLINT
#include <sys/timeb.h> // NOLINT
#include <sys/types.h> // NOLINT

#ifdef HTEST_OS_WINDOWS_MINGW
#include <sys/time.h> // NOLINT
#endif // HTEST_OS_WINDOWS_MINGW

#else

// cpplint thinks that the header is already included, so we want to
// silence it.
#include <sys/time.h> // NOLINT
#include <unistd.h> // NOLINT

#endif // HTEST_OS_LINUX

#if HTEST_HAS_EXCEPTIONS
#include <stdexcept>
#endif

#if HTEST_CAN_STREAM_RESULTS_
#include <arpa/inet.h> // NOLINT
#include <netdb.h> // NOLINT
#include <sys/socket.h> // NOLINT
#include <sys/types.h> // NOLINT
#endif

#include <hawktest/common/internal-inl.h>

#ifdef HTEST_OS_WINDOWS
#define vsnprintf _vsnprintf
#endif // HTEST_OS_WINDOWS

#ifdef HTEST_OS_MAC
#ifndef HTEST_OS_IOS
#include <crt_externs.h>
#endif
#endif



namespace hawk {

    using internal::CountIf;
    using internal::ForEach;
    using internal::GetElementOr;
    using internal::Shuffle;

    namespace internal {

        // g_help_flag is true if and only if the --help flag or an equivalent form
        // is specified on the command line.
        bool g_help_flag = false;


    } // namespace internal

} // namespace hawk

namespace hawk {
    namespace internal {


        std::set<std::string>* GetIgnoredParameterizedTestSuites() {
            return UnitTest::GetInstance()->impl()->ignored_parameterized_test_suites();
        }

        void RegisterTypeParameterizedTestSuite(const char* test_suite_name,
            CodeLocation code_location) {
            GetUnitTestImpl()->type_parameterized_test_registry().RegisterTestSuite(
                test_suite_name, std::move(code_location));
        }

        void RegisterTypeParameterizedTestSuiteInstantiation(const char* case_name) {
            GetUnitTestImpl()->type_parameterized_test_registry().RegisterInstantiation(
                case_name);
        }

        void TypeParameterizedTestSuiteRegistry::RegisterTestSuite(
            const char* test_suite_name, CodeLocation code_location) {
            suites_.emplace(std::string(test_suite_name),
                TypeParameterizedTestSuiteInfo(std::move(code_location)));
        }


    } // namespace internal

    namespace internal {

        // Returns the type ID of ::hawk::Test.  We should always call this
        // instead of GetTypeId< ::hawk::Test>() to get the type ID of
        // hawk::Test.  This is to work around a suspected linker bug when
        // using Hawk Test as a framework on Mac OS X.  The bug causes
        // GetTypeId< ::hawk::Test>() to return different values depending
        // on whether the call is from the Hawk Test framework itself or
        // from user test code.  GetTestTypeId() is guaranteed to always
        // return the same value, as it always calls GetTypeId<>() from the
        // htest.cc, which is within the Hawk Test framework.
        TypeId GetTestTypeId() {
            return GetTypeId<Test>();
        }

        // The value of GetTestTypeId() as seen from within the Hawk Test
        // library.  This is solely for testing GetTestTypeId().
        extern const TypeId kTestTypeIdInHawkTest = GetTestTypeId();


    } // namespace internal

    namespace internal {

#ifdef HTEST_OS_WINDOWS

        namespace {

            // Helper function for IsHRESULT{SuccessFailure} predicates
            AssertionResult HRESULTFailureHelper(const char* expr, const char* expected,
                long hr) { // NOLINT
#if defined(HTEST_OS_WINDOWS_MOBILE) || defined(HTEST_OS_WINDOWS_TV_TITLE)

                // Windows CE doesn't support FormatMessage.
                const char error_text[] = "";

#else

                // Looks up the human-readable system message for the HRESULT code
                // and since we're not passing any params to FormatMessage, we don't
                // want inserts expanded.
                const DWORD kFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
                const DWORD kBufSize = 4096;
                // Gets the system's human readable message string for this HRESULT.
                char error_text[kBufSize] = { '\0' };
                DWORD message_length = ::FormatMessageA(kFlags,
                    0, // no source, we're asking system
                    static_cast<DWORD>(hr), // the error
                    0, // no line width restrictions
                    error_text, // output buffer
                    kBufSize, // buf size
                    nullptr); // no arguments for inserts
                // Trims tailing white space (FormatMessage leaves a trailing CR-LF)
                for (; message_length && IsSpace(error_text[message_length - 1]);
                    --message_length) {
                    error_text[message_length - 1] = '\0';
                }

#endif // HTEST_OS_WINDOWS_MOBILE

                const std::string error_hex("0x" + String::FormatHexInt(hr));
                return ::hawk::AssertionFailure()
                    << "Expected: " << expr << " " << expected << ".\n"
                    << "  Actual: " << error_hex << " " << error_text << "\n";
            }

        } // namespace

        AssertionResult IsHRESULTSuccess(const char* expr, long hr) { // NOLINT
            if (SUCCEEDED(hr)) {
                return AssertionSuccess();
            }
            return HRESULTFailureHelper(expr, "succeeds", hr);
        }

        AssertionResult IsHRESULTFailure(const char* expr, long hr) { // NOLINT
            if (FAILED(hr)) {
                return AssertionSuccess();
            }
            return HRESULTFailureHelper(expr, "fails", hr);
        }

#endif // HTEST_OS_WINDOWS

    } // namespace internal

    namespace internal {

        // Creates a new TestInfo object and registers it with Hawk Test;
        // returns the created object.
        //
        // Arguments:
        //
        //   test_suite_name:  name of the test suite
        //   name:             name of the test
        //   type_param:       the name of the test's type parameter, or NULL if
        //                     this is not a typed or a type-parameterized test.
        //   value_param:      text representation of the test's value parameter,
        //                     or NULL if this is not a value-parameterized test.
        //   code_location:    code location where the test is defined
        //   fixture_class_id: ID of the test fixture class
        //   set_up_tc:        pointer to the function that sets up the test suite
        //   tear_down_tc:     pointer to the function that tears down the test suite
        //   factory:          pointer to the factory that creates a test object.
        //                     The newly created TestInfo instance will assume
        //                     ownership of the factory object.
        TestInfo* MakeAndRegisterTestInfo(
            std::string test_suite_name, const char* name, const char* type_param,
            const char* value_param, CodeLocation code_location,
            TypeId fixture_class_id, SetUpTestSuiteFunc set_up_tc,
            TearDownTestSuiteFunc tear_down_tc, TestFactoryBase* factory) {
            TestInfo* const test_info = new TestInfo(std::move(test_suite_name), name, type_param, value_param,
                std::move(code_location), fixture_class_id, factory);
            GetUnitTestImpl()->AddTestInfo(set_up_tc, tear_down_tc, test_info);
            return test_info;
        }


    } // namespace internal

    namespace internal {

        // Checks whether sharding is enabled by examining the relevant command line
        // arguments. If the arguments are present, but inconsistent
        // (i.e., shard_index >= total_shards), prints an error and exits.
        // If in_subprocess_for_death_test, sharding is disabled because it must only
        // be applied to the original test process. Otherwise, we could filter out death
        // tests we intended to execute.
        bool ShouldShard(bool in_subprocess_for_death_test) {
            if (in_subprocess_for_death_test) {
                return false;
            }

            const int32_t total_shards = FLAGS_htest_total_shards;
            const int32_t shard_index = FLAGS_htest_shard_index;

            if (total_shards == -1 && shard_index == -1) {
                return false;
            } else if (total_shards == -1 && shard_index != -1) {
                const hawk::Message msg = hawk::Message()
                    << "Invalid sharding: you have " << hawk::Constants::kTestShardIndex
                    << " = " << shard_index << ", but have left "
                    << hawk::Constants::kTestTotalShards << " unset.\n";
                hawk::stdout_printer().fmt_red("{}", msg.GetString());
                fflush(stdout);
                exit(EXIT_FAILURE);
            } else if (total_shards != -1 && shard_index == -1) {
                const hawk::Message msg = hawk::Message()
                    << "Invalid sharding: you have " << hawk::Constants::kTestTotalShards
                    << " = " << total_shards << ", but have left "
                    << hawk::Constants::kTestShardIndex << " unset.\n";
                hawk::stdout_printer().fmt_red("{}", msg.GetString());
                fflush(stdout);
                exit(EXIT_FAILURE);
            } else if (shard_index < 0 || shard_index >= total_shards) {
                const hawk::Message msg = hawk::Message() << "Invalid sharding: we require 0 <= " << hawk::Constants::kTestShardIndex
                                              << " < " << hawk::Constants::kTestTotalShards << ", but you have "
                                              << hawk::Constants::kTestShardIndex << "=" << shard_index << ", "
                                              << hawk::Constants::kTestTotalShards << "=" << total_shards << ".\n";
                hawk::stdout_printer().fmt_red("{}", msg.GetString());
                fflush(stdout);
                exit(EXIT_FAILURE);
            }

            return total_shards > 1;
        }

        // Given the total number of shards, the shard index, and the test id,
        // returns true if and only if the test should be run on this shard. The test id
        // is some arbitrary but unique non-negative integer assigned to each test
        // method. Assumes that 0 <= shard_index < total_shards.
        bool ShouldRunTestOnShard(int total_shards, int shard_index, int test_id) {
            return (test_id % total_shards) == shard_index;
        }


        // Used by the HTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_ macro to
        // suppress unreachable code warnings.
        namespace {
            class ClassUniqueToAlwaysTrue { };
        } // namespace

        bool IsTrue(bool condition) {
            return condition;
        }

        bool AlwaysTrue() {
#if HTEST_HAS_EXCEPTIONS
            // This condition is always false so AlwaysTrue() never actually throws,
            // but it makes the compiler think that it may throw.
            if (IsTrue(false))
                throw ClassUniqueToAlwaysTrue();
#endif // HTEST_HAS_EXCEPTIONS
            return true;
        }



    } // namespace internal

#if !defined(HTEST_CUSTOM_TEMPDIR_FUNCTION_) || !defined(HTEST_CUSTOM_SRCDIR_FUNCTION_)
    // Returns the value of the first environment variable that is set and contains
    // a non-empty string. If there are none, returns the "fallback" string. Adds
    // the director-separator character as a suffix if not provided in the
    // environment variable value.
    static std::string GetDirFromEnv(
        std::initializer_list<const char*> environment_variables,
        const char* fallback, char separator) {
        for (const char* variable_name : environment_variables) {
            const std::optional<std::string> value = hawk::get_env(variable_name);
            if (value.has_value() && !value->empty()) {
                if (value->back() != separator) {
                    return std::string(*value).append(1, separator);
                }
                return *value;
            }
        }
        return fallback;
    }
#endif

    std::string TempDir() {
#if defined(HTEST_CUSTOM_TEMPDIR_FUNCTION_)
        return HTEST_CUSTOM_TEMPDIR_FUNCTION_();
#elif defined(HTEST_OS_WINDOWS) || defined(HTEST_OS_WINDOWS_MOBILE)
        return GetDirFromEnv({ "TEST_TMPDIR", "TEMP" }, "\\temp\\", '\\');
#elif defined(HTEST_OS_LINUX_ANDROID)
        return GetDirFromEnv({ "TEST_TMPDIR", "TMPDIR" }, "/data/local/tmp/", '/');
#else
        return GetDirFromEnv({ "TEST_TMPDIR", "TMPDIR" }, "/tmp/", '/');
#endif
    }

#if HTEST_HAS_FILE_SYSTEM && !defined(HTEST_CUSTOM_SRCDIR_FUNCTION_)
    // Returns the directory path (including terminating separator) of the current
    // executable as derived from argv[0].
    static std::string GetCurrentExecutableDirectory() {
        internal::FilePath argv_0(internal::GetArgvs()[0]);
        return argv_0.RemoveFileName().string();
    }
#endif

#if HTEST_HAS_FILE_SYSTEM
    std::string SrcDir() {
#if defined(HTEST_CUSTOM_SRCDIR_FUNCTION_)
        return HTEST_CUSTOM_SRCDIR_FUNCTION_();
#elif defined(HTEST_OS_WINDOWS) || defined(HTEST_OS_WINDOWS_MOBILE)
        return GetDirFromEnv({ "TEST_SRCDIR" }, GetCurrentExecutableDirectory().c_str(),
            '\\');
#elif defined(HTEST_OS_LINUX_ANDROID)
        return GetDirFromEnv({ "TEST_SRCDIR" }, GetCurrentExecutableDirectory().c_str(),
            '/');
#else
        return GetDirFromEnv({ "TEST_SRCDIR" }, GetCurrentExecutableDirectory().c_str(),
            '/');
#endif
    }
#endif

} // namespace hawk
