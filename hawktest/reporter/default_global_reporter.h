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

namespace testing {
    namespace internal {

        // This is the default global test part result reporter used in UnitTestImpl.
        // This class should only be used by UnitTestImpl.
        class DefaultGlobalTestPartResultReporter
            : public TestPartResultReporterInterface {
        public:
            explicit DefaultGlobalTestPartResultReporter(UnitTestImpl* unit_test);
            // Implements the TestPartResultReporterInterface. Reports the test part
            // result in the current test.
            void ReportTestPartResult(const TestPartResult& result) override;

        private:
            UnitTestImpl* const unit_test_;

            DefaultGlobalTestPartResultReporter(
                const DefaultGlobalTestPartResultReporter&) = delete;
            DefaultGlobalTestPartResultReporter& operator=(
                const DefaultGlobalTestPartResultReporter&) = delete;
        };


        // This is the default per thread test part result reporter used in
        // UnitTestImpl. This class should only be used by UnitTestImpl.
        class DefaultPerThreadTestPartResultReporter
            : public TestPartResultReporterInterface {
        public:
            explicit DefaultPerThreadTestPartResultReporter(UnitTestImpl* unit_test);
            // Implements the TestPartResultReporterInterface. The implementation just
            // delegates to the current global test part result reporter of *unit_test_.
            void ReportTestPartResult(const TestPartResult& result) override;

        private:
            UnitTestImpl* const unit_test_;

            DefaultPerThreadTestPartResultReporter(
                const DefaultPerThreadTestPartResultReporter&) = delete;
            DefaultPerThreadTestPartResultReporter& operator=(
                const DefaultPerThreadTestPartResultReporter&) = delete;
        };


    }
}