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

#include <hawktest/reporter/json_unit_test_result_printer.h>
#include <hawktest/test/test_suite.h>
#include <hawktest/test/unit_test.h>
#include <hawktest/reporter/constants.h>

namespace hawk {
    namespace internal {

        // Creates a new JsonUnitTestResultPrinter.
        JsonUnitTestResultPrinter::JsonUnitTestResultPrinter(const char* output_file)
            : output_file_(output_file) {
            if (output_file_.empty()) {
                HTEST_LOG_(FATAL) << "JSON output file may not be null";
            }
        }

        void JsonUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
            int /*iteration*/) {
            FILE* jsonout = OpenFileForWriting(output_file_);
            std::stringstream stream;
            PrintJsonUnitTest(&stream, unit_test);
            fprintf(jsonout, "%s", StringStreamToString(&stream).c_str());
            fclose(jsonout);
        }

        // Returns an JSON-escaped copy of the input string str.
        std::string JsonUnitTestResultPrinter::EscapeJson(const std::string& str) {
            hawk::Message m;

            for (size_t i = 0; i < str.size(); ++i) {
                const char ch = str[i];
                switch (ch) {
                case '\\':
                case '"':
                case '/':
                    m << '\\' << ch;
                    break;
                case '\b':
                    m << "\\b";
                    break;
                case '\t':
                    m << "\\t";
                    break;
                case '\n':
                    m << "\\n";
                    break;
                case '\f':
                    m << "\\f";
                    break;
                case '\r':
                    m << "\\r";
                    break;
                default:
                    if (ch < ' ') {
                        m << "\\u00" << String::FormatByte(static_cast<unsigned char>(ch));
                    } else {
                        m << ch;
                    }
                    break;
                }
            }

            return m.GetString();
        }

        void JsonUnitTestResultPrinter::OutputJsonKey(std::ostream* stream,
      const std::string& element_name,
      const std::string& name,
      const std::string& value,
      const std::string& indent,
      bool comma) {
            const std::vector<std::string>& allowed_names = GetReservedOutputAttributesForElement(element_name);

            HTEST_CHECK_(std::find(allowed_names.begin(), allowed_names.end(), name) != allowed_names.end())
                << "Key \"" << name << "\" is not allowed for value \"" << element_name
                << "\".";

            *stream << indent << "\"" << name << "\": \"" << EscapeJson(value) << "\"";
            if (comma)
                *stream << ",\n";
        }

        std::vector<std::string> GetReservedOutputAttributesForElement(
        const std::string& xml_element) {
            if (xml_element == "testsuites") {
                return hawk::Constants::kReservedTestSuitesAttributes;
            } else if (xml_element == "testsuite") {
                return hawk::Constants::kReservedTestSuiteAttributes;
            } else if (xml_element == "testcase") {
                return hawk::Constants::kReservedOutputTestCaseAttributes;
            } else {
                HTEST_CHECK_(false) << "Unrecognized xml_element provided: " << xml_element;
            }
            // This code is unreachable but some compilers may not realizes that.
            return std::vector<std::string>{};
        }

        void JsonUnitTestResultPrinter::OutputJsonKey(
        std::ostream* stream, const std::string& element_name,
        const std::string& name, int value, const std::string& indent, bool comma) {
            const std::vector<std::string>& allowed_names = GetReservedOutputAttributesForElement(element_name);

            HTEST_CHECK_(std::find(allowed_names.begin(), allowed_names.end(), name) != allowed_names.end())
                << "Key \"" << name << "\" is not allowed for value \"" << element_name
                << "\".";

            *stream << indent << "\"" << name << "\": " << hawk::internal::StreamableToString(value);
            if (comma)
                *stream << ",\n";
        }


        // Streams a test suite JSON stanza containing the given test result.
        void JsonUnitTestResultPrinter::OutputJsonTestSuiteForTestResult(
            ::std::ostream* stream, const TestResult& result) {
            // Output the boilerplate for a new test suite.
            *stream << Indent(4) << "{\n";
            OutputJsonKey(stream, "testsuite", "name", "NonTestSuiteFailure", Indent(6));
            OutputJsonKey(stream, "testsuite", "tests", 1, Indent(6));
            if (!FLAGS_htest_list_tests) {
                OutputJsonKey(stream, "testsuite", "failures", 1, Indent(6));
                OutputJsonKey(stream, "testsuite", "disabled", 0, Indent(6));
                OutputJsonKey(stream, "testsuite", "skipped", 0, Indent(6));
                OutputJsonKey(stream, "testsuite", "errors", 0, Indent(6));
                OutputJsonKey(stream, "testsuite", "time",
                    hawk::FormatTimeInMillisAsDuration(result.elapsed_time()),
                    Indent(6));
                OutputJsonKey(stream, "testsuite", "timestamp",
                    hawk::FormatEpochTimeInMillisAsRFC3339(result.start_timestamp()),
                    Indent(6));
            }
            *stream << Indent(6) << "\"testsuite\": [\n";

            OutputJsonTestCaseForTestResult(stream, result);

            // Finish the test suite.
            *stream << "\n"
                    << Indent(6) << "]\n"
                    << Indent(4) << "}";
        }



        // Streams a test case JSON stanza containing the given test result.
        void JsonUnitTestResultPrinter::OutputJsonTestCaseForTestResult(
            ::std::ostream* stream, const TestResult& result) {
            // Output the boilerplate for a new test case.
            *stream << Indent(8) << "{\n";
            OutputJsonKey(stream, "testcase", "name", "", Indent(10));
            OutputJsonKey(stream, "testcase", "status", "RUN", Indent(10));
            OutputJsonKey(stream, "testcase", "result", "COMPLETED", Indent(10));
            OutputJsonKey(stream, "testcase", "timestamp",
                hawk::FormatEpochTimeInMillisAsRFC3339(result.start_timestamp()),
                Indent(10));
            OutputJsonKey(stream, "testcase", "time",
                hawk::FormatTimeInMillisAsDuration(result.elapsed_time()),
                Indent(10));
            OutputJsonKey(stream, "testcase", "classname", "", Indent(10), false);
            *stream << TestPropertiesAsJson(result, Indent(10));

            // Output the actual test result.
            OutputJsonTestResult(stream, result);
        }



        // Prints a JSON representation of a TestInfo object.
        void JsonUnitTestResultPrinter::OutputJsonTestInfo(::std::ostream* stream,
            const char* test_suite_name,
            const TestInfo& test_info) {
            const TestResult& result = *test_info.result();
            const std::string kTestsuite = "testcase";
            const std::string kIndent = Indent(10);

            *stream << Indent(8) << "{\n";
            OutputJsonKey(stream, kTestsuite, "name", test_info.name(), kIndent);

            if (test_info.value_param() != nullptr) {
                OutputJsonKey(stream, kTestsuite, "value_param", test_info.value_param(),
                    kIndent);
            }
            if (test_info.type_param() != nullptr) {
                OutputJsonKey(stream, kTestsuite, "type_param", test_info.type_param(),
                    kIndent);
            }

            OutputJsonKey(stream, kTestsuite, "file", test_info.file(), kIndent);
            OutputJsonKey(stream, kTestsuite, "line", test_info.line(), kIndent, false);
            if (FLAGS_htest_list_tests) {
                *stream << "\n"
                        << Indent(8) << "}";
                return;
            } else {
                *stream << ",\n";
            }

            OutputJsonKey(stream, kTestsuite, "status",
                test_info.should_run() ? "RUN" : "NOTRUN", kIndent);
            OutputJsonKey(stream, kTestsuite, "result",
                test_info.should_run()
                    ? (result.Skipped() ? "SKIPPED" : "COMPLETED")
                    : "SUPPRESSED",
                kIndent);
            OutputJsonKey(stream, kTestsuite, "timestamp",
                hawk::FormatEpochTimeInMillisAsRFC3339(result.start_timestamp()),
                kIndent);
            OutputJsonKey(stream, kTestsuite, "time",
                hawk::FormatTimeInMillisAsDuration(result.elapsed_time()), kIndent);
            OutputJsonKey(stream, kTestsuite, "classname", test_suite_name, kIndent,
                false);
            *stream << TestPropertiesAsJson(result, kIndent);

            OutputJsonTestResult(stream, result);
        }

        void JsonUnitTestResultPrinter::OutputJsonTestResult(::std::ostream* stream,
            const TestResult& result) {
            const std::string kIndent = Indent(10);

            {
                int failures = 0;
                for (int i = 0; i < result.total_part_count(); ++i) {
                    const TestPartResult& part = result.GetTestPartResult(i);
                    if (part.failed()) {
                        *stream << ",\n";
                        if (++failures == 1) {
                            *stream << kIndent << "\"" << "failures" << "\": [\n";
                        }
                        const std::string location = internal::FormatCompilerIndependentFileLocation(part.file_name(),
                            part.line_number());
                        const std::string message = EscapeJson(location + "\n" + part.message());
                        *stream << kIndent << "  {\n"
                                << kIndent << "    \"failure\": \"" << message << "\",\n"
                                << kIndent << "    \"type\": \"\"\n"
                                << kIndent << "  }";
                    }
                }

                if (failures > 0)
                    *stream << "\n"
                            << kIndent << "]";
            }

            {
                int skipped = 0;
                for (int i = 0; i < result.total_part_count(); ++i) {
                    const TestPartResult& part = result.GetTestPartResult(i);
                    if (part.skipped()) {
                        *stream << ",\n";
                        if (++skipped == 1) {
                            *stream << kIndent << "\"" << "skipped" << "\": [\n";
                        }
                        const std::string location = internal::FormatCompilerIndependentFileLocation(part.file_name(),
                            part.line_number());
                        const std::string message = EscapeJson(location + "\n" + part.message());
                        *stream << kIndent << "  {\n"
                                << kIndent << "    \"message\": \"" << message << "\"\n"
                                << kIndent << "  }";
                    }
                }

                if (skipped > 0)
                    *stream << "\n"
                            << kIndent << "]";
            }

            *stream << "\n"
                    << Indent(8) << "}";
        }


        // Prints an JSON representation of a TestSuite object
        void JsonUnitTestResultPrinter::PrintJsonTestSuite(
            std::ostream* stream, const TestSuite& test_suite) {
            const std::string kTestsuite = "testsuite";
            const std::string kIndent = Indent(6);

            *stream << Indent(4) << "{\n";
            OutputJsonKey(stream, kTestsuite, "name", test_suite.name(), kIndent);
            OutputJsonKey(stream, kTestsuite, "tests", test_suite.reportable_test_count(),
                kIndent);
            if (!FLAGS_htest_list_tests) {
                OutputJsonKey(stream, kTestsuite, "failures",
                    test_suite.failed_test_count(), kIndent);
                OutputJsonKey(stream, kTestsuite, "disabled",
                    test_suite.reportable_disabled_test_count(), kIndent);
                OutputJsonKey(stream, kTestsuite, "errors", 0, kIndent);
                OutputJsonKey(
                    stream, kTestsuite, "timestamp",
                    hawk::FormatEpochTimeInMillisAsRFC3339(test_suite.start_timestamp()),
                    kIndent);
                OutputJsonKey(stream, kTestsuite, "time",
                    hawk::FormatTimeInMillisAsDuration(test_suite.elapsed_time()),
                    kIndent, false);
                *stream << TestPropertiesAsJson(test_suite.ad_hoc_test_result(), kIndent)
                        << ",\n";
            }

            *stream << kIndent << "\"" << kTestsuite << "\": [\n";

            bool comma = false;
            for (int i = 0; i < test_suite.total_test_count(); ++i) {
                if (test_suite.GetTestInfo(i)->is_reportable()) {
                    if (comma) {
                        *stream << ",\n";
                    } else {
                        comma = true;
                    }
                    OutputJsonTestInfo(stream, test_suite.name(), *test_suite.GetTestInfo(i));
                }
            }

            // If there was a failure in the test suite setup or teardown include that in
            // the output.
            if (test_suite.ad_hoc_test_result().Failed()) {
                if (comma) {
                    *stream << ",\n";
                }
                OutputJsonTestCaseForTestResult(stream, test_suite.ad_hoc_test_result());
            }

            *stream << "\n"
                    << kIndent << "]\n"
                    << Indent(4) << "}";
        }

        // Prints a JSON summary of unit_test to output stream out.
        void JsonUnitTestResultPrinter::PrintJsonUnitTest(std::ostream* stream,
            const UnitTest& unit_test) {
            const std::string kTestsuites = "testsuites";
            const std::string kIndent = Indent(2);
            *stream << "{\n";

            OutputJsonKey(stream, kTestsuites, "tests", unit_test.reportable_test_count(),
                kIndent);
            OutputJsonKey(stream, kTestsuites, "failures", unit_test.failed_test_count(),
                kIndent);
            OutputJsonKey(stream, kTestsuites, "disabled",
                unit_test.reportable_disabled_test_count(), kIndent);
            OutputJsonKey(stream, kTestsuites, "errors", 0, kIndent);
            if (FLAGS_htest_shuffle) {
                OutputJsonKey(stream, kTestsuites, "random_seed", unit_test.random_seed(),
                    kIndent);
            }
            OutputJsonKey(stream, kTestsuites, "timestamp",
                hawk::FormatEpochTimeInMillisAsRFC3339(unit_test.start_timestamp()),
                kIndent);
            OutputJsonKey(stream, kTestsuites, "time",
                hawk::FormatTimeInMillisAsDuration(unit_test.elapsed_time()), kIndent,
                false);

            *stream << TestPropertiesAsJson(unit_test.ad_hoc_test_result(), kIndent)
                    << ",\n";

            OutputJsonKey(stream, kTestsuites, "name", "AllTests", kIndent);
            *stream << kIndent << "\"" << kTestsuites << "\": [\n";

            bool comma = false;
            for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
                if (unit_test.GetTestSuite(i)->reportable_test_count() > 0) {
                    if (comma) {
                        *stream << ",\n";
                    } else {
                        comma = true;
                    }
                    PrintJsonTestSuite(stream, *unit_test.GetTestSuite(i));
                }
            }

            // If there was a test failure outside of one of the test suites (like in a
            // test environment) include that in the output.
            if (unit_test.ad_hoc_test_result().Failed()) {
                if (comma) {
                    *stream << ",\n";
                }
                OutputJsonTestSuiteForTestResult(stream, unit_test.ad_hoc_test_result());
            }

            *stream << "\n"
                    << kIndent << "]\n"
                    << "}\n";
        }

        void JsonUnitTestResultPrinter::PrintJsonTestList(
            std::ostream* stream, const std::vector<TestSuite*>& test_suites) {
            const std::string kTestsuites = "testsuites";
            const std::string kIndent = Indent(2);
            *stream << "{\n";
            int total_tests = 0;
            for (auto test_suite : test_suites) {
                total_tests += test_suite->total_test_count();
            }
            OutputJsonKey(stream, kTestsuites, "tests", total_tests, kIndent);

            OutputJsonKey(stream, kTestsuites, "name", "AllTests", kIndent);
            *stream << kIndent << "\"" << kTestsuites << "\": [\n";

            for (size_t i = 0; i < test_suites.size(); ++i) {
                if (i != 0) {
                    *stream << ",\n";
                }
                PrintJsonTestSuite(stream, *test_suites[i]);
            }

            *stream << "\n"
                    << kIndent << "]\n"
                    << "}\n";
        }

        // a JSON dictionary.
        std::string JsonUnitTestResultPrinter::TestPropertiesAsJson(
            const TestResult& result, const std::string& indent) {
            hawk::Message attributes;
            for (int i = 0; i < result.test_property_count(); ++i) {
                const TestProperty& property = result.GetTestProperty(i);
                attributes << ",\n"
                           << indent << "\"" << property.key() << "\": " << "\""
                           << EscapeJson(property.value()) << "\"";
            }
            return attributes.GetString();
        }

    }
}