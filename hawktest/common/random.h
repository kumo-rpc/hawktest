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

namespace hawk {

    // A simple Linear Congruential Generator for generating random
    // numbers with a uniform distribution.  Unlike rand() and srand(), it
    // doesn't use global state (and therefore can't interfere with user
    // code).  Unlike rand_r(), it's portable.  An LCG isn't very random,
    // but it's good enough for our purposes.
    class HTEST_API_ [[nodiscard]] Random {
        public:
        static const uint32_t kMaxRange = 1u << 31;

        explicit Random(uint32_t seed)
            : state_(seed) { }

        void Reseed(uint32_t seed) { state_ = seed; }

        // Generates a random number from [0, range).  Crashes if 'range' is
        // 0 or greater than kMaxRange.
        uint32_t Generate(uint32_t range);

        private:
        uint32_t state_;
        Random(const Random&) = delete;
        Random& operator=(const Random&) = delete;
    };

} // namespace hawk
