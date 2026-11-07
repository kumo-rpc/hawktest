// Copyright (C) 2005 Google Inc. All rights reserved.
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
// For third-party components and portions derived from GoogleTest or
// Google Mock, the applicable original license terms and notices are
// retained in accordance with their respective licenses.
//
//
// For third-party components and portions derived from GoogleTest or
// Google Mock, the applicable original license terms and notices are
// retained in accordance with their respective licenses.
//

#pragma once


#include <iosfwd>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include <hawktest/common/internal.h>
#include <hawktest/common/string.h>

HTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 /* class A needs to have dll-interface to be used by clients of class B */)

namespace testing {

    // A copyable object representing the result of a test part (i.e. an
    // assertion or an explicit FAIL(), ADD_FAILURE(), or SUCCESS()).
    //
    // Don't inherit from TestPartResult as its destructor is not virtual.
    class HTEST_API_ [[nodiscard]] TestPartResult {
    public:
        // The possible outcomes of a test part (i.e. an assertion or an
        // explicit SUCCEED(), FAIL(), or ADD_FAILURE()).
        enum Type {
            kSuccess, // Succeeded.
            kNonFatalFailure, // Failed but the test can continue.
            kFatalFailure, // Failed and the test should be terminated.
            kSkip // Skipped.
        };

        // C'tor.  TestPartResult does NOT have a default constructor.
        // Always use this constructor (with parameters) to create a
        // TestPartResult object.
        TestPartResult(Type a_type, std::string_view a_file_name, int a_line_number,
            std::string_view a_message)
            : type_(a_type)
            , file_name_(a_file_name)
            , line_number_(a_line_number)
            , summary_(ExtractSummary(a_message))
            , message_(a_message) { }

        // Gets the outcome of the test part.
        Type type() const { return type_; }

        // Gets the name of the source file where the test part took place, or
        // NULL if it's unknown.
        const char* file_name() const {
            return file_name_.empty() ? nullptr : file_name_.c_str();
        }

        // Gets the line in the source file where the test part took place,
        // or -1 if it's unknown.
        int line_number() const { return line_number_; }

        // Gets the summary of the failure message.
        const char* summary() const { return summary_.c_str(); }

        // Gets the message associated with the test part.
        const char* message() const { return message_.c_str(); }

        // Returns true if and only if the test part was skipped.
        bool skipped() const { return type_ == kSkip; }

        // Returns true if and only if the test part passed.
        bool passed() const { return type_ == kSuccess; }

        // Returns true if and only if the test part non-fatally failed.
        bool nonfatally_failed() const { return type_ == kNonFatalFailure; }

        // Returns true if and only if the test part fatally failed.
        bool fatally_failed() const { return type_ == kFatalFailure; }

        // Returns true if and only if the test part failed.
        bool failed() const { return fatally_failed() || nonfatally_failed(); }

    private:
        Type type_;

        // Gets the summary of the failure message by omitting the stack
        // trace in it.
        static std::string ExtractSummary(std::string_view message);

        // The name of the source file where the test part took place, or
        // "" if the source file is unknown.
        std::string file_name_;
        // The line in the source file where the test part took place, or -1
        // if the line number is unknown.
        int line_number_;
        std::string summary_; // The test failure summary.
        std::string message_; // The test failure message.
    };

    // Prints a TestPartResult object.
    std::ostream& operator<<(std::ostream& os, const TestPartResult& result);

    // An array of TestPartResult objects.
    //
    // Don't inherit from TestPartResultArray as its destructor is not
    // virtual.
    class HTEST_API_ [[nodiscard]] TestPartResultArray {
    public:
        TestPartResultArray() = default;

        // Appends the given TestPartResult to the array.
        void Append(const TestPartResult& result);

        // Returns the TestPartResult at the given index (0-based).
        const TestPartResult& GetTestPartResult(int index) const;

        // Returns the number of TestPartResult objects in the array.
        int size() const;

    private:
        std::vector<TestPartResult> array_;

        TestPartResultArray(const TestPartResultArray&) = delete;
        TestPartResultArray& operator=(const TestPartResultArray&) = delete;
    };

    // This interface knows how to report a test part result.
    class HTEST_API_ [[nodiscard]] TestPartResultReporterInterface {
    public:
        virtual ~TestPartResultReporterInterface() = default;

        virtual void ReportTestPartResult(const TestPartResult& result) = 0;
    };

    namespace internal {

        // This helper class is used by {ASSERT|EXPECT}_NO_FATAL_FAILURE to check if a
        // statement generates new fatal failures. To do so it registers itself as the
        // current test part result reporter. Besides checking if fatal failures were
        // reported, it only delegates the reporting to the former result reporter.
        // The original result reporter is restored in the destructor.
        // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
        class HTEST_API_ [[nodiscard]] HasNewFatalFailureHelper
            : public TestPartResultReporterInterface {
        public:
            HasNewFatalFailureHelper();
            ~HasNewFatalFailureHelper() override;
            void ReportTestPartResult(const TestPartResult& result) override;
            bool has_new_fatal_failure() const { return has_new_fatal_failure_; }

        private:
            bool has_new_fatal_failure_;
            TestPartResultReporterInterface* original_reporter_;

            HasNewFatalFailureHelper(const HasNewFatalFailureHelper&) = delete;
            HasNewFatalFailureHelper& operator=(const HasNewFatalFailureHelper&) = delete;
        };

    } // namespace internal

} // namespace testing

HTEST_DISABLE_MSC_WARNINGS_POP_() //  4251
