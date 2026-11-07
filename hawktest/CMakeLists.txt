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

kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME htest
        SOURCES
        test/assertion-result.cc
        test/death-test.cc
        common/filepath.cc
        test/matchers.cc
        test/port.cc
        test/printers.cc
        test/test-part.cc
        test/typed-test.cc
        test/htest.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
        UNITY
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
        hawktest::htest_static
)


kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME hmock
        SOURCES
        mock/cardinalities.cc
        mock/internal-utils.cc
        mock/matchers.cc
        mock/spec-builders.cc
        mock/hmock.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
        UNITY
        LINKS
        hawktest::htest_static
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
        hawktest::hmock_static
)


kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME hbench
        SOURCES
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
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
        UNITY
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
        hawktest::hbench_static
)


kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME hcmark
        SOURCES
        markdown/blocks.cc
        markdown/buffer.cc
        markdown/cmark.cc
        markdown/cmark_ctype.cc
        markdown/commonmark.cc
        markdown/houdini_href_e.cc
        markdown/houdini_html_e.cc
        markdown/houdini_html_u.cc
        markdown/html.cc
        markdown/inlines.cc
        markdown/iterator.cc
        markdown/latex.cc
        markdown/man.cc
        markdown/node.cc
        markdown/references.cc
        markdown/render.cc
        markdown/scanners.cc
        markdown/utf8.cc
        markdown/xml.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
        UNITY
)