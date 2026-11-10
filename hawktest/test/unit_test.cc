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

#include <hawktest/test/unit_test.h>
#include <hawktest/test/scoped_trace.h>
#include <hawktest/test/exception.h>
#include <hawktest/test/internal/unit_test_impl.h>
#include <hawktest/reporter/constants.h>

// Checks builtin compiler feature |x| while avoiding an extra layer of #ifdefs
// at the callsite.
#if defined(__has_builtin)
#define HTEST_HAS_BUILTIN(x) __has_builtin(x)
#else
#define HTEST_HAS_BUILTIN(x) 0
#endif // defined(__has_builtin)

namespace testing {

    // Destructor of UnitTest.
    UnitTest::~UnitTest() {
        delete impl_;
    }


    // Creates an empty UnitTest.
    UnitTest::UnitTest() {
        impl_ = new internal::UnitTestImpl(this);
    }


    // Pushes a trace defined by SCOPED_TRACE() on to the per-thread
    // Hawk Test trace stack.
    void UnitTest::PushHTestTrace(const internal::TraceInfo& trace)
        HTEST_LOCK_EXCLUDED_(mutex_) {
        internal::MutexLock lock(mutex_);
        impl_->htest_trace_stack().push_back(trace);
    }



    // Pops a trace from the per-thread Hawk Test trace stack.
    void UnitTest::PopHTestTrace() HTEST_LOCK_EXCLUDED_(mutex_) {
        internal::MutexLock lock(mutex_);
        impl_->htest_trace_stack().pop_back();
    }

    // Returns the random seed used at the start of the current test run.
    int UnitTest::random_seed() const {
        return impl_->random_seed();
    }

    // Returns ParameterizedTestSuiteRegistry object used to keep track of
    // value-parameterized tests and instantiate and register them.
    internal::ParameterizedTestSuiteRegistry&
    UnitTest::parameterized_test_registry() HTEST_LOCK_EXCLUDED_(mutex_) {
        return impl_->parameterized_test_registry();
    }

    // Returns the TestInfo object for the test that's currently running,
    // or NULL if no test is running.
    const TestInfo* UnitTest::current_test_info() const
        HTEST_LOCK_EXCLUDED_(mutex_) {
        internal::MutexLock lock(mutex_);
        return impl_->current_test_info();
    }



    // Legacy API is still available but deprecated
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
    const TestCase* UnitTest::current_test_case() const
        HTEST_LOCK_EXCLUDED_(mutex_) {
        internal::MutexLock lock(mutex_);
        return impl_->current_test_suite();
    }
#endif


    // Returns the TestSuite object for the test that's currently running,
    // or NULL if no test is running.
    const TestSuite* UnitTest::current_test_suite() const
        HTEST_LOCK_EXCLUDED_(mutex_) {
        internal::MutexLock lock(mutex_);
        return impl_->current_test_suite();
    }


#if HTEST_HAS_FILE_SYSTEM
    // Returns the working directory when the first TEST() or TEST_F() was
    // executed.
    const char* UnitTest::original_working_dir() const {
        return impl_->original_working_dir_.c_str();
    }
#endif // HTEST_HAS_FILE_SYSTEM


    // Adds a TestProperty to the current TestResult object when invoked from
    // inside a test, to current TestSuite's ad_hoc_test_result_ when invoked
    // from SetUpTestSuite or TearDownTestSuite, or to the global property set
    // when invoked elsewhere.  If the result already contains a property with
    // the same key, the value will be updated.
    void UnitTest::RecordProperty(const std::string& key,
        const std::string& value) {
        impl_->RecordProperty(TestProperty(key, value));
    }


    // Adds a TestPartResult to the current TestResult object.  All Hawk Test
    // assertion macros (e.g. ASSERT_TRUE, EXPECT_EQ, etc) eventually call
    // this to report their results.  The user code should use the
    // assertion macros instead of calling this directly.
    void UnitTest::AddTestPartResult(TestPartResult::Type result_type,
        const std::string_view file_name,
        int line_number, const std::string& message,
        const std::string& os_stack_trace)
        HTEST_LOCK_EXCLUDED_(mutex_) {
        Message msg;
        msg << message;

        internal::MutexLock lock(mutex_);
        if (!impl_->htest_trace_stack().empty()) {
            msg << "\n"
                << HTEST_NAME_ << " trace:";

            for (size_t i = impl_->htest_trace_stack().size(); i > 0; --i) {
                const internal::TraceInfo& trace = impl_->htest_trace_stack()[i - 1];
                msg << "\n"
                    << internal::FormatFileLocation(trace.file, trace.line) << " "
                    << trace.message;
            }
        }

        if (os_stack_trace.c_str() != nullptr && !os_stack_trace.empty()) {
            msg << hawk::Constants::kStackTraceMarker << os_stack_trace;
        } else {
            msg << "\n";
        }

        const TestPartResult result = TestPartResult(
            result_type, file_name, line_number, msg.GetString().c_str());
        impl_->GetTestPartResultReporterForCurrentThread()->ReportTestPartResult(
            result);

        if (result_type != TestPartResult::kSuccess && result_type != TestPartResult::kSkip) {
            // htest_break_on_failure takes precedence over
            // htest_throw_on_failure.  This allows a user to set the latter
            // in the code (perhaps in order to use Hawk Test assertions
            // with another testing framework) and specify the former on the
            // command line for debugging.
            if (FLAGS_htest_break_on_failure) {
#if defined(HTEST_OS_WINDOWS) && !defined(HTEST_OS_WINDOWS_PHONE) && !defined(HTEST_OS_WINDOWS_RT)
                // Using DebugBreak on Windows allows htest to still break into a debugger
                // when a failure happens and both the --htest_break_on_failure and
                // the --htest_catch_exceptions flags are specified.
                DebugBreak();
#elif (!defined(__native_client__)) && ((defined(__clang__) || defined(__GNUC__)) && (defined(__x86_64__) || defined(__i386__)))
                // with clang/gcc we can achieve the same effect on x86 by invoking int3
                asm("int3");
#elif HTEST_HAS_BUILTIN(__builtin_trap)
                __builtin_trap();
#elif defined(SIGTRAP)
                raise(SIGTRAP);
#else
                // Dereference nullptr through a volatile pointer to prevent the compiler
                // from removing. We use this rather than abort() or __builtin_trap() for
                // portability: some debuggers don't correctly trap abort().
                *static_cast<volatile int*>(nullptr) = 1;
#endif // HTEST_OS_WINDOWS
            } else if (FLAGS_htest_throw_on_failure) {
#if HTEST_HAS_EXCEPTIONS
                throw internal::HawkTestFailureException(result);
#else
                // We cannot call abort() as it generates a pop-up in debug mode
                // that cannot be suppressed in VC 7.1 or below.
                exit(1);
#endif
            }
        }
    }


    // Gets the number of successful test suites.
    int UnitTest::successful_test_suite_count() const {
        return impl()->successful_test_suite_count();
    }

    // Gets the number of failed test suites.
    int UnitTest::failed_test_suite_count() const {
        return impl()->failed_test_suite_count();
    }

    // Gets the number of all test suites.
    int UnitTest::total_test_suite_count() const {
        return impl()->total_test_suite_count();
    }


    // Gets the number of all test suites that contain at least one test
    // that should run.
    int UnitTest::test_suite_to_run_count() const {
        return impl()->test_suite_to_run_count();
    }

//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
    int UnitTest::successful_test_case_count() const {
        return impl()->successful_test_suite_count();
    }
    int UnitTest::failed_test_case_count() const {
        return impl()->failed_test_suite_count();
    }
    int UnitTest::total_test_case_count() const {
        return impl()->total_test_suite_count();
    }
    int UnitTest::test_case_to_run_count() const {
        return impl()->test_suite_to_run_count();
    }
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

    // Gets the number of successful tests.
    int UnitTest::successful_test_count() const {
        return impl()->successful_test_count();
    }

    // Gets the number of skipped tests.
    int UnitTest::skipped_test_count() const {
        return impl()->skipped_test_count();
    }

    // Gets the number of failed tests.
    int UnitTest::failed_test_count() const {
        return impl()->failed_test_count();
    }

    // Gets the number of disabled tests that will be reported in the XML report.
    int UnitTest::reportable_disabled_test_count() const {
        return impl()->reportable_disabled_test_count();
    }

    // Gets the number of disabled tests.
    int UnitTest::disabled_test_count() const {
        return impl()->disabled_test_count();
    }

    // Gets the number of tests to be printed in the XML report.
    int UnitTest::reportable_test_count() const {
        return impl()->reportable_test_count();
    }

    // Gets the number of all tests.
    int UnitTest::total_test_count() const {
        return impl()->total_test_count();
    }

    // Gets the number of tests that should run.
    int UnitTest::test_to_run_count() const {
        return impl()->test_to_run_count();
    }

    // Gets the time of the test program start, in ms from the start of the
    // UNIX epoch.
    hawk::TimeInMillis UnitTest::start_timestamp() const {
        return impl()->start_timestamp();
    }

    // Gets the elapsed time, in milliseconds.
    hawk::TimeInMillis UnitTest::elapsed_time() const {
        return impl()->elapsed_time();
    }

    // Returns true if and only if the unit test passed (i.e. all test suites
    // passed).
    bool UnitTest::Passed() const {
        return impl()->Passed();
    }


    // Returns true if and only if the unit test failed (i.e. some test suite
    // failed or something outside of all tests failed).
    bool UnitTest::Failed() const {
        return impl()->Failed();
    }

    // Gets the i-th test suite among all the test suites. i can range from 0 to
    // total_test_suite_count() - 1. If i is not in that range, returns NULL.
    const TestSuite* UnitTest::GetTestSuite(int i) const {
        return impl()->GetTestSuite(i);
    }

//  Legacy API is deprecated but still available
#ifndef HTEST_REMOVE_LEGACY_TEST_CASEAPI_
    const TestCase* UnitTest::GetTestCase(int i) const {
        return impl()->GetTestCase(i);
    }
#endif //  HTEST_REMOVE_LEGACY_TEST_CASEAPI_

    // Returns the TestResult containing information on test failures and
    // properties logged outside of individual test suites.
    const TestResult& UnitTest::ad_hoc_test_result() const {
        return *impl()->ad_hoc_test_result();
    }

    // Gets the i-th test suite among all the test suites. i can range from 0 to
    // total_test_suite_count() - 1. If i is not in that range, returns NULL.
    TestSuite* UnitTest::GetMutableTestSuite(int i) {
        return impl()->GetMutableSuiteCase(i);
    }

    void UnitTest::UponLeavingHTest() {
        impl()->os_stack_trace_getter()->UponLeavingHTest();
    }

    // Sets the TestSuite object for the test that's currently running.
    void UnitTest::set_current_test_suite(TestSuite* a_current_test_suite) {
        internal::MutexLock lock(mutex_);
        impl_->set_current_test_suite(a_current_test_suite);
    }

    // Sets the TestInfo object for the test that's currently running.
    void UnitTest::set_current_test_info(TestInfo* a_current_test_info) {
        internal::MutexLock lock(mutex_);
        impl_->set_current_test_info(a_current_test_info);
    }

    // Returns the list of event listeners that can be used to track events
    // inside Hawk Test.
    TestEventListeners& UnitTest::listeners() {
        return *impl()->listeners();
    }



    // Registers and returns a global test environment.  When a test
    // program is run, all global test environments will be set-up in the
    // order they were registered.  After all tests in the program have
    // finished, all global test environments will be torn-down in the
    // *reverse* order they were registered.
    //
    // The UnitTest object takes ownership of the given environment.
    //
    // We don't protect this under mutex_, as we only support calling it
    // from the main thread.
    Environment* UnitTest::AddEnvironment(Environment* env) {
        if (env == nullptr) {
            return nullptr;
        }

        impl_->environments().push_back(env);
        return env;
    }



    // Gets the singleton UnitTest object.  The first time this method is
    // called, a UnitTest object is constructed and returned.  Consecutive
    // calls will return the same object.
    //
    // We don't protect this under mutex_ as a user is not supposed to
    // call this before main() starts, from which point on the return
    // value will never change.
    UnitTest* UnitTest::GetInstance() {
        // CodeGear C++Builder insists on a public destructor for the
        // default implementation.  Use this implementation to keep good OO
        // design with private destructor.

#if defined(__BORLANDC__)
        static UnitTest* const instance = new UnitTest;
        return instance;
#else
        static UnitTest instance;
        return &instance;
#endif // defined(__BORLANDC__)
    }


    namespace internal {
#ifdef HTEST_INTERNAL_HAS_PREMATURE_EXIT_FILE
        // A helper class that creates the premature-exit file in its
        // constructor and deletes the file in its destructor.
        class ScopedPrematureExitFile {
        public:
            explicit ScopedPrematureExitFile(const char* premature_exit_filepath)
                : premature_exit_filepath_(
                      premature_exit_filepath ? premature_exit_filepath : "") {
                // If a path to the premature-exit file is specified...
                if (!premature_exit_filepath_.empty()) {
                    // create the file with a single "0" character in it.  I/O
                    // errors are ignored as there's nothing better we can do and we
                    // don't want to fail the test because of this.
                    if (FILE* pfile = posix::FOpen(premature_exit_filepath_.c_str(), "w")) {
                        fwrite("0", 1, 1, pfile);
                        fclose(pfile);
                    } else {
                        premature_exit_filepath_.clear();
                    }
                }
            }

            ~ScopedPrematureExitFile() {
#ifndef HTEST_OS_ESP8266
                if (!premature_exit_filepath_.empty()) {
                    int retval = remove(premature_exit_filepath_.c_str());
                    if (retval) {
                        HTEST_LOG_(ERROR) << "Failed to remove premature exit filepath \""
                                          << premature_exit_filepath_ << "\" with error "
                                          << retval;
                    }
                }
#endif
            }

        private:
            std::string premature_exit_filepath_;

            ScopedPrematureExitFile(const ScopedPrematureExitFile&) = delete;
            ScopedPrematureExitFile& operator=(const ScopedPrematureExitFile&) = delete;
        };
#endif // HTEST_INTERNAL_HAS_PREMATURE_EXIT_FILE
    }
    // Runs all tests in this UnitTest object and prints the result.
    // Returns 0 if successful, or 1 otherwise.
    //
    // We don't protect this under mutex_, as we only support calling it
    // from the main thread.
    int UnitTest::Run() {
#ifdef HTEST_INTERNAL_HAS_PREMATURE_EXIT_FILE
        const bool in_death_test_child_process = !FLAGS_htest_internal_run_death_test.empty();

        // Hawk Test implements this protocol for catching that a test
        // program exits before returning control to Hawk Test:
        //
        //   1. Upon start, Hawk Test creates a file whose absolute path
        //      is specified by the environment variable
        //      TEST_PREMATURE_EXIT_FILE.
        //   2. When Hawk Test has finished its work, it deletes the file.
        //
        // This allows a test runner to set TEST_PREMATURE_EXIT_FILE before
        // running a Google-Test-based test program and check the existence
        // of the file at the end of the test execution to see if it has
        // exited prematurely.

        // If we are in the child process of a death test, don't
        // create/delete the premature exit file, as doing so is unnecessary
        // and will confuse the parent process.  Otherwise, create/delete
        // the file upon entering/leaving this function.  If the program
        // somehow exits before this function has a chance to return, the
        // premature-exit file will be left undeleted, causing a test runner
        // that understands the premature-exit-file protocol to report the
        // test as having failed.
        const internal::ScopedPrematureExitFile premature_exit_file(
            in_death_test_child_process
                ? nullptr
                : internal::posix::GetEnv("TEST_PREMATURE_EXIT_FILE"));
#else
        const bool in_death_test_child_process = false;
#endif // HTEST_INTERNAL_HAS_PREMATURE_EXIT_FILE

        // Captures the value of FLAGS_htest_catch_exceptions.  This value will be
        // used for the duration of the program.
        impl()->set_catch_exceptions(FLAGS_htest_catch_exceptions);

#ifdef HTEST_OS_WINDOWS
        // Either the user wants Hawk Test to catch exceptions thrown by the
        // tests or this is executing in the context of death test child
        // process. In either case the user does not want to see pop-up dialogs
        // about crashes - they are expected.
        if (impl()->catch_exceptions() || in_death_test_child_process) {
#if !defined(HTEST_OS_WINDOWS_MOBILE) && !defined(HTEST_OS_WINDOWS_PHONE) && !defined(HTEST_OS_WINDOWS_RT) && !defined(HTEST_OS_WINDOWS_GAMES)
            // SetErrorMode doesn't exist on CE.
            SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#endif // !HTEST_OS_WINDOWS_MOBILE

#if (defined(_MSC_VER) || defined(HTEST_OS_WINDOWS_MINGW)) && !defined(HTEST_OS_WINDOWS_MOBILE)
            // Death test children can be terminated with _abort().  On Windows,
            // _abort() can show a dialog with a warning message.  This forces the
            // abort message to go to stderr instead.
            _set_error_mode(_OUT_TO_STDERR);
#endif

#if defined(_MSC_VER) && !defined(HTEST_OS_WINDOWS_MOBILE)
            // In the debug version, Visual Studio pops up a separate dialog
            // offering a choice to debug the aborted program. We need to suppress
            // this dialog or it will pop up for every EXPECT/ASSERT_DEATH statement
            // executed. Hawk Test will notify the user of any unexpected
            // failure via stderr.
            if (!FLAGS_htest_break_on_failure)
                _set_abort_behavior(
                    0x0, // Clear the following flags:
                    _WRITE_ABORT_MSG | _CALL_REPORTFAULT); // pop-up window, core dump.

            // In debug mode, the Windows CRT can crash with an assertion over invalid
            // input (e.g. passing an invalid file descriptor).  The default handling
            // for these assertions is to pop up a dialog and wait for user input.
            // Instead ask the CRT to dump such assertions to stderr non-interactively.
            if (!IsDebuggerPresent()) {
                (void)_CrtSetReportMode(_CRT_ASSERT,
                    _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
                (void)_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
            }
#endif
        }
#else
        (void)in_death_test_child_process; // Needed inside the #if block above
#endif // HTEST_OS_WINDOWS

        return internal::HandleExceptionsInMethodIfSupported(
                   impl(), &internal::UnitTestImpl::RunAllTests,
                   "auxiliary test code (environments or event listeners)")
            ? 0
            : 1;
    }
}