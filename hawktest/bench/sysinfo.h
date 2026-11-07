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

#ifndef BENCHMARK_SYSINFO_H_
#define BENCHMARK_SYSINFO_H_

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <string>
#include <vector>

#include <hawktest/bench/macros.h>

namespace benchmark {

struct BENCHMARK_EXPORT CPUInfo {
  struct CacheInfo {
    std::string type;
    int level;
    int size;
    int num_sharing;
  };

  enum Scaling { UNKNOWN, ENABLED, DISABLED };

  int num_cpus;
  Scaling scaling;
  double cycles_per_second;
  std::vector<CacheInfo> caches;
  std::vector<double> load_avg;

  static const CPUInfo& Get();

 private:
  CPUInfo();
  BENCHMARK_DISALLOW_COPY_AND_ASSIGN(CPUInfo);
};

struct BENCHMARK_EXPORT SystemInfo {
  enum class ASLR { UNKNOWN, ENABLED, DISABLED };

  std::string name;
  ASLR ASLRStatus;
  static const SystemInfo& Get();

 private:
  SystemInfo();
  BENCHMARK_DISALLOW_COPY_AND_ASSIGN(SystemInfo);
};

}  // namespace benchmark

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif  // BENCHMARK_SYSINFO_H_
