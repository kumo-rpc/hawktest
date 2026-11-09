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

#include <hawktest/common/port.h>
#include <hawktest/test/test-part.h>
#include <set>

namespace testing {

    // The upper limit for valid stack trace depths.
    const int kMaxStackTraceDepth = 100;

    namespace internal {

        class AssertHelper;
        class DefaultGlobalTestPartResultReporter;
        class ExecDeathTest;
        class NoExecDeathTest;
        class FinalSuccessChecker;
        class HTestFlagSaver;
        class StreamingListenerTest;
        class TestResultAccessor;
        class TestEventListenersAccessor;
        class TestEventRepeater;
        class UnitTestRecordPropertyTestHelper;
        class WindowsDeathTest;
        class FuchsiaDeathTest;
        class UnitTestImpl* GetUnitTestImpl();
        void ReportFailureInUnknownLocation(TestPartResult::Type result_type,
            const std::string& message);
        std::set<std::string>* GetIgnoredParameterizedTestSuites();

        // A base class that prevents subclasses from being copyable.
        // We do this instead of using '= delete' so as to avoid triggering warnings
        // inside user code regarding any of our declarations.
        class [[nodiscard]] HTestNonCopyable {
        public:
            HTestNonCopyable() = default;
            HTestNonCopyable(const HTestNonCopyable&) = delete;
            HTestNonCopyable& operator=(const HTestNonCopyable&) = delete;
            ~HTestNonCopyable() = default;
        };

    } // namespace internal


    // The friend relationship of some of these classes is cyclic.
    // If we don't forward declare them the compiler might confuse the classes
    // in friendship clauses with same named classes on the scope.
    class [[nodiscard]] Test;
    class [[nodiscard]] TestSuite;

    // Old API is still available but deprecated
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
    using TestCase = TestSuite;
#endif
    class [[nodiscard]] TestInfo;
    class [[nodiscard]] UnitTest;



    typedef internal::TimeInMillis TimeInMillis;

}  // namespace testing

