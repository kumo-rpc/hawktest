// Copyright 2015 Google Inc. All rights reserved.
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

#ifndef BENCHMARK_SRC_COUNTER_H_
#define BENCHMARK_SRC_COUNTER_H_

#include <hawktest/bench/counter.h>
#include <hawktest/bench/export.h>
#include <hawktest/bench/types.h>

namespace benchmark {

namespace internal {
void Finish(UserCounters* l, IterationCount iterations, double time,
            double num_threads);
void Increment(UserCounters* l, UserCounters const& r);
bool SameNames(UserCounters const& l, UserCounters const& r);
}  // end namespace internal

}  // end namespace benchmark

#endif  // BENCHMARK_SRC_COUNTER_H_
