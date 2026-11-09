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

#include <hawktest/flags/builtin.h>

namespace testing {

    namespace internal {
                // This class saves the values of all Hawk Test flags in its c'tor, and
        // restores them in its d'tor.
        class HTestFlagSaver {
        public:
            // The c'tor.
            HTestFlagSaver() {
                also_run_disabled_tests_ = FLAGS_htest_also_run_disabled_tests;
                break_on_failure_ = FLAGS_htest_break_on_failure;
                catch_exceptions_ = FLAGS_htest_catch_exceptions;
                color_ = FLAGS_htest_color;
                death_test_style_ = FLAGS_htest_death_test_style;
                death_test_use_fork_ = FLAGS_htest_death_test_use_fork;
                fail_fast_ = FLAGS_htest_fail_fast;
                filter_ = FLAGS_htest_filter;
                internal_run_death_test_ = FLAGS_htest_internal_run_death_test;
                list_tests_ = FLAGS_htest_list_tests;
                output_ = FLAGS_htest_output;
                brief_ = FLAGS_htest_brief;
                print_time_ = FLAGS_htest_print_time;
                print_utf8_ = FLAGS_htest_print_utf8;
                random_seed_ = FLAGS_htest_random_seed;
                repeat_ = FLAGS_htest_repeat;
                recreate_environments_when_repeating_ = FLAGS_htest_recreate_environments_when_repeating;
                shuffle_ = FLAGS_htest_shuffle;
                stack_trace_depth_ = FLAGS_htest_stack_trace_depth;
                stream_result_to_ = FLAGS_htest_stream_result_to;
                throw_on_failure_ = FLAGS_htest_throw_on_failure;
            }

            // The d'tor is not virtual.  DO NOT INHERIT FROM THIS CLASS.
            ~HTestFlagSaver() {
                (void)(FLAGS_htest_also_run_disabled_tests = (also_run_disabled_tests_));
                (void)(FLAGS_htest_break_on_failure = (break_on_failure_));
                (void)(FLAGS_htest_catch_exceptions = (catch_exceptions_));
                (void)(FLAGS_htest_color = (color_));
                (void)(FLAGS_htest_death_test_style = (death_test_style_));
                (void)(FLAGS_htest_death_test_use_fork = (death_test_use_fork_));
                (void)(FLAGS_htest_filter = (filter_));
                (void)(FLAGS_htest_fail_fast = (fail_fast_));
                (void)(FLAGS_htest_internal_run_death_test = (internal_run_death_test_));
                (void)(FLAGS_htest_list_tests = (list_tests_));
                (void)(FLAGS_htest_output = (output_));
                (void)(FLAGS_htest_brief = (brief_));
                (void)(FLAGS_htest_print_time = (print_time_));
                (void)(FLAGS_htest_print_utf8 = (print_utf8_));
                (void)(FLAGS_htest_random_seed = (random_seed_));
                (void)(FLAGS_htest_repeat = (repeat_));
                (void)(FLAGS_htest_recreate_environments_when_repeating = (recreate_environments_when_repeating_));
                (void)(FLAGS_htest_shuffle = (shuffle_));
                (void)(FLAGS_htest_stack_trace_depth = (stack_trace_depth_));
                (void)(FLAGS_htest_stream_result_to = (stream_result_to_));
                (void)(FLAGS_htest_throw_on_failure = (throw_on_failure_));
            }

        private:
            // Fields for saving the original values of flags.
            bool also_run_disabled_tests_;
            bool break_on_failure_;
            bool catch_exceptions_;
            std::string color_;
            std::string death_test_style_;
            bool death_test_use_fork_;
            bool fail_fast_;
            std::string filter_;
            std::string internal_run_death_test_;
            bool list_tests_;
            std::string output_;
            bool brief_;
            bool print_time_;
            bool print_utf8_;
            int32_t random_seed_;
            int32_t repeat_;
            bool recreate_environments_when_repeating_;
            bool shuffle_;
            int32_t stack_trace_depth_;
            std::string stream_result_to_;
            bool throw_on_failure_;
        };
    }  // namespace internal
}  // namespace testing
