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

#include <hawktest/test/fwd.h>
#include <hawktest/reporter/listener.h>
#include <hawktest/test/test.h>

namespace hawk {
    namespace internal {

         // This class generates an JSON output file.
        class JsonUnitTestResultPrinter : public EmptyTestEventListener {
        public:
            explicit JsonUnitTestResultPrinter(const char* output_file);

            void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override;

            // Prints an JSON summary of all unit tests.
            static void PrintJsonTestList(::std::ostream* stream,
                const std::vector<TestSuite*>& test_suites);

        private:
            // Returns an JSON-escaped copy of the input string str.
            static std::string EscapeJson(const std::string& str);

            //// Verifies that the given attribute belongs to the given element and
            //// streams the attribute as JSON.
            static void OutputJsonKey(std::ostream* stream,
                const std::string& element_name,
                const std::string& name, const std::string& value,
                const std::string& indent, bool comma = true);
            static void OutputJsonKey(std::ostream* stream,
                const std::string& element_name,
                const std::string& name, int value,
                const std::string& indent, bool comma = true);

            // Streams a test suite JSON stanza containing the given test result.
            //
            // Requires: result.Failed()
            static void OutputJsonTestSuiteForTestResult(::std::ostream* stream,
                const TestResult& result);

            // Streams a test case JSON stanza containing the given test result.
            //
            // Requires: result.Failed()
            static void OutputJsonTestCaseForTestResult(::std::ostream* stream,
                const TestResult& result);

            // Streams a JSON representation of a TestResult object.
            static void OutputJsonTestResult(::std::ostream* stream,
                const TestResult& result);

            // Streams a JSON representation of a TestInfo object.
            static void OutputJsonTestInfo(::std::ostream* stream,
                const char* test_suite_name,
                const TestInfo& test_info);

            // Prints a JSON representation of a TestSuite object
            static void PrintJsonTestSuite(::std::ostream* stream,
                const TestSuite& test_suite);

            // Prints a JSON summary of unit_test to output stream out.
            static void PrintJsonUnitTest(::std::ostream* stream,
                const UnitTest& unit_test);

            // Produces a string representing the test properties in a result as
            // a JSON dictionary.
            static std::string TestPropertiesAsJson(const TestResult& result,
                const std::string& indent);

            // The output file.
            const std::string output_file_;

            JsonUnitTestResultPrinter(const JsonUnitTestResultPrinter&) = delete;
            JsonUnitTestResultPrinter& operator=(const JsonUnitTestResultPrinter&) = delete;
        };

        std::vector<std::string> GetReservedOutputAttributesForElement(const std::string& xml_element);

    }


}