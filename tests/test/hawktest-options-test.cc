// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Hawk Test UnitTestOptions tests
//
// This file tests classes and functions used internally by
// Hawk Test.  They are subject to change without notice.
//
// This file is #included from htest.cc, to avoid changing build or
// make-files on Windows and other platforms. Do not #include this file
// anywhere else!

#include <string>

#include <hawktest/test/htest.h>

#ifdef HTEST_OS_WINDOWS_MOBILE
#include <windows.h>
#elif defined(HTEST_OS_WINDOWS)
#include <direct.h>
#elif defined(HTEST_OS_OS2)
// For strcasecmp on OS/2
#include <strings.h>
#endif  // HTEST_OS_WINDOWS_MOBILE

#include <hawktest/common/internal-inl.h>

namespace testing {
namespace internal {
namespace {

// Turns the given relative path into an absolute path.
FilePath GetAbsolutePathOf(const FilePath& relative_path) {
  return FilePath::ConcatPaths(FilePath::GetCurrentDir(), relative_path);
}

// Testing UnitTestOptions::GetOutputFormat/GetOutputFile.

TEST(XmlOutputTest, GetOutputFormatDefault) {
  HTEST_FLAG_SET(output, "");
  EXPECT_STREQ("", UnitTestOptions::GetOutputFormat().c_str());
}

TEST(XmlOutputTest, GetOutputFormat) {
  HTEST_FLAG_SET(output, "xml:filename");
  EXPECT_STREQ("xml", UnitTestOptions::GetOutputFormat().c_str());
}

TEST(XmlOutputTest, GetOutputFileDefault) {
  HTEST_FLAG_SET(output, "");
  EXPECT_EQ(GetAbsolutePathOf(FilePath("test_detail.xml")).string(),
            UnitTestOptions::GetAbsolutePathToOutputFile());
}

TEST(XmlOutputTest, GetOutputFileSingleFile) {
  HTEST_FLAG_SET(output, "xml:filename.abc");
  EXPECT_EQ(GetAbsolutePathOf(FilePath("filename.abc")).string(),
            UnitTestOptions::GetAbsolutePathToOutputFile());
}

TEST(XmlOutputTest, GetOutputFileFromDirectoryPath) {
  HTEST_FLAG_SET(output, "xml:path" HTEST_PATH_SEP_);
  const std::string expected_output_file =
      GetAbsolutePathOf(FilePath(std::string("path") + HTEST_PATH_SEP_ +
                                 GetCurrentExecutableName().string() + ".xml"))
          .string();
  const std::string& output_file =
      UnitTestOptions::GetAbsolutePathToOutputFile();
#ifdef HTEST_OS_WINDOWS
  EXPECT_STRCASEEQ(expected_output_file.c_str(), output_file.c_str());
#else
  EXPECT_EQ(expected_output_file, output_file.c_str());
#endif
}

TEST(OutputFileHelpersTest, GetCurrentExecutableName) {
  const std::string exe_str = GetCurrentExecutableName().string();
#ifdef HTEST_OS_WINDOWS
  const bool success =
      _strcmpi("hawktest-options-test", exe_str.c_str()) == 0 ||
      _strcmpi("htest-options-ex_test", exe_str.c_str()) == 0 ||
      _strcmpi("htest_all_test", exe_str.c_str()) == 0 ||
      _strcmpi("htest_dll_test", exe_str.c_str()) == 0;
#elif defined(HTEST_OS_OS2)
  const bool success =
      strcasecmp("hawktest-options-test", exe_str.c_str()) == 0 ||
      strcasecmp("htest-options-ex_test", exe_str.c_str()) == 0 ||
      strcasecmp("htest_all_test", exe_str.c_str()) == 0 ||
      strcasecmp("htest_dll_test", exe_str.c_str()) == 0;
#elif defined(HTEST_OS_FUCHSIA)
  const bool success = exe_str == "app";
#elif defined(__EMSCRIPTEN__)
  const bool success = exe_str == "patched_hawktest-options-test.js";
#else
  const bool success =
      exe_str == "ut_hawktest-options-test" || exe_str == "htest_all_test" ||
      exe_str == "lt-htest_all_test" || exe_str == "htest_dll_test";
#endif  // platform ifdefs

  if (!success) FAIL() << "GetCurrentExecutableName() returns " << exe_str;
}

#ifndef HTEST_OS_FUCHSIA

class XmlOutputChangeDirTest : public Test {
 protected:
  void SetUp() override {
    original_working_dir_ = FilePath::GetCurrentDir();
    posix::ChDir("..");
    // This will make the test fail if run from the root directory.
    EXPECT_NE(original_working_dir_.string(),
              FilePath::GetCurrentDir().string());
  }

  void TearDown() override {
    posix::ChDir(original_working_dir_.string().c_str());
  }

  FilePath original_working_dir_;
};

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithDefault) {
  HTEST_FLAG_SET(output, "");
  EXPECT_EQ(
      FilePath::ConcatPaths(original_working_dir_, FilePath("test_detail.xml"))
          .string(),
      UnitTestOptions::GetAbsolutePathToOutputFile());
}

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithDefaultXML) {
  HTEST_FLAG_SET(output, "xml");
  EXPECT_EQ(
      FilePath::ConcatPaths(original_working_dir_, FilePath("test_detail.xml"))
          .string(),
      UnitTestOptions::GetAbsolutePathToOutputFile());
}

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithRelativeFile) {
  HTEST_FLAG_SET(output, "xml:filename.abc");
  EXPECT_EQ(
      FilePath::ConcatPaths(original_working_dir_, FilePath("filename.abc"))
          .string(),
      UnitTestOptions::GetAbsolutePathToOutputFile());
}

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithRelativePath) {
  HTEST_FLAG_SET(output, "xml:path" HTEST_PATH_SEP_);
  const std::string expected_output_file =
      FilePath::ConcatPaths(
          original_working_dir_,
          FilePath(std::string("path") + HTEST_PATH_SEP_ +
                   GetCurrentExecutableName().string() + ".xml"))
          .string();
  const std::string& output_file =
      UnitTestOptions::GetAbsolutePathToOutputFile();
#ifdef HTEST_OS_WINDOWS
  EXPECT_STRCASEEQ(expected_output_file.c_str(), output_file.c_str());
#else
  EXPECT_EQ(expected_output_file, output_file.c_str());
#endif
}

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithAbsoluteFile) {
#ifdef HTEST_OS_WINDOWS
  HTEST_FLAG_SET(output, "xml:c:\\tmp\\filename.abc");
  EXPECT_EQ(FilePath("c:\\tmp\\filename.abc").string(),
            UnitTestOptions::GetAbsolutePathToOutputFile());
#else
  HTEST_FLAG_SET(output, "xml:/tmp/filename.abc");
  EXPECT_EQ(FilePath("/tmp/filename.abc").string(),
            UnitTestOptions::GetAbsolutePathToOutputFile());
#endif
}

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithAbsolutePath) {
#ifdef HTEST_OS_WINDOWS
  const std::string path = "c:\\tmp\\";
#else
  const std::string path = "/tmp/";
#endif

  HTEST_FLAG_SET(output, "xml:" + path);
  const std::string expected_output_file =
      path + GetCurrentExecutableName().string() + ".xml";
  const std::string& output_file =
      UnitTestOptions::GetAbsolutePathToOutputFile();

#ifdef HTEST_OS_WINDOWS
  EXPECT_STRCASEEQ(expected_output_file.c_str(), output_file.c_str());
#else
  EXPECT_EQ(expected_output_file, output_file.c_str());
#endif
}

#endif  // !HTEST_OS_FUCHSIA

}  // namespace
}  // namespace internal
}  // namespace testing
