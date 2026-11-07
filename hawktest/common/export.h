// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
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

#pragma once

//
// Project-wide symbol export macros. Single source of truth for every
// hawktest library (htest / hmock / hbench / hcmark / future ones).
//
// All libraries are static by default (kmcmake), so exports are no-ops.
// When a shared build (kmcmake_cc_library SHARE) is introduced, define
// HAWKTEST_BUILD_SHARED (per-target exports handle dllexport/import) to
// activate real visibility semantics from this one place.
//

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(HAWKTEST_BUILD_SHARED)
#if defined(hawktest_EXPORTS)
#define HAWKTEST_EXPORT __declspec(dllexport)
#else
#define HAWKTEST_EXPORT __declspec(dllimport)
#endif
#else
#define HAWKTEST_EXPORT
#endif
#define HAWKTEST_NO_EXPORT
#else
#if defined(HAWKTEST_BUILD_SHARED)
#define HAWKTEST_EXPORT __attribute__((visibility("default")))
#define HAWKTEST_NO_EXPORT __attribute__((visibility("hidden")))
#else
#define HAWKTEST_EXPORT
#define HAWKTEST_NO_EXPORT
#endif
#endif

#if defined(__cplusplus)
#define HAWKTEST_DEPRECATED(msg) [[deprecated(msg)]]
#else
#define HAWKTEST_DEPRECATED(msg) __attribute__((deprecated(msg)))
#endif

//
// cmark port compatibility aliases (upstream generated cmark_export.h).
//

#define CMARK_EXPORT HAWKTEST_EXPORT
#define CMARK_NO_EXPORT HAWKTEST_NO_EXPORT
#define CMARK_DEPRECATED(msg) HAWKTEST_DEPRECATED(msg)
