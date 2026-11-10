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
#include <hawktest/test/internal/unit_test_impl.h>

namespace hawk {

#if HTEST_HAS_EXCEPTIONS
    namespace internal {

        HTEST_DISABLE_MSC_WARNINGS_PUSH_(
            4275 /* an exported class was derived from a class that was not exported */)

        // This exception is thrown by (and only by) a failed Hawk Test
        // assertion when FLAGS_htest_throw_on_failure is true (if exceptions
        // are enabled).  We derive it from std::runtime_error, which is for
        // errors presumably detectable only at run time.  Since
        // std::runtime_error inherits from std::exception, many testing
        // frameworks know how to extract and print the message inside it.
        class HTEST_API_ [[nodiscard]] HawkTestFailureException
            : public ::std::runtime_error {
            public:
            explicit HawkTestFailureException(const TestPartResult& failure);
            };

        HTEST_DISABLE_MSC_WARNINGS_POP_() //  4275

    }


    // Exception which can be thrown from TestEventListener::OnTestPartResult.
    class HTEST_API_ [[nodiscard]] AssertionException
        : public internal::HawkTestFailureException {
        public:
        explicit AssertionException(const TestPartResult& result)
            : HawkTestFailureException(result) { }
        };

    namespace internal {


        // Adds an "exception thrown" fatal failure to the current test.
        static std::string FormatCxxExceptionMessage(const char* description,
            const char* location) {
            hawk::Message message;
            if (description != nullptr) {
                message << "C++ exception with description \"" << description << "\"";
            } else {
                message << "Unknown C++ exception";
            }
            message << " thrown in " << location << ".";

            return message.GetString();
        }

        // We put these helper functions in the internal namespace as IBM's xlC
        // compiler rejects the code if they were declared static.

        // Runs the given method and handles SEH exceptions it throws, when
        // SEH is supported; returns the 0-value for type Result in case of an
        // SEH exception.  (Microsoft compilers cannot handle SEH and C++
        // exceptions in the same function.  Therefore, we provide a separate
        // wrapper function for handling SEH exceptions.)
        template <class T, typename Result>
        Result HandleSehExceptionsInMethodIfSupported(T* object, Result (T::*method)(),
            const char* location) {
#if HTEST_HAS_SEH
            __try {
                return (object->*method)();
            } __except (internal::UnitTestOptions::HTestProcessSEH( // NOLINT
                GetExceptionCode(), location)) {
                    return static_cast<Result>(0);
                }
#else
            (void)location;
            return (object->*method)();
#endif // HTEST_HAS_SEH
        }
        // Runs the given method and catches and reports C++ and/or SEH-style
        // exceptions, if they are supported; returns the default-value for type
        // Result in case of an SEH exception.
        template <class T, typename Result>
        Result HandleExceptionsInMethodIfSupported(T* object, Result (T::*method)(),
            const char* location) {
            // NOTE: The user code can affect the way in which Hawk Test handles
            // exceptions by setting FLAGS_htest_catch_exceptions, but only before
            // RUN_ALL_TESTS() starts. It is technically possible to check the flag
            // after the exception is caught and either report or re-throw the
            // exception based on the flag's value:
            //
            // try {
            //   // Perform the test method.
            // } catch (...) {
            //   if (FLAGS_htest_catch_exceptions)
            //     // Report the exception as failure.
            //   else
            //     throw;  // Re-throws the original exception.
            // }
            //
            // However, the purpose of this flag is to allow the program to drop into
            // the debugger when the exception is thrown. On most platforms, once the
            // control enters the catch block, the exception origin information is
            // lost and the debugger will stop the program at the point of the
            // re-throw in this function -- instead of at the point of the original
            // throw statement in the code under test.  For this reason, we perform
            // the check early, sacrificing the ability to affect Hawk Test's
            // exception handling in the method where the exception is thrown.
            if (internal::GetUnitTestImpl()->catch_exceptions()) {
#if HTEST_HAS_EXCEPTIONS
                try {
                    return HandleSehExceptionsInMethodIfSupported(object, method, location);
                } catch (const AssertionException&) { // NOLINT
                    // This failure was reported already.
                } catch (const internal::HawkTestFailureException&) { // NOLINT
                    // This exception type can only be thrown by a failed Google
                    // Test assertion with the intention of letting another testing
                    // framework catch it.  Therefore we just re-throw it.
                    throw;
                } catch (const std::exception& e) { // NOLINT
                    internal::ReportFailureInUnknownLocation(
                        TestPartResult::kFatalFailure,
                        FormatCxxExceptionMessage(e.what(), location));
                } catch (...) { // NOLINT
                    internal::ReportFailureInUnknownLocation(
                        TestPartResult::kFatalFailure,
                        FormatCxxExceptionMessage(nullptr, location));
                }
                return Result();
#else
                return HandleSehExceptionsInMethodIfSupported(object, method, location);
#endif // HTEST_HAS_EXCEPTIONS
            } else {
                return (object->*method)();
            }
        }
    }
#endif // HTEST_HAS_EXCEPTIONS

}  // namespace hawk
