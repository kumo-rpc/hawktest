// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
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

#pragma once

#include <hawktest/flags/gflags_declare.h>

DECLARE_bool(htest_also_run_disabled_tests);
DECLARE_bool(htest_break_on_failure);
DECLARE_bool(htest_catch_exceptions);
DECLARE_string(htest_color);
DECLARE_bool(htest_fail_fast);
DECLARE_bool(htest_fail_if_no_test_linked);
DECLARE_bool(htest_fail_if_no_test_selected);
DECLARE_string(htest_filter);
DECLARE_bool(htest_install_failure_signal_handler);
DECLARE_bool(htest_list_tests);
DECLARE_string(htest_output);
DECLARE_bool(htest_brief);
DECLARE_bool(htest_print_time);
DECLARE_bool(htest_print_utf8);
DECLARE_int32(htest_random_seed);
DECLARE_int32(htest_repeat);
DECLARE_bool(htest_recreate_environments_when_repeating);
DECLARE_int32(htest_shard_index);
DECLARE_int32(htest_total_shards);
DECLARE_bool(htest_show_internal_stack_frames);
DECLARE_bool(htest_shuffle);
DECLARE_int32(htest_stack_trace_depth);
DECLARE_bool(htest_throw_on_failure);
DECLARE_string(htest_stream_result_to);
DECLARE_string(htest_flagfile);

DECLARE_string(htest_death_test_style);
DECLARE_bool(htest_death_test_use_fork);
DECLARE_string(htest_internal_run_death_test);
