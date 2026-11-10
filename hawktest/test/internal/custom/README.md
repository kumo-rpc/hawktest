# Customization Points

The custom directory is an injection point for custom user configurations.

## Header `htest.h`

### The following macros can be defined:

*   `HTEST_OS_STACK_TRACE_GETTER_` - The name of an implementation of
    `OsStackTraceGetterInterface`.
*   `HTEST_CUSTOM_TEMPDIR_FUNCTION_` - An override for `hawk::TempDir()`. See
    `hawk::TempDir` for semantics and signature.

## Header `htest-port.h`

The following macros can be defined:

### Logging:

*   `HTEST_LOG_(severity)`
*   `HTEST_CHECK_(condition)`
*   Functions `LogToStderr()` and `FlushInfoLog()` have to be provided too.

### Threading:

*   `HTEST_HAS_NOTIFICATION_` - Enabled if Notification is already provided.
*   `HTEST_HAS_MUTEX_AND_THREAD_LOCAL_` - Enabled if `Mutex` and `ThreadLocal`
    are already provided. Must also provide `HTEST_DECLARE_STATIC_MUTEX_(mutex)`
    and `HTEST_DEFINE_STATIC_MUTEX_(mutex)`
*   `HTEST_EXCLUSIVE_LOCK_REQUIRED_(locks)`
*   `HTEST_LOCK_EXCLUDED_(locks)`

### Underlying library support features

*   `HTEST_HAS_CXXABI_H_`

### Exporting API symbols:

*   `HTEST_API_` - Specifier for exported symbols.

## Header `htest-printers.h`

*   See documentation at `htest/htest-printers.h` for details on how to define a
    custom printer.
