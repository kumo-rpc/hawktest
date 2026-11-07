# Copyright (C) Kumo inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Threads and system deps are resolved by cmake/hawktest_deps.cmake
# into KMCMAKE_DEPS_LINK. Do not find_package here.
#
# Entry patterns (port remaining entries from cmake_old.cmake):
#   with main    -> LINKS hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
#   without main -> LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
#   htest test   -> kmcmake_cc_test(MODULE bm) + hmock_main_static + hbench_static

kmcmake_cc_test(
        NAME basic_test
        MODULE bm
        SOURCES basic_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME basic_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME link_main_test
        MODULE bm
        SOURCES link_main_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME link_main_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

# Entries without their own main (originally compile_benchmark_test,
# linked against hbench_static)

kmcmake_cc_test(
        NAME benchmark_test
        MODULE bm
        SOURCES benchmark_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME benchmark_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME spec_arg_test
        MODULE bm
        SOURCES spec_arg_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME spec_arg_test
        MODULE bm
        ARGS --benchmark_filter=BM_NotChosen)

kmcmake_cc_test(
        NAME spec_arg_verbosity_test
        MODULE bm
        SOURCES spec_arg_verbosity_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME spec_arg_verbosity_test
        MODULE bm
        ARGS --v=42)

kmcmake_cc_test(
        NAME benchmark_setup_teardown_test
        MODULE bm
        SOURCES benchmark_setup_teardown_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME filter_test
        MODULE bm
        SOURCES filter_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)
# filter_test: 16 filters x 2 (run + list_only) ext registrations below

kmcmake_cc_test(
        NAME benchmark_min_time_flag_time_test
        MODULE bm
        SOURCES benchmark_min_time_flag_time_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME benchmark_min_time_flag_iters_test
        MODULE bm
        SOURCES benchmark_min_time_flag_iters_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME options_test
        MODULE bm
        SOURCES options_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME options_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME skip_with_error_test
        MODULE bm
        SOURCES skip_with_error_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME skip_with_error_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME donotoptimize_test
        MODULE bm
        SOURCES donotoptimize_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME donotoptimize_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

# donotoptimize_test extra flags (from cmake_old.cmake):
# error on deprecated declarations + force -O3 (issues only show with opt).
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Werror=deprecated-declarations HAWK_HAS_DEPRECATED_DECLARATIONS_FLAG)
if (HAWK_HAS_DEPRECATED_DECLARATIONS_FLAG)
    target_compile_options(bm_donotoptimize_test PRIVATE "-Werror=deprecated-declarations")
endif ()
check_cxx_compiler_flag(-O3 HAWK_HAS_O3_FLAG)
if (HAWK_HAS_O3_FLAG)
    set_target_properties(bm_donotoptimize_test PROPERTIES COMPILE_FLAGS "-O3")
endif ()

kmcmake_cc_test(
        NAME fixture_test
        MODULE bm
        SOURCES fixture_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME fixture_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME register_benchmark_test
        MODULE bm
        SOURCES register_benchmark_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME register_benchmark_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME map_test
        MODULE bm
        SOURCES map_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME map_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME multiple_ranges_test
        MODULE bm
        SOURCES multiple_ranges_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME multiple_ranges_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME args_product_test
        MODULE bm
        SOURCES args_product_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME args_product_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME profiler_manager_iterations_test
        MODULE bm
        SOURCES profiler_manager_iterations_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME profiler_manager_state_test
        MODULE bm
        SOURCES profiler_manager_state_test.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

# ---------------------------------------------------------------------------
# output_test_helper: static lib used by output tests (originally
# add_library(output_test_helper STATIC output_test_helper.cc output_test.h))
# No PUBLIC -> internal target, not installed. Default namespace is
# hawktest:: so tests link hawktest::output_test_helper_static.
# ---------------------------------------------------------------------------

kmcmake_cc_library(
        NAME output_test_helper
        SOURCES output_test_helper.cc
        HEADERS output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        PLINKS hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

# ---------------------------------------------------------------------------
# Output tests (originally compile_output_test: link output_test_helper +
# benchmark_main). All are EXT entries; run args copied from cmake_old.cmake.
# ---------------------------------------------------------------------------

kmcmake_cc_test(
        NAME repetitions_test
        MODULE bm
        SOURCES repetitions_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME repetitions_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s --benchmark_repetitions=3)

kmcmake_cc_test(
        NAME reporter_output_test
        MODULE bm
        SOURCES reporter_output_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME reporter_output_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME templated_fixture_test
        MODULE bm
        SOURCES templated_fixture_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME templated_fixture_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME templated_fixture_method_test
        MODULE bm
        SOURCES templated_fixture_method_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME templated_fixture_method_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME user_counters_test
        MODULE bm
        SOURCES user_counters_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME user_counters_test
        MODULE bm
        ARGS --benchmark_min_time=0.2s)

kmcmake_cc_test(
        NAME user_counters_threads_test
        MODULE bm
        SOURCES user_counters_threads_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME user_counters_threads_test
        MODULE bm
        ARGS --benchmark_min_time=0.2s)

kmcmake_cc_test(
        NAME perf_counters_test
        MODULE bm
        SOURCES perf_counters_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME perf_counters_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s --benchmark_perf_counters=CYCLES,INSTRUCTIONS)

kmcmake_cc_test(
        NAME internal_threading_test
        MODULE bm
        SOURCES internal_threading_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME internal_threading_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME manual_threading_test
        MODULE bm
        SOURCES manual_threading_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME manual_threading_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME report_aggregates_only_test
        MODULE bm
        SOURCES report_aggregates_only_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME report_aggregates_only_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME display_aggregates_only_test
        MODULE bm
        SOURCES display_aggregates_only_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME display_aggregates_only_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME user_counters_tabular_test
        MODULE bm
        SOURCES user_counters_tabular_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME user_counters_tabular_test
        MODULE bm
        ARGS --benchmark_counters_tabular=true --benchmark_min_time=0.2s)

kmcmake_cc_test(
        NAME user_counters_thousands_test
        MODULE bm
        SOURCES user_counters_thousands_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME user_counters_thousands_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME memory_manager_test
        MODULE bm
        SOURCES memory_manager_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME memory_manager_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME profiler_manager_test
        MODULE bm
        SOURCES profiler_manager_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME profiler_manager_test
        MODULE bm
        ARGS --benchmark_min_time=0.01s)

kmcmake_cc_test(
        NAME complexity_test
        MODULE bm
        SOURCES complexity_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK}
        EXT)

kmcmake_cc_test_ext(
        NAME complexity_test
        MODULE bm
        ARGS --benchmark_min_time=1000000x)

kmcmake_cc_test(
        NAME locale_impermeability_test
        MODULE bm
        SOURCES locale_impermeability_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME scoped_pause_test
        MODULE bm
        SOURCES scoped_pause_test.cc output_test.h
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::output_test_helper_static hawktest::hbench_main_static ${KMCMAKE_DEPS_LINK})

# ---------------------------------------------------------------------------
# filter_test parameterized registrations (originally add_filter_test):
# for each filter, one run entry + one --benchmark_list_tests entry.
# Both reuse the bm_filter_test target built above via EXT.
# ---------------------------------------------------------------------------

macro(add_hawk_filter_test name filter expect)
    kmcmake_cc_test_ext(
            NAME filter_test
            MODULE bm
            ALIAS ${name}
            ARGS --benchmark_min_time=0.01s --benchmark_filter=${filter} ${expect})
    kmcmake_cc_test_ext(
            NAME filter_test
            MODULE bm
            ALIAS ${name}_list_only
            ARGS --benchmark_list_tests --benchmark_filter=${filter} ${expect})
endmacro()

add_hawk_filter_test(filter_simple "Foo" 3)
add_hawk_filter_test(filter_simple_negative "-Foo" 2)
add_hawk_filter_test(filter_suffix "BM_.*" 4)
add_hawk_filter_test(filter_suffix_negative "-BM_.*" 1)
add_hawk_filter_test(filter_regex_all ".*" 5)
add_hawk_filter_test(filter_regex_all_negative "-.*" 0)
add_hawk_filter_test(filter_regex_blank "" 5)
add_hawk_filter_test(filter_regex_blank_negative "-" 0)
add_hawk_filter_test(filter_regex_none "monkey" 0)
add_hawk_filter_test(filter_regex_none_negative "-monkey" 5)
add_hawk_filter_test(filter_regex_wildcard ".*Foo.*" 3)
add_hawk_filter_test(filter_regex_wildcard_negative "-.*Foo.*" 2)
add_hawk_filter_test(filter_regex_begin "^BM_.*" 4)
add_hawk_filter_test(filter_regex_begin_negative "-^BM_.*" 1)
add_hawk_filter_test(filter_regex_begin2 "^N" 1)
add_hawk_filter_test(filter_regex_begin2_negative "-^N" 4)
add_hawk_filter_test(filter_regex_end ".*Ba$" 1)
add_hawk_filter_test(filter_regex_end_negative "-.*Ba$" 4)

# ---------------------------------------------------------------------------
# HawkTest unit tests (originally add_htest under
# BENCHMARK_ENABLE_HTEST_TESTS). gmock_main -> hawktest::hmock_main_static,
# benchmark::benchmark -> hawktest::hbench_static.
# ---------------------------------------------------------------------------

kmcmake_cc_test(
        NAME benchmark_htest
        MODULE bm
        SOURCES benchmark_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME benchmark_name_htest
        MODULE bm
        SOURCES benchmark_name_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME benchmark_random_interleaving_htest
        MODULE bm
        SOURCES benchmark_random_interleaving_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME commandlineflags_htest
        MODULE bm
        SOURCES commandlineflags_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME statistics_htest
        MODULE bm
        SOURCES statistics_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME string_util_htest
        MODULE bm
        SOURCES string_util_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME perf_counters_htest
        MODULE bm
        SOURCES perf_counters_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME reporter_list_htest
        MODULE bm
        SOURCES reporter_list_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME time_unit_htest
        MODULE bm
        SOURCES time_unit_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME min_time_parse_htest
        MODULE bm
        SOURCES min_time_parse_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME profiler_manager_htest
        MODULE bm
        SOURCES profiler_manager_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME benchmark_setup_teardown_cb_types_htest
        MODULE bm
        SOURCES benchmark_setup_teardown_cb_types_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME memory_results_htest
        MODULE bm
        SOURCES memory_results_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

kmcmake_cc_test(
        NAME memory_manager_ordering_htest
        MODULE bm
        SOURCES memory_manager_ordering_htest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS hawktest::hmock_main_static hawktest::hbench_static ${KMCMAKE_DEPS_LINK})

