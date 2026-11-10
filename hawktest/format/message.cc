// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
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

#include <hawktest/format/message.h>
#include <hawktest/common/string.h>
#include <iomanip>

namespace hawk {

    // Constructs an empty Message.
    // We allocate the stringstream separately because otherwise each use of
    // ASSERT/EXPECT in a procedure adds over 200 bytes to the procedure's
    // stack frame leading to huge stack frames in some cases; gcc does not reuse
    // the stack space.
    Message::Message()
        : ss_(new ::std::stringstream) {
        // By default, we want there to be enough precision when printing
        // a double to a Message.
        *ss_ << std::setprecision(std::numeric_limits<double>::digits10 + 2);
    }

    // These two overloads allow streaming a wide C string to a Message
    // using the UTF-8 encoding.
    Message& Message::operator<<(const wchar_t* wide_c_str) {
        return *this << testing::internal::String::ShowWideCString(wide_c_str);
    }
    Message& Message::operator<<(wchar_t* wide_c_str) {
        return *this << testing::internal::String::ShowWideCString(wide_c_str);
    }

    namespace internal {

#if HTEST_HAS_STD_WSTRING

        // Converts an array of wide chars to a narrow string using the UTF-8
        // encoding, and streams the result to the given Message object.
        static void StreamWideCharsToMessage(const wchar_t* wstr, size_t length,
            Message* msg) {
            for (size_t i = 0; i != length;) { // NOLINT
                if (wstr[i] != L'\0') {
                    *msg << testing::internal::WideStringToUtf8(wstr + i, static_cast<int>(length - i));
                    while (i != length && wstr[i] != L'\0')
                        i++;
                } else {
                    *msg << '\0';
                    i++;
                }
            }
        }

#endif // HTEST_HAS_STD_WSTRING

        // Appends the user-supplied message to the Google-Test-generated message.
        std::string AppendUserMessage(const std::string& htest_msg,
            const Message& user_msg) {
            // Appends the user message if it's non-empty.
            const std::string user_msg_string = user_msg.GetString();
            if (user_msg_string.empty()) {
                return htest_msg;
            }
            if (htest_msg.empty()) {
                return user_msg_string;
            }
            return htest_msg + "\n" + user_msg_string;
        }

    }
#if HTEST_HAS_STD_WSTRING
    // Converts the given wide string to a narrow string using the UTF-8
    // encoding, and streams the result to this Message object.
    Message& Message::operator<<(const ::std::wstring& wstr) {
        internal::StreamWideCharsToMessage(wstr.c_str(), wstr.length(), this);
        return *this;
    }
#endif // HTEST_HAS_STD_WSTRING

    // Gets the text streamed to this object so far as an std::string.
    // Each '\0' character in the buffer is replaced with "\\0".
    std::string Message::GetString() const {
        return testing::internal::StringStreamToString(ss_.get());
    }
}  // namespace hawk