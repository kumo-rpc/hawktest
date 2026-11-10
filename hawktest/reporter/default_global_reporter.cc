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

#include <hawktest/reporter/default_global_reporter.h>
#include <hawktest/test/unit_test.h>
#include <hawktest/test/internal/unit_test_impl.h>

namespace hawk {
    namespace internal {


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

    }
}