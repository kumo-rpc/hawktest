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

#include <hawktest/reporter/constants.h>

namespace hawk {

    const std::vector<std::string> Constants::kReservedTestSuiteAttributes = {
        "disabled", "errors", "failures", "name",
        "tests", "time", "timestamp", "skipped"
    };

    // The list of reserved attributes used in the <testsuites> element of XML
    // output.
    const std::vector<std::string> Constants::kReservedTestSuitesAttributes = {
        "disabled", "errors", "failures", "name",
        "random_seed", "tests", "time", "timestamp"
    };


    // Use a slightly different set for allowed output to ensure existing tests can
    // still RecordProperty("result") or RecordProperty("timestamp")
    const std::vector<std::string> Constants::kReservedOutputTestCaseAttributes = {
        "classname", "name", "status", "time", "type_param",
        "value_param", "file", "line", "result", "timestamp"
    };

    // The list of reserved attributes used in the <testcase> element of XML output.
    const std::vector<std::string> Constants::kReservedTestCaseAttributes = {
        "classname", "name", "status", "time",
        "type_param", "value_param", "file", "line"
    };

    // The environment variable name for the test shard index.
    const std::string Constants::kTestShardIndex = "HTEST_SHARD_INDEX";
    // The environment variable name for the total number of test shards.
    const std::string Constants::kTestTotalShards = "HTEST_TOTAL_SHARDS";
    // The environment variable name for the test shard status file.
    const std::string Constants::kTestShardStatusFile = "HTEST_SHARD_STATUS_FILE";

    const std::string Constants::kDisableTestFilter = "DISABLED_*:*/DISABLED_*";

    // The environment variable name for the test output warnings file.
    const std::string Constants::kTestWarningsOutputFile = "TEST_WARNINGS_OUTPUT_FILE";

    // The default output format.
    const std::string Constants::kDefaultOutputFormat = "xml";

    // The default output file.
    const std::string Constants::kDefaultOutputFile = "test_detail";

    // A test filter that matches everything.
    const std::string Constants::kUniversalFilter = "*";
}  // namespace hawk
