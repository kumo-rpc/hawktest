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

#pragma once

#include <vector>
#include <string>

namespace hawk {

    struct Constants {
        // The list of reserved attributes used in the <testsuite> element of XML
        // output.
        static const std::vector<std::string> kReservedTestSuiteAttributes;

        // The list of reserved attributes used in the <testsuites> element of XML
        // output.
        static const std::vector<std::string> kReservedTestSuitesAttributes;

        // Use a slightly different set for allowed output to ensure existing tests can
        // still RecordProperty("result") or RecordProperty("timestamp")
        static  const std::vector<std::string> kReservedOutputTestCaseAttributes;

        // The list of reserved attributes used in the <testcase> element of XML output.
        static const std::vector<std::string> kReservedTestCaseAttributes;

        // The text used in failure messages to indicate the start of the
        // stack trace.
        static  constexpr std::string_view kStackTraceMarker = "\nStack trace:\n";

        // Text printed in Hawk Test's text output and --htest_list_tests
        // output to label the type parameter and value parameter for a test.
        static constexpr std::string_view kTypeParamLabel = "TypeParam";
        static constexpr std::string_view kValueParamLabel = "GetParam()";

        /// may using c_str;
        // The environment variable name for the test shard index.
         static const std::string kTestShardIndex;
        // The environment variable name for the total number of test shards.
         static const std::string kTestTotalShards;
        // The environment variable name for the test shard status file.
         static const std::string kTestShardStatusFile;

        // A test suite whose name matches this filter is considered a death
        // test suite and will be run before test suites whose name doesn't
        // match this filter.
        // constexpr: TEST() registers suites during dynamic initialization,
        // before a std::string in another .cc is constructed.
        static constexpr std::string_view kDeathTestSuiteFilter = "*DeathTest:*DeathTest/*";

        // A test whose test suite name or test name matches this filter is
        // disabled and not run.
       static const std::string kDisableTestFilter;


        // The environment variable name for the test output warnings file.
        static const std::string kTestWarningsOutputFile;

        // The default output format.
        static const std::string kDefaultOutputFormat;

        // The default output file.
        static const std::string kDefaultOutputFile;

        // A test filter that matches everything.
        static const std::string kUniversalFilter;

        // Flag characters for reporting a death test that did not die.
        static constexpr char kDeathTestLived = 'L';
        static constexpr char kDeathTestReturned = 'R';
        static constexpr char kDeathTestThrew = 'T';
        static constexpr char kDeathTestInternalError = 'I';
    };


}  // namespace hawk
