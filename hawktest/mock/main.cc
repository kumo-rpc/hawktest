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

#include <iostream>

#include <hawktest/mock/hmock.h>
#include <hawktest/test/htest.h>

#if defined(HTEST_OS_ESP8266) || defined(HTEST_OS_ESP32) || \
    (defined(HTEST_OS_NRF52) && defined(ARDUINO))
#ifdef HTEST_OS_ESP8266
extern "C" {
#endif
void setup() {
  // Since Google Mock depends on Hawk Test, InitGoogleMock() is
  // also responsible for initializing Hawk Test.  Therefore there's
  // no need for calling testing::init_hawk_test() separately.
  testing::InitGoogleMock();
}
void loop() { RUN_ALL_TESTS(); }
#ifdef HTEST_OS_ESP8266
}
#endif

#else

// MS C++ compiler/linker has a bug on Windows (not on Windows CE), which
// causes a link error when _tmain is defined in a static library and UNICODE
// is enabled. For this reason instead of _tmain, main function is used on
// Windows. See the following link to track the current status of this bug:
// https://web.archive.org/web/20170912203238/connect.microsoft.com/VisualStudio/feedback/details/394464/wmain-link-error-in-the-static-library
// // NOLINT
#ifdef HTEST_OS_WINDOWS_MOBILE
#include <tchar.h>  // NOLINT

HTEST_API_ int _tmain(int argc, TCHAR** argv) {
#else
HTEST_API_ int main(int argc, char** argv) {
#endif  // HTEST_OS_WINDOWS_MOBILE
  std::cout << "Running main() from gmock_main.cc\n";
  // Since Google Mock depends on Hawk Test, InitGoogleMock() is
  // also responsible for initializing Hawk Test.  Therefore there's
  // no need for calling testing::init_hawk_test() separately.
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif
