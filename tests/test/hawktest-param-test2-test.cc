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
// Tests for Hawk Test itself.  This verifies that the basic constructs of
// Hawk Test work.

#include <hawktest/test/htest.h>
#include <tests/test/hawktest-param-test-test.h>

using ::testing::Values;
using ::testing::internal::ParamGenerator;

// Tests that generators defined in a different translation unit
// are functional. The test using extern_gen is defined
// in hawktest-param-test-test.cc.
ParamGenerator<int> extern_gen = Values(33);

// Tests that a parameterized test case can be defined in one translation unit
// and instantiated in another. The test is defined in
// hawktest-param-test-test.cc and ExternalInstantiationTest fixture class is
// defined in htest-param-test_test.h.
INSTANTIATE_TEST_SUITE_P(MultiplesOf33, ExternalInstantiationTest,
                         Values(33, 66));

// Tests that a parameterized test case can be instantiated
// in multiple translation units. Another instantiation is defined
// in hawktest-param-test-test.cc and
// InstantiationInMultipleTranslationUnitsTest fixture is defined in
// htest-param-test_test.h
INSTANTIATE_TEST_SUITE_P(Sequence2, InstantiationInMultipleTranslationUnitsTest,
                         Values(42 * 3, 42 * 4, 42 * 5));
