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
        // Information about a Hawk Test trace point.
        struct TraceInfo {
            const char* file;
            int line;
            std::string message;
        };
    }
    // Causes a trace (including the given source file path and line number,
    // and the given message) to be included in every test failure message generated
    // by code in the scope of the lifetime of an instance of this class. The effect
    // is undone with the destruction of the instance.
    //
    // The message argument can be anything streamable to std::ostream.
    //
    // Example:
    //   testing::ScopedTrace trace("file.cc", 123, "message");
    //
    class HTEST_API_ [[nodiscard]] ScopedTrace {
        public:
        // The c'tor pushes the given source file location and message onto
        // a trace stack maintained by Hawk Test.

        // Template version. Uses Message() to convert the values into strings.
        // Slow, but flexible.
        template <typename T>
        ScopedTrace(const char* file, int line, const T& message) {
            PushTrace(file, line, (Message() << message).GetString());
        }

        // Optimize for some known types.
        ScopedTrace(const char* file, int line, const char* message) {
            PushTrace(file, line, message ? message : "(null)");
        }

        ScopedTrace(const char* file, int line, const std::string& message) {
            PushTrace(file, line, message);
        }

        // The d'tor pops the info pushed by the c'tor.
        //
        // Note that the d'tor is not virtual in order to be efficient.
        // Don't inherit from ScopedTrace!
        ~ScopedTrace();

        private:
        void PushTrace(const char* file, int line, std::string message);

        ScopedTrace(const ScopedTrace&) = delete;
        ScopedTrace& operator=(const ScopedTrace&) = delete;
    };
}  // namespace testing
