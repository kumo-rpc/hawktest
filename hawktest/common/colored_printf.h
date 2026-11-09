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

namespace testing {
    namespace internal {
        enum class HTestColor { kDefault,
    kRed,
    kGreen,
    kYellow };

        HTEST_ATTRIBUTE_PRINTF_(2, 3)
        void ColoredPrintf(HTestColor color, const char* fmt, ...);

        // Returns true if and only if Hawk Test should use colors in the output.
        HTEST_API_ bool ShouldUseColor(bool stdout_is_tty);

    }
}