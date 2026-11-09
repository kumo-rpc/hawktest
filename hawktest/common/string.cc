// Copyright 2005, Google Inc.
// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Portions of this software are derived from GoogleTest and Google Mock,
// originally distributed under the BSD 3-Clause License.
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

#include <hawktest/common/string.h>
#include <hawktest/common/port.h>
#include <hawktest/common/internal.h>
#include <iomanip>
#include <iostream>

namespace testing {
    namespace internal {

        // Compares two C strings.  Returns true if and only if they have the same
        // content.
        //
        // Unlike strcmp(), this function can handle NULL argument(s).  A NULL
        // C string is considered different to any non-NULL C string,
        // including the empty string.
        bool String::CStringEquals(const char* lhs, const char* rhs) {
            if (lhs == nullptr)
                return rhs == nullptr;

            if (rhs == nullptr)
                return false;

            return strcmp(lhs, rhs) == 0;
        }


#ifdef HTEST_OS_WINDOWS_MOBILE
        // Creates a UTF-16 wide string from the given ANSI string, allocating
        // memory using new. The caller is responsible for deleting the return
        // value using delete[]. Returns the wide string, or NULL if the
        // input is NULL.
        LPCWSTR String::AnsiToUtf16(const char* ansi) {
            if (!ansi)
                return nullptr;
            const int length = strlen(ansi);
            const int unicode_length = MultiByteToWideChar(CP_ACP, 0, ansi, length, nullptr, 0);
            WCHAR* unicode = new WCHAR[unicode_length + 1];
            MultiByteToWideChar(CP_ACP, 0, ansi, length, unicode, unicode_length);
            unicode[unicode_length] = 0;
            return unicode;
        }

        // Creates an ANSI string from the given wide string, allocating
        // memory using new. The caller is responsible for deleting the return
        // value using delete[]. Returns the ANSI string, or NULL if the
        // input is NULL.
        const char* String::Utf16ToAnsi(LPCWSTR utf16_str) {
            if (!utf16_str)
                return nullptr;
            const int ansi_length = WideCharToMultiByte(CP_ACP, 0, utf16_str, -1, nullptr,
                0, nullptr, nullptr);
            char* ansi = new char[ansi_length + 1];
            WideCharToMultiByte(CP_ACP, 0, utf16_str, -1, ansi, ansi_length, nullptr,
                nullptr);
            ansi[ansi_length] = 0;
            return ansi;
        }

#endif // HTEST_OS_WINDOWS_MOBILE


        // Compares two C strings, ignoring case.  Returns true if and only if they have
        // the same content.
        //
        // Unlike strcasecmp(), this function can handle NULL argument(s).  A
        // NULL C string is considered different to any non-NULL C string,
        // including the empty string.
        bool String::CaseInsensitiveCStringEquals(const char* lhs, const char* rhs) {
            if (lhs == nullptr)
                return rhs == nullptr;
            if (rhs == nullptr)
                return false;
            return posix::StrCaseCmp(lhs, rhs) == 0;
        }


        // A Unicode code-point can have up to 21 bits, and is encoded in UTF-8
        // like this:
        //
        // Code-point length   Encoding
        //   0 -  7 bits       0xxxxxxx
        //   8 - 11 bits       110xxxxx 10xxxxxx
        //  12 - 16 bits       1110xxxx 10xxxxxx 10xxxxxx
        //  17 - 21 bits       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

        // The maximum code-point a one-byte UTF-8 sequence can represent.
        constexpr uint32_t kMaxCodePoint1 = (static_cast<uint32_t>(1) << 7) - 1;

        // The maximum code-point a two-byte UTF-8 sequence can represent.
        constexpr uint32_t kMaxCodePoint2 = (static_cast<uint32_t>(1) << (5 + 6)) - 1;

        // The maximum code-point a three-byte UTF-8 sequence can represent.
        constexpr uint32_t kMaxCodePoint3 = (static_cast<uint32_t>(1) << (4 + 2 * 6)) - 1;

        // The maximum code-point a four-byte UTF-8 sequence can represent.
        constexpr uint32_t kMaxCodePoint4 = (static_cast<uint32_t>(1) << (3 + 3 * 6)) - 1;



        // Chops off the n lowest bits from a bit pattern.  Returns the n
        // lowest bits.  As a side effect, the original bit pattern will be
        // shifted to the right by n bits.
        inline uint32_t ChopLowBits(uint32_t* bits, int n) {
            const uint32_t low_bits = *bits & ((static_cast<uint32_t>(1) << n) - 1);
            *bits >>= n;
            return low_bits;
        }


        // Converts a Unicode code point to a narrow string in UTF-8 encoding.
        // code_point parameter is of type uint32_t because wchar_t may not be
        // wide enough to contain a code point.
        // If the code_point is not a valid Unicode code point
        // (i.e. outside of Unicode range U+0 to U+10FFFF) it will be converted
        // to "(Invalid Unicode 0xXXXXXXXX)".
        std::string CodePointToUtf8(uint32_t code_point) {
            if (code_point > kMaxCodePoint4) {
                return "(Invalid Unicode 0x" + String::FormatHexUInt32(code_point) + ")";
            }

            char str[5]; // Big enough for the largest valid code point.
            if (code_point <= kMaxCodePoint1) {
                str[1] = '\0';
                str[0] = static_cast<char>(code_point); // 0xxxxxxx
            } else if (code_point <= kMaxCodePoint2) {
                str[2] = '\0';
                str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6)); // 10xxxxxx
                str[0] = static_cast<char>(0xC0 | code_point); // 110xxxxx
            } else if (code_point <= kMaxCodePoint3) {
                str[3] = '\0';
                str[2] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6)); // 10xxxxxx
                str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6)); // 10xxxxxx
                str[0] = static_cast<char>(0xE0 | code_point); // 1110xxxx
            } else { // code_point <= kMaxCodePoint4
                str[4] = '\0';
                str[3] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6)); // 10xxxxxx
                str[2] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6)); // 10xxxxxx
                str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6)); // 10xxxxxx
                str[0] = static_cast<char>(0xF0 | code_point); // 11110xxx
            }
            return str;
        }



        // The following two functions only make sense if the system
        // uses UTF-16 for wide string encoding. All supported systems
        // with 16 bit wchar_t (Windows, Cygwin) do use UTF-16.

        // Determines if the arguments constitute UTF-16 surrogate pair
        // and thus should be combined into a single Unicode code point
        // using CreateCodePointFromUtf16SurrogatePair.
        inline bool IsUtf16SurrogatePair(wchar_t first, wchar_t second) {
            return sizeof(wchar_t) == 2 && (first & 0xFC00) == 0xD800 && (second & 0xFC00) == 0xDC00;
        }



        // Creates a Unicode code point from UTF16 surrogate pair.
        inline uint32_t CreateCodePointFromUtf16SurrogatePair(wchar_t first,
            wchar_t second) {
            const auto first_u = static_cast<uint32_t>(first);
            const auto second_u = static_cast<uint32_t>(second);
            const uint32_t mask = (1 << 10) - 1;
            return (sizeof(wchar_t) == 2)
                ? (((first_u & mask) << 10) | (second_u & mask)) + 0x10000
                :
                // This function should not be called when the condition is
                // false, but we provide a sensible default in case it is.
                first_u;
        }


        // Converts a wide string to a narrow string in UTF-8 encoding.
        // The wide string is assumed to have the following encoding:
        //   UTF-16 if sizeof(wchar_t) == 2 (on Windows, Cygwin)
        //   UTF-32 if sizeof(wchar_t) == 4 (on Linux)
        // Parameter str points to a null-terminated wide string.
        // Parameter num_chars may additionally limit the number
        // of wchar_t characters processed. -1 is used when the entire string
        // should be processed.
        // If the string contains code points that are not valid Unicode code points
        // (i.e. outside of Unicode range U+0 to U+10FFFF) they will be output
        // as '(Invalid Unicode 0xXXXXXXXX)'. If the string is in UTF16 encoding
        // and contains invalid UTF-16 surrogate pairs, values in those pairs
        // will be encoded as individual Unicode characters from Basic Normal Plane.
        std::string WideStringToUtf8(const wchar_t* str, int num_chars) {
            if (num_chars == -1)
                num_chars = static_cast<int>(wcslen(str));

            ::std::stringstream stream;
            for (int i = 0; i < num_chars; ++i) {
                uint32_t unicode_code_point;

                if (str[i] == L'\0') {
                    break;
                } else if (i + 1 < num_chars && IsUtf16SurrogatePair(str[i], str[i + 1])) {
                    unicode_code_point = CreateCodePointFromUtf16SurrogatePair(str[i], str[i + 1]);
                    i++;
                } else {
                    unicode_code_point = static_cast<uint32_t>(str[i]);
                }

                stream << CodePointToUtf8(unicode_code_point);
            }
            return StringStreamToString(&stream);
        }

        // Converts a wide C string to an std::string using the UTF-8 encoding.
        // NULL will be converted to "(null)".
        std::string String::ShowWideCString(const wchar_t* wide_c_str) {
            if (wide_c_str == nullptr)
                return "(null)";

            return internal::WideStringToUtf8(wide_c_str, -1);
        }



        // Compares two wide C strings, ignoring case.  Returns true if and only if they
        // have the same content.
        //
        // Unlike wcscasecmp(), this function can handle NULL argument(s).
        // A NULL C string is considered different to any non-NULL wide C string,
        // including the empty string.
        // NB: The implementations on different platforms slightly differ.
        // On windows, this method uses _wcsicmp which compares according to LC_CTYPE
        // environment variable. On GNU platform this method uses wcscasecmp
        // which compares according to LC_CTYPE category of the current locale.
        // On MacOS X, it uses towlower, which also uses LC_CTYPE category of the
        // current locale.
        bool String::CaseInsensitiveWideCStringEquals(const wchar_t* lhs,
            const wchar_t* rhs) {
            if (lhs == nullptr)
                return rhs == nullptr;

            if (rhs == nullptr)
                return false;

#ifdef HTEST_OS_WINDOWS
            return _wcsicmp(lhs, rhs) == 0;
#elif defined(HTEST_OS_LINUX) && !defined(HTEST_OS_LINUX_ANDROID)
            return wcscasecmp(lhs, rhs) == 0;
#else
            // Android, Mac OS X and Cygwin don't define wcscasecmp.
            // Other unknown OSes may not define it either.
            wint_t left, right;
            do {
                left = towlower(static_cast<wint_t>(*lhs++));
                right = towlower(static_cast<wint_t>(*rhs++));
            } while (left && left == right);
            return left == right;
#endif // OS selector
        }


        // Returns true if and only if str ends with the given suffix, ignoring case.
        // Any string is considered to end with an empty suffix.
        bool String::EndsWithCaseInsensitive(const std::string& str,
            const std::string& suffix) {
            const size_t str_len = str.length();
            const size_t suffix_len = suffix.length();
            return (str_len >= suffix_len) && CaseInsensitiveCStringEquals(str.c_str() + str_len - suffix_len, suffix.c_str());
        }

        // Formats an int value as "%02d".
        std::string String::FormatIntWidth2(int value) {
            return FormatIntWidthN(value, 2);
        }

        // Formats an int value to given width with leading zeros.
        std::string String::FormatIntWidthN(int value, int width) {
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(width) << value;
            return ss.str();
        }

        // Formats an int value as "%X".
        std::string String::FormatHexUInt32(uint32_t value) {
            std::stringstream ss;
            ss << std::hex << std::uppercase << value;
            return ss.str();
        }

        // Formats an int value as "%X".
        std::string String::FormatHexInt(int value) {
            return FormatHexUInt32(static_cast<uint32_t>(value));
        }

        // Formats a byte as "%02X".
        std::string String::FormatByte(unsigned char value) {
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
               << static_cast<unsigned int>(value);
            return ss.str();
        }


        // Compares two wide C strings.  Returns true if and only if they have the
        // same content.
        //
        // Unlike wcscmp(), this function can handle NULL argument(s).  A NULL
        // C string is considered different to any non-NULL C string,
        // including the empty string.
        bool String::WideCStringEquals(const wchar_t* lhs, const wchar_t* rhs) {
            if (lhs == nullptr)
                return rhs == nullptr;

            if (rhs == nullptr)
                return false;

            return wcscmp(lhs, rhs) == 0;
        }

        void SplitString(const ::std::string& str, char delimiter,
         ::std::vector<::std::string>* dest) {
            ::std::vector<::std::string> parsed;
            ::std::string::size_type pos = 0;
            while (::testing::internal::AlwaysTrue()) {
                const ::std::string::size_type colon = str.find(delimiter, pos);
                if (colon == ::std::string::npos) {
                    parsed.push_back(str.substr(pos));
                    break;
                } else {
                    parsed.push_back(str.substr(pos, colon - pos));
                    pos = colon + 1;
                }
            }
            dest->swap(parsed);
        }

                // Returns true if and only if the wildcard pattern matches the string. Each
        // pattern consists of regular characters, single-character wildcards (?), and
        // multi-character wildcards (*).
        //
        // This function implements a linear-time string globbing algorithm based on
        // https://research.swtch.com/glob.
        bool PatternMatchesString(const std::string& name_str,
            const char* pattern, const char* pattern_end) {
            const char* name = name_str.c_str();
            const char* const name_begin = name;
            const char* const name_end = name + name_str.size();

            const char* pattern_next = pattern;
            const char* name_next = name;

            while (pattern < pattern_end || name < name_end) {
                if (pattern < pattern_end) {
                    switch (*pattern) {
                    default: // Match an ordinary character.
                        if (name < name_end && *name == *pattern) {
                            ++pattern;
                            ++name;
                            continue;
                        }
                        break;
                    case '?': // Match any single character.
                        if (name < name_end) {
                            ++pattern;
                            ++name;
                            continue;
                        }
                        break;
                    case '*':
                        // Match zero or more characters. Start by skipping over the wildcard
                        // and matching zero characters from name. If that fails, restart and
                        // match one more character than the last attempt.
                        pattern_next = pattern;
                        name_next = name + 1;
                        ++pattern;
                        continue;
                    }
                }
                // Failed to match a character. Restart if possible.
                if (name_begin < name_next && name_next <= name_end) {
                    pattern = pattern_next;
                    name = name_next;
                    continue;
                }
                return false;
            }
            return true;
        }

        // Converts the buffer in a stringstream to an std::string, converting NUL
        // bytes to "\\0" along the way.
        std::string StringStreamToString(::std::stringstream* ss) {
            const ::std::string& str = ss->str();
            const char* const start = str.c_str();
            const char* const end = start + str.length();

            std::string result;
            result.reserve(static_cast<size_t>(2 * (end - start)));
            for (const char* ch = start; ch != end; ++ch) {
                if (*ch == '\0') {
                    result += "\\0"; // Replaces NUL with "\\0";
                } else {
                    result += *ch;
                }
            }

            return result;
        }

    }
}