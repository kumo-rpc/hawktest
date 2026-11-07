// Copyright 2016 Ismael Jimenez Martinez. All rights reserved.
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

// Source project : https://github.com/ismaelJimenez/cpp.leastsq
// Adapted to be used with google benchmark

#ifndef COMPLEXITY_H_
#define COMPLEXITY_H_

#include <string>
#include <vector>

#include <hawktest/bench/reporter.h>
#include <hawktest/bench/statistics.h>

namespace benchmark {

// Return a vector containing the bigO and RMS information for the specified
// list of reports. If 'reports.size() < 2' an empty vector is returned.
std::vector<BenchmarkReporter::Run> ComputeBigO(
    const std::vector<BenchmarkReporter::Run>& reports);

// This data structure will contain the result returned by MinimalLeastSq
//   - coef        : Estimated coefficient for the high-order term as
//                   interpolated from data.
//   - rms         : Normalized Root Mean Squared Error.
//   - complexity  : Scalability form (e.g. oN, oNLogN). In case a scalability
//                   form has been provided to MinimalLeastSq this will return
//                   the same value. In case BigO::oAuto has been selected, this
//                   parameter will return the best fitting curve detected.

struct LeastSq {
  LeastSq() : coef(0.0), rms(0.0), complexity(oNone) {}

  double coef;
  double rms;
  BigO complexity;
};

// Function to return an string for the calculated complexity
std::string GetBigOString(BigO complexity);

}  // end namespace benchmark

#endif  // COMPLEXITY_H_
