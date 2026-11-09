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

#include <hawktest/test/utility.h>

namespace testing {

        namespace internal {


            // Helper function for *_STREQ on wide strings.
            AssertionResult CmpHelperSTREQ(const char* lhs_expression,
                const char* rhs_expression, const wchar_t* lhs,
                const wchar_t* rhs) {
                if (String::WideCStringEquals(lhs, rhs)) {
                    return AssertionSuccess();
                }

                return EqFailure(lhs_expression, rhs_expression, PrintToString(lhs),
                    PrintToString(rhs), false);
            }

            // Helper function for *_STRNE on wide strings.
            AssertionResult CmpHelperSTRNE(const char* s1_expression,
                const char* s2_expression, const wchar_t* s1,
                const wchar_t* s2) {
                if (!String::WideCStringEquals(s1, s2)) {
                    return AssertionSuccess();
                }

                return AssertionFailure()
                    << "Expected: (" << s1_expression << ") != (" << s2_expression
                    << "), actual: " << PrintToString(s1) << " vs " << PrintToString(s2);
            }

        // The helper function for {ASSERT|EXPECT}_STREQ.
        AssertionResult CmpHelperSTREQ(const char* lhs_expression,
            const char* rhs_expression, const char* lhs,
            const char* rhs) {
            if (String::CStringEquals(lhs, rhs)) {
                return AssertionSuccess();
            }

            return EqFailure(lhs_expression, rhs_expression, PrintToString(lhs),
                PrintToString(rhs), false);
        }

        // The helper function for {ASSERT|EXPECT}_STRCASEEQ.
        AssertionResult CmpHelperSTRCASEEQ(const char* lhs_expression,
            const char* rhs_expression, const char* lhs,
            const char* rhs) {
            if (String::CaseInsensitiveCStringEquals(lhs, rhs)) {
                return AssertionSuccess();
            }

            return EqFailure(lhs_expression, rhs_expression, PrintToString(lhs),
                PrintToString(rhs), true);
        }

        // The helper function for {ASSERT|EXPECT}_STRNE.
        AssertionResult CmpHelperSTRNE(const char* s1_expression,
            const char* s2_expression, const char* s1,
            const char* s2) {
            if (!String::CStringEquals(s1, s2)) {
                return AssertionSuccess();
            } else {
                return AssertionFailure()
                    << "Expected: (" << s1_expression << ") != (" << s2_expression
                    << "), actual: \"" << s1 << "\" vs \"" << s2 << "\"";
            }
        }

        // The helper function for {ASSERT|EXPECT}_STRCASENE.
        AssertionResult CmpHelperSTRCASENE(const char* s1_expression,
            const char* s2_expression, const char* s1,
            const char* s2) {
            if (!String::CaseInsensitiveCStringEquals(s1, s2)) {
                return AssertionSuccess();
            } else {
                return AssertionFailure()
                    << "Expected: (" << s1_expression << ") != (" << s2_expression
                    << ") (ignoring case), actual: \"" << s1 << "\" vs \"" << s2 << "\"";
            }
        }

    } // namespace internal


    namespace {

        // Helper functions for implementing IsSubString() and IsNotSubstring().

        // This group of overloaded functions return true if and only if needle
        // is a substring of haystack.  NULL is considered a substring of
        // itself only.

        bool IsSubstringPred(const char* needle, const char* haystack) {
            if (needle == nullptr || haystack == nullptr)
                return needle == haystack;

            return strstr(haystack, needle) != nullptr;
        }

        bool IsSubstringPred(const wchar_t* needle, const wchar_t* haystack) {
            if (needle == nullptr || haystack == nullptr)
                return needle == haystack;

            return wcsstr(haystack, needle) != nullptr;
        }

        // StringType here can be either ::std::string or ::std::wstring.
        template <typename StringType>
        bool IsSubstringPred(const StringType& needle, const StringType& haystack) {
            return haystack.find(needle) != StringType::npos;
        }

        // This function implements either IsSubstring() or IsNotSubstring(),
        // depending on the value of the expected_to_be_substring parameter.
        // StringType here can be const char*, const wchar_t*, ::std::string,
        // or ::std::wstring.
        template <typename StringType>
        AssertionResult IsSubstringImpl(bool expected_to_be_substring,
            const char* needle_expr,
            const char* haystack_expr,
            const StringType& needle,
            const StringType& haystack) {
            if (IsSubstringPred(needle, haystack) == expected_to_be_substring)
                return AssertionSuccess();

            const bool is_wide_string = sizeof(needle[0]) > 1;
            const char* const begin_string_quote = is_wide_string ? "L\"" : "\"";
            return AssertionFailure()
                << "Value of: " << needle_expr << "\n"
                << "  Actual: " << begin_string_quote << needle << "\"\n"
                << "Expected: " << (expected_to_be_substring ? "" : "not ")
                << "a substring of " << haystack_expr << "\n"
                << "Which is: " << begin_string_quote << haystack << "\"";
        }

    } // namespace

    // IsSubstring() and IsNotSubstring() check whether needle is a
    // substring of haystack (NULL is considered a substring of itself
    // only), and return an appropriate error message when they fail.

    AssertionResult IsSubstring(const char* needle_expr, const char* haystack_expr,
        const char* needle, const char* haystack) {
        return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
    }

    AssertionResult IsSubstring(const char* needle_expr, const char* haystack_expr,
        const wchar_t* needle, const wchar_t* haystack) {
        return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
    }

    AssertionResult IsNotSubstring(const char* needle_expr,
        const char* haystack_expr, const char* needle,
        const char* haystack) {
        return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
    }

    AssertionResult IsNotSubstring(const char* needle_expr,
        const char* haystack_expr, const wchar_t* needle,
        const wchar_t* haystack) {
        return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
    }

    AssertionResult IsSubstring(const char* needle_expr, const char* haystack_expr,
        const ::std::string& needle,
        const ::std::string& haystack) {
        return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
    }

    AssertionResult IsNotSubstring(const char* needle_expr,
        const char* haystack_expr,
        const ::std::string& needle,
        const ::std::string& haystack) {
        return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
    }

#if HTEST_HAS_STD_WSTRING
    AssertionResult IsSubstring(const char* needle_expr, const char* haystack_expr,
        const ::std::wstring& needle,
        const ::std::wstring& haystack) {
        return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
    }

    AssertionResult IsNotSubstring(const char* needle_expr,
        const char* haystack_expr,
        const ::std::wstring& needle,
        const ::std::wstring& haystack) {
        return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
    }
#endif // HTEST_HAS_STD_WSTRING

}  // namespace testing
