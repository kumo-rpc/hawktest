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
                // This class generates an XML output file.
        class XmlUnitTestResultPrinter : public EmptyTestEventListener {
        public:
            explicit XmlUnitTestResultPrinter(const char* output_file);

            void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override;
            void ListTestsMatchingFilter(const std::vector<TestSuite*>& test_suites);

            // Prints an XML summary of all unit tests.
            static void PrintXmlTestsList(std::ostream* stream,
                const std::vector<TestSuite*>& test_suites);

        private:
            // Is c a whitespace character that is normalized to a space character
            // when it appears in an XML attribute value?
            static bool IsNormalizableWhitespace(unsigned char c) {
                return c == '\t' || c == '\n' || c == '\r';
            }

            // May c appear in a well-formed XML document?
            // https://www.w3.org/TR/REC-xml/#charsets
            static bool IsValidXmlCharacter(unsigned char c) {
                return IsNormalizableWhitespace(c) || c >= 0x20;
            }

            // Returns an XML-escaped copy of the input string str.  If
            // is_attribute is true, the text is meant to appear as an attribute
            // value, and normalizable whitespace is preserved by replacing it
            // with character references.
            static std::string EscapeXml(const std::string& str, bool is_attribute);

            // Returns the given string with all characters invalid in XML removed.
            static std::string RemoveInvalidXmlCharacters(const std::string& str);

            // Convenience wrapper around EscapeXml when str is an attribute value.
            static std::string EscapeXmlAttribute(const std::string& str) {
                return EscapeXml(str, true);
            }

            // Convenience wrapper around EscapeXml when str is not an attribute value.
            static std::string EscapeXmlText(const char* str) {
                return EscapeXml(str, false);
            }

            // Verifies that the given attribute belongs to the given element and
            // streams the attribute as XML.
            static void OutputXmlAttribute(std::ostream* stream,
                const std::string& element_name,
                const std::string& name,
                const std::string& value);

            // Streams an XML CDATA section, escaping invalid CDATA sequences as needed.
            static void OutputXmlCDataSection(::std::ostream* stream, const char* data);

            // Streams a test suite XML stanza containing the given test result.
            //
            // Requires: result.Failed()
            static void OutputXmlTestSuiteForTestResult(::std::ostream* stream,
                const TestResult& result);

            // Streams a test case XML stanza containing the given test result.
            //
            // Requires: result.Failed()
            static void OutputXmlTestCaseForTestResult(::std::ostream* stream,
                const TestResult& result);

            // Streams an XML representation of a TestResult object.
            static void OutputXmlTestResult(::std::ostream* stream,
                const TestResult& result);

            // Streams an XML representation of a TestInfo object.
            static void OutputXmlTestInfo(::std::ostream* stream,
                const char* test_suite_name,
                const TestInfo& test_info);

            // Prints an XML representation of a TestSuite object
            static void PrintXmlTestSuite(::std::ostream* stream,
                const TestSuite& test_suite);

            // Prints an XML summary of unit_test to output stream out.
            static void PrintXmlUnitTest(::std::ostream* stream,
                const UnitTest& unit_test);

            // Streams an XML representation of the test properties of a TestResult
            // object.
            static void OutputXmlTestProperties(std::ostream* stream,
                const TestResult& result,
                const std::string& indent);

            // The output file.
            const std::string output_file_;

            XmlUnitTestResultPrinter(const XmlUnitTestResultPrinter&) = delete;
            XmlUnitTestResultPrinter& operator=(const XmlUnitTestResultPrinter&) = delete;
        };
    }
}