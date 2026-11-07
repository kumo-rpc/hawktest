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

// Unit test for Hawk Test's --htest_list_tests flag.
//
// A user can ask Hawk Test to list all tests that will run
// so that when using a filter, a user will know what
// tests to look for. The tests will not be run after listing.
//
// This program will be invoked from a Python unit test.
// Don't run it directly.

#include <ostream>
#include <string>

#include <hawktest/test/htest.h>

// Several different test cases and tests that will be listed.
TEST(Foo, Bar1) {}

TEST(Foo, Bar2) {}

TEST(Foo, DISABLED_Bar3) {}

TEST(Abc, Xyz) {}

TEST(Abc, Def) {}

TEST(FooBar, Baz) {}

class FooTest : public testing::Test {};

TEST_F(FooTest, Test1) {}

TEST_F(FooTest, DISABLED_Test2) {}

TEST_F(FooTest, Test3) {}

TEST(FooDeathTest, Test1) {}

// A group of value-parameterized tests.

class MyType {
 public:
  explicit MyType(const std::string& a_value) : value_(a_value) {}

  const std::string& value() const { return value_; }

 private:
  std::string value_;
};

// Teaches Hawk Test how to print a MyType.
void PrintTo(const MyType& x, std::ostream* os) { *os << x.value(); }

class ValueParamTest : public testing::TestWithParam<MyType> {};

TEST_P(ValueParamTest, TestA) {}

TEST_P(ValueParamTest, TestB) {}

INSTANTIATE_TEST_SUITE_P(
    MyInstantiation, ValueParamTest,
    testing::Values(
        MyType("one line"), MyType("two\nlines"),
        MyType("a "
               "very\nloooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
               "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
               "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
               "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
               "ooooong line")));  // NOLINT

// A group of typed tests.

// A deliberately long type name for testing the line-truncating
// behavior when printing a type parameter.
class
    VeryLoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooogName {  // NOLINT
};

template <typename T>
class TypedTest : public testing::Test {};

template <typename T, int kSize>
class MyArray {};

typedef testing::Types<
    VeryLoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooogName,  // NOLINT
    int*, MyArray<bool, 42> >
    MyTypes;

TYPED_TEST_SUITE(TypedTest, MyTypes);

TYPED_TEST(TypedTest, TestA) {}

TYPED_TEST(TypedTest, TestB) {}

// A group of type-parameterized tests.

template <typename T>
class TypeParamTest : public testing::Test {};

TYPED_TEST_SUITE_P(TypeParamTest);

TYPED_TEST_P(TypeParamTest, TestA) {}

TYPED_TEST_P(TypeParamTest, TestB) {}

REGISTER_TYPED_TEST_SUITE_P(TypeParamTest, TestA, TestB);

INSTANTIATE_TYPED_TEST_SUITE_P(My, TypeParamTest, MyTypes);

int main(int argc, char** argv) {
  ::testing::init_hawk_test(&argc, argv);

  return RUN_ALL_TESTS();
}
