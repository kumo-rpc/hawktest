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

#include <cstdint>
#include <hawktest/common/port.h>

namespace hawk {

    // Integer types of known sizes.
    using TimeInMillis = int64_t; // Represents time in milliseconds.


    // Returns the current time in milliseconds.
    HTEST_API_ TimeInMillis GetTimeInMillis();


    // Formats the given time in milliseconds as seconds. If the input is an exact N
    // seconds, the output has a trailing decimal point (e.g., "N." instead of "N").
    HTEST_API_ std::string FormatTimeInMillisAsSeconds(TimeInMillis ms);

    // Converts the given time in milliseconds to a date string in the ISO 8601
    // format, without the timezone information.  N.B.: due to the use the
    // non-reentrant localtime() function, this function is not thread safe.  Do
    // not use it in any code that can be called from multiple threads.
    HTEST_API_ std::string FormatEpochTimeInMillisAsIso8601(TimeInMillis ms);

    bool PortableLocaltime(time_t seconds, struct tm* out);

    std::string FormatTimeInMillisAsDuration(TimeInMillis ms);
    std::string FormatEpochTimeInMillisAsRFC3339(TimeInMillis ms);

    // A helper class for measuring elapsed times.
    class Timer {
    public:
        Timer()
            : start_(clock::now()) { }

        // Return time elapsed in milliseconds since the timer was created.
        TimeInMillis Elapsed() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start_)
                .count();
        }

    private:
        // Fall back to the system_clock when building with newlib on a system
        // without a monotonic clock.
#if defined(_NEWLIB_VERSION) && !defined(CLOCK_MONOTONIC)
        using clock = std::chrono::system_clock;
#else
        using clock = std::chrono::steady_clock;
#endif
        clock::time_point start_;
    };

}  // namespace hawk
