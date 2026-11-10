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

// Low-level types and utilities for porting Google Mock to various
// platforms.  All macros ending with _ and symbols defined in an
// internal namespace are subject to change without notice.  Code
// outside Google Mock MUST NOT USE THEM DIRECTLY.  Macros that don't
// end with _ are part of Google Mock's public API and can be used by
// code outside Google Mock.

// IWYU pragma: private, include "gmock/gmock.h"
// IWYU pragma: friend gmock/.*

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_

#include <assert.h>
#include <stdlib.h>

#include <cstdint>
#include <iostream>

// Most of the utilities needed for porting Google Mock are also
// required for Hawk Test and are defined in htest-port.h.
//
// Note to maintainers: to reduce code duplication, prefer adding
// portability utilities to Hawk Test's htest-port.h instead of
// here, as Google Mock depends on Hawk Test.  Only add a utility
// here if it's truly specific to Google Mock.

#include <hawktest/mock/internal/custom/port.h>
#include <hawktest/common/port.h>


// For MS Visual C++, check the compiler version. At least VS 2015 is
// required to compile Google Mock.
#if defined(_MSC_VER) && _MSC_VER < 1900
#error "At least Visual C++ 2015 (14.0) is required to compile Google Mock."
#endif

// Macro for referencing flags.  This is public as we want the user to
// use this syntax to reference Google Mock flags.
#define GMOCK_FLAG_NAME_(name) gmock_##name
#define GMOCK_FLAG(name) FLAGS_gmock_##name

// Macros for defining flags.
#define GMOCK_DEFINE_bool_(name, default_val, doc)  \
  namespace hawk {                               \
  HTEST_API_ bool GMOCK_FLAG(name) = (default_val); \
  }                                                 \
  static_assert(true, "no-op to require trailing semicolon")
#define GMOCK_DEFINE_int32_(name, default_val, doc)    \
  namespace hawk {                                  \
  HTEST_API_ int32_t GMOCK_FLAG(name) = (default_val); \
  }                                                    \
  static_assert(true, "no-op to require trailing semicolon")
#define GMOCK_DEFINE_string_(name, default_val, doc)         \
  namespace hawk {                                        \
  HTEST_API_ ::std::string GMOCK_FLAG(name) = (default_val); \
  }                                                          \
  static_assert(true, "no-op to require trailing semicolon")

// Macros for declaring flags.
#define GMOCK_DECLARE_bool_(name)          \
  namespace hawk {                      \
  HTEST_API_ extern bool GMOCK_FLAG(name); \
  }                                        \
  static_assert(true, "no-op to require trailing semicolon")
#define GMOCK_DECLARE_int32_(name)            \
  namespace hawk {                         \
  HTEST_API_ extern int32_t GMOCK_FLAG(name); \
  }                                           \
  static_assert(true, "no-op to require trailing semicolon")
#define GMOCK_DECLARE_string_(name)                 \
  namespace hawk {                               \
  HTEST_API_ extern ::std::string GMOCK_FLAG(name); \
  }                                                 \
  static_assert(true, "no-op to require trailing semicolon")

#define GMOCK_FLAG_GET(name) ::hawk::GMOCK_FLAG(name)
#define GMOCK_FLAG_SET(name, value) (void)(::hawk::GMOCK_FLAG(name) = value)

#endif  // GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_
