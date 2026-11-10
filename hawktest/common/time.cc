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

#include <hawktest/common/time.h>
#include <hawktest/common/string.h>
#include <hawktest/format/message.h>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace hawk {

    // Returns a timestamp as milliseconds since the epoch. Note this time may jump
    // around subject to adjustments by the system, to measure elapsed time use
    // Timer instead.
    TimeInMillis GetTimeInMillis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - std::chrono::system_clock::from_time_t(0))
            .count();
    }

    // Formats the given time in milliseconds as seconds.
    std::string FormatTimeInMillisAsSeconds(TimeInMillis ms) {
        ::std::stringstream ss;
        // For the exact N seconds, makes sure output has a trailing decimal point.
        // Sets precision so that we won't have many trailing zeros (e.g., 300 ms
        // will be just 0.3, 410 ms 0.41, and so on)
        ss << std::fixed
           << std::setprecision(
                  ms % 1000 == 0 ? 0 : (ms % 100 == 0 ? 1 : (ms % 10 == 0 ? 2 : 3)))
           << std::showpoint;
        ss << (static_cast<double>(ms) * 1e-3);
        return ss.str();
    }



    bool PortableLocaltime(time_t seconds, struct tm* out) {
#if defined(_MSC_VER)
        return localtime_s(out, &seconds) == 0;
#elif defined(__MINGW32__) || defined(__MINGW64__)
        // MINGW <time.h> provides neither localtime_r nor localtime_s, but uses
        // Windows' localtime(), which has a thread-local tm buffer.
        struct tm* tm_ptr = localtime(&seconds); // NOLINT
        if (tm_ptr == nullptr)
            return false;
        *out = *tm_ptr;
        return true;
#elif defined(__STDC_LIB_EXT1__)
        // Uses localtime_s when available as localtime_r is only available from
        // C23 standard.
        return localtime_s(&seconds, out) != nullptr;
#else
        return localtime_r(&seconds, out) != nullptr;
#endif
    }


    // Converts the given epoch time in milliseconds to a date string in the ISO
    // 8601 format, without the timezone information.
    std::string FormatEpochTimeInMillisAsIso8601(TimeInMillis ms) {
        struct tm time_struct;
        if (!PortableLocaltime(static_cast<time_t>(ms / 1000), &time_struct))
            return "";
        // YYYY-MM-DDThh:mm:ss.sss
        return hawk::internal::StreamableToString(time_struct.tm_year + 1900) + "-" + testing::internal::String::FormatIntWidth2(time_struct.tm_mon + 1) + "-" + testing::internal::String::FormatIntWidth2(time_struct.tm_mday) + "T" + testing::internal::String::FormatIntWidth2(time_struct.tm_hour) + ":" + testing::internal::String::FormatIntWidth2(time_struct.tm_min) + ":" + testing::internal::String::FormatIntWidth2(time_struct.tm_sec) + "." + testing::internal::String::FormatIntWidthN(static_cast<int>(ms % 1000), 3);
    }

    // Formats the given time in milliseconds as seconds.
    std::string FormatTimeInMillisAsDuration(TimeInMillis ms) {
        ::std::stringstream ss;
        ss << (static_cast<double>(ms) * 1e-3) << "s";
        return ss.str();
    }

    // Converts the given epoch time in milliseconds to a date string in the
    // RFC3339 format, without the timezone information.
    std::string FormatEpochTimeInMillisAsRFC3339(TimeInMillis ms) {
        struct tm time_struct;
        if (!hawk::PortableLocaltime(static_cast<time_t>(ms / 1000), &time_struct))
            return "";
        // YYYY-MM-DDThh:mm:ss
        return hawk::internal::StreamableToString(time_struct.tm_year + 1900) + "-" +
            testing::internal::String::FormatIntWidth2(time_struct.tm_mon + 1) + "-" +
            testing::internal::String::FormatIntWidth2(time_struct.tm_mday) + "T" +
                testing::internal::String::FormatIntWidth2(time_struct.tm_hour) + ":" +
                   testing::internal::String::FormatIntWidth2(time_struct.tm_min) + ":" +
                testing::internal::String::FormatIntWidth2(time_struct.tm_sec) + "Z";
    }
}