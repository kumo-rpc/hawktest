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

// Google Mock - a framework for writing C++ mock classes.
//
// This file tests code in gmock.cc.

#include <hawktest/mock/hmock.h>

#include <string>

#include <hawktest/test/htest.h>
#include <hawktest/test/internal/custom/htest.h>

#if !defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)

using testing::InitGoogleMock;

// Verifies that calling InitGoogleMock() on argv results in new_argv,
// and the gmock_verbose flag's value is set to expected_gmock_verbose.
template <typename Char, int M, int N>
void TestInitGoogleMock(const Char* (&argv)[M], const Char* (&new_argv)[N],
                        const ::std::string& expected_gmock_verbose) {
  const ::std::string old_verbose = GMOCK_FLAG_GET(verbose);

  int argc = M - 1;
  InitGoogleMock(&argc, const_cast<Char**>(argv));
  ASSERT_EQ(N - 1, argc) << "The new argv has wrong number of elements.";

  for (int i = 0; i < N; i++) {
    EXPECT_STREQ(new_argv[i], argv[i]);
  }

  EXPECT_EQ(expected_gmock_verbose, GMOCK_FLAG_GET(verbose));
  GMOCK_FLAG_SET(verbose, old_verbose);  // Restores the gmock_verbose flag.
}

TEST(InitGoogleMockTest, ParsesInvalidCommandLine) {
  const char* argv[] = {nullptr};

  const char* new_argv[] = {nullptr};

  TestInitGoogleMock(argv, new_argv, GMOCK_FLAG_GET(verbose));
}

TEST(InitGoogleMockTest, ParsesEmptyCommandLine) {
  const char* argv[] = {"foo.exe", nullptr};

  const char* new_argv[] = {"foo.exe", nullptr};

  TestInitGoogleMock(argv, new_argv, GMOCK_FLAG_GET(verbose));
}

TEST(InitGoogleMockTest, ParsesSingleFlag) {
  const char* argv[] = {"foo.exe", "--gmock_verbose=info", nullptr};

  const char* new_argv[] = {"foo.exe", nullptr};

  TestInitGoogleMock(argv, new_argv, "info");
}

TEST(InitGoogleMockTest, ParsesMultipleFlags) {
  int old_default_behavior = GMOCK_FLAG_GET(default_mock_behavior);
  const wchar_t* argv[] = {L"foo.exe", L"--gmock_verbose=info",
                           L"--gmock_default_mock_behavior=2", nullptr};

  const wchar_t* new_argv[] = {L"foo.exe", nullptr};

  TestInitGoogleMock(argv, new_argv, "info");
  EXPECT_EQ(2, GMOCK_FLAG_GET(default_mock_behavior));
  EXPECT_NE(2, old_default_behavior);
  GMOCK_FLAG_SET(default_mock_behavior, old_default_behavior);
}

TEST(InitGoogleMockTest, ParsesUnrecognizedFlag) {
  const char* argv[] = {"foo.exe", "--non_gmock_flag=blah", nullptr};

  const char* new_argv[] = {"foo.exe", "--non_gmock_flag=blah", nullptr};

  TestInitGoogleMock(argv, new_argv, GMOCK_FLAG_GET(verbose));
}

TEST(InitGoogleMockTest, ParsesGoogleMockFlagAndUnrecognizedFlag) {
  const char* argv[] = {"foo.exe", "--non_gmock_flag=blah",
                        "--gmock_verbose=error", nullptr};

  const char* new_argv[] = {"foo.exe", "--non_gmock_flag=blah", nullptr};

  TestInitGoogleMock(argv, new_argv, "error");
}

TEST(WideInitGoogleMockTest, ParsesInvalidCommandLine) {
  const wchar_t* argv[] = {nullptr};

  const wchar_t* new_argv[] = {nullptr};

  TestInitGoogleMock(argv, new_argv, GMOCK_FLAG_GET(verbose));
}

TEST(WideInitGoogleMockTest, ParsesEmptyCommandLine) {
  const wchar_t* argv[] = {L"foo.exe", nullptr};

  const wchar_t* new_argv[] = {L"foo.exe", nullptr};

  TestInitGoogleMock(argv, new_argv, GMOCK_FLAG_GET(verbose));
}

TEST(WideInitGoogleMockTest, ParsesSingleFlag) {
  const wchar_t* argv[] = {L"foo.exe", L"--gmock_verbose=info", nullptr};

  const wchar_t* new_argv[] = {L"foo.exe", nullptr};

  TestInitGoogleMock(argv, new_argv, "info");
}

TEST(WideInitGoogleMockTest, ParsesMultipleFlags) {
  int old_default_behavior = GMOCK_FLAG_GET(default_mock_behavior);
  const wchar_t* argv[] = {L"foo.exe", L"--gmock_verbose=info",
                           L"--gmock_default_mock_behavior=2", nullptr};

  const wchar_t* new_argv[] = {L"foo.exe", nullptr};

  TestInitGoogleMock(argv, new_argv, "info");
  EXPECT_EQ(2, GMOCK_FLAG_GET(default_mock_behavior));
  EXPECT_NE(2, old_default_behavior);
  GMOCK_FLAG_SET(default_mock_behavior, old_default_behavior);
}

TEST(WideInitGoogleMockTest, ParsesUnrecognizedFlag) {
  const wchar_t* argv[] = {L"foo.exe", L"--non_gmock_flag=blah", nullptr};

  const wchar_t* new_argv[] = {L"foo.exe", L"--non_gmock_flag=blah", nullptr};

  TestInitGoogleMock(argv, new_argv, GMOCK_FLAG_GET(verbose));
}

TEST(WideInitGoogleMockTest, ParsesGoogleMockFlagAndUnrecognizedFlag) {
  const wchar_t* argv[] = {L"foo.exe", L"--non_gmock_flag=blah",
                           L"--gmock_verbose=error", nullptr};

  const wchar_t* new_argv[] = {L"foo.exe", L"--non_gmock_flag=blah", nullptr};

  TestInitGoogleMock(argv, new_argv, "error");
}

#endif  // !defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)

// Makes sure Google Mock flags can be accessed in code.
TEST(FlagTest, IsAccessibleInCode) {
  bool dummy =
      GMOCK_FLAG_GET(catch_leaked_mocks) && GMOCK_FLAG_GET(verbose).empty();
  (void)dummy;  // Avoids the "unused local variable" warning.
}
