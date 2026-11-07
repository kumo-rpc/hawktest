// Copyright 2007, Google Inc.
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
// This is the main header file a user should include.

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_H_

// This file implements the following syntax:
//
//   ON_CALL(mock_object, Method(...))
//     .With(...) ?
//     .WillByDefault(...);
//
// where With() is optional and WillByDefault() must appear exactly
// once.
//
//   EXPECT_CALL(mock_object, Method(...))
//     .With(...) ?
//     .Times(...) ?
//     .InSequence(...) *
//     .WillOnce(...) *
//     .WillRepeatedly(...) ?
//     .RetiresOnSaturation() ? ;
//
// where all clauses are optional and WillOnce() can be repeated.

#include <hawktest/mock/actions.h>  // IWYU pragma: export
#include <hawktest/mock/cardinalities.h>  // IWYU pragma: export
#include <hawktest/mock/function-mocker.h>  // IWYU pragma: export
#include <hawktest/mock/matchers.h>  // IWYU pragma: export
#include <hawktest/mock/more-actions.h>  // IWYU pragma: export
#include <hawktest/mock/more-matchers.h>  // IWYU pragma: export
#include <hawktest/mock/nice-strict.h>  // IWYU pragma: export
#include <hawktest/mock/spec-builders.h>  // IWYU pragma: export
#include <hawktest/mock/internal/internal-utils.h>
#include <hawktest/mock/internal/port.h>

// Declares Google Mock flags that we want a user to use programmatically.
GMOCK_DECLARE_bool_(catch_leaked_mocks);
GMOCK_DECLARE_string_(verbose);
GMOCK_DECLARE_int32_(default_mock_behavior);

namespace testing {

// Initializes Google Mock.  This must be called before running the
// tests.  In particular, it parses the command line for the flags
// that Google Mock recognizes.  Whenever a Google Mock flag is seen,
// it is removed from argv, and *argc is decremented.
//
// No value is returned.  Instead, the Google Mock flag variables are
// updated.
//
// Since Hawk Test is needed for Google Mock to work, this function
// also initializes Hawk Test and parses its flags, if that hasn't
// been done.
HTEST_API_ void InitGoogleMock(int* argc, char** argv);

// This overloaded version can be used in Windows programs compiled in
// UNICODE mode.
HTEST_API_ void InitGoogleMock(int* argc, wchar_t** argv);

// This overloaded version can be used on Arduino/embedded platforms where
// there is no argc/argv.
HTEST_API_ void InitGoogleMock();

}  // namespace testing

#endif  // GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_H_
