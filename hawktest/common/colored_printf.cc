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

#include <hawktest/common/colored_printf.h>
#include <hawktest/common/string.h>
#include <hawktest/flags/builtin.h>
#include <cassert>

namespace testing {
    namespace internal {


        // Returns the ANSI color code for the given color. HTestColor::kDefault is
        // an invalid input.
        static const char* GetAnsiColorCode(HTestColor color) {
            switch (color) {
            case HTestColor::kRed:
                return "1";
            case HTestColor::kGreen:
                return "2";
            case HTestColor::kYellow:
                return "3";
            default:
                assert(false);
                return "9";
            }
        }


        // Returns true if and only if Hawk Test should use colors in the output.
        bool ShouldUseColor(bool stdout_is_tty) {
            std::string c = FLAGS_htest_color;
            const char* const htest_color = c.c_str();

            if (String::CaseInsensitiveCStringEquals(htest_color, "auto")) {
#if defined(HTEST_OS_WINDOWS) && !defined(HTEST_OS_WINDOWS_MINGW)
                // On Windows the TERM variable is usually not set, but the
                // console there does support colors.
                return stdout_is_tty;
#else
                // On non-Windows platforms, we rely on the TERM variable.
                const char* const term = posix::GetEnv("TERM");
                const bool term_supports_color = term != nullptr && (String::CStringEquals(term, "xterm") || String::CStringEquals(term, "xterm-color") || String::CStringEquals(term, "xterm-ghostty") || String::CStringEquals(term, "xterm-kitty") || String::CStringEquals(term, "alacritty") || String::CStringEquals(term, "screen") || String::CStringEquals(term, "tmux") || String::CStringEquals(term, "rxvt-unicode") || String::CStringEquals(term, "linux") || String::CStringEquals(term, "cygwin") || String::EndsWithCaseInsensitive(term, "-256color"));
                return stdout_is_tty && term_supports_color;
#endif // HTEST_OS_WINDOWS
            }

            return String::CaseInsensitiveCStringEquals(htest_color, "yes") || String::CaseInsensitiveCStringEquals(htest_color, "true") || String::CaseInsensitiveCStringEquals(htest_color, "t") || String::CStringEquals(htest_color, "1");
            // We take "yes", "true", "t", and "1" as meaning "yes".  If the
            // value is neither one of these nor "auto", we treat it as "no" to
            // be conservative.
        }

        // Helpers for printing colored strings to stdout. Note that on Windows, we
        // cannot simply emit special characters and have the terminal change colors.
        // This routine must actually emit the characters rather than return a string
        // that would be colored when printed, as can be done on Linux.

        HTEST_ATTRIBUTE_PRINTF_(2, 3)
        void ColoredPrintf(HTestColor color, const char* fmt, ...) {
            va_list args;
            va_start(args, fmt);

            static const bool in_color_mode =
                // We don't condition this on HTEST_HAS_FILE_SYSTEM because we still need
                // to be able to detect terminal I/O regardless.
                ShouldUseColor(posix::IsATTY(posix::FileNo(stdout)) != 0);

            const bool use_color = in_color_mode && (color != HTestColor::kDefault);

            if (!use_color) {
                vprintf(fmt, args);
                va_end(args);
                return;
            }

#if defined(HTEST_OS_WINDOWS) && !defined(HTEST_OS_WINDOWS_MOBILE) && !defined(HTEST_OS_WINDOWS_GAMES) && !defined(HTEST_OS_WINDOWS_PHONE) && !defined(HTEST_OS_WINDOWS_RT) && !defined(HTEST_OS_WINDOWS_MINGW)
            const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

            // Gets the current text color.
            CONSOLE_SCREEN_BUFFER_INFO buffer_info;
            GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
            const WORD old_color_attrs = buffer_info.wAttributes;
            const WORD new_color = GetNewColor(color, old_color_attrs);

            // We need to flush the stream buffers into the console before each
            // SetConsoleTextAttribute call lest it affect the text that is already
            // printed but has not yet reached the console.
            fflush(stdout);
            SetConsoleTextAttribute(stdout_handle, new_color);

            vprintf(fmt, args);

            fflush(stdout);
            // Restores the text color.
            SetConsoleTextAttribute(stdout_handle, old_color_attrs);
#else
            printf("\033[0;3%sm", GetAnsiColorCode(color));
            vprintf(fmt, args);
            printf("\033[m"); // Resets the terminal to default.
#endif // HTEST_OS_WINDOWS && !HTEST_OS_WINDOWS_MOBILE
            va_end(args);
        }

    }
}