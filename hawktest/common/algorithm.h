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

#pragma once

#include <hawktest/common/port.h>

namespace testing {
    namespace internal {

        // Returns the i-th element of the vector, or default_value if i is not
        // in range [0, v.size()).
        template <typename E>
        inline E GetElementOr(const std::vector<E>& v, int i, E default_value) {
            return (i < 0 || i >= static_cast<int>(v.size())) ? default_value
                                                              : v[static_cast<size_t>(i)];
        }

        // Applies a function/functor to each element in the container.
        template <class Container, typename Functor>
        void ForEach(const Container& c, Functor functor) {
            std::for_each(c.begin(), c.end(), functor);
        }
    }
}