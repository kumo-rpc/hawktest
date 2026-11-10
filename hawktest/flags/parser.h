// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <hawktest/common/port.h>

namespace testing {
namespace internal {

    // Parses a string for an Int32 flag, in the form of "--flag=value".
    //
    // On success, stores the value of the flag in *value, and returns
    // true.  On failure, returns false without changing *value.
HTEST_API_ bool ParseFlag(const char* str, const char* flag, int32_t* value);
HTEST_API_ void parse_hawk_test_flags_only(int* argc, char** argv);
HTEST_API_ void parse_hawk_test_flags_only(int* argc, wchar_t** argv);

    // g_help_flag is true if and only if the --help flag or an equivalent form
    // is specified on the command line.
    HTEST_API_ extern bool g_help_flag;
} // namespace internal
} // namespace testing
