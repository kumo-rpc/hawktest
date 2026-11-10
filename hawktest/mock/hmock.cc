// Copyright (C) 2005 Google Inc. All rights reserved.
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

#include <hawktest/mock/hmock.h>

#include <string>

#include <hawktest/mock/internal/port.h>

GMOCK_DEFINE_bool_(catch_leaked_mocks, true,
                   "true if and only if Google Mock should report leaked "
                   "mock objects as failures.");

GMOCK_DEFINE_string_(verbose, testing::internal::kWarningVerbosity,
                     "Controls how verbose Google Mock's output is."
                     "  Valid values:\n"
                     "  info    - prints all messages.\n"
                     "  warning - prints warnings and errors.\n"
                     "  error   - prints errors only.");

GMOCK_DEFINE_int32_(default_mock_behavior, 1,
                    "Controls the default behavior of mocks."
                    "  Valid values:\n"
                    "  0 - by default, mocks act as NiceMocks.\n"
                    "  1 - by default, mocks act as NaggyMocks.\n"
                    "  2 - by default, mocks act as StrictMocks.");

namespace testing {
namespace internal {

// Parses a string as a command line flag.  The string should have the
// format "--gmock_flag=value".  When def_optional is true, the
// "=value" part can be omitted.
//
// Returns the value of the flag, or NULL if the parsing failed.
static const char* ParseGoogleMockFlagValue(const char* str,
                                            const char* flag_name,
                                            bool def_optional) {
  // str and flag must not be NULL.
  if (str == nullptr || flag_name == nullptr) return nullptr;

  // The flag must start with "--gmock_".
  const std::string flag_name_str = std::string("--gmock_") + flag_name;
  const size_t flag_name_len = flag_name_str.length();
  if (strncmp(str, flag_name_str.c_str(), flag_name_len) != 0) return nullptr;

  // Skips the flag name.
  const char* flag_end = str + flag_name_len;

  // When def_optional is true, it's OK to not have a "=value" part.
  if (def_optional && (flag_end[0] == '\0')) {
    return flag_end;
  }

  // If def_optional is true and there are more characters after the
  // flag name, or if def_optional is false, there must be a '=' after
  // the flag name.
  if (flag_end[0] != '=') return nullptr;

  // Returns the string after "=".
  return flag_end + 1;
}

// Parses a string for a Google Mock bool flag, in the form of
// "--gmock_flag=value".
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
static bool ParseGoogleMockFlag(const char* str, const char* flag_name,
                                bool* value) {
  // Gets the value of the flag as a string.
  const char* const value_str = ParseGoogleMockFlagValue(str, flag_name, true);

  // Aborts if the parsing failed.
  if (value_str == nullptr) return false;

  // Converts the string value to a bool.
  *value = !(*value_str == '0' || *value_str == 'f' || *value_str == 'F');
  return true;
}

// Parses a string for a Google Mock string flag, in the form of
// "--gmock_flag=value".
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
template <typename String>
static bool ParseGoogleMockFlag(const char* str, const char* flag_name,
                                String* value) {
  // Gets the value of the flag as a string.
  const char* const value_str = ParseGoogleMockFlagValue(str, flag_name, false);

  // Aborts if the parsing failed.
  if (value_str == nullptr) return false;

  // Sets *value to the value of the flag.
  *value = value_str;
  return true;
}

static bool ParseGoogleMockFlag(const char* str, const char* flag_name,
                                int32_t* value) {
  // Gets the value of the flag as a string.
  const char* const value_str = ParseGoogleMockFlagValue(str, flag_name, true);

  // Aborts if the parsing failed.
  if (value_str == nullptr) return false;

  // Sets *value to the value of the flag.
  return ParseInt32(hawk::Message() << "The value of flag --" << flag_name, value_str,
                    value);
}

// The internal implementation of InitGoogleMock().
//
// The type parameter CharType can be instantiated to either char or
// wchar_t.
template <typename CharType>
void InitGoogleMockImpl(int* argc, CharType** argv) {
  // Makes sure Hawk Test is initialized.  init_hawk_test() is
  // idempotent, so it's fine if the user has already called it.
  init_hawk_test(argc, argv);
  if (*argc <= 0) return;

  for (int i = 1; i != *argc; i++) {
    const std::string arg_string = hawk::internal::StreamableToString(argv[i]);
    const char* const arg = arg_string.c_str();

    // Do we see a Google Mock flag?
    bool found_gmock_flag = false;

#define GMOCK_INTERNAL_PARSE_FLAG(flag_name)            \
  if (!found_gmock_flag) {                              \
    auto value = GMOCK_FLAG_GET(flag_name);             \
    if (ParseGoogleMockFlag(arg, #flag_name, &value)) { \
      GMOCK_FLAG_SET(flag_name, value);                 \
      found_gmock_flag = true;                          \
    }                                                   \
  }

    GMOCK_INTERNAL_PARSE_FLAG(catch_leaked_mocks)
    GMOCK_INTERNAL_PARSE_FLAG(verbose)
    GMOCK_INTERNAL_PARSE_FLAG(default_mock_behavior)

    if (found_gmock_flag) {
      // Yes.  Shift the remainder of the argv list left by one.  Note
      // that argv has (*argc + 1) elements, the last one always being
      // NULL.  The following loop moves the trailing NULL element as
      // well.
      for (int j = i; j != *argc; j++) {
        argv[j] = argv[j + 1];
      }

      // Decrements the argument count.
      (*argc)--;

      // We also need to decrement the iterator as we just removed
      // an element.
      i--;
    }
  }
}

}  // namespace internal

// Initializes Google Mock.  This must be called before running the
// tests.  In particular, it parses a command line for the flags that
// Google Mock recognizes.  Whenever a Google Mock flag is seen, it is
// removed from argv, and *argc is decremented.
//
// No value is returned.  Instead, the Google Mock flag variables are
// updated.
//
// Since Hawk Test is needed for Google Mock to work, this function
// also initializes Hawk Test and parses its flags, if that hasn't
// been done.
HTEST_API_ void InitGoogleMock(int* argc, char** argv) {
  internal::InitGoogleMockImpl(argc, argv);
}

// This overloaded version can be used in Windows programs compiled in
// UNICODE mode.
HTEST_API_ void InitGoogleMock(int* argc, wchar_t** argv) {
  internal::InitGoogleMockImpl(argc, argv);
}

// This overloaded version can be used on Arduino/embedded platforms where
// there is no argc/argv.
HTEST_API_ void InitGoogleMock() {
  // Since Arduino doesn't have a command line, fake out the argc/argv arguments
  int argc = 1;
  const auto arg0 = "dummy";
  char* argv0 = const_cast<char*>(arg0);
  char** argv = &argv0;

  internal::InitGoogleMockImpl(&argc, argv);
}

}  // namespace testing
