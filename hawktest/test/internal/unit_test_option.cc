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

#include <hawktest/test/internal/unit_test_option.h>

#include "hawktest/reportor/constants.h"

#include <hawktest/common/filepath.h>
#include <hawktest/flags/builtin.h>
#include <hawktest/test/unit_test.h>

namespace testing::internal {

    // Returns the output format, or "" for normal printed output.
    std::string UnitTestOptions::GetOutputFormat() {
        std::string s = FLAGS_htest_output;
        const char* const htest_output_flag = s.c_str();
        const char* const colon = strchr(htest_output_flag, ':');
        return (colon == nullptr)
            ? std::string(htest_output_flag)
            : std::string(htest_output_flag,
                  static_cast<size_t>(colon - htest_output_flag));
    }


#if HTEST_HAS_FILE_SYSTEM
    // Returns the name of the requested output file, or the default if none
    // was explicitly specified.
    std::string UnitTestOptions::GetAbsolutePathToOutputFile() {
        std::string s = FLAGS_htest_output;
        const char* const htest_output_flag = s.c_str();

        std::string format = GetOutputFormat();
        if (format.empty())
            format = hawk::Constants::kDefaultOutputFormat;

        const char* const colon = strchr(htest_output_flag, ':');
        if (colon == nullptr)
            return internal::FilePath::MakeFileName(
                internal::FilePath(
                    UnitTest::GetInstance()->original_working_dir()),
                internal::FilePath( hawk::Constants::kDefaultOutputFile), 0, format.c_str())
                .string();

        internal::FilePath output_name(colon + 1);
        if (!output_name.IsAbsolutePath())
            output_name = internal::FilePath::ConcatPaths(
                internal::FilePath(UnitTest::GetInstance()->original_working_dir()),
                internal::FilePath(colon + 1));

        if (!output_name.IsDirectory())
            return output_name.string();

        internal::FilePath result(internal::FilePath::GenerateUniqueFileName(
            output_name, internal::GetCurrentExecutableName(),
            GetOutputFormat().c_str()));
        return result.string();
    }
#endif // HTEST_HAS_FILE_SYSTEM


    bool UnitTestOptions::MatchesFilter(const std::string& name_str,
     const char* filter) {
        return UnitTestFilter(filter).MatchesName(name_str);
    }


#if HTEST_HAS_SEH
    static std::string FormatSehExceptionMessage(DWORD exception_code,
        const char* location) {
        Message message;
        message << "SEH exception with code 0x" << std::setbase(16) << exception_code
                << std::setbase(10) << " thrown in " << location << ".";
        return message.GetString();
    }

    int UnitTestOptions::HTestProcessSEH(DWORD seh_code, const char* location) {
        // Hawk Test should handle a SEH exception if:
        //   1. the user wants it to, AND
        //   2. this is not a breakpoint exception or stack overflow, AND
        //   3. this is not a C++ exception (VC++ implements them via SEH,
        //      apparently).
        //
        // SEH exception code for C++ exceptions.
        // (see https://support.microsoft.com/kb/185294 for more information).
        const DWORD kCxxExceptionCode = 0xe06d7363;

        if (!FLAGS_htest_catch_exceptions || seh_code == kCxxExceptionCode || seh_code == EXCEPTION_BREAKPOINT || seh_code == EXCEPTION_STACK_OVERFLOW) {
            return EXCEPTION_CONTINUE_SEARCH; // Don't handle these exceptions
        }

        internal::ReportFailureInUnknownLocation(
            TestPartResult::kFatalFailure,
            FormatSehExceptionMessage(seh_code, location) + "\n"
                                                            "Stack trace:\n"
                + ::testing::internal::GetCurrentOsStackTraceExceptTop(1));

        return EXCEPTION_EXECUTE_HANDLER;
    }
#endif // HTEST_HAS_SEH


    // Returns true if and only if the user-specified filter matches the test
    // suite name and the test name.
    bool UnitTestOptions::FilterMatchesTest(const std::string& test_suite_name,
        const std::string& test_name) {
        // Split --htest_filter at '-', if there is one, to separate into
        // positive filter and negative filter portions
        return PositiveAndNegativeUnitTestFilter(FLAGS_htest_filter)
            .MatchesTest(test_suite_name, test_name);
    }


}  // namespace testing::internal
