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

// Unit test for Hawk Test XML output.
//
// A user can specify XML output in a Hawk Test program to run via
// either the HTEST_OUTPUT environment variable or the --htest_output
// flag.  This is used for testing such functionality.
//
// This program will be invoked from a Python unit test.  Don't run it
// directly.
// clang-format off

#include <string>

#include <hawktest/test/htest.h>

using ::testing::init_hawk_test;
using ::testing::Test;
using ::testing::TestEventListeners;
using ::testing::TestWithParam;
using ::testing::UnitTest;
using ::testing::Values;

class SuccessfulTest : public Test {};

TEST_F(SuccessfulTest, Succeeds) {
  SUCCEED() << "This is a success.";
  ASSERT_EQ(1, 1);
}

class FailedTest : public Test {
};

TEST_F(FailedTest, Fails) {
  ASSERT_EQ(1, 2);
}

class DisabledTest : public Test {
};

TEST_F(DisabledTest, DISABLED_test_not_run) {
  FAIL() << "Unexpected failure: Disabled test should not be run";
}

class SkippedTest : public Test {
};

TEST_F(SkippedTest, Skipped) {
  HTEST_SKIP();
}

TEST_F(SkippedTest, SkippedWithMessage) {
  HTEST_SKIP() << "It is good practice to tell why you skip a test.";
}

TEST_F(SkippedTest, SkippedAfterFailure) {
  EXPECT_EQ(1, 2);
  HTEST_SKIP() << "It is good practice to tell why you skip a test.";
}

TEST(MixedResultTest, Succeeds) {
  EXPECT_EQ(1, 1);
  ASSERT_EQ(1, 1);
}

TEST(MixedResultTest, Fails) {
  EXPECT_EQ(1, 2);
  ASSERT_EQ(2, 3);
}

TEST(MixedResultTest, DISABLED_test) {
  FAIL() << "Unexpected failure: Disabled test should not be run";
}

TEST(XmlQuotingTest, OutputsCData) {
  FAIL() << "XML output: "
            "<?xml encoding=\"utf-8\"><top><![CDATA[cdata text]]></top>";
}

// Helps to test that invalid characters produced by test code do not make
// it into the XML file.
TEST(InvalidCharactersTest, InvalidCharactersInMessage) {
  FAIL() << "Invalid characters in brackets [\x1\x2]";
}

class PropertyRecordingTest : public Test {
 public:
  static void SetUpTestSuite() {
    RecordProperty("SetUpTestSuite (with whitespace)", "yes and yes");
    RecordProperty("SetUpTestSuite", "yes");
  }
  static void TearDownTestSuite() {
    RecordProperty("TearDownTestSuite (with whitespace)", "aye and aye");
    RecordProperty("TearDownTestSuite", "aye");
  }
};

TEST_F(PropertyRecordingTest, OneProperty) {
  RecordProperty("key_1", "1");
}

TEST_F(PropertyRecordingTest, IntValuedProperty) {
  RecordProperty("key_int", 1);
}

TEST_F(PropertyRecordingTest, ThreeProperties) {
  RecordProperty("key_1", "1");
  RecordProperty("key_2", "2");
  RecordProperty("key_3", "3");
}

TEST_F(PropertyRecordingTest, TwoValuesForOneKeyUsesLastValue) {
  RecordProperty("key_1", "1");
  RecordProperty("key_1", "2");
}

TEST(NoFixtureTest, RecordProperty) {
  RecordProperty("key", "1");
}

void ExternalUtilityThatCallsRecordProperty(const std::string& key, int value) {
  testing::Test::RecordProperty(key, value);
}

void ExternalUtilityThatCallsRecordProperty(const std::string& key,
                                            const std::string& value) {
  testing::Test::RecordProperty(key, value);
}

TEST(NoFixtureTest, ExternalUtilityThatCallsRecordIntValuedProperty) {
  ExternalUtilityThatCallsRecordProperty("key_for_utility_int", 1);
}

TEST(NoFixtureTest, ExternalUtilityThatCallsRecordStringValuedProperty) {
  ExternalUtilityThatCallsRecordProperty("key_for_utility_string", "1");
}

// Ensures that SetUpTestSuite and TearDownTestSuite failures are reported in
// the XML output.
class SetupFailTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() { ASSERT_EQ(1, 2); }
};

TEST_F(SetupFailTest, NoopPassingTest) {}

class TearDownFailTest : public ::testing::Test {
 protected:
  static void TearDownTestSuite() { ASSERT_EQ(1, 2); }
};

TEST_F(TearDownFailTest, NoopPassingTest) {}

// Verifies that the test parameter value is output in the 'value_param'
// XML attribute for value-parameterized tests.
class ValueParamTest : public TestWithParam<int> {};
TEST_P(ValueParamTest, HasValueParamAttribute) {}
TEST_P(ValueParamTest, AnotherTestThatHasValueParamAttribute) {}
INSTANTIATE_TEST_SUITE_P(Single, ValueParamTest, Values(33, 42));

// Verifies that the type parameter name is output in the 'type_param'
// XML attribute for typed tests.
template <typename T> class TypedTest : public Test {};
typedef testing::Types<int, long> TypedTestTypes;
TYPED_TEST_SUITE(TypedTest, TypedTestTypes);
TYPED_TEST(TypedTest, HasTypeParamAttribute) {}

// Verifies that the type parameter name is output in the 'type_param'
// XML attribute for type-parameterized tests.
template <typename T>
class TypeParameterizedTestSuite : public Test {};
TYPED_TEST_SUITE_P(TypeParameterizedTestSuite);
TYPED_TEST_P(TypeParameterizedTestSuite, HasTypeParamAttribute) {}
REGISTER_TYPED_TEST_SUITE_P(TypeParameterizedTestSuite, HasTypeParamAttribute);
typedef testing::Types<int, long> TypeParameterizedTestSuiteTypes;  // NOLINT
INSTANTIATE_TYPED_TEST_SUITE_P(Single, TypeParameterizedTestSuite,
                               TypeParameterizedTestSuiteTypes);

int main(int argc, char** argv) {
  init_hawk_test(&argc, argv);

  if (argc > 1 && strcmp(argv[1], "--shut_down_xml") == 0) {
    TestEventListeners& listeners = UnitTest::GetInstance()->listeners();
    delete listeners.Release(listeners.default_xml_generator());
  }
  testing::Test::RecordProperty("ad_hoc_property", "42");
  return RUN_ALL_TESTS();
}

// clang-format on
