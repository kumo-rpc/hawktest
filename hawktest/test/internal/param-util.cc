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

#include <hawktest/test/internal/param-util.h>
#include <hawktest/test/htest.h>
#include <hawktest/test/test.h>

namespace testing {
    namespace internal {

        void TypeParameterizedTestSuiteRegistry::RegisterInstantiation(
    const char* test_suite_name) {
            auto it = suites_.find(std::string(test_suite_name));
            if (it != suites_.end()) {
                it->second.instantiated = true;
            } else {
                HTEST_LOG_(ERROR) << "Unknown type parameterized test suit '"
                                  << test_suite_name << "'";
            }
        }

        constexpr bool kErrorOnUninstantiatedTypeParameterizedTest = true;

        void TypeParameterizedTestSuiteRegistry::CheckForInstantiations() {
            const auto& ignored = *GetIgnoredParameterizedTestSuites();
            for (const auto& testcase : suites_) {
                if (testcase.second.instantiated)
                    continue;
                if (ignored.find(testcase.first) != ignored.end())
                    continue;

                std::string message = "Type parameterized test suite " + testcase.first + " is defined via REGISTER_TYPED_TEST_SUITE_P, but never instantiated "
                                                                                          "via INSTANTIATE_TYPED_TEST_SUITE_P. None of the test cases will run."
                                                                                          "\n\n"
                                                                                          "Ideally, TYPED_TEST_P definitions should only ever be included as "
                                                                                          "part of binaries that intend to use them. (As opposed to, for "
                                                                                          "example, being placed in a library that may be linked in to get "
                                                                                          "other "
                                                                                          "utilities.)"
                                                                                          "\n\n"
                                                                                          "To suppress this error for this test suite, insert the following "
                                                                                          "line "
                                                                                          "(in a non-header) in the namespace it is defined in:"
                                                                                          "\n\n"
                                                                                          "HTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST("
                    + testcase.first + ");";

                std::string full_name = "UninstantiatedTypeParameterizedTestSuite<" + testcase.first + ">";
                RegisterTest( //
                    "HawkTestVerification", full_name.c_str(),
                    nullptr, // No type parameter.
                    nullptr, // No value parameter.
                    testcase.second.code_location.file.c_str(),
                    testcase.second.code_location.line, [message, testcase] {
                        return new FailureTest(testcase.second.code_location, message,
                            kErrorOnUninstantiatedTypeParameterizedTest);
                    });
            }
        }

        // Add a given test_suit to the list of them allow to go un-instantiated.
        MarkAsIgnored::MarkAsIgnored(const char* test_suite) {
            GetIgnoredParameterizedTestSuites()->insert(test_suite);
        }

        namespace {

            // When TEST_P is found without a matching INSTANTIATE_TEST_SUITE_P
            // to creates test cases for it, a synthetic test case is
            // inserted to report ether an error or a log message.
            //
            // This configuration bit will likely be removed at some point.
            constexpr bool kErrorOnUninstantiatedParameterizedTest = true;

        } // namespace

        // If this parameterized test suite has no instantiations (and that
        // has not been marked as okay), emit a test case reporting that.
        void InsertSyntheticTestCase(const std::string& name, CodeLocation location,
            bool has_test_p) {
            const auto& ignored = *GetIgnoredParameterizedTestSuites();
            if (ignored.find(name) != ignored.end())
                return;

            const char kMissingInstantiation[] = //
                " is defined via TEST_P, but never instantiated. None of the test "
                "cases "
                "will run. Either no INSTANTIATE_TEST_SUITE_P is provided or the only "
                "ones provided expand to nothing."
                "\n\n"
                "Ideally, TEST_P definitions should only ever be included as part of "
                "binaries that intend to use them. (As opposed to, for example, being "
                "placed in a library that may be linked in to get other utilities.)";

            const char kMissingTestCase[] = //
                " is instantiated via INSTANTIATE_TEST_SUITE_P, but no tests are "
                "defined via TEST_P . No test cases will run."
                "\n\n"
                "Ideally, INSTANTIATE_TEST_SUITE_P should only ever be invoked from "
                "code that always depend on code that provides TEST_P. Failing to do "
                "so is often an indication of dead code, e.g. the last TEST_P was "
                "removed but the rest got left behind.";

            std::string message = "Parameterized test suite " + name + (has_test_p ? kMissingInstantiation : kMissingTestCase) + "\n\n"
                                                                                                                                 "To suppress this error for this test suite, insert the following line "
                                                                                                                                 "(in a non-header) in the namespace it is defined in:"
                                                                                                                                 "\n\n"
                                                                                                                                 "HTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST("
                + name + ");";

            std::string full_name = "UninstantiatedParameterizedTestSuite<" + name + ">";
            RegisterTest( //
                "HawkTestVerification", full_name.c_str(),
                nullptr, // No type parameter.
                nullptr, // No value parameter.
                location.file.c_str(), location.line, [message, location] {
                    return new FailureTest(location, message,
                        kErrorOnUninstantiatedParameterizedTest);
                });
        }

        void ReportInvalidTestSuiteType(const char* test_suite_name,
      const CodeLocation& code_location) {
            Message errors;
            errors
                << "Attempted redefinition of test suite " << test_suite_name << ".\n"
                << "All tests in the same test suite must use the same test fixture\n"
                << "class.  However, in test suite " << test_suite_name << ", you tried\n"
                << "to define a test using a fixture class different from the one\n"
                << "used earlier. This can happen if the two fixture classes are\n"
                << "from different namespaces and have the same name. You should\n"
                << "probably rename one of the classes to put the tests into different\n"
                << "test suites.";

            HTEST_LOG_(ERROR) << FormatFileLocation(code_location.file.c_str(),
                code_location.line)
                              << " " << errors.GetString();
        }

    }
}