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

# Default exported target (install(EXPORT) requires at least one).
# Teaching demos (foo / xxd / binaries) live in CMakeLists.examples.txt
# and are installed only when generating with -DKMCMAKE_GEN_EXAMPLES=ON.

kmcmake_cc_interface(
        NAMESPACE ${PROJECT_NAME}
        NAME skills
        HEADERS
        skills.h
        version.h
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
)


##################################################
# cmark headers

enable_language(C)

set(_cmark_ver_major "${PROJECT_VERSION_MAJOR}")
set(_cmark_ver_minor "${PROJECT_VERSION_MINOR}")
set(_cmark_ver_patch "${PROJECT_VERSION_PATCH}")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 31)
set(PROJECT_VERSION_PATCH 2)
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmark/cmark_version.h.in
        ${CMAKE_BINARY_DIR}/hawktest/cmark/cmark_version.h
        @ONLY)
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmark/cmark_export.h.in
        ${CMAKE_BINARY_DIR}/hawktest/cmark/cmark_export.h
        @ONLY)
set(PROJECT_VERSION_MAJOR "${_cmark_ver_major}")
set(PROJECT_VERSION_MINOR "${_cmark_ver_minor}")
set(PROJECT_VERSION_PATCH "${_cmark_ver_patch}")

kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME hawktest
        SOURCES
        common/ascii.cc
        common/utf8.cc
        common/filepath.cc
        test/assertion-result.cc
        test/death-test.cc
        test/matchers.cc
        test/port.cc
        test/printers.cc
        test/test-part.cc
        test/typed-test.cc
        test/htest.cc
        mock/cardinalities.cc
        mock/internal-utils.cc
        mock/matchers.cc
        mock/spec-builders.cc
        mock/hmock.cc
        bench/benchmark.cc
        bench/benchmark_api_internal.cc
        bench/benchmark_name.cc
        bench/benchmark_register.cc
        bench/benchmark_runner.cc
        bench/check.cc
        bench/colorprint.cc
        bench/commandlineflags.cc
        bench/complexity.cc
        bench/console_reporter.cc
        bench/counter.cc
        bench/csv_reporter.cc
        bench/json_reporter.cc
        bench/perf_counters.cc
        bench/reporter.cc
        bench/statistics.cc
        bench/string_util.cc
        bench/sysinfo.cc
        bench/timers.cc
        cmark/blocks.c
        cmark/buffer.c
        cmark/cmark.c
        cmark/cmark_ctype.c
        cmark/commonmark.c
        cmark/houdini_href_e.c
        cmark/houdini_html_e.c
        cmark/houdini_html_u.c
        cmark/html.c
        cmark/inlines.c
        cmark/iterator.c
        cmark/latex.c
        cmark/man.c
        cmark/node.c
        cmark/references.c
        cmark/render.c
        cmark/scanners.c
        cmark/utf8.c
        cmark/xml.c
        mark/md_types.cc
        mark/md_compare.cc
        mark/suite.cc
        mark/cmd.cc
        mark/loader.cc
        mark/matcher.cc
        mark/runner.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
        LINKS
        ${KMCMAKE_SYSTEM_DYLINK}
)

kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME htest_main
        SOURCES
        test/main.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
        LINKS
        ${PROJECT_NAME}::hawktest_static
)

kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME hmock_main
        SOURCES
        mock/main.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
        LINKS
        ${PROJECT_NAME}::hawktest_static
)

kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME hbench_main
        SOURCES
        bench/benchmark_main.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
        LINKS
        ${PROJECT_NAME}::hawktest_static
)

################################################
