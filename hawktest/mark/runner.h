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

#pragma once

#include <hawktest/common/label.h>
#include <hawktest/mark/suite.h>

#include <string>

namespace md_test {

    struct MarkRunResult {
        bool skipped{false};
        std::string skip_reason;
        bool ok{true};
        std::string error;
    };

    /// Apply skip, flags, and env, resolve labels, then compare each match.
    MarkRunResult run_registered_suite(const RegisteredSuite &registered, const hawk::LabelRegistry &labels);

}  // namespace md_test
