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

#include <hawktest/common/port.h>

namespace hawk::internal {

            // Defines RE.

#if defined(HTEST_USES_POSIX_RE) || defined(HTEST_USES_SIMPLE_RE)
        HTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 /* class A needs to have dll-interface to be used by clients of class B */)

        // A simple C++ wrapper for <regex.h>.  It uses the POSIX Extended
        // Regular Expression syntax.
        class HTEST_API_ [[nodiscard]] RE {
        public:
            // A copy constructor is required by the Standard to initialize object
            // references from r-values.
            RE(const RE& other) { Init(other.pattern()); }

            // Constructs an RE from a string.
            RE(const ::std::string& regex) { Init(regex.c_str()); } // NOLINT

            RE(const char* regex) { Init(regex); } // NOLINT
            ~RE();

            // Returns the string representation of the regex.
            const char* pattern() const { return pattern_.c_str(); }

            // FullMatch(str, re) returns true if and only if regular expression re
            // matches the entire str.
            // PartialMatch(str, re) returns true if and only if regular expression re
            // matches a substring of str (including str itself).
            static bool FullMatch(const ::std::string& str, const RE& re) {
                return FullMatch(str.c_str(), re);
            }
            static bool PartialMatch(const ::std::string& str, const RE& re) {
                return PartialMatch(str.c_str(), re);
            }

            static bool FullMatch(const char* str, const RE& re);
            static bool PartialMatch(const char* str, const RE& re);

        private:
            void Init(const char* regex);
            std::string pattern_;
            bool is_valid_;

#ifdef HTEST_USES_POSIX_RE

            regex_t full_regex_; // For FullMatch().
            regex_t partial_regex_; // For PartialMatch().

#else // HTEST_USES_SIMPLE_RE

            std::string full_pattern_; // For FullMatch();

#endif
        };
        HTEST_DISABLE_MSC_WARNINGS_POP_() // 4251
#endif // ::hawk::internal::RE implementation

}  // namespace hawk::internal
