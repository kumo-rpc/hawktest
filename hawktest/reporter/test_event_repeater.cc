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

#include <hawktest/reporter/test_event_repeater.h>
#include <hawktest/common/internal-inl.h>

namespace testing {
    namespace internal {


        TestEventRepeater::~TestEventRepeater() {
            ForEach(listeners_, Delete<TestEventListener>);
        }

        void TestEventRepeater::Append(TestEventListener* listener) {
            listeners_.push_back(listener);
        }


        TestEventListener* TestEventRepeater::Release(TestEventListener* listener) {
            for (size_t i = 0; i < listeners_.size(); ++i) {
                if (listeners_[i] == listener) {
                    listeners_.erase(listeners_.begin() + static_cast<int>(i));
                    return listener;
                }
            }

            return nullptr;
        }


        void TestEventRepeater::OnTestIterationStart(const UnitTest& unit_test,
            int iteration) {
            if (forwarding_enabled_) {
                for (size_t i = 0; i < listeners_.size(); i++) {
                    listeners_[i]->OnTestIterationStart(unit_test, iteration);
                }
            }
        }

        void TestEventRepeater::OnTestIterationEnd(const UnitTest& unit_test,
            int iteration) {
            if (forwarding_enabled_) {
                for (size_t i = listeners_.size(); i > 0; i--) {
                    listeners_[i - 1]->OnTestIterationEnd(unit_test, iteration);
                }
            }
        }


// Since most methods are very similar, use macros to reduce boilerplate.
// This defines a member that forwards the call to all listeners.
#define HTEST_REPEATER_METHOD_(Name, Type)                   \
    void TestEventRepeater::Name(const Type& parameter) {    \
        if (forwarding_enabled_) {                           \
            for (size_t i = 0; i < listeners_.size(); i++) { \
                listeners_[i]->Name(parameter);              \
            }                                                \
        }                                                    \
    }
// This defines a member that forwards the call to all listeners in reverse
// order.
#define HTEST_REVERSE_REPEATER_METHOD_(Name, Type)            \
    void TestEventRepeater::Name(const Type& parameter) {     \
        if (forwarding_enabled_) {                            \
            for (size_t i = listeners_.size(); i != 0; i--) { \
                listeners_[i - 1]->Name(parameter);           \
            }                                                 \
        }                                                     \
    }

        HTEST_REPEATER_METHOD_(OnTestProgramStart, UnitTest)
        HTEST_REPEATER_METHOD_(OnEnvironmentsSetUpStart, UnitTest)
//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        HTEST_REPEATER_METHOD_(OnTestCaseStart, TestSuite)
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        HTEST_REPEATER_METHOD_(OnTestSuiteStart, TestSuite)
        HTEST_REPEATER_METHOD_(OnTestStart, TestInfo)
        HTEST_REPEATER_METHOD_(OnTestDisabled, TestInfo)
        HTEST_REPEATER_METHOD_(OnTestPartResult, TestPartResult)
        HTEST_REPEATER_METHOD_(OnEnvironmentsTearDownStart, UnitTest)
        HTEST_REVERSE_REPEATER_METHOD_(OnEnvironmentsSetUpEnd, UnitTest)
        HTEST_REVERSE_REPEATER_METHOD_(OnEnvironmentsTearDownEnd, UnitTest)
        HTEST_REVERSE_REPEATER_METHOD_(OnTestEnd, TestInfo)
//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        HTEST_REVERSE_REPEATER_METHOD_(OnTestCaseEnd, TestSuite)
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_
        HTEST_REVERSE_REPEATER_METHOD_(OnTestSuiteEnd, TestSuite)
        HTEST_REVERSE_REPEATER_METHOD_(OnTestProgramEnd, UnitTest)

#undef HTEST_REPEATER_METHOD_
#undef HTEST_REVERSE_REPEATER_METHOD_



    }
}