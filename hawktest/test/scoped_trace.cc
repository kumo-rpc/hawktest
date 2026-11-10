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

#include <hawktest/test/scoped_trace.h>
#include <hawktest/test/unit_test.h>

namespace hawk {


    // Pushes the given source file location and message onto a per-thread
    // trace stack maintained by Hawk Test.
    void ScopedTrace::PushTrace(const char* file, int line, std::string message) {
        internal::TraceInfo trace;
        trace.file = file;
        trace.line = line;
        trace.message.swap(message);

        UnitTest::GetInstance()->PushHTestTrace(trace);
    }

    // Pops the info pushed by the c'tor.
    ScopedTrace::~ScopedTrace() HTEST_LOCK_EXCLUDED_(&UnitTest::mutex_) {
        UnitTest::GetInstance()->PopHTestTrace();
    }
}  // namespace hawk
