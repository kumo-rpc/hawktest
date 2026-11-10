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
#include <hawktest/reporter/listener.h>
#include <hawktest/test/test_suite.h>
#include <hawktest/test/unit_test.h>

namespace hawk {
    namespace internal {

#if HTEST_CAN_STREAM_RESULTS_

        // Streams test results to the given port on the given host machine.
        class StreamingListener : public EmptyTestEventListener {
        public:
            // Abstract base class for writing strings to a socket.
            class AbstractSocketWriter {
            public:
                virtual ~AbstractSocketWriter() = default;

                // Sends a string to the socket.
                virtual void Send(const std::string& message) = 0;

                // Closes the socket.
                virtual void CloseConnection() { }

                // Sends a string and a newline to the socket.
                void SendLn(const std::string& message) { Send(message + "\n"); }
            };

            // Concrete class for actually writing strings to a socket.
            class SocketWriter : public AbstractSocketWriter {
            public:
                SocketWriter(const std::string& host, const std::string& port)
                    : sockfd_(-1)
                    , host_name_(host)
                    , port_num_(port) {
                    MakeConnection();
                }

                ~SocketWriter() override {
                    if (sockfd_ != -1)
                        CloseConnection();
                }

                // Sends a string to the socket.
                void Send(const std::string& message) override {
                    HTEST_CHECK_(sockfd_ != -1)
                        << "Send() can be called only when there is a connection.";

                    const size_t len = message.length();
                    if (write(sockfd_, message.c_str(), len) != static_cast<ssize_t>(len)) {
                        HTEST_LOG_(WARNING) << "stream_result_to: failed to stream to "
                                            << host_name_ << ":" << port_num_;
                    }
                }

            private:
                // Creates a client socket and connects to the server.
                void MakeConnection();

                // Closes the socket.
                void CloseConnection() override {
                    HTEST_CHECK_(sockfd_ != -1)
                        << "CloseConnection() can be called only when there is a connection.";

                    close(sockfd_);
                    sockfd_ = -1;
                }

                int sockfd_; // socket file descriptor
                const std::string host_name_;
                const std::string port_num_;

                SocketWriter(const SocketWriter&) = delete;
                SocketWriter& operator=(const SocketWriter&) = delete;
            }; // class SocketWriter

            // Escapes '=', '&', '%', and '\n' characters in str as "%xx".
            static std::string UrlEncode(const char* str);

            StreamingListener(const std::string& host, const std::string& port)
                : socket_writer_(new SocketWriter(host, port)) {
                Start();
            }

            explicit StreamingListener(AbstractSocketWriter* socket_writer)
                : socket_writer_(socket_writer) {
                Start();
            }

            void OnTestProgramStart(const UnitTest& /* unit_test */) override {
                SendLn("event=TestProgramStart");
            }

            void OnTestProgramEnd(const UnitTest& unit_test) override {
                // Note that Hawk Test current only report elapsed time for each
                // test iteration, not for the entire test program.
                SendLn("event=TestProgramEnd&passed=" + FormatBool(unit_test.Passed()));

                // Notify the streaming server to stop.
                socket_writer_->CloseConnection();
            }

            void OnTestIterationStart(const UnitTest& /* unit_test */,
                int iteration) override {
                SendLn("event=TestIterationStart&iteration=" +
                    hawk::internal::StreamableToString(iteration));
            }

            void OnTestIterationEnd(const UnitTest& unit_test,
                int /* iteration */) override {
                SendLn("event=TestIterationEnd&passed=" + FormatBool(unit_test.Passed()) +
                    "&elapsed_time=" + hawk::internal::StreamableToString(unit_test.elapsed_time()) + "ms");
            }

            // Note that "event=TestCaseStart" is a wire format and has to remain
            // "case" for compatibility
            void OnTestSuiteStart(const TestSuite& test_suite) override {
                SendLn(std::string("event=TestCaseStart&name=") + test_suite.name());
            }

            // Note that "event=TestCaseEnd" is a wire format and has to remain
            // "case" for compatibility
            void OnTestSuiteEnd(const TestSuite& test_suite) override {
                SendLn("event=TestCaseEnd&passed=" + FormatBool(test_suite.Passed()) + "&elapsed_time=" +
                    hawk::internal::StreamableToString(test_suite.elapsed_time()) + "ms");
            }

            void OnTestStart(const TestInfo& test_info) override {
                SendLn(std::string("event=TestStart&name=") + test_info.name());
            }

            void OnTestEnd(const TestInfo& test_info) override {
                SendLn("event=TestEnd&passed=" + FormatBool((test_info.result())->Passed()) + "&elapsed_time=" +
                    hawk::internal::StreamableToString((test_info.result())->elapsed_time()) + "ms");
            }

            void OnTestPartResult(const TestPartResult& test_part_result) override {
                const char* file_name = test_part_result.file_name();
                if (file_name == nullptr)
                    file_name = "";
                SendLn("event=TestPartResult&file=" + UrlEncode(file_name) + "&line=" +
                    hawk::internal::StreamableToString(test_part_result.line_number()) + "&message=" + UrlEncode(test_part_result.message()));
            }

        private:
            // Sends the given message and a newline to the socket.
            void SendLn(const std::string& message) { socket_writer_->SendLn(message); }

            // Called at the start of streaming to notify the receiver what
            // protocol we are using.
            void Start() { SendLn("htest_streaming_protocol_version=1.0"); }

            std::string FormatBool(bool value) { return value ? "1" : "0"; }

            const std::unique_ptr<AbstractSocketWriter> socket_writer_;

            StreamingListener(const StreamingListener&) = delete;
            StreamingListener& operator=(const StreamingListener&) = delete;
        }; // class StreamingListener

#endif // HTEST_CAN_STREAM_RESULTS_

    }
}