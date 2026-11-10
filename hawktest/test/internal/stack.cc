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
#include <hawktest/test/internal/stack.h>
#include <hawktest/test/internal/unit_test_impl.h>

namespace hawk {
    namespace internal {

        // Returns the current OS stack trace as an std::string.
        //
        // The maximum number of stack frames to be included is specified by
        // the htest_stack_trace_depth flag.  The skip_count parameter
        // specifies the number of top frames to be skipped, which doesn't
        // count against the number of frames to be included.
        //
        // For example, if Foo() calls Bar(), which in turn calls
        // GetCurrentOsStackTraceExceptTop(..., 1), Foo() will be included in
        // the trace but Bar() and GetCurrentOsStackTraceExceptTop() won't.
        HTEST_NO_INLINE_ HTEST_NO_TAIL_CALL_ std::string
        GetCurrentOsStackTraceExceptTop(int skip_count) {
            // We pass skip_count + 1 to skip this wrapper function in addition
            // to what the user really wants to skip.
            return GetUnitTestImpl()->CurrentOsStackTraceExceptTop(skip_count + 1);
        }

        std::string OsStackTraceGetter::CurrentStackTrace(int max_depth, int skip_count)
          HTEST_LOCK_EXCLUDED_(mutex_) {

            static_cast<void>(max_depth);
            static_cast<void>(skip_count);
            return "";
        }

        void OsStackTraceGetter::UponLeavingHTest() HTEST_LOCK_EXCLUDED_(mutex_) {
        }


        const char* const OsStackTraceGetterInterface::kElidedFramesMarker = "... " HTEST_NAME_ " internal frames ...";


    }
}