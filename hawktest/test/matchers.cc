// Copyright 2007, Google Inc.
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

// The Google C++ Testing and Mocking Framework (Hawk Test)
//
// This file implements just enough of the matcher interface to allow
// EXPECT_DEATH and friends to accept a matcher argument.

#include <hawktest/test/matchers.h>

#include <string>

#include <hawktest/common/internal.h>
#include <hawktest/common/port.h>

namespace testing {

    // Constructs a matcher that matches a const std::string& whose value is
    // equal to s.
    Matcher<const std::string&>::Matcher(const std::string& s) {
        *this = Eq(s);
    }

    // Constructs a matcher that matches a const std::string& whose value is
    // equal to s.
    Matcher<const std::string&>::Matcher(const char* s) {
        *this = Eq(std::string(s));
    }

    // Constructs a matcher that matches a std::string whose value is equal to
    // s.
    Matcher<std::string>::Matcher(const std::string& s) {
        *this = Eq(s);
    }

    // Constructs a matcher that matches a std::string whose value is equal to
    // s.
    Matcher<std::string>::Matcher(const char* s) {
        *this = Eq(std::string(s));
    }

    // Constructs a matcher that matches a const StringView& whose value is
    // equal to s.
    Matcher<const internal::StringView&>::Matcher(const std::string& s) {
        *this = Eq(s);
    }

    // Constructs a matcher that matches a const StringView& whose value is
    // equal to s.
    Matcher<const internal::StringView&>::Matcher(const char* s) {
        *this = Eq(std::string(s));
    }

    // Constructs a matcher that matches a const StringView& whose value is
    // equal to s.
    Matcher<const internal::StringView&>::Matcher(internal::StringView s) {
        *this = Eq(std::string(s));
    }

    // Constructs a matcher that matches a StringView whose value is equal to
    // s.
    Matcher<internal::StringView>::Matcher(const std::string& s) {
        *this = Eq(s);
    }

    // Constructs a matcher that matches a StringView whose value is equal to
    // s.
    Matcher<internal::StringView>::Matcher(const char* s) {
        *this = Eq(std::string(s));
    }

    // Constructs a matcher that matches a StringView whose value is equal to
    // s.
    Matcher<internal::StringView>::Matcher(internal::StringView s) {
        *this = Eq(std::string(s));
    }

} // namespace testing
