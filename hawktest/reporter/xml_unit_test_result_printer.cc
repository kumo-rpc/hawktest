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

#include <hawktest/reporter/xml_unit_test_result_printer.h>
#include <hawktest/reporter/json_unit_test_result_printer.h>
#include <hawktest/test/test_suite.h>
#include <hawktest/test/unit_test.h>

namespace hawk {
    namespace internal {

        // Creates a new XmlUnitTestResultPrinter.
        XmlUnitTestResultPrinter::XmlUnitTestResultPrinter(const char* output_file)
            : output_file_(output_file) {
            if (output_file_.empty()) {
                HTEST_LOG_(FATAL) << "XML output file may not be null";
            }
        }

        // Called after the unit test ends.
        void XmlUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
            int /*iteration*/) {
            FILE* xmlout = OpenFileForWriting(output_file_);
            std::stringstream stream;
            PrintXmlUnitTest(&stream, unit_test);
            fprintf(xmlout, "%s", StringStreamToString(&stream).c_str());
            fclose(xmlout);
        }

        void XmlUnitTestResultPrinter::ListTestsMatchingFilter(
            const std::vector<TestSuite*>& test_suites) {
            FILE* xmlout = OpenFileForWriting(output_file_);
            std::stringstream stream;
            PrintXmlTestsList(&stream, test_suites);
            fprintf(xmlout, "%s", StringStreamToString(&stream).c_str());
            fclose(xmlout);
        }



        // Returns an XML-escaped copy of the input string str.  If is_attribute
        // is true, the text is meant to appear as an attribute value, and
        // normalizable whitespace is preserved by replacing it with character
        // references.
        //
        // Invalid XML characters in str, if any, are stripped from the output.
        // It is expected that most, if not all, of the text processed by this
        // module will consist of ordinary English text.
        // If this module is ever modified to produce version 1.1 XML output,
        // most invalid characters can be retained using character references.
        std::string XmlUnitTestResultPrinter::EscapeXml(const std::string& str,
            bool is_attribute) {
            hawk::Message m;

            for (size_t i = 0; i < str.size(); ++i) {
                const char ch = str[i];
                switch (ch) {
                case '<':
                    m << "&lt;";
                    break;
                case '>':
                    m << "&gt;";
                    break;
                case '&':
                    m << "&amp;";
                    break;
                case '\'':
                    if (is_attribute)
                        m << "&apos;";
                    else
                        m << '\'';
                    break;
                case '"':
                    if (is_attribute)
                        m << "&quot;";
                    else
                        m << '"';
                    break;
                default:
                    if (IsValidXmlCharacter(static_cast<unsigned char>(ch))) {
                        if (is_attribute && IsNormalizableWhitespace(static_cast<unsigned char>(ch)))
                            m << "&#x" << String::FormatByte(static_cast<unsigned char>(ch))
                              << ";";
                        else
                            m << ch;
                    }
                    break;
                }
            }

            return m.GetString();
        }



        // Returns the given string with all characters invalid in XML removed.
        // Currently invalid characters are dropped from the string. An
        // alternative is to replace them with certain characters such as . or ?.
        std::string XmlUnitTestResultPrinter::RemoveInvalidXmlCharacters(
            const std::string& str) {
            std::string output;
            output.reserve(str.size());
            for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
                if (IsValidXmlCharacter(static_cast<unsigned char>(*it)))
                    output.push_back(*it);

            return output;
        }


        // The following routines generate an XML representation of a UnitTest
        // object.
        //
        // This is how Hawk Test concepts map to the DTD:
        //
        // <testsuites name="AllTests">        <-- corresponds to a UnitTest object
        //   <testsuite name="testcase-name">  <-- corresponds to a TestSuite object
        //     <testcase name="test-name">     <-- corresponds to a TestInfo object
        //       <failure message="...">...</failure>
        //       <failure message="...">...</failure>
        //       <failure message="...">...</failure>
        //                                     <-- individual assertion failures
        //     </testcase>
        //   </testsuite>
        // </testsuites>


        // Streams an XML CDATA section, escaping invalid CDATA sequences as needed.
        void XmlUnitTestResultPrinter::OutputXmlCDataSection(::std::ostream* stream,
            const char* data) {
            const char* segment = data;
            *stream << "<![CDATA[";
            for (;;) {
                const char* const next_segment = strstr(segment, "]]>");
                if (next_segment != nullptr) {
                    stream->write(segment, next_segment - segment);
                    *stream << "]]>]]&gt;<![CDATA[";
                    segment = next_segment + strlen("]]>");
                } else {
                    *stream << segment;
                    break;
                }
            }
            *stream << "]]>";
        }


        void XmlUnitTestResultPrinter::OutputXmlAttribute(
            std::ostream* stream, const std::string& element_name,
            const std::string& name, const std::string& value) {
            const std::vector<std::string>& allowed_names = GetReservedOutputAttributesForElement(element_name);

            HTEST_CHECK_(std::find(allowed_names.begin(), allowed_names.end(), name) != allowed_names.end())
                << "Attribute " << name << " is not allowed for element <" << element_name
                << ">.";

            *stream << " " << name << "=\"" << EscapeXmlAttribute(value) << "\"";
        }


        // Streams a test suite XML stanza containing the given test result.
        void XmlUnitTestResultPrinter::OutputXmlTestSuiteForTestResult(
            ::std::ostream* stream, const TestResult& result) {
            // Output the boilerplate for a minimal test suite with one test.
            *stream << "  <testsuite";
            OutputXmlAttribute(stream, "testsuite", "name", "NonTestSuiteFailure");
            OutputXmlAttribute(stream, "testsuite", "tests", "1");
            OutputXmlAttribute(stream, "testsuite", "failures", "1");
            OutputXmlAttribute(stream, "testsuite", "disabled", "0");
            OutputXmlAttribute(stream, "testsuite", "skipped", "0");
            OutputXmlAttribute(stream, "testsuite", "errors", "0");
            OutputXmlAttribute(stream, "testsuite", "time",
                hawk::FormatTimeInMillisAsSeconds(result.elapsed_time()));
            OutputXmlAttribute(
                stream, "testsuite", "timestamp",
                hawk::FormatEpochTimeInMillisAsIso8601(result.start_timestamp()));
            *stream << ">";

            OutputXmlTestCaseForTestResult(stream, result);

            // Complete the test suite.
            *stream << "  </testsuite>\n";
        }

        // Streams a test case XML stanza containing the given test result.
        void XmlUnitTestResultPrinter::OutputXmlTestCaseForTestResult(
            ::std::ostream* stream, const TestResult& result) {
            // Output the boilerplate for a minimal test case with a single test.
            *stream << "    <testcase";
            OutputXmlAttribute(stream, "testcase", "name", "");
            OutputXmlAttribute(stream, "testcase", "status", "run");
            OutputXmlAttribute(stream, "testcase", "result", "completed");
            OutputXmlAttribute(stream, "testcase", "classname", "");
            OutputXmlAttribute(stream, "testcase", "time",
                hawk::FormatTimeInMillisAsSeconds(result.elapsed_time()));
            OutputXmlAttribute(
                stream, "testcase", "timestamp",
                hawk::FormatEpochTimeInMillisAsIso8601(result.start_timestamp()));

            // Output the actual test result.
            OutputXmlTestResult(stream, result);
        }

           // Prints an XML representation of a TestInfo object.
        void XmlUnitTestResultPrinter::OutputXmlTestInfo(::std::ostream* stream,
            const char* test_suite_name,
            const TestInfo& test_info) {
            const TestResult& result = *test_info.result();
            const std::string kTestsuite = "testcase";

            if (test_info.is_in_another_shard()) {
                return;
            }

            *stream << "    <testcase";
            OutputXmlAttribute(stream, kTestsuite, "name", test_info.name());

            if (test_info.value_param() != nullptr) {
                OutputXmlAttribute(stream, kTestsuite, "value_param",
                    test_info.value_param());
            }
            if (test_info.type_param() != nullptr) {
                OutputXmlAttribute(stream, kTestsuite, "type_param",
                    test_info.type_param());
            }

            OutputXmlAttribute(stream, kTestsuite, "file", test_info.file());
            OutputXmlAttribute(stream, kTestsuite, "line",
                hawk::internal::StreamableToString(test_info.line()));
            if (FLAGS_htest_list_tests) {
                *stream << " />\n";
                return;
            }

            OutputXmlAttribute(stream, kTestsuite, "status",
                test_info.should_run() ? "run" : "notrun");
            OutputXmlAttribute(stream, kTestsuite, "result",
                test_info.should_run()
                    ? (result.Skipped() ? "skipped" : "completed")
                    : "suppressed");
            OutputXmlAttribute(stream, kTestsuite, "time",
                hawk::FormatTimeInMillisAsSeconds(result.elapsed_time()));
            OutputXmlAttribute(
                stream, kTestsuite, "timestamp",
                hawk::FormatEpochTimeInMillisAsIso8601(result.start_timestamp()));
            OutputXmlAttribute(stream, kTestsuite, "classname", test_suite_name);

            OutputXmlTestResult(stream, result);
        }

        void XmlUnitTestResultPrinter::OutputXmlTestResult(::std::ostream* stream,
            const TestResult& result) {
            int failures = 0;
            int skips = 0;
            for (int i = 0; i < result.total_part_count(); ++i) {
                const TestPartResult& part = result.GetTestPartResult(i);
                if (part.failed()) {
                    if (++failures == 1 && skips == 0) {
                        *stream << ">\n";
                    }
                    const std::string location = internal::FormatCompilerIndependentFileLocation(part.file_name(),
                        part.line_number());
                    const std::string summary = location + "\n" + part.summary();
                    *stream << "      <failure message=\"" << EscapeXmlAttribute(summary)
                            << "\" type=\"\">";
                    const std::string detail = location + "\n" + part.message();
                    OutputXmlCDataSection(stream, RemoveInvalidXmlCharacters(detail).c_str());
                    *stream << "</failure>\n";
                } else if (part.skipped()) {
                    if (++skips == 1 && failures == 0) {
                        *stream << ">\n";
                    }
                    const std::string location = internal::FormatCompilerIndependentFileLocation(part.file_name(),
                        part.line_number());
                    const std::string summary = location + "\n" + part.summary();
                    *stream << "      <skipped message=\"" << EscapeXmlAttribute(summary)
                            << "\">";
                    const std::string detail = location + "\n" + part.message();
                    OutputXmlCDataSection(stream, RemoveInvalidXmlCharacters(detail).c_str());
                    *stream << "</skipped>\n";
                }
            }

            if (failures == 0 && skips == 0 && result.test_property_count() == 0) {
                *stream << " />\n";
            } else {
                if (failures == 0 && skips == 0) {
                    *stream << ">\n";
                }
                OutputXmlTestProperties(stream, result, /*indent=*/"      ");
                *stream << "    </testcase>\n";
            }
        }


        // Prints an XML representation of a TestSuite object
        void XmlUnitTestResultPrinter::PrintXmlTestSuite(std::ostream* stream,
            const TestSuite& test_suite) {
            const std::string kTestsuite = "testsuite";
            *stream << "  <" << kTestsuite;
            OutputXmlAttribute(stream, kTestsuite, "name", test_suite.name());
            OutputXmlAttribute(stream, kTestsuite, "tests",
                hawk::internal::StreamableToString(test_suite.reportable_test_count()));
            if (!FLAGS_htest_list_tests) {
                OutputXmlAttribute(stream, kTestsuite, "failures",
                    hawk::internal::StreamableToString(test_suite.failed_test_count()));
                OutputXmlAttribute(
                    stream, kTestsuite, "disabled",
                    hawk::internal::StreamableToString(test_suite.reportable_disabled_test_count()));
                OutputXmlAttribute(stream, kTestsuite, "skipped",
                    hawk::internal::StreamableToString(test_suite.skipped_test_count()));

                OutputXmlAttribute(stream, kTestsuite, "errors", "0");

                OutputXmlAttribute(stream, kTestsuite, "time",
                    hawk::FormatTimeInMillisAsSeconds(test_suite.elapsed_time()));
                OutputXmlAttribute(
                    stream, kTestsuite, "timestamp",
                    hawk::FormatEpochTimeInMillisAsIso8601(test_suite.start_timestamp()));
            }
            *stream << ">\n";
            OutputXmlTestProperties(stream, test_suite.ad_hoc_test_result(),
                /*indent=*/"    ");
            for (int i = 0; i < test_suite.total_test_count(); ++i) {
                if (test_suite.GetTestInfo(i)->is_reportable())
                    OutputXmlTestInfo(stream, test_suite.name(), *test_suite.GetTestInfo(i));
            }
            if (test_suite.ad_hoc_test_result().Failed()) {
                OutputXmlTestCaseForTestResult(stream, test_suite.ad_hoc_test_result());
            }

            *stream << "  </" << kTestsuite << ">\n";
        }

        // Prints an XML summary of unit_test to output stream out.
        void XmlUnitTestResultPrinter::PrintXmlUnitTest(std::ostream* stream,
            const UnitTest& unit_test) {
            const std::string kTestsuites = "testsuites";

            *stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            *stream << "<" << kTestsuites;

            OutputXmlAttribute(stream, kTestsuites, "tests",
                hawk::internal::StreamableToString(unit_test.reportable_test_count()));
            OutputXmlAttribute(stream, kTestsuites, "failures",
                hawk::internal::StreamableToString(unit_test.failed_test_count()));
            OutputXmlAttribute(
                stream, kTestsuites, "disabled",
                hawk::internal::StreamableToString(unit_test.reportable_disabled_test_count()));
            OutputXmlAttribute(stream, kTestsuites, "errors", "0");
            OutputXmlAttribute(stream, kTestsuites, "time",
                hawk::FormatTimeInMillisAsSeconds(unit_test.elapsed_time()));
            OutputXmlAttribute(
                stream, kTestsuites, "timestamp",
                hawk::FormatEpochTimeInMillisAsIso8601(unit_test.start_timestamp()));

            if (FLAGS_htest_shuffle) {
                OutputXmlAttribute(stream, kTestsuites, "random_seed",
                    hawk::internal::StreamableToString(unit_test.random_seed()));
            }

            OutputXmlAttribute(stream, kTestsuites, "name", "AllTests");
            *stream << ">\n";

            OutputXmlTestProperties(stream, unit_test.ad_hoc_test_result(),
                /*indent=*/"  ");
            for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
                if (unit_test.GetTestSuite(i)->reportable_test_count() > 0)
                    PrintXmlTestSuite(stream, *unit_test.GetTestSuite(i));
            }

            // If there was a test failure outside of one of the test suites (like in a
            // test environment) include that in the output.
            if (unit_test.ad_hoc_test_result().Failed()) {
                OutputXmlTestSuiteForTestResult(stream, unit_test.ad_hoc_test_result());
            }

            *stream << "</" << kTestsuites << ">\n";
        }

        void XmlUnitTestResultPrinter::PrintXmlTestsList(
            std::ostream* stream, const std::vector<TestSuite*>& test_suites) {
            const std::string kTestsuites = "testsuites";

            *stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            *stream << "<" << kTestsuites;

            int total_tests = 0;
            for (auto test_suite : test_suites) {
                total_tests += test_suite->total_test_count();
            }
            OutputXmlAttribute(stream, kTestsuites, "tests",
                hawk::internal::StreamableToString(total_tests));
            OutputXmlAttribute(stream, kTestsuites, "name", "AllTests");
            *stream << ">\n";

            for (auto test_suite : test_suites) {
                PrintXmlTestSuite(stream, *test_suite);
            }
            *stream << "</" << kTestsuites << ">\n";
        }


        void XmlUnitTestResultPrinter::OutputXmlTestProperties(
            std::ostream* stream, const TestResult& result, const std::string& indent) {
            const std::string kProperties = "properties";
            const std::string kProperty = "property";

            if (result.test_property_count() <= 0) {
                return;
            }

            *stream << indent << "<" << kProperties << ">\n";
            for (int i = 0; i < result.test_property_count(); ++i) {
                const TestProperty& property = result.GetTestProperty(i);
                *stream << indent << "  <" << kProperty;
                *stream << " name=\"" << EscapeXmlAttribute(property.key()) << "\"";
                *stream << " value=\"" << EscapeXmlAttribute(property.value()) << "\"";
                *stream << "/>\n";
            }
            *stream << indent << "</" << kProperties << ">\n";
        }

    }
}
