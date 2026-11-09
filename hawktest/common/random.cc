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

#include <hawktest/common/random.h>

namespace hawk {

    const uint32_t Random::kMaxRange;

    // Generates a random number from [0, range), using a Linear
    // Congruential Generator (LCG).  Crashes if 'range' is 0 or greater
    // than kMaxRange.
    uint32_t Random::Generate(uint32_t range) {
        // These constants are the same as are used in glibc's rand(3).
        // Use wider types than necessary to prevent unsigned overflow diagnostics.
        state_ = static_cast<uint32_t>(1103515245ULL * state_ + 12345U) % kMaxRange;

        HTEST_CHECK_(range > 0) << "Cannot generate a number in the range [0, 0).";
        HTEST_CHECK_(range <= kMaxRange)
            << "Generation of a number in [0, " << range << ") was requested, "
            << "but this can only generate numbers in [0, " << kMaxRange << ").";

        // Converting via modulus introduces a bit of downward bias, but
        // it's simple, and a linear congruential generator isn't too good
        // to begin with.
        return state_ % range;
    }
} // namespace hawk
