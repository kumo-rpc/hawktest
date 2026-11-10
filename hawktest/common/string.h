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

#pragma once


#ifdef __BORLANDC__
// string.h is not guaranteed to provide strcpy on C++ Builder.
#include <mem.h>
#endif

#include <string.h>

#include <cstdint>
#include <sstream>
#include <string>

#include <hawktest/common/port.h>

namespace hawk {
    namespace internal {

        // Converts a Unicode code point to a narrow string in UTF-8 encoding.
        // code_point parameter is of type UInt32 because wchar_t may not be
        // wide enough to contain a code point.
        // If the code_point is not a valid Unicode code point
        // (i.e. outside of Unicode range U+0 to U+10FFFF) it will be converted
        // to "(Invalid Unicode 0xXXXXXXXX)".
        HTEST_API_ std::string CodePointToUtf8(uint32_t code_point);


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
        HTEST_API_ std::string WideStringToUtf8(const wchar_t* str, int num_chars);

        // String - an abstract class holding static string utilities.
        class HTEST_API_ [[nodiscard]] String {
        public:
            // Static utility methods

#ifdef HTEST_OS_WINDOWS_MOBILE
            // Windows CE does not have the 'ANSI' versions of Win32 APIs. To be
            // able to pass strings to Win32 APIs on CE we need to convert them
            // to 'Unicode', UTF-16.

            // Creates a UTF-16 wide string from the given ANSI string, allocating
            // memory using new. The caller is responsible for deleting the return
            // value using delete[]. Returns the wide string, or NULL if the
            // input is NULL.
            //
            // The wide string is created using the ANSI codepage (CP_ACP) to
            // match the behaviour of the ANSI versions of Win32 calls and the
            // C runtime.
            static LPCWSTR AnsiToUtf16(const char* c_str);

            // Creates an ANSI string from the given wide string, allocating
            // memory using new. The caller is responsible for deleting the return
            // value using delete[]. Returns the ANSI string, or NULL if the
            // input is NULL.
            //
            // The returned string is created using the ANSI codepage (CP_ACP) to
            // match the behaviour of the ANSI versions of Win32 calls and the
            // C runtime.
            static const char* Utf16ToAnsi(LPCWSTR utf16_str);
#endif

            // Compares two C strings.  Returns true if and only if they have the same
            // content.
            //
            // Unlike strcmp(), this function can handle NULL argument(s).  A
            // NULL C string is considered different to any non-NULL C string,
            // including the empty string.
            static bool CStringEquals(const char* lhs, const char* rhs);

            // Converts a wide C string to a String using the UTF-8 encoding.
            // NULL will be converted to "(null)".  If an error occurred during
            // the conversion, "(failed to convert from wide string)" is
            // returned.
            static std::string ShowWideCString(const wchar_t* wide_c_str);

            // Compares two wide C strings.  Returns true if and only if they have the
            // same content.
            //
            // Unlike wcscmp(), this function can handle NULL argument(s).  A
            // NULL C string is considered different to any non-NULL C string,
            // including the empty string.
            static bool WideCStringEquals(const wchar_t* lhs, const wchar_t* rhs);

            // Compares two C strings, ignoring case.  Returns true if and only if
            // they have the same content.
            //
            // Unlike strcasecmp(), this function can handle NULL argument(s).
            // A NULL C string is considered different to any non-NULL C string,
            // including the empty string.
            static bool CaseInsensitiveCStringEquals(const char* lhs, const char* rhs);

            // Compares two wide C strings, ignoring case.  Returns true if and only if
            // they have the same content.
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
            static bool CaseInsensitiveWideCStringEquals(const wchar_t* lhs,
                const wchar_t* rhs);

            // Returns true if and only if the given string ends with the given suffix,
            // ignoring case. Any string is considered to end with an empty suffix.
            static bool EndsWithCaseInsensitive(const std::string& str,
                const std::string& suffix);

            // Formats an int value as "%02d".
            static std::string FormatIntWidth2(int value); // "%02d" for width == 2

            // Formats an int value to given width with leading zeros.
            static std::string FormatIntWidthN(int value, int width);

            // Formats an int value as "%X".
            static std::string FormatHexInt(int value);

            // Formats an int value as "%X".
            static std::string FormatHexUInt32(uint32_t value);

            // Formats a byte as "%02X".
            static std::string FormatByte(unsigned char value);

        private:
            String(); // Not meant to be instantiated.
        }; // class String

        // Gets the content of the stringstream's buffer as an std::string.  Each '\0'
        // character in the buffer is replaced with "\\0".
        HTEST_API_ std::string StringStreamToString(::std::stringstream* stream);

        // Splits a given string on a given delimiter, populating a given
        // vector with the fields.
        void SplitString(const ::std::string& str, char delimiter,
            ::std::vector<::std::string>* dest);

        bool PatternMatchesString(const std::string& name_str,
            const char* pattern, const char* pattern_end);

    } // namespace internal
} // namespace hawk

