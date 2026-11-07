// Copyright 2015, Google Inc.
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

// The Google C++ Testing and Mocking Framework (Hawk Test)
//
// This header file defines the HTEST_OS_* macro.
// It is separate from htest-port.h so that custom/htest-port.h can include it.

#pragma once

// Determines the platform on which Hawk Test is compiled.
#ifdef __CYGWIN__
#define HTEST_OS_CYGWIN 1
#elif defined(__MINGW__) || defined(__MINGW32__) || defined(__MINGW64__)
#define HTEST_OS_WINDOWS_MINGW 1
#define HTEST_OS_WINDOWS 1
#elif defined _WIN32
#define HTEST_OS_WINDOWS 1
#ifdef _WIN32_WCE
#define HTEST_OS_WINDOWS_MOBILE 1
#elif defined(WINAPI_FAMILY)
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define HTEST_OS_WINDOWS_DESKTOP 1
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
#define HTEST_OS_WINDOWS_PHONE 1
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define HTEST_OS_WINDOWS_RT 1
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_TV_TITLE)
#define HTEST_OS_WINDOWS_PHONE 1
#define HTEST_OS_WINDOWS_TV_TITLE 1
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_GAMES)
#define HTEST_OS_WINDOWS_GAMES 1
#else
// WINAPI_FAMILY defined but no known partition matched.
// Default to desktop.
#define HTEST_OS_WINDOWS_DESKTOP 1
#endif
#else
#define HTEST_OS_WINDOWS_DESKTOP 1
#endif // _WIN32_WCE
#elif defined __OS2__
#define HTEST_OS_OS2 1
#elif defined __APPLE__
#define HTEST_OS_MAC 1
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define HTEST_OS_IOS 1
#endif
#elif defined __DragonFly__
#define HTEST_OS_DRAGONFLY 1
#elif defined __FreeBSD__
#define HTEST_OS_FREEBSD 1
#elif defined __Fuchsia__
#define HTEST_OS_FUCHSIA 1
#elif defined(__GNU__)
#define HTEST_OS_GNU_HURD 1
#elif defined(__GLIBC__) && defined(__FreeBSD_kernel__)
#define HTEST_OS_GNU_KFREEBSD 1
#elif defined __linux__
#define HTEST_OS_LINUX 1
#if defined __ANDROID__
#define HTEST_OS_LINUX_ANDROID 1
#endif
#elif defined __MVS__
#define HTEST_OS_ZOS 1
#elif defined(__sun) && defined(__SVR4)
#define HTEST_OS_SOLARIS 1
#elif defined(_AIX)
#define HTEST_OS_AIX 1
#elif defined(__hpux)
#define HTEST_OS_HPUX 1
#elif defined __native_client__
#define HTEST_OS_NACL 1
#elif defined __NetBSD__
#define HTEST_OS_NETBSD 1
#elif defined __OpenBSD__
#define HTEST_OS_OPENBSD 1
#elif defined __QNX__
#define HTEST_OS_QNX 1
#elif defined(__HAIKU__)
#define HTEST_OS_HAIKU 1
#elif defined ESP8266
#define HTEST_OS_ESP8266 1
#elif defined ESP32
#define HTEST_OS_ESP32 1
#elif defined(__XTENSA__)
#define HTEST_OS_XTENSA 1
#elif defined(__hexagon__)
#define HTEST_OS_QURT 1
#elif defined(CPU_QN9090) || defined(CPU_QN9090HN)
#define HTEST_OS_NXP_QN9090 1
#elif defined(NRF52)
#define HTEST_OS_NRF52 1
#elif defined(__EMSCRIPTEN__)
#define HTEST_OS_EMSCRIPTEN 1
#endif // __CYGWIN__

