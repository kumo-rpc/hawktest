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

#include <filesystem>
#include <string>

namespace md_test {

    /// Load ## / ### suites, label fences, expose, and env declarations.
    /// Match, fixture, and flag lines are ignored. Returns false on a hard error.
    bool load_mark_file(const std::filesystem::path &path, SuiteRegistry &suites, hawk::LabelRegistry &labels, std::string *error);

}  // namespace md_test
