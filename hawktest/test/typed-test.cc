// Copyright 2008 Google Inc.
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

#include <hawktest/test/typed-test.h>

#include <set>
#include <string>
#include <vector>

#include <hawktest/test/htest.h>

namespace hawk {
    namespace internal {

        // Skips to the first non-space char in str. Returns an empty string if str
        // contains only whitespace characters.
        static const char* SkipSpaces(const char* str) {
            while (IsSpace(*str))
                str++;
            return str;
        }

        static std::vector<std::string> SplitIntoTestNames(const char* src) {
            std::vector<std::string> name_vec;
            src = SkipSpaces(src);
            for (; src != nullptr; src = SkipComma(src)) {
                name_vec.push_back(StripTrailingSpaces(GetPrefixUntilComma(src)));
            }
            return name_vec;
        }

        // Verifies that registered_tests match the test names in
        // registered_tests_; returns registered_tests if successful, or
        // aborts the program otherwise.
        const char* TypedTestSuitePState::VerifyRegisteredTestNames(
            const char* test_suite_name, const char* file, int line,
            const char* registered_tests) {
            RegisterTypeParameterizedTestSuite(test_suite_name, CodeLocation(file, line));

            typedef RegisteredTestsMap::const_iterator RegisteredTestIter;
            registered_ = true;

            std::vector<std::string> name_vec = SplitIntoTestNames(registered_tests);

            hawk::Message errors;

            std::set<std::string> tests;
            for (std::vector<std::string>::const_iterator name_it = name_vec.begin();
                name_it != name_vec.end(); ++name_it) {
                const std::string& name = *name_it;
                if (tests.count(name) != 0) {
                    errors << "Test " << name << " is listed more than once.\n";
                    continue;
                }

                if (registered_tests_.count(name) != 0) {
                    tests.insert(name);
                } else {
                    errors << "No test named " << name
                           << " can be found in this test suite.\n";
                }
            }

            for (RegisteredTestIter it = registered_tests_.begin();
                it != registered_tests_.end(); ++it) {
                if (tests.count(it->first) == 0) {
                    errors << "You forgot to list test " << it->first << ".\n";
                }
            }

            const std::string& errors_str = errors.GetString();
            if (!errors_str.empty()) {
                fprintf(stderr, "%s %s", FormatFileLocation(file, line).c_str(),
                    errors_str.c_str());
                fflush(stderr);
                posix::Abort();
            }

            return registered_tests;
        }

    } // namespace internal
} // namespace hawk
