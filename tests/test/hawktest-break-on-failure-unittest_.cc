// Copyright 2006, Google Inc.
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

// Unit test for Hawk Test's break-on-failure mode.
//
// A user can ask Hawk Test to seg-fault when an assertion fails, using
// either the HTEST_BREAK_ON_FAILURE environment variable or the
// --htest_break_on_failure flag.  This file is used for testing such
// functionality.
//
// This program will be invoked from a Python unit test.  It is
// expected to fail.  Don't run it directly.

#include <hawktest/test/htest.h>

#ifdef HTEST_OS_WINDOWS
#include <stdlib.h>
#include <windows.h>
#endif

namespace {

// A test that's expected to fail.
TEST(Foo, Bar) { EXPECT_EQ(2, 3); }

#if HTEST_HAS_SEH && !defined(HTEST_OS_WINDOWS_MOBILE)
// On Windows Mobile global exception handlers are not supported.
LONG WINAPI
ExitWithExceptionCode(struct _EXCEPTION_POINTERS* exception_pointers) {
  exit(exception_pointers->ExceptionRecord->ExceptionCode);
}
#endif

}  // namespace

int main(int argc, char** argv) {
#ifdef HTEST_OS_WINDOWS
  // Suppresses display of the Windows error dialog upon encountering
  // a general protection fault (segment violation).
  SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);

#if HTEST_HAS_SEH && !defined(HTEST_OS_WINDOWS_MOBILE)

  // The default unhandled exception filter does not always exit
  // with the exception code as exit code - for example it exits with
  // 0 for EXCEPTION_ACCESS_VIOLATION and 1 for EXCEPTION_BREAKPOINT
  // if the application is compiled in debug mode. Thus we use our own
  // filter which always exits with the exception code for unhandled
  // exceptions.
  SetUnhandledExceptionFilter(ExitWithExceptionCode);

#endif
#endif  // HTEST_OS_WINDOWS
  hawk::init_hawk_test(&argc, argv);

  return RUN_ALL_TESTS();
}
