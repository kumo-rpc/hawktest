//
// Created by cyclone on 2026/11/11.
//

#include <hawktest/numeric/parse.h>

#include <cstdio>
#include <cstdlib>

namespace hawk {
    namespace {

        template<typename T>
        T get_numeric_or_die(const char* var, T default_val) {
            const std::optional<std::string> str_val = get_env(var);
            if (!str_val.has_value()) {
                return default_val;
            }

            Message err;
            const std::optional<T> parsed = parse_numeric<T>(*str_val, err);
            if (!parsed.has_value()) {
                Message msg;
                msg << "WARNING: The value of environment variable " << var
                    << " is expected to be a number, but actually has value \""
                    << *str_val << "\".\n";
                printf("%s", msg.GetString().c_str());
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
            return *parsed;
        }

    } // namespace

    std::int8_t get_int8_from_env_or_die(const char* var, std::int8_t default_val) {
        return get_numeric_or_die(var, default_val);
    }
    std::int16_t get_int16_from_env_or_die(const char* var, std::int16_t default_val) {
        return get_numeric_or_die(var, default_val);
    }
    std::int32_t get_int32_from_env_or_die(const char* var, std::int32_t default_val) {
        return get_numeric_or_die(var, default_val);
    }
    std::int64_t get_int64_from_env_or_die(const char* var, std::int64_t default_val) {
        return get_numeric_or_die(var, default_val);
    }
    std::uint8_t get_uint8_from_env_or_die(const char* var, std::uint8_t default_val) {
        return get_numeric_or_die(var, default_val);
    }
    std::uint16_t get_uint16_from_env_or_die(const char* var, std::uint16_t default_val) {
        return get_numeric_or_die(var, default_val);
    }
    std::uint32_t get_uint32_from_env_or_die(const char* var, std::uint32_t default_val) {
        return get_numeric_or_die(var, default_val);
    }
    std::uint64_t get_uint64_from_env_or_die(const char* var, std::uint64_t default_val) {
        return get_numeric_or_die(var, default_val);
    }
    float get_float_from_env_or_die(const char* var, float default_val) {
        return get_numeric_or_die(var, default_val);
    }
    double get_double_from_env_or_die(const char* var, double default_val) {
        return get_numeric_or_die(var, default_val);
    }

} // namespace hawk
