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

#include <hawktest/test/htest.h>
#include <hawktest/reportor/printer.h>
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
#include <hawktest/test/assertion-result.h>
#include <hawktest/test/internal/custom/htest.h>
#include <hawktest/test/spi.h>
#include <hawktest/test/internal/pretty_unit_test_result_printer.h>
#include <hawktest/test/internal/brief_unit_test_result_printer.h>
#include <hawktest/test/internal/json_unit_test_result_printer.h>
#include <hawktest/test/internal/xml_unit_test_result_printer.h>
#include <hawktest/test/internal/test_event_repeater.h>
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



namespace testing {

    using internal::CountIf;
    using internal::ForEach;
    using internal::GetElementOr;
    using internal::Shuffle;

    // Constants.


    // These environment variables are set by Bazel.
    // https://bazel.build/reference/test-encyclopedia#initial-conditions
    //
    // The environment variable name for the test shard index.
    static const char kTestShardIndex[] = "HTEST_SHARD_INDEX";
    // The environment variable name for the total number of test shards.
    static const char kTestTotalShards[] = "HTEST_TOTAL_SHARDS";
    // The environment variable name for the test shard status file.
    static const char kTestShardStatusFile[] = "HTEST_SHARD_STATUS_FILE";


    namespace internal {

        // The text used in failure messages to indicate the start of the
        // stack trace.
        const char kStackTraceMarker[] = "\nStack trace:\n";

        // g_help_flag is true if and only if the --help flag or an equivalent form
        // is specified on the command line.
        bool g_help_flag = false;


    } // namespace internal

} // namespace testing

namespace testing {
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

        // Returns the type ID of ::testing::Test.  We should always call this
        // instead of GetTypeId< ::testing::Test>() to get the type ID of
        // testing::Test.  This is to work around a suspected linker bug when
        // using Hawk Test as a framework on Mac OS X.  The bug causes
        // GetTypeId< ::testing::Test>() to return different values depending
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

        DefaultGlobalTestPartResultReporter::DefaultGlobalTestPartResultReporter(
            UnitTestImpl* unit_test)
            : unit_test_(unit_test) { }

        void DefaultGlobalTestPartResultReporter::ReportTestPartResult(
            const TestPartResult& result) {
            unit_test_->current_test_result()->AddTestPartResult(result);
            unit_test_->listeners()->repeater()->OnTestPartResult(result);
        }

        DefaultPerThreadTestPartResultReporter::DefaultPerThreadTestPartResultReporter(
            UnitTestImpl* unit_test)
            : unit_test_(unit_test) { }

        void DefaultPerThreadTestPartResultReporter::ReportTestPartResult(
            const TestPartResult& result) {
            unit_test_->GetGlobalTestPartResultReporter()->ReportTestPartResult(result);
        }

    } // namespace internal

    namespace internal {

        namespace edit_distance {
            std::vector<EditType> CalculateOptimalEdits(const std::vector<size_t>& left,
                const std::vector<size_t>& right) {
                std::vector<std::vector<double>> costs(
                    left.size() + 1, std::vector<double>(right.size() + 1));
                std::vector<std::vector<EditType>> best_move(
                    left.size() + 1, std::vector<EditType>(right.size() + 1));

                // Populate for empty right.
                for (size_t l_i = 0; l_i < costs.size(); ++l_i) {
                    costs[l_i][0] = static_cast<double>(l_i);
                    best_move[l_i][0] = kRemove;
                }
                // Populate for empty left.
                for (size_t r_i = 1; r_i < costs[0].size(); ++r_i) {
                    costs[0][r_i] = static_cast<double>(r_i);
                    best_move[0][r_i] = kAdd;
                }

                for (size_t l_i = 0; l_i < left.size(); ++l_i) {
                    for (size_t r_i = 0; r_i < right.size(); ++r_i) {
                        if (left[l_i] == right[r_i]) {
                            // Found a match. Consume it.
                            costs[l_i + 1][r_i + 1] = costs[l_i][r_i];
                            best_move[l_i + 1][r_i + 1] = kMatch;
                            continue;
                        }

                        const double add = costs[l_i + 1][r_i];
                        const double remove = costs[l_i][r_i + 1];
                        const double replace = costs[l_i][r_i];
                        if (add < remove && add < replace) {
                            costs[l_i + 1][r_i + 1] = add + 1;
                            best_move[l_i + 1][r_i + 1] = kAdd;
                        } else if (remove < add && remove < replace) {
                            costs[l_i + 1][r_i + 1] = remove + 1;
                            best_move[l_i + 1][r_i + 1] = kRemove;
                        } else {
                            // We make replace a little more expensive than add/remove to lower
                            // their priority.
                            costs[l_i + 1][r_i + 1] = replace + 1.00001;
                            best_move[l_i + 1][r_i + 1] = kReplace;
                        }
                    }
                }

                // Reconstruct the best path. We do it in reverse order.
                std::vector<EditType> best_path;
                for (size_t l_i = left.size(), r_i = right.size(); l_i > 0 || r_i > 0;) {
                    EditType move = best_move[l_i][r_i];
                    best_path.push_back(move);
                    l_i -= move != kAdd;
                    r_i -= move != kRemove;
                }
                std::reverse(best_path.begin(), best_path.end());
                return best_path;
            }

            namespace {

                // Helper class to convert string into ids with deduplication.
                class InternalStrings {
                public:
                    size_t GetId(const std::string& str) {
                        IdMap::iterator it = ids_.find(str);
                        if (it != ids_.end())
                            return it->second;
                        size_t id = ids_.size();
                        return ids_[str] = id;
                    }

                private:
                    typedef std::map<std::string, size_t> IdMap;
                    IdMap ids_;
                };

            } // namespace

            std::vector<EditType> CalculateOptimalEdits(
                const std::vector<std::string>& left,
                const std::vector<std::string>& right) {
                std::vector<size_t> left_ids, right_ids;
                {
                    InternalStrings intern_table;
                    for (size_t i = 0; i < left.size(); ++i) {
                        left_ids.push_back(intern_table.GetId(left[i]));
                    }
                    for (size_t i = 0; i < right.size(); ++i) {
                        right_ids.push_back(intern_table.GetId(right[i]));
                    }
                }
                return CalculateOptimalEdits(left_ids, right_ids);
            }

            namespace {

                // Helper class that holds the state for one hunk and prints it out to the
                // stream.
                // It reorders adds/removes when possible to group all removes before all
                // adds. It also adds the hunk header before printint into the stream.
                class Hunk {
                public:
                    Hunk(size_t left_start, size_t right_start)
                        : left_start_(left_start)
                        , right_start_(right_start)
                        , adds_()
                        , removes_()
                        , common_() { }

                    void PushLine(char edit, const char* line) {
                        switch (edit) {
                        case ' ':
                            ++common_;
                            FlushEdits();
                            hunk_.push_back(std::make_pair(' ', line));
                            break;
                        case '-':
                            ++removes_;
                            hunk_removes_.push_back(std::make_pair('-', line));
                            break;
                        case '+':
                            ++adds_;
                            hunk_adds_.push_back(std::make_pair('+', line));
                            break;
                        }
                    }

                    void PrintTo(std::ostream* os) {
                        PrintHeader(os);
                        FlushEdits();
                        for (std::list<std::pair<char, const char*>>::const_iterator it = hunk_.begin();
                            it != hunk_.end(); ++it) {
                            *os << it->first << it->second << "\n";
                        }
                    }

                    bool has_edits() const { return adds_ || removes_; }

                private:
                    void FlushEdits() {
                        hunk_.splice(hunk_.end(), hunk_removes_);
                        hunk_.splice(hunk_.end(), hunk_adds_);
                    }

                    // Print a unified diff header for one hunk.
                    // The format is
                    //   "@@ -<left_start>,<left_length> +<right_start>,<right_length> @@"
                    // where the left/right lengths are omitted if unnecessary.
                    void PrintHeader(std::ostream* ss) const {
                        size_t left_length = removes_ + common_;
                        size_t right_length = adds_ + common_;
                        *ss << "@@ " << "-" << left_start_;
                        if (left_length != 1) {
                            *ss << "," << left_length;
                        }
                        *ss << " " << "+" << right_start_;
                        if (right_length != 1) {
                            *ss << "," << right_length;
                        }
                        *ss << " @@\n";
                    }

                    size_t left_start_, right_start_;
                    size_t adds_, removes_, common_;
                    std::list<std::pair<char, const char*>> hunk_, hunk_adds_, hunk_removes_;
                };

            } // namespace

            // Create a list of diff hunks in Unified diff format.
            // Each hunk has a header generated by PrintHeader above plus a body with
            // lines prefixed with ' ' for no change, '-' for deletion and '+' for
            // addition.
            // 'context' represents the desired unchanged prefix/suffix around the diff.
            // If two hunks are close enough that their contexts overlap, then they are
            // joined into one hunk.
            std::string CreateUnifiedDiff(const std::vector<std::string>& left,
                const std::vector<std::string>& right,
                size_t context) {
                const std::vector<EditType> edits = CalculateOptimalEdits(left, right);

                size_t l_i = 0, r_i = 0, edit_i = 0;
                std::stringstream ss;
                while (edit_i < edits.size()) {
                    // Find first edit.
                    while (edit_i < edits.size() && edits[edit_i] == kMatch) {
                        ++l_i;
                        ++r_i;
                        ++edit_i;
                    }

                    // Find the first line to include in the hunk.
                    const size_t prefix_context = std::min(l_i, context);
                    Hunk hunk(l_i - prefix_context + 1, r_i - prefix_context + 1);
                    for (size_t i = prefix_context; i > 0; --i) {
                        hunk.PushLine(' ', left[l_i - i].c_str());
                    }

                    // Iterate the edits until we found enough suffix for the hunk or the input
                    // is over.
                    size_t n_suffix = 0;
                    for (; edit_i < edits.size(); ++edit_i) {
                        if (n_suffix >= context) {
                            // Continue only if the next hunk is very close.
                            auto it = edits.begin() + static_cast<int>(edit_i);
                            while (it != edits.end() && *it == kMatch)
                                ++it;
                            if (it == edits.end() || static_cast<size_t>(it - edits.begin()) - edit_i >= context) {
                                // There is no next edit or it is too far away.
                                break;
                            }
                        }

                        EditType edit = edits[edit_i];
                        // Reset count when a non match is found.
                        n_suffix = edit == kMatch ? n_suffix + 1 : 0;

                        if (edit == kMatch || edit == kRemove || edit == kReplace) {
                            hunk.PushLine(edit == kMatch ? ' ' : '-', left[l_i].c_str());
                        }
                        if (edit == kAdd || edit == kReplace) {
                            hunk.PushLine('+', right[r_i].c_str());
                        }

                        // Advance indices, depending on edit type.
                        l_i += edit != kAdd;
                        r_i += edit != kRemove;
                    }

                    if (!hunk.has_edits()) {
                        // We are done. We don't want this hunk.
                        break;
                    }

                    hunk.PrintTo(&ss);
                }
                return ss.str();
            }

        } // namespace edit_distance

        namespace {

            // The string representation of the values received in EqFailure() are already
            // escaped. Split them on escaped '\n' boundaries. Leave all other escaped
            // characters the same.
            std::vector<std::string> SplitEscapedString(const std::string& str) {
                std::vector<std::string> lines;
                size_t start = 0, end = str.size();
                if (end > 2 && str[0] == '"' && str[end - 1] == '"') {
                    ++start;
                    --end;
                }
                bool escaped = false;
                for (size_t i = start; i + 1 < end; ++i) {
                    if (escaped) {
                        escaped = false;
                        if (str[i] == 'n') {
                            lines.push_back(str.substr(start, i - start - 1));
                            start = i + 1;
                        }
                    } else {
                        escaped = str[i] == '\\';
                    }
                }
                lines.push_back(str.substr(start, end - start));
                return lines;
            }

        } // namespace

        // Constructs and returns the message for an equality assertion
        // (e.g. ASSERT_EQ, EXPECT_STREQ, etc) failure.
        //
        // The first four parameters are the expressions used in the assertion
        // and their values, as strings.  For example, for ASSERT_EQ(foo, bar)
        // where foo is 5 and bar is 6, we have:
        //
        //   lhs_expression: "foo"
        //   rhs_expression: "bar"
        //   lhs_value:      "5"
        //   rhs_value:      "6"
        //
        // The ignoring_case parameter is true if and only if the assertion is a
        // *_STRCASEEQ*.  When it's true, the string "Ignoring case" will
        // be inserted into the message.
        AssertionResult EqFailure(const char* lhs_expression,
            const char* rhs_expression,
            const std::string& lhs_value,
            const std::string& rhs_value, bool ignoring_case) {
            Message msg;
            msg << "Expected equality of these values:";
            msg << "\n  " << lhs_expression;
            if (lhs_value != lhs_expression) {
                msg << "\n    Which is: " << lhs_value;
            }
            msg << "\n  " << rhs_expression;
            if (rhs_value != rhs_expression) {
                msg << "\n    Which is: " << rhs_value;
            }

            if (ignoring_case) {
                msg << "\nIgnoring case";
            }

            if (!lhs_value.empty() && !rhs_value.empty()) {
                const std::vector<std::string> lhs_lines = SplitEscapedString(lhs_value);
                const std::vector<std::string> rhs_lines = SplitEscapedString(rhs_value);
                if (lhs_lines.size() > 1 || rhs_lines.size() > 1) {
                    msg << "\nWith diff:\n"
                        << edit_distance::CreateUnifiedDiff(lhs_lines, rhs_lines);
                }
            }

            return AssertionFailure() << msg;
        }

        // Constructs a failure message for Boolean assertions such as EXPECT_TRUE.
        std::string GetBoolAssertionFailureMessage(
            const AssertionResult& assertion_result, const char* expression_text,
            const char* actual_predicate_value, const char* expected_predicate_value) {
            const char* actual_message = assertion_result.message();
            Message msg;
            msg << "Value of: " << expression_text
                << "\n  Actual: " << actual_predicate_value;
            if (actual_message[0] != '\0')
                msg << " (" << actual_message << ")";
            msg << "\nExpected: " << expected_predicate_value;
            return msg.GetString();
        }

    } // namespace internal

    // Asserts that val1 is less than, or almost equal to, val2.  Fails
    // otherwise.  In particular, it fails if either val1 or val2 is NaN.
    AssertionResult FloatLE(const char* expr1, const char* expr2, float val1,
        float val2) {
        return internal::FloatingPointLE<float>(expr1, expr2, val1, val2);
    }

    // Asserts that val1 is less than, or almost equal to, val2.  Fails
    // otherwise.  In particular, it fails if either val1 or val2 is NaN.
    AssertionResult DoubleLE(const char* expr1, const char* expr2, double val1,
        double val2) {
        return internal::FloatingPointLE<double>(expr1, expr2, val1, val2);
    }

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
                return ::testing::AssertionFailure()
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

// class PrettyUnitTestResultPrinter
#if defined(HTEST_OS_WINDOWS) && !defined(HTEST_OS_WINDOWS_MOBILE) && !defined(HTEST_OS_WINDOWS_GAMES) && !defined(HTEST_OS_WINDOWS_PHONE) && !defined(HTEST_OS_WINDOWS_RT) && !defined(HTEST_OS_WINDOWS_MINGW)

        // Returns the character attribute for the given color.
        static WORD GetColorAttribute(HTestColor color) {
            switch (color) {
            case HTestColor::kRed:
                return FOREGROUND_RED;
            case HTestColor::kGreen:
                return FOREGROUND_GREEN;
            case HTestColor::kYellow:
                return FOREGROUND_RED | FOREGROUND_GREEN;
            default:
                return 0;
            }
        }

        static int GetBitOffset(WORD color_mask) {
            if (color_mask == 0)
                return 0;

            int bitOffset = 0;
            while ((color_mask & 1) == 0) {
                color_mask >>= 1;
                ++bitOffset;
            }
            return bitOffset;
        }

        static WORD GetNewColor(HTestColor color, WORD old_color_attrs) {
            // Let's reuse the BG
            static const WORD background_mask = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
            static const WORD foreground_mask = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
            const WORD existing_bg = old_color_attrs & background_mask;

            WORD new_color = GetColorAttribute(color) | existing_bg | FOREGROUND_INTENSITY;
            static const int bg_bitOffset = GetBitOffset(background_mask);
            static const int fg_bitOffset = GetBitOffset(foreground_mask);

            if (((new_color & background_mask) >> bg_bitOffset) == ((new_color & foreground_mask) >> fg_bitOffset)) {
                new_color ^= FOREGROUND_INTENSITY; // invert intensity
            }
            return new_color;
        }

#else

#endif // HTEST_OS_WINDOWS && !HTEST_OS_WINDOWS_MOBILE

    } // namespace internal



    // class UnitTest


    namespace internal {


#if HTEST_HAS_FILE_SYSTEM
        // Reads the HTEST_SHARD_STATUS_FILE environment variable, and creates the file
        // if the variable is present. If a file already exists at this location, this
        // function will write over it. If the variable is present, but the file cannot
        // be created, prints an error and exits.
        void WriteToShardStatusFileIfNeeded() {
            const char* const test_shard_file = posix::GetEnv(kTestShardStatusFile);
            if (test_shard_file != nullptr) {
                FILE* const file = posix::FOpen(test_shard_file, "w");
                if (file == nullptr) {
                    hawk::stdout_printer().fmt_red(
                        "Could not write to the test shard status file \"{}\" "
                        "specified by the {} environment variable.\n",
                        test_shard_file, kTestShardStatusFile);
                    fflush(stdout);
                    exit(EXIT_FAILURE);
                }
                fclose(file);
            }
        }
#endif // HTEST_HAS_FILE_SYSTEM

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
                const Message msg = Message()
                    << "Invalid sharding: you have " << kTestShardIndex
                    << " = " << shard_index << ", but have left "
                    << kTestTotalShards << " unset.\n";
                hawk::stdout_printer().fmt_red("{}", msg.GetString());
                fflush(stdout);
                exit(EXIT_FAILURE);
            } else if (total_shards != -1 && shard_index == -1) {
                const Message msg = Message()
                    << "Invalid sharding: you have " << kTestTotalShards
                    << " = " << total_shards << ", but have left "
                    << kTestShardIndex << " unset.\n";
                hawk::stdout_printer().fmt_red("{}", msg.GetString());
                fflush(stdout);
                exit(EXIT_FAILURE);
            } else if (shard_index < 0 || shard_index >= total_shards) {
                const Message msg = Message() << "Invalid sharding: we require 0 <= " << kTestShardIndex
                                              << " < " << kTestTotalShards << ", but you have "
                                              << kTestShardIndex << "=" << shard_index << ", "
                                              << kTestTotalShards << "=" << total_shards << ".\n";
                hawk::stdout_printer().fmt_red("{}", msg.GetString());
                fflush(stdout);
                exit(EXIT_FAILURE);
            }

            return total_shards > 1;
        }

        // Parses the environment variable var as an Int32. If it is unset,
        // returns default_val. If it is not an Int32, prints an error
        // and aborts.
        int32_t Int32FromEnvOrDie(const char* var, int32_t default_val) {
            const char* str_val = posix::GetEnv(var);
            if (str_val == nullptr) {
                return default_val;
            }

            int32_t result;
            if (!ParseInt32(Message() << "The value of environment variable " << var,
                    str_val, &result)) {
                exit(EXIT_FAILURE);
            }
            return result;
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
            const char* value = internal::posix::GetEnv(variable_name);
            if (value != nullptr && value[0] != '\0') {
                if (value[strlen(value) - 1] != separator) {
                    return std::string(value).append(1, separator);
                }
                return value;
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

} // namespace testing
