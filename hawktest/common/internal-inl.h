// Copyright 2005, Google Inc.
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

#pragma once

#ifndef _WIN32_WCE
#include <errno.h>
#endif // !_WIN32_WCE
#include <stddef.h>
#include <stdlib.h> // For strtoll/_strtoul64/malloc/free.
#include <string.h> // For memmove.

#include <algorithm>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <hawktest/common/port.h>
#include <hawktest/common/time.h>

#if HTEST_CAN_STREAM_RESULTS_
#include <arpa/inet.h> // NOLINT
#include <netdb.h> // NOLINT
#endif

#ifdef HTEST_OS_WINDOWS
#include <windows.h> // NOLINT
#endif // HTEST_OS_WINDOWS

#include <hawktest/test/htest.h>
#include <hawktest/test/spi.h>

HTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 /* class A needs to have dll-interface to be used by clients of class B */)

namespace testing {
    namespace internal {

        // The value of GetTestTypeId() as seen from within the Hawk Test
        // library.  This is solely for testing GetTestTypeId().
        HTEST_API_ extern const TypeId kTestTypeIdInHawkTest;

        // A valid random seed must be in [1, kMaxRandomSeed].
        const int kMaxRandomSeed = 99999;

        // Returns a random seed in range [1, kMaxRandomSeed] based on the
        // given --htest_random_seed flag value.
        inline int GetRandomSeedFromFlag(int32_t random_seed_flag) {
            const unsigned int raw_seed = (random_seed_flag == 0) ? static_cast<unsigned int>(hawk::GetTimeInMillis())
                                                                  : static_cast<unsigned int>(random_seed_flag);

            // Normalizes the actual seed to range [1, kMaxRandomSeed] such that
            // it's easy to type.
            const int normalized_seed = static_cast<int>((raw_seed - 1U) % static_cast<unsigned int>(kMaxRandomSeed)) + 1;
            return normalized_seed;
        }

        // Returns the first valid random seed after 'seed'.  The behavior is
        // undefined if 'seed' is invalid.  The seed after kMaxRandomSeed is
        // considered to be 1.
        inline int GetNextRandomSeed(int seed) {
            HTEST_CHECK_(1 <= seed && seed <= kMaxRandomSeed)
                << "Invalid random seed " << seed << " - must be in [1, "
                << kMaxRandomSeed << "].";
            const int next_seed = seed + 1;
            return (next_seed > kMaxRandomSeed) ? 1 : next_seed;
        }



        // Reads the HTEST_SHARD_STATUS_FILE environment variable, and creates the file
        // if the variable is present. If a file already exists at this location, this
        // function will write over it. If the variable is present, but the file cannot
        // be created, prints an error and exits.
        void WriteToShardStatusFileIfNeeded();

        // Checks whether sharding is enabled by examining the relevant flag values.
        // If the flags are set, but inconsistent (e.g., shard_index >= total_shards),
        // prints an error and exits. If in_subprocess_for_death_test, sharding is
        // disabled because it must only be applied to the original test
        // process. Otherwise, we could filter out death tests we intended to execute.
        HTEST_API_ bool ShouldShard(bool in_subprocess_for_death_test);

        // Parses the environment variable var as a 32-bit integer. If it is unset,
        // returns default_val. If it is not a 32-bit integer, prints an error and
        // and aborts.
        HTEST_API_ int32_t Int32FromEnvOrDie(const char* env_var, int32_t default_val);

        // Given the total number of shards, the shard index, and the test id,
        // returns true if and only if the test should be run on this shard. The test id
        // is some arbitrary but unique non-negative integer assigned to each test
        // method. Assumes that 0 <= shard_index < total_shards.
        HTEST_API_ bool ShouldRunTestOnShard(int total_shards, int shard_index,
            int test_id);

        // STL container utilities.

        // Returns the number of elements in the given container that satisfy
        // the given predicate.
        template <class Container, typename Predicate>
        inline int CountIf(const Container& c, Predicate predicate) {
            // Implemented as an explicit loop since std::count_if() in libCstd on
            // Solaris has a non-standard signature.
            int count = 0;
            for (auto it = c.begin(); it != c.end(); ++it) {
                if (predicate(*it))
                    ++count;
            }
            return count;
        }


        // Performs an in-place shuffle of a range of the vector's elements.
        // 'begin' and 'end' are element indices as an STL-style range;
        // i.e. [begin, end) are shuffled, where 'end' == size() means to
        // shuffle to the end of the vector.
        template <typename E>
        void ShuffleRange(hawk::Random* random, int begin, int end,
            std::vector<E>* v) {
            const int size = static_cast<int>(v->size());
            HTEST_CHECK_(0 <= begin && begin <= size)
                << "Invalid shuffle range start " << begin << ": must be in range [0, "
                << size << "].";
            HTEST_CHECK_(begin <= end && end <= size)
                << "Invalid shuffle range finish " << end << ": must be in range ["
                << begin << ", " << size << "].";

            // Fisher-Yates shuffle, from
            // https://en.wikipedia.org/wiki/Fisher-Yates_shuffle
            for (int range_width = end - begin; range_width >= 2; range_width--) {
                const int last_in_range = begin + range_width - 1;
                const int selected = begin + static_cast<int>(random->Generate(static_cast<uint32_t>(range_width)));
                std::swap((*v)[static_cast<size_t>(selected)],
                    (*v)[static_cast<size_t>(last_in_range)]);
            }
        }

        // Performs an in-place shuffle of the vector's elements.
        template <typename E>
        inline void Shuffle(hawk::Random* random, std::vector<E>* v) {
            ShuffleRange(random, 0, static_cast<int>(v->size()), v);
        }

        // A function for deleting an object.  Handy for being used as a
        // functor.
        template <typename T>
        static void Delete(T* x) {
            delete x;
        }

        // A predicate that checks the key of a TestProperty against a known key.
        //
        // TestPropertyKeyIs is copyable.
        class TestPropertyKeyIs {
        public:
            // Constructor.
            //
            // TestPropertyKeyIs has NO default constructor.
            explicit TestPropertyKeyIs(const std::string& key)
                : key_(key) { }

            // Returns true if and only if the test name of test property matches on key_.
            bool operator()(const TestProperty& test_property) const {
                return test_property.key() == key_;
            }

        private:
            std::string key_;
        };

#ifdef HTEST_USES_SIMPLE_RE

        // Internal helper functions for implementing the simple regular
        // expression matcher.
        HTEST_API_ bool IsInSet(char ch, const char* str);
        HTEST_API_ bool IsAsciiDigit(char ch);
        HTEST_API_ bool IsAsciiPunct(char ch);
        HTEST_API_ bool IsRepeat(char ch);
        HTEST_API_ bool IsAsciiWhiteSpace(char ch);
        HTEST_API_ bool IsAsciiWordChar(char ch);
        HTEST_API_ bool IsValidEscape(char ch);
        HTEST_API_ bool AtomMatchesChar(bool escaped, char pattern, char ch);
        HTEST_API_ bool ValidateRegex(const char* regex);
        HTEST_API_ bool MatchRegexAtHead(const char* regex, const char* str);
        HTEST_API_ bool MatchRepetitionAndRegexAtHead(bool escaped, char ch,
            char repeat, const char* regex,
            const char* str);
        HTEST_API_ bool MatchRegexAnywhere(const char* regex, const char* str);

#endif // HTEST_USES_SIMPLE_RE

        // Parses the command line for Hawk Test flags, without initializing
        // other parts of Hawk Test.
        HTEST_API_ void parse_hawk_test_flags_only(int* argc, char** argv);
        HTEST_API_ void parse_hawk_test_flags_only(int* argc, wchar_t** argv);

#ifdef HTEST_HAS_DEATH_TEST

        // Returns the message describing the last system error, regardless of the
        // platform.
        HTEST_API_ std::string GetLastErrnoDescription();

        // Attempts to parse a string into a positive integer pointed to by the
        // number parameter.  Returns true if that is possible.
        // HTEST_HAS_DEATH_TEST implies that we have ::std::string, so we can use
        // it here.
        template <typename Integer>
        bool ParseNaturalNumber(const ::std::string& str, Integer* number) {
            // Fail fast if the given string does not begin with a digit;
            // this bypasses strtoXXX's "optional leading whitespace and plus
            // or minus sign" semantics, which are undesirable here.
            if (str.empty() || !IsDigit(str[0])) {
                return false;
            }
            errno = 0;

            char* end;
            // BiggestConvertible is the largest integer type that system-provided
            // string-to-number conversion routines can return.
            using BiggestConvertible = unsigned long long; // NOLINT

            const BiggestConvertible parsed = strtoull(str.c_str(), &end, 10); // NOLINT
            const bool parse_success = *end == '\0' && errno == 0;

            HTEST_CHECK_(sizeof(Integer) <= sizeof(parsed));

            const Integer result = static_cast<Integer>(parsed);
            if (parse_success && static_cast<BiggestConvertible>(result) == parsed) {
                *number = result;
                return true;
            }
            return false;
        }
#endif // HTEST_HAS_DEATH_TEST

        // TestResult contains some private methods that should be hidden from
        // Hawk Test user but are required for testing. This class allow our tests
        // to access them.
        //
        // This class is supplied only for the purpose of testing Hawk Test's own
        // constructs. Do not use it in user tests, either directly or indirectly.
        class TestResultAccessor {
        public:
            static void RecordProperty(TestResult* test_result,
                const std::string& xml_element,
                const TestProperty& property) {
                test_result->RecordProperty(xml_element, property);
            }

            static void ClearTestPartResults(TestResult* test_result) {
                test_result->ClearTestPartResults();
            }

            static const std::vector<testing::TestPartResult>& test_part_results(
                const TestResult& test_result) {
                return test_result.test_part_results();
            }
        };

    } // namespace internal
} // namespace testing

HTEST_DISABLE_MSC_WARNINGS_POP_() //  4251
