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
        // All command line arguments.
        HTEST_API_ std::vector<std::string> GetArgvs();
        bool HTestIsInitialized();

        ::std::vector<std::string> GetArgvs();
    }

    // Initializes Hawk Test.  This must be called before calling
    // RUN_ALL_TESTS().  In particular, it parses a command line for the
    // flags that Hawk Test recognizes.  Whenever a Hawk Test flag is
    // seen, it is removed from argv, and *argc is decremented.
    //
    // No value is returned.  Instead, the Hawk Test flag variables are
    // updated.
    //
    // Calling the function for the second time has no user-visible effect.
    HTEST_API_ void init_hawk_test(int* argc, char** argv);

    // This overloaded version can be used in Windows programs compiled in
    // UNICODE mode.
    HTEST_API_ void init_hawk_test(int* argc, wchar_t** argv);

    // This overloaded version can be used on Arduino/embedded platforms where
    // there is no argc/argv.
    HTEST_API_ void init_hawk_test();

}  // namespace testing
