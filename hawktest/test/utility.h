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

#pragma once

#include <hawktest/test/fwd.h>
#include <hawktest/test/assertion-result.h>
#include <hawktest/test/printers.h>
#include <hawktest/test/test-part.h>
#include <iomanip>
#include <iostream>

namespace testing {


    namespace internal {

        // Separate the error generating code from the code path to reduce the stack
        // frame size of CmpHelperEQ. This helps reduce the overhead of some sanitizers
        // when calling EXPECT_* in a tight loop.
        template <typename T1, typename T2>
        AssertionResult CmpHelperEQFailure(const char* lhs_expression,
            const char* rhs_expression, const T1& lhs,
            const T2& rhs) {
            return EqFailure(lhs_expression, rhs_expression,
                FormatForComparisonFailureMessage(lhs, rhs),
                FormatForComparisonFailureMessage(rhs, lhs), false);
        }

        // This block of code defines operator==/!=
        // to block lexical scope lookup.
        // It prevents using invalid operator==/!= defined at namespace scope.
        struct faketype { };
        inline bool operator==(faketype, faketype) {
            return true;
        }
        inline bool operator!=(faketype, faketype) {
            return false;
        }

        // The helper function for {ASSERT|EXPECT}_EQ.
        template <typename T1, typename T2>
        AssertionResult CmpHelperEQ(const char* lhs_expression,
            const char* rhs_expression, const T1& lhs,
            const T2& rhs) {
            if (lhs == rhs) {
                return AssertionSuccess();
            }

            return CmpHelperEQFailure(lhs_expression, rhs_expression, lhs, rhs);
        }

        class [[nodiscard]] EqHelper {
        public:
            // This templatized version is for the general case.
            template <
                typename T1, typename T2,
                // Disable this overload for cases where one argument is a pointer
                // and the other is the null pointer constant.
                typename std::enable_if<!std::is_integral<T1>::value || !std::is_pointer<T2>::value>::type* = nullptr>
            static AssertionResult Compare(const char* lhs_expression,
                const char* rhs_expression, const T1& lhs,
                const T2& rhs) {
                return CmpHelperEQ(lhs_expression, rhs_expression, lhs, rhs);
            }

            // With this overloaded version, we allow anonymous enums to be used
            // in {ASSERT|EXPECT}_EQ when compiled with gcc 4, as anonymous
            // enums can be implicitly cast to BiggestInt.
            //
            // Even though its body looks the same as the above version, we
            // cannot merge the two, as it will make anonymous enums unhappy.
            static AssertionResult Compare(const char* lhs_expression,
                const char* rhs_expression, BiggestInt lhs,
                BiggestInt rhs) {
                return CmpHelperEQ(lhs_expression, rhs_expression, lhs, rhs);
            }

            template <typename T>
            static AssertionResult Compare(
                const char* lhs_expression, const char* rhs_expression,
                // Handle cases where '0' is used as a null pointer literal.
                std::nullptr_t /* lhs */, T* rhs) {
                // We already know that 'lhs' is a null pointer.
                return CmpHelperEQ(lhs_expression, rhs_expression, static_cast<T*>(nullptr),
                    rhs);
            }
        };

        // Separate the error generating code from the code path to reduce the stack
        // frame size of CmpHelperOP. This helps reduce the overhead of some sanitizers
        // when calling EXPECT_OP in a tight loop.
        template <typename T1, typename T2>
        AssertionResult CmpHelperOpFailure(const char* expr1, const char* expr2,
            const T1& val1, const T2& val2,
            const char* op) {
            return AssertionFailure()
                << "Expected: (" << expr1 << ") " << op << " (" << expr2
                << "), actual: " << FormatForComparisonFailureMessage(val1, val2)
                << " vs " << FormatForComparisonFailureMessage(val2, val1);
        }

        // A macro for implementing the helper functions needed to implement
        // ASSERT_?? and EXPECT_??.  It is here just to avoid copy-and-paste
        // of similar code.
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.

#define HTEST_IMPL_CMP_HELPER_(op_name, op)                                  \
    template <typename T1, typename T2>                                      \
    AssertionResult CmpHelper##op_name(const char* expr1, const char* expr2, \
        const T1& val1, const T2& val2) {                                    \
        if (val1 op val2) {                                                  \
            return AssertionSuccess();                                       \
        } else {                                                             \
            return CmpHelperOpFailure(expr1, expr2, val1, val2, #op);        \
        }                                                                    \
    }

        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.

        // Implements the helper function for {ASSERT|EXPECT}_NE
        HTEST_IMPL_CMP_HELPER_(NE, !=)
        // Implements the helper function for {ASSERT|EXPECT}_LE
        HTEST_IMPL_CMP_HELPER_(LE, <=)
        // Implements the helper function for {ASSERT|EXPECT}_LT
        HTEST_IMPL_CMP_HELPER_(LT, <)
        // Implements the helper function for {ASSERT|EXPECT}_GE
        HTEST_IMPL_CMP_HELPER_(GE, >=)
        // Implements the helper function for {ASSERT|EXPECT}_GT
        HTEST_IMPL_CMP_HELPER_(GT, >)

#undef HTEST_IMPL_CMP_HELPER_

        // The helper function for {ASSERT|EXPECT}_STREQ.
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        HTEST_API_ AssertionResult CmpHelperSTREQ(const char* s1_expression,
            const char* s2_expression,
            const char* s1, const char* s2);

        // The helper function for {ASSERT|EXPECT}_STRCASEEQ.
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        HTEST_API_ AssertionResult CmpHelperSTRCASEEQ(const char* s1_expression,
            const char* s2_expression,
            const char* s1, const char* s2);

        // The helper function for {ASSERT|EXPECT}_STRNE.
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        HTEST_API_ AssertionResult CmpHelperSTRNE(const char* s1_expression,
            const char* s2_expression,
            const char* s1, const char* s2);

        // The helper function for {ASSERT|EXPECT}_STRCASENE.
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        HTEST_API_ AssertionResult CmpHelperSTRCASENE(const char* s1_expression,
            const char* s2_expression,
            const char* s1, const char* s2);

        // Helper function for *_STREQ on wide strings.
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        HTEST_API_ AssertionResult CmpHelperSTREQ(const char* s1_expression,
            const char* s2_expression,
            const wchar_t* s1, const wchar_t* s2);

        // Helper function for *_STRNE on wide strings.
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        HTEST_API_ AssertionResult CmpHelperSTRNE(const char* s1_expression,
            const char* s2_expression,
            const wchar_t* s1, const wchar_t* s2);

    } // namespace internal

    // IsSubstring() and IsNotSubstring() are intended to be used as the
    // first argument to {EXPECT,ASSERT}_PRED_FORMAT2(), not by
    // themselves.  They check whether needle is a substring of haystack
    // (NULL is considered a substring of itself only), and return an
    // appropriate error message when they fail.
    //
    // The {needle,haystack}_expr arguments are the stringified
    // expressions that generated the two real arguments.
    HTEST_API_ AssertionResult IsSubstring(const char* needle_expr,
        const char* haystack_expr,
        const char* needle,
        const char* haystack);
    HTEST_API_ AssertionResult IsSubstring(const char* needle_expr,
        const char* haystack_expr,
        const wchar_t* needle,
        const wchar_t* haystack);
    HTEST_API_ AssertionResult IsNotSubstring(const char* needle_expr,
        const char* haystack_expr,
        const char* needle,
        const char* haystack);
    HTEST_API_ AssertionResult IsNotSubstring(const char* needle_expr,
        const char* haystack_expr,
        const wchar_t* needle,
        const wchar_t* haystack);
    HTEST_API_ AssertionResult IsSubstring(const char* needle_expr,
        const char* haystack_expr,
        const ::std::string& needle,
        const ::std::string& haystack);
    HTEST_API_ AssertionResult IsNotSubstring(const char* needle_expr,
        const char* haystack_expr,
        const ::std::string& needle,
        const ::std::string& haystack);

#if HTEST_HAS_STD_WSTRING
    HTEST_API_ AssertionResult IsSubstring(const char* needle_expr,
        const char* haystack_expr,
        const ::std::wstring& needle,
        const ::std::wstring& haystack);
    HTEST_API_ AssertionResult IsNotSubstring(const char* needle_expr,
        const char* haystack_expr,
        const ::std::wstring& needle,
        const ::std::wstring& haystack);
#endif // HTEST_HAS_STD_WSTRING

    namespace internal {

        // Helper template function for comparing floating-points.
        //
        // Template parameter:
        //
        //   RawType: the raw floating-point type (either float or double)
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        template <typename RawType>
        AssertionResult CmpHelperFloatingPointEQ(const char* lhs_expression,
            const char* rhs_expression,
            RawType lhs_value, RawType rhs_value) {
            const FloatingPoint<RawType> lhs(lhs_value), rhs(rhs_value);

            if (lhs.AlmostEquals(rhs)) {
                return AssertionSuccess();
            }

            ::std::stringstream lhs_ss;
            lhs_ss.precision(std::numeric_limits<RawType>::digits10 + 2);
            lhs_ss << lhs_value;

            ::std::stringstream rhs_ss;
            rhs_ss.precision(std::numeric_limits<RawType>::digits10 + 2);
            rhs_ss << rhs_value;

            return EqFailure(lhs_expression, rhs_expression,
                StringStreamToString(&lhs_ss), StringStreamToString(&rhs_ss),
                false);
        }

        // Helper function for implementing ASSERT_NEAR.
        //
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        HTEST_API_ AssertionResult DoubleNearPredFormat(const char* expr1,
            const char* expr2,
            const char* abs_error_expr,
            double val1, double val2,
            double abs_error);

        using HawkTest_NotSupported_OnFunctionReturningNonVoid = void;

        // INTERNAL IMPLEMENTATION - DO NOT USE IN USER CODE.
        // A class that enables one to stream messages to assertion macros
        class HTEST_API_ [[nodiscard]] AssertHelper {
        public:
            // Constructor.
            AssertHelper(TestPartResult::Type type, const char* file, int line,
                const char* message);
            AssertHelper(TestPartResult::Type type, std::string_view file, int line,
                std::string_view message);
            ~AssertHelper();

            // Message assignment is a semantic trick to enable assertion
            // streaming; see the HTEST_MESSAGE_ macro below.
            HawkTest_NotSupported_OnFunctionReturningNonVoid operator=(
                const Message& message) const;

        private:
            // We put our data in a struct so that the size of the AssertHelper class can
            // be as small as possible.  This is important because gcc is incapable of
            // re-using stack space even for temporary variables, so every EXPECT_EQ
            // reserves stack space for another AssertHelper.
            struct AssertHelperData {
                AssertHelperData(TestPartResult::Type t, std::string_view srcfile,
                    int line_num, std::string_view msg)
                    : type(t)
                    , file(srcfile)
                    , line(line_num)
                    , message(msg) { }

                TestPartResult::Type const type;
                const std::string_view file;
                int const line;
                std::string const message;

            private:
                AssertHelperData(const AssertHelperData&) = delete;
                AssertHelperData& operator=(const AssertHelperData&) = delete;
            };

            AssertHelperData* const data_;

            AssertHelper(const AssertHelper&) = delete;
            AssertHelper& operator=(const AssertHelper&) = delete;
        };


        // Helper template for implementing FloatLE() and DoubleLE().
        template <typename RawType>
        AssertionResult FloatingPointLE(const char* expr1, const char* expr2,
            RawType val1, RawType val2) {
            // Returns success if val1 is less than val2,
            if (val1 < val2) {
                return AssertionSuccess();
            }

            // or if val1 is almost equal to val2.
            const FloatingPoint<RawType> lhs(val1), rhs(val2);
            if (lhs.AlmostEquals(rhs)) {
                return AssertionSuccess();
            }

            // Note that the above two checks will both fail if either val1 or
            // val2 is NaN, as the IEEE floating-point standard requires that
            // any predicate involving a NaN must return false.

            ::std::stringstream val1_ss;
            val1_ss << std::setprecision(std::numeric_limits<RawType>::digits10 + 2)
                    << val1;

            ::std::stringstream val2_ss;
            val2_ss << std::setprecision(std::numeric_limits<RawType>::digits10 + 2)
                    << val2;

            return AssertionFailure()
                << "Expected: (" << expr1 << ") <= (" << expr2 << ")\n"
                << "  Actual: " << StringStreamToString(&val1_ss) << " vs "
                << StringStreamToString(&val2_ss);
        }
    } // namespace internal
}  // namespace testing
