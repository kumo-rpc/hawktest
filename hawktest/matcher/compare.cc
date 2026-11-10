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
//

#include <hawktest/matcher/compare.h>
#include <hawktest/common/string.h>

namespace hawk {

    using hawk::PrintToString;

    // Helper function for *_STREQ on wide strings.
    AssertionResult CmpHelperSTREQ(const char* lhs_expression,
        const char* rhs_expression, const wchar_t* lhs,
        const wchar_t* rhs) {
        if (hawk::internal::String::WideCStringEquals(lhs, rhs)) {
            return AssertionSuccess();
        }

        return EqFailure(lhs_expression, rhs_expression, hawk::PrintToString(lhs),
            hawk::PrintToString(rhs), false);
    }

    // The helper function for {ASSERT|EXPECT}_STREQ.
    AssertionResult CmpHelperSTREQ(const char* lhs_expression,
        const char* rhs_expression, const char* lhs,
        const char* rhs) {
        if (hawk::internal::String::CStringEquals(lhs, rhs)) {
            return AssertionSuccess();
        }

        return EqFailure(lhs_expression, rhs_expression,
            hawk::PrintToString(lhs),
            hawk::PrintToString(rhs), false);
    }

    // Helper function for *_STRNE on wide strings.
    AssertionResult CmpHelperSTRNE(const char* s1_expression,
        const char* s2_expression, const wchar_t* s1,
        const wchar_t* s2) {
        if (!hawk::internal::String::WideCStringEquals(s1, s2)) {
            return AssertionSuccess();
        }

        return AssertionFailure()
            << "Expected: (" << s1_expression << ") != (" << s2_expression
            << "), actual: " << PrintToString(s1) << " vs " << PrintToString(s2);
    }


    // The helper function for {ASSERT|EXPECT}_STRCASEEQ.
    AssertionResult CmpHelperSTRCASEEQ(const char* lhs_expression,
        const char* rhs_expression, const char* lhs,
        const char* rhs) {
        if (hawk::internal::String::CaseInsensitiveCStringEquals(lhs, rhs)) {
            return AssertionSuccess();
        }

        return EqFailure(lhs_expression, rhs_expression, PrintToString(lhs),
            PrintToString(rhs), true);
    }

    // The helper function for {ASSERT|EXPECT}_STRNE.
    AssertionResult CmpHelperSTRNE(const char* s1_expression,
        const char* s2_expression, const char* s1,
        const char* s2) {
        if (!hawk::internal::String::CStringEquals(s1, s2)) {
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
        if (!hawk::internal::String::CaseInsensitiveCStringEquals(s1, s2)) {
            return AssertionSuccess();
        } else {
            return AssertionFailure()
                << "Expected: (" << s1_expression << ") != (" << s2_expression
                << ") (ignoring case), actual: \"" << s1 << "\" vs \"" << s2 << "\"";
        }
    }

           // Helper function for implementing ASSERT_NEAR. Treats infinity as a specific
            // value, such that comparing infinity to infinity is equal, the distance
            // between -infinity and +infinity is infinity, and infinity <= infinity is
            // true.
            AssertionResult DoubleNearPredFormat(const char* expr1, const char* expr2,
                const char* abs_error_expr, double val1,
                double val2, double abs_error) {
                // We want to return success when the two values are infinity and at least
                // one of the following is true:
                //  * The values are the same-signed infinity.
                //  * The error limit itself is infinity.
                // This is done here so that we don't end up with a NaN when calculating the
                // difference in values.
                if (std::isinf(val1) && std::isinf(val2) && (std::signbit(val1) == std::signbit(val2) || (abs_error > 0.0 && std::isinf(abs_error)))) {
                    return AssertionSuccess();
                }

                const double diff = fabs(val1 - val2);
                if (diff <= abs_error)
                    return AssertionSuccess();

                // Find the value which is closest to zero.
                const double min_abs = std::min(fabs(val1), fabs(val2));
                // Find the distance to the next double from that value.
                const double epsilon = nextafter(min_abs, std::numeric_limits<double>::infinity()) - min_abs;
                // Detect the case where abs_error is so small that EXPECT_NEAR is
                // effectively the same as EXPECT_EQUAL, and give an informative error
                // message so that the situation can be more easily understood without
                // requiring exotic floating-point knowledge.
                // Don't do an epsilon check if abs_error is zero because that implies
                // that an equality check was actually intended.
                if (!(std::isnan)(val1) && !(std::isnan)(val2) && abs_error > 0 && abs_error < epsilon) {
                    return AssertionFailure()
                        << "The difference between " << expr1 << " and " << expr2 << " is "
                        << diff << ", where\n"
                        << expr1 << " evaluates to " << val1 << ",\n"
                        << expr2 << " evaluates to " << val2 << ".\nThe abs_error parameter "
                        << abs_error_expr << " evaluates to " << abs_error
                        << " which is smaller than the minimum distance between doubles for "
                           "numbers of this magnitude which is "
                        << epsilon
                        << ", thus making this EXPECT_NEAR check equivalent to "
                           "EXPECT_EQUAL. Consider using EXPECT_DOUBLE_EQ instead.";
                }
                return AssertionFailure()
                    << "The difference between " << expr1 << " and " << expr2 << " is "
                    << diff << ", which exceeds " << abs_error_expr << ", where\n"
                    << expr1 << " evaluates to " << val1 << ",\n"
                    << expr2 << " evaluates to " << val2 << ", and\n"
                    << abs_error_expr << " evaluates to " << abs_error << ".";
            }


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


    // Asserts that val1 is less than, or almost equal to, val2.  Fails
    // otherwise.  In particular, it fails if either val1 or val2 is NaN.
    AssertionResult FloatLE(const char* expr1, const char* expr2, float val1,
        float val2) {
        return FloatingPointLE<float>(expr1, expr2, val1, val2);
    }

    // Asserts that val1 is less than, or almost equal to, val2.  Fails
    // otherwise.  In particular, it fails if either val1 or val2 is NaN.
    AssertionResult DoubleLE(const char* expr1, const char* expr2, double val1,
        double val2) {
        return FloatingPointLE<double>(expr1, expr2, val1, val2);
    }
}  // namespace hawk

