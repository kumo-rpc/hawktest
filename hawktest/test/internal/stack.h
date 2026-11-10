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

namespace hawk::internal {

    // The role interface for getting the OS stack trace as a string.
    class OsStackTraceGetterInterface {
    public:
        OsStackTraceGetterInterface() = default;
        virtual ~OsStackTraceGetterInterface() = default;

        // Returns the current OS stack trace as an std::string.  Parameters:
        //
        //   max_depth  - the maximum number of stack frames to be included
        //                in the trace.
        //   skip_count - the number of top frames to be skipped; doesn't count
        //                against max_depth.
        virtual std::string CurrentStackTrace(int max_depth, int skip_count) = 0;

        // UponLeavingHTest() should be called immediately before Hawk Test calls
        // user code. It saves some information about the current stack that
        // CurrentStackTrace() will use to find and hide Hawk Test stack frames.
        virtual void UponLeavingHTest() = 0;

        // This string is inserted in place of stack frames that are part of
        // Hawk Test's implementation.
        static const char* const kElidedFramesMarker;

    private:
        OsStackTraceGetterInterface(const OsStackTraceGetterInterface&) = delete;
        OsStackTraceGetterInterface& operator=(const OsStackTraceGetterInterface&) = delete;
    };

    // A working implementation of the OsStackTraceGetterInterface interface.
    class OsStackTraceGetter : public OsStackTraceGetterInterface {
    public:
        OsStackTraceGetter() = default;

        std::string CurrentStackTrace(int max_depth, int skip_count) override;
        void UponLeavingHTest() override;

    private:
        OsStackTraceGetter(const OsStackTraceGetter&) = delete;
        OsStackTraceGetter& operator=(const OsStackTraceGetter&) = delete;
    };


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
    HTEST_API_ std::string GetCurrentOsStackTraceExceptTop(int skip_count);

}  // namespace hawk::internal
