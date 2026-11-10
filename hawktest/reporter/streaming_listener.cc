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

#include <hawktest/reporter/streaming_listener.h>


#ifdef HTEST_OS_LINUX

#include <fcntl.h> // NOLINT
#include <limits.h> // NOLINT
#include <sched.h> // NOLINT
// Declares vsnprintf().  This header is not available on Windows.
#include <strings.h> // NOLINT
#include <sys/mman.h> // NOLINT
#include <sys/time.h> // NOLINT
#include <unistd.h> // NOLINT

#include <string>

#elif defined(HTEST_OS_ZOS)
#include <sys/time.h> // NOLINT

// On z/OS we additionally need strings.h for strcasecmp.
#include <strings.h> // NOLINT

#elif defined(HTEST_OS_WINDOWS_MOBILE) // We are on Windows CE.

#include <windows.h> // NOLINT
#undef min

#elif defined(HTEST_OS_WINDOWS) // We are on Windows proper.

#include <windows.h> // NOLINT
#undef min

#ifdef _MSC_VER
#include <crtdbg.h> // NOLINT
#endif

#include <io.h> // NOLINT
#include <sys/stat.h> // NOLINT
#include <sys/timeb.h> // NOLINT
#include <sys/types.h> // NOLINT

#ifdef HTEST_OS_WINDOWS_MINGW
#include <sys/time.h> // NOLINT
#endif // HTEST_OS_WINDOWS_MINGW

#else

// cpplint thinks that the header is already included, so we want to
// silence it.
#include <sys/time.h> // NOLINT
#include <unistd.h> // NOLINT

#endif // HTEST_OS_LINUX

#if HTEST_HAS_EXCEPTIONS
#include <stdexcept>
#endif

#if HTEST_CAN_STREAM_RESULTS_
#include <arpa/inet.h> // NOLINT
#include <netdb.h> // NOLINT
#include <sys/socket.h> // NOLINT
#include <sys/types.h> // NOLINT
#endif

#include <hawktest/common/string.h>

namespace hawk {
    namespace internal {
#if HTEST_CAN_STREAM_RESULTS_

        // Checks if str contains '=', '&', '%' or '\n' characters. If yes,
        // replaces them by "%xx" where xx is their hexadecimal value. For
        // example, replaces "=" with "%3D".  This algorithm is O(strlen(str))
        // in both time and space -- important as the input str may contain an
        // arbitrarily long test failure message and stack trace.
        std::string StreamingListener::UrlEncode(const char* str) {
            std::string result;
            result.reserve(strlen(str) + 1);
            for (char ch = *str; ch != '\0'; ch = *++str) {
                switch (ch) {
                case '%':
                case '=':
                case '&':
                case '\n':
                    result.push_back('%');
                    result.append(String::FormatByte(static_cast<unsigned char>(ch)));
                    break;
                default:
                    result.push_back(ch);
                    break;
                }
            }
            return result;
        }

        void StreamingListener::SocketWriter::MakeConnection() {
            HTEST_CHECK_(sockfd_ == -1)
                << "MakeConnection() can't be called when there is already a connection.";

            addrinfo hints;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC; // To allow both IPv4 and IPv6 addresses.
            hints.ai_socktype = SOCK_STREAM;
            addrinfo* servinfo = nullptr;

            // Use the getaddrinfo() to get a linked list of IP addresses for
            // the given host name.
            const int error_num = getaddrinfo(host_name_.c_str(), port_num_.c_str(), &hints, &servinfo);
            if (error_num != 0) {
                HTEST_LOG_(WARNING) << "stream_result_to: getaddrinfo() failed: "
                                    << gai_strerror(error_num);
            }

            // Loop through all the results and connect to the first we can.
            for (addrinfo* cur_addr = servinfo; sockfd_ == -1 && cur_addr != nullptr;
                cur_addr = cur_addr->ai_next) {
                sockfd_ = socket(cur_addr->ai_family, cur_addr->ai_socktype,
                    cur_addr->ai_protocol);
                if (sockfd_ != -1) {
                    // Connect the client socket to the server socket.
                    if (connect(sockfd_, cur_addr->ai_addr, cur_addr->ai_addrlen) == -1) {
                        close(sockfd_);
                        sockfd_ = -1;
                    }
                }
                }

            freeaddrinfo(servinfo); // all done with this structure

            if (sockfd_ == -1) {
                HTEST_LOG_(WARNING) << "stream_result_to: failed to connect to "
                                    << host_name_ << ":" << port_num_;
            }
        }

        // End of class Streaming Listener
#endif // HTEST_CAN_STREAM_RESULTS__
    }
}
