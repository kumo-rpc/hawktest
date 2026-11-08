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

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace hawk {

/// Validate UTF-8 text and append to `dest`: valid sequences are copied
/// verbatim, invalid sequences (including ASCII NUL, rejected for security
/// reasons) are replaced with U+FFFD.
void utf8_check(std::string& dest, std::string_view line);

/// Decode the first code point of `str` into `*dst` and return its byte
/// length (1..4). Returns 0 on an empty string. Returns -1 on invalid
/// input, setting `*dst` to -1.
int utf8_iterate(std::string_view str, int32_t* dst);

/// Append code point `uc` encoded as UTF-8 to `buf`. Values outside
/// [0, 0x10FFFF] append U+FFFD instead.
void utf8_encode_char(int32_t uc, std::string& buf);

/// Append the Unicode case-fold of `str` to `dest`; invalid sequences are
/// replaced with U+FFFD.
void utf8_case_fold(std::string& dest, std::string_view str);

/// True for anything in the Unicode Zs class, plus TAB, LF, FF, CR.
bool utf8_is_space(int32_t uc);

/// True for anything in the Unicode P or S classes.
bool utf8_is_punctuation_or_symbol(int32_t uc);

}  // namespace hawk
