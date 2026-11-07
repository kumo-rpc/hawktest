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
//
// htest_xml_outfile2_test_ writes some xml via TestProperty used by
// htest_xml_outfiles_test.py

#include <atomic>

#include <hawktest/test/htest.h>

class PropertyTwo : public testing::Test {
 protected:
  void SetUp() override { RecordProperty("SetUpProp", 2); }
  void TearDown() override { RecordProperty("TearDownProp", 2); }
};

TEST_F(PropertyTwo, TestInt64ConvertibleProperties) {
  float float_prop = 3.25;
  RecordProperty("TestFloatProperty", float_prop);

  double double_prop = 4.75;
  RecordProperty("TestDoubleProperty", double_prop);

  // Validate we can write an unsigned size_t as a property
  size_t size_t_prop = 5;
  RecordProperty("TestSizetProperty", size_t_prop);

  bool bool_prop = true;
  RecordProperty("TestBoolProperty", bool_prop);

  char char_prop = 'A';
  RecordProperty("TestCharProperty", char_prop);

  int16_t int16_prop = 6;
  RecordProperty("TestInt16Property", int16_prop);

  int32_t int32_prop = 7;
  RecordProperty("TestInt32Property", int32_prop);

  int64_t int64_prop = 8;
  RecordProperty("TestInt64Property", int64_prop);

  enum Foo {
    NINE = 9,
  };
  Foo enum_prop = NINE;
  RecordProperty("TestEnumProperty", enum_prop);

  std::atomic<int> atomic_int_prop(10);
  RecordProperty("TestAtomicIntProperty", atomic_int_prop);
}
