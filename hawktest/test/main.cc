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

#include <cstdio>

#include <hawktest/test/htest.h>

#if defined(HTEST_OS_ESP8266) || defined(HTEST_OS_ESP32) || (defined(HTEST_OS_NRF52) && defined(ARDUINO))
// Arduino-like platforms: program entry points are setup/loop instead of main.

#ifdef HTEST_OS_ESP8266
extern "C" {
#endif

void setup() {
    hawk::init_hawk_test();
}

void loop() {
    RUN_ALL_TESTS();
}

#ifdef HTEST_OS_ESP8266
}
#endif

#elif defined(HTEST_OS_QURT)
// QuRT: program entry point is main, but argc/argv are unusable.

HTEST_API_ int main() {
    printf("Running main() from %s\n", __FILE__);
    hawk::init_hawk_test();
    return RUN_ALL_TESTS();
}
#else
// Normal platforms: program entry point is main, argc/argv are initialized.

HTEST_API_ int main(int argc, char** argv) {
    printf("Running main() from %s\n", __FILE__);
    hawk::init_hawk_test(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
