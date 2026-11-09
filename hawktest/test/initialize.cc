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

#include <hawktest/test/initialize.h>
#include <hawktest/flags/parser.h>
#include <hawktest/test/unit_test.h>
#include <hawktest/test/internal/unit_test_impl.h>

namespace testing {

    namespace internal {
        // HTestIsInitialized() returns true if and only if the user has initialized
        // Hawk Test.  Useful for catching the user mistake of not initializing
        // Hawk Test before calling RUN_ALL_TESTS().
        bool HTestIsInitialized() {
            return !GetArgvs().empty();
        }


        // A copy of all command line arguments.  Set by init_hawk_test().
        static ::std::vector<std::string> g_argvs;


        ::std::vector<std::string> GetArgvs() {
#if defined(HTEST_CUSTOM_GET_ARGVS_)
            // HTEST_CUSTOM_GET_ARGVS_() may return a container of std::string or
            // ::string. This code converts it to the appropriate type.
            const auto& custom = HTEST_CUSTOM_GET_ARGVS_();
            return ::std::vector<std::string>(custom.begin(), custom.end());
#else // defined(HTEST_CUSTOM_GET_ARGVS_)
            return g_argvs;
#endif // defined(HTEST_CUSTOM_GET_ARGVS_)
        }


        // The internal implementation of init_hawk_test().
        //
        // The type parameter CharType can be instantiated to either char or
        // wchar_t.
        template <typename CharType>
        void init_hawk_test_impl(int* argc, CharType** argv) {
            // We don't want to run the initialization code twice.
            if (HTestIsInitialized())
                return;

            if (*argc <= 0)
                return;

            g_argvs.clear();
            for (int i = 0; i != *argc; i++) {
                g_argvs.push_back(StreamableToString(argv[i]));
            }

            parse_hawk_test_flags_only(argc, argv);
            GetUnitTestImpl()->PostFlagParsingInit();
        }

    }


    // Initializes Hawk Test.  This must be called before calling
    // RUN_ALL_TESTS().  In particular, it parses a command line for the
    // flags that Hawk Test recognizes.  Whenever a Hawk Test flag is
    // seen, it is removed from argv, and *argc is decremented.
    //
    // No value is returned.  Instead, the Hawk Test flag variables are
    // updated.
    //
    // Calling the function for the second time has no user-visible effect.
    void init_hawk_test(int* argc, char** argv) {
#if defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
        HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_(argc, argv);
#else // defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
        internal::init_hawk_test_impl(argc, argv);
#endif // defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
    }

    // This overloaded version can be used in Windows programs compiled in
    // UNICODE mode.
    void init_hawk_test(int* argc, wchar_t** argv) {
#if defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
        HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_(argc, argv);
#else // defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
        internal::init_hawk_test_impl(argc, argv);
#endif // defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
    }


    // This overloaded version can be used on Arduino/embedded platforms where
    // there is no argc/argv.
    void init_hawk_test() {
        // Since Arduino doesn't have a command line, fake out the argc/argv arguments
        int argc = 1;
        const auto arg0 = "dummy";
        char* argv0 = const_cast<char*>(arg0);
        char** argv = &argv0;

#if defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
        HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_(&argc, argv);
#else // defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
        internal::init_hawk_test_impl(&argc, argv);
#endif // defined(HTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
    }
}  // namespace testing
