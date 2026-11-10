// Copyright 2008 Google Inc.
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

// Type utilities needed for implementing typed and type-parameterized
// tests.

#pragma once

#include <string>
#include <type_traits>
#include <typeinfo>

#include <hawktest/common/port.h>

// #ifdef __GNUC__ is too general here.  It is possible to use gcc without using
// libstdc++ (which is where cxxabi.h comes from).
#if HTEST_HAS_CXXABI_H_
#include <cxxabi.h>
#elif defined(__HP_aCC)
#include <acxx_demangle.h>
#endif // HTEST_HASH_CXXABI_H_

namespace hawk {
    namespace internal {

        // Canonicalizes a given name with respect to the Standard C++ Library.
        // This handles removing the inline namespace within `std` that is
        // used by various standard libraries (e.g., `std::__1`).  Names outside
        // of namespace std are returned unmodified.
        inline std::string canonicalize_for_std_lib_versioning(std::string s) {
            static const char prefix[] = "std::__";
            if (s.compare(0, strlen(prefix), prefix) == 0) {
                std::string::size_type end = s.find("::", strlen(prefix));
                if (end != s.npos) {
                    // Erase everything between the initial `std` and the second `::`.
                    s.erase(strlen("std"), end - strlen("std"));
                }
            }

            // Strip redundant spaces in typename to match MSVC
            // For example, std::pair<int, bool> -> std::pair<int,bool>
            static const char to_search[] = ", ";
            const char replace_char = ',';
            size_t pos = 0;
            while (true) {
                // Get the next occurrence from the current position
                pos = s.find(to_search, pos);
                if (pos == std::string::npos) {
                    break;
                }
                // Replace this occurrence of substring
                s.replace(pos, strlen(to_search), 1, replace_char);
                ++pos;
            }
            return s;
        }

#if HTEST_HAS_RTTI
        // GetTypeName(const std::type_info&) returns a human-readable name of type T.
        inline std::string GetTypeName(const std::type_info& type) {
            const char* const name = type.name();
#if HTEST_HAS_CXXABI_H_ || defined(__HP_aCC)
            int status = 0;
            // gcc's implementation of typeid(T).name() mangles the type name,
            // so we have to demangle it.
#if HTEST_HAS_CXXABI_H_
            using abi::__cxa_demangle;
#endif // HTEST_HAS_CXXABI_H_
            char* const readable_name = __cxa_demangle(name, nullptr, nullptr, &status);
            const std::string name_str(status == 0 ? readable_name : name);
            free(readable_name);
            return canonicalize_for_std_lib_versioning(name_str);
#elif defined(_MSC_VER)
            // Strip struct and class due to differences between
            // MSVC and other compilers. std::pair<int,bool> is printed as
            // "struct std::pair<int,bool>" when using MSVC vs "std::pair<int, bool>" with
            // other compilers.
            std::string s = name;
            // Only strip the leading "struct " and "class ", so uses rfind == 0 to
            // ensure that
            if (s.rfind("struct ", 0) == 0) {
                s = s.substr(strlen("struct "));
            } else if (s.rfind("class ", 0) == 0) {
                s = s.substr(strlen("class "));
            }
            return s;
#else
            return name;
#endif // HTEST_HAS_CXXABI_H_ || __HP_aCC
        }
#endif // HTEST_HAS_RTTI

        // GetTypeName<T>() returns a human-readable name of type T if and only if
        // RTTI is enabled, otherwise it returns a dummy type name.
        // NB: This function is also used in Google Mock, so don't move it inside of
        // the typed-test-only section below.
        template <typename T>
        std::string GetTypeName() {
#if HTEST_HAS_RTTI
            return GetTypeName(typeid(T));
#else
            return "<type>";
#endif // HTEST_HAS_RTTI
        }

        // A unique type indicating an empty node
        struct None { };

#define HTEST_TEMPLATE_   \
    template <typename T> \
    class

        // The template "selector" struct TemplateSel<Tmpl> is used to
        // represent Tmpl, which must be a class template with one type
        // parameter, as a type.  TemplateSel<Tmpl>::Bind<T>::type is defined
        // as the type Tmpl<T>.  This allows us to actually instantiate the
        // template "selected" by TemplateSel<Tmpl>.
        //
        // This trick is necessary for simulating typedef for class templates,
        // which C++ doesn't support directly.
        template <HTEST_TEMPLATE_ Tmpl>
        struct TemplateSel {
            template <typename T>
            struct Bind {
                typedef Tmpl<T> type;
            };
        };

#define HTEST_BIND_(TmplSel, T) TmplSel::template Bind<T>::type

        template <HTEST_TEMPLATE_ Head_, HTEST_TEMPLATE_... Tail_>
        struct Templates {
            using Head = TemplateSel<Head_>;
            using Tail = Templates<Tail_...>;
        };

        template <HTEST_TEMPLATE_ Head_>
        struct Templates<Head_> {
            using Head = TemplateSel<Head_>;
            using Tail = None;
        };

        // Tuple-like type lists
        template <typename Head_, typename... Tail_>
        struct Types {
            using Head = Head_;
            using Tail = Types<Tail_...>;
        };

        template <typename Head_>
        struct Types<Head_> {
            using Head = Head_;
            using Tail = None;
        };

        // Helper metafunctions to tell apart a single type from types
        // generated by ::hawk::Types
        template <typename... Ts>
        struct ProxyTypeList {
            using type = Types<Ts...>;
        };

        template <typename>
        struct is_proxy_type_list : std::false_type { };

        template <typename... Ts>
        struct is_proxy_type_list<ProxyTypeList<Ts...>> : std::true_type { };

        // Generator which conditionally creates type lists.
        // It recognizes if a requested type list should be created
        // and prevents creating a new type list nested within another one.
        template <typename T>
        struct GenerateTypeList {
        private:
            using proxy = typename std::conditional<is_proxy_type_list<T>::value, T,
                ProxyTypeList<T>>::type;

        public:
            using type = typename proxy::type;
        };

    } // namespace internal

    template <typename... Ts>
    using Types = internal::ProxyTypeList<Ts...>;

} // namespace hawk
