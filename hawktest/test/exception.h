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

#include <hawktest/test/fwd.h>

namespace testing {


#if HTEST_HAS_EXCEPTIONS

    // Exception which can be thrown from TestEventListener::OnTestPartResult.
    class HTEST_API_ [[nodiscard]] AssertionException
        : public internal::HawkTestFailureException {
        public:
        explicit AssertionException(const TestPartResult& result)
            : HawkTestFailureException(result) { }
        };

#endif // HTEST_HAS_EXCEPTIONS

}  // namespace testing
