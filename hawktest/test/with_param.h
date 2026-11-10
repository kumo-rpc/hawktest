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

#include <hawktest/test/fwd.h>

namespace hawk {

    // The pure interface class that all value-parameterized tests inherit from.
    // A value-parameterized class must inherit from both ::hawk::Test and
    // ::hawk::WithParamInterface. In most cases that just means inheriting
    // from ::hawk::TestWithParam, but more complicated test hierarchies
    // may need to inherit from Test and WithParamInterface at different levels.
    //
    // This interface has support for accessing the test parameter value via
    // the GetParam() method.
    //
    // Use it with one of the parameter generator defining functions, like Range(),
    // Values(), ValuesIn(), Bool(), Combine(), and ConvertGenerator<T>().
    //
    // class FooTest : public ::hawk::TestWithParam<int> {
    //  protected:
    //   FooTest() {
    //     // Can use GetParam() here.
    //   }
    //   ~FooTest() override {
    //     // Can use GetParam() here.
    //   }
    //   void SetUp() override {
    //     // Can use GetParam() here.
    //   }
    //   void TearDown override {
    //     // Can use GetParam() here.
    //   }
    // };
    // TEST_P(FooTest, DoesBar) {
    //   // Can use GetParam() method here.
    //   Foo foo;
    //   ASSERT_TRUE(foo.DoesBar(GetParam()));
    // }
    // INSTANTIATE_TEST_SUITE_P(OneToTenRange, FooTest, ::hawk::Range(1, 10));

    template <typename T>
    class [[nodiscard]] WithParamInterface {
    public:
        typedef T ParamType;
        virtual ~WithParamInterface() = default;

        // The current parameter value. Is also available in the test fixture's
        // constructor.
        [[nodiscard]] static const ParamType& GetParam() {
            HTEST_CHECK_(parameter_ != nullptr)
                << "GetParam() can only be called inside a value-parameterized test "
                << "-- did you intend to write TEST_P instead of TEST_F?";
            return *parameter_;
        }

    private:
        // Sets parameter value. The caller is responsible for making sure the value
        // remains alive and unchanged throughout the current test.
        static void SetParam(const ParamType* parameter) { parameter_ = parameter; }

        // Static value used for accessing parameter during a test lifetime.
        static const ParamType* parameter_;

        // TestClass must be a subclass of WithParamInterface<T> and Test.
        template <class TestClass>
        friend class internal::ParameterizedTestFactory;
    };

    template <typename T>
    const T* WithParamInterface<T>::parameter_ = nullptr;

    // Most value-parameterized classes can ignore the existence of
    // WithParamInterface, and can just inherit from ::hawk::TestWithParam.

    template <typename T>
    class [[nodiscard]] TestWithParam : public Test,
                                        public WithParamInterface<T> { };
}  // namespace hawk
