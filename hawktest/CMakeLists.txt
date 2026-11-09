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

########################################################
# flags

include(CheckIncludeFileCXX)
include(CheckCXXSymbolExists)
include(CheckTypeSize)

if (WIN32 AND NOT CYGWIN)
    set(OS_WINDOWS 1)
else ()
    set(OS_WINDOWS 0)
endif ()

if (MSVC)
    set(HAVE_SYS_TYPES_H 1)
    set(HAVE_STDDEF_H 1)
    set(HAVE_UNISTD_H 0)
    set(HAVE_SYS_STAT_H 1)
    set(HAVE_SHLWAPI_H 1)
    set(HAVE_STDINT_H 1)
    set(HAVE_INTTYPES_H 1)
    set(HAVE_FNMATCH_H 0)
    set(HAVE_STRTOLL 0)
    set(HAVE_STRTOQ 0)
else ()
    foreach (fname IN ITEMS unistd stdint inttypes sys/types sys/stat fnmatch)
        string(TOUPPER "${fname}" FNAME)
        string(REPLACE "/" "_" FNAME "${FNAME}")
        if (NOT HAVE_${FNAME}_H)
            check_include_file_cxx("${fname}.h" HAVE_${FNAME}_H)
        endif ()
    endforeach ()
    if (NOT HAVE_FNMATCH_H AND OS_WINDOWS)
        check_include_file_cxx("shlwapi.h" HAVE_SHLWAPI_H)
    endif ()
    if (NOT HAVE_SHLWAPI_H)
        set(HAVE_SHLWAPI_H 0)
    endif ()
    check_cxx_symbol_exists(strtoll stdlib.h HAVE_STRTOLL)
    if (NOT HAVE_STRTOLL)
        check_cxx_symbol_exists(strtoq stdlib.h HAVE_STRTOQ)
    endif ()
    if (NOT HAVE_STRTOLL)
        set(HAVE_STRTOLL 0)
    endif ()
    if (NOT HAVE_STRTOQ)
        set(HAVE_STRTOQ 0)
    endif ()
endif ()

foreach (v IN ITEMS
        HAVE_STDINT_H HAVE_SYS_TYPES_H HAVE_INTTYPES_H HAVE_SYS_STAT_H
        HAVE_UNISTD_H HAVE_FNMATCH_H HAVE_SHLWAPI_H HAVE_STRTOLL HAVE_STRTOQ)
    if (${v})
        set(${v} 1)
    else ()
        set(${v} 0)
    endif ()
endforeach ()

set(GFLAGS_INTTYPES_FORMAT_C99 0)
set(GFLAGS_INTTYPES_FORMAT_BSD 0)
set(GFLAGS_INTTYPES_FORMAT_VC7 0)
set(_flags_int_types uint32_t u_int32_t)
if (MSVC)
    list(INSERT _flags_int_types 0 __int32)
endif ()
foreach (type IN LISTS _flags_int_types)
    check_type_size(${type} ${type} LANGUAGE CXX)
    if (HAVE_${type})
        break()
    endif ()
endforeach ()
if (HAVE_uint32_t)
    set(GFLAGS_INTTYPES_FORMAT_C99 1)
elseif (HAVE_u_int32_t)
    set(GFLAGS_INTTYPES_FORMAT_BSD 1)
elseif (HAVE___int32)
    set(GFLAGS_INTTYPES_FORMAT_VC7 1)
else ()
    message(FATAL_ERROR "flags: no 32-bit integer type (uint32_t, u_int32_t, __int32)")
endif ()

if (Threads_FOUND AND CMAKE_USE_PTHREADS_INIT)
    set(HAVE_PTHREAD 1)
    check_type_size(pthread_rwlock_t RWLOCK LANGUAGE CXX)
    if (HAVE_RWLOCK)
        set(HAVE_RWLOCK 1)
    else ()
        set(HAVE_RWLOCK 0)
    endif ()
else ()
    set(HAVE_PTHREAD 0)
    set(HAVE_RWLOCK 0)
endif ()

set(GFLAGS_NAMESPACE htest::flags)
set(INCLUDE_GFLAGS_NS_H "")
set(GFLAGS_IS_A_DLL 0)
if (CMAKE_COMPILER_IS_GNUCXX)
    set(GFLAGS_ATTRIBUTE_UNUSED "__attribute((unused))")
else ()
    set(GFLAGS_ATTRIBUTE_UNUSED "")
endif ()

set(_flags_gen_dir "${CMAKE_CURRENT_SOURCE_DIR}/flags")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/flags/defines.h.in ${_flags_gen_dir}/defines.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/flags/gflags.h.in ${_flags_gen_dir}/gflags.h @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/flags/gflags_declare.h.in ${_flags_gen_dir}/gflags_declare.h @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/flags/gflags_completions.h.in ${_flags_gen_dir}/gflags_completions.h @ONLY)

set(_hawktest_flags_sources
        flags/gflags.cc
        flags/gflags_reporting.cc
        flags/gflags_completions.cc
        flags/builtin.cc
        flags/parser.cc
)
if (OS_WINDOWS)
    list(APPEND _hawktest_flags_sources flags/windows_port.cc)
endif ()

########################################################
kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME hawktest
        SOURCES
        common/ascii.cc
        common/utf8.cc
        common/filepath.cc
        common/random.cc
        common/time.cc
        common/string.cc
        common/colored_printf.cc
        test/assertion-result.cc
        test/death-test.cc
        test/matchers.cc
        test/message.cc
        test/exception.cc
        test/initialize.cc
        test/scoped_trace.cc
        test/listener.cc
        test/internal/streaming_listener.cc
        test/internal/stack.cc
        test/spi.cc
        test/test_suite.cc
        test/internal/unit_test_impl.cc
        test/internal/pretty_unit_test_result_printer.cc
        test/internal/param-util.cc
        test/internal/brief_unit_test_result_printer.cc
        test/internal/json_unit_test_result_printer.cc
        test/internal/xml_unit_test_result_printer.cc
        test/internal/test_event_repeater.cc
        test/internal/unit_test_option.cc
        test/port.cc
        test/test.cc
        test/unit_test.cc
        test/printers.cc
        test/test-part.cc
        test/typed-test.cc
        test/htest.cc
        test/utility.cc
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
        ${_hawktest_flags_sources}
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
