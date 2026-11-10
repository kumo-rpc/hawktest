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

#include <hawktest/common/environment.h>
#include <hawktest/test/internal/unit_test_impl.h>
#include <hawktest/format/printer.h>
#include <hawktest/test/internal/unit_test_option.h>
#include <hawktest/reporter/pretty_unit_test_result_printer.h>
#include <hawktest/reporter/xml_unit_test_result_printer.h>
#include <hawktest/reporter/json_unit_test_result_printer.h>
#include <hawktest/reporter/brief_unit_test_result_printer.h>
#include <hawktest/reporter/streaming_listener.h>
#include <hawktest/reporter/constants.h>
#include <hawktest/common/internal-inl.h>
#include <hawktest/flags/parser.h>

namespace testing {
    namespace internal {


        UnitTestImpl::UnitTestImpl(UnitTest* parent)
            : parent_(parent)
            , HTEST_DISABLE_MSC_WARNINGS_PUSH_(4355 /* using this in initializer */)
                  default_global_test_part_result_reporter_(this)
            , default_per_thread_test_part_result_reporter_(this)
            , HTEST_DISABLE_MSC_WARNINGS_POP_() global_test_part_result_reporter_(
                  &default_global_test_part_result_reporter_)
            , per_thread_test_part_result_reporter_(
                  &default_per_thread_test_part_result_reporter_)
            , parameterized_test_registry_()
            , parameterized_tests_registered_(false)
            , last_death_test_suite_(-1)
            , current_test_suite_(nullptr)
            , current_test_info_(nullptr)
            , ad_hoc_test_result_()
            , os_stack_trace_getter_(nullptr)
            , post_flag_parse_init_performed_(false)
            , random_seed_(0)
            , // Will be overridden by the flag before first use.
            random_(0)
            , // Will be reseeded before first use.
            start_timestamp_(0)
            , elapsed_time_(0)
            ,
#ifdef HTEST_HAS_DEATH_TEST
            death_test_factory_(new DefaultDeathTestFactory)
            ,
#endif
            // Will be overridden by the flag before first use.
            catch_exceptions_(false) {
            listeners()->SetDefaultResultPrinter(new PrettyUnitTestResultPrinter);
        }


        UnitTestImpl::~UnitTestImpl() {
            // Deletes every TestSuite.
            ForEach(test_suites_, internal::Delete<TestSuite>);

            // Deletes every Environment.
            ForEach(environments_, internal::Delete<Environment>);

            delete os_stack_trace_getter_;
        }

        // Adds a TestProperty to the current TestResult object when invoked in a
        // context of a test, to current test suite's ad_hoc_test_result when invoke
        // from SetUpTestSuite/TearDownTestSuite, or to the global property set
        // otherwise.  If the result already contains a property with the same key,
        // the value will be updated.
        void UnitTestImpl::RecordProperty(const TestProperty& test_property) {
            std::string xml_element;
            TestResult* test_result; // TestResult appropriate for property recording.

            if (current_test_info_ != nullptr) {
                xml_element = "testcase";
                test_result = &(current_test_info_->result_);
            } else if (current_test_suite_ != nullptr) {
                xml_element = "testsuite";
                test_result = &(current_test_suite_->ad_hoc_test_result_);
            } else {
                xml_element = "testsuites";
                test_result = &ad_hoc_test_result_;
            }
            test_result->RecordProperty(xml_element, test_property);
        }

#ifdef HTEST_HAS_DEATH_TEST
        // Disables event forwarding if the control is currently in a death test
        // subprocess. Must not be called before init_hawk_test.
        void UnitTestImpl::SuppressTestEventsIfInSubprocess() {
            if (internal_run_death_test_flag_ != nullptr)
                listeners()->SuppressEventForwarding(true);
        }
#endif // HTEST_HAS_DEATH_TEST


        // Initializes event listeners performing XML output as specified by
        // UnitTestOptions. Must not be called before init_hawk_test.
        void UnitTestImpl::ConfigureXmlOutput() {
            const std::string& output_format = UnitTestOptions::GetOutputFormat();
#if HTEST_HAS_FILE_SYSTEM
            if (output_format == "xml") {
                listeners()->SetDefaultXmlGenerator(new XmlUnitTestResultPrinter(
                    UnitTestOptions::GetAbsolutePathToOutputFile().c_str()));
            } else if (output_format == "json") {
                listeners()->SetDefaultXmlGenerator(new JsonUnitTestResultPrinter(
                    UnitTestOptions::GetAbsolutePathToOutputFile().c_str()));
            } else if (!output_format.empty()) {
                HTEST_LOG_(WARNING) << "WARNING: unrecognized output format \""
                                    << output_format << "\" ignored.";
            }
#else
            if (!output_format.empty()) {
                HTEST_LOG_(ERROR) << "ERROR: alternative output formats require "
                                  << "HTEST_HAS_FILE_SYSTEM to be enabled";
            }
#endif // HTEST_HAS_FILE_SYSTEM
        }

#if HTEST_CAN_STREAM_RESULTS_
        // Initializes event listeners for streaming test results in string form.
        // Must not be called before init_hawk_test.
        void UnitTestImpl::ConfigureStreamingOutput() {
            const std::string& target = FLAGS_htest_stream_result_to;
            if (!target.empty()) {
                const size_t pos = target.find(':');
                if (pos != std::string::npos) {
                    listeners()->Append(
                        new StreamingListener(target.substr(0, pos), target.substr(pos + 1)));
                } else {
                    HTEST_LOG_(WARNING) << "unrecognized streaming target \"" << target
                                        << "\" ignored.";
                }
            }
        }
#endif // HTEST_CAN_STREAM_RESULTS_

          // Returns the global test part result reporter.
        TestPartResultReporterInterface*
        UnitTestImpl::GetGlobalTestPartResultReporter() {
            internal::MutexLock lock(global_test_part_result_reporter_mutex_);
            return global_test_part_result_reporter_;
        }

        // Sets the global test part result reporter.
        void UnitTestImpl::SetGlobalTestPartResultReporter(
            TestPartResultReporterInterface* reporter) {
            internal::MutexLock lock(global_test_part_result_reporter_mutex_);
            global_test_part_result_reporter_ = reporter;
        }

        // Returns the test part result reporter for the current thread.
        TestPartResultReporterInterface*
        UnitTestImpl::GetTestPartResultReporterForCurrentThread() {
            return per_thread_test_part_result_reporter_.get();
        }

        // Sets the test part result reporter for the current thread.
        void UnitTestImpl::SetTestPartResultReporterForCurrentThread(
            TestPartResultReporterInterface* reporter) {
            per_thread_test_part_result_reporter_.set(reporter);
        }


        // Iterates over a vector of TestSuites, keeping a running sum of the
        // results of calling a given int-returning method on each.
        // Returns the sum.
        static int SumOverTestSuiteList(const std::vector<TestSuite*>& case_list,
            int (TestSuite::*method)() const) {
            int sum = 0;
            for (size_t i = 0; i < case_list.size(); i++) {
                sum += (case_list[i]->*method)();
            }
            return sum;
        }

        // Returns true if and only if the test suite passed.
        static bool TestSuitePassed(const TestSuite* test_suite) {
            return test_suite->should_run() && test_suite->Passed();
        }


        // Returns true if and only if test_suite contains at least one test that
        // should run.
        static bool ShouldRunTestSuite(const TestSuite* test_suite) {
            return test_suite->should_run();
        }

        // Returns true if and only if the test suite failed.
        static bool TestSuiteFailed(const TestSuite* test_suite) {
            return test_suite->should_run() && test_suite->Failed();
        }

        // Gets the number of successful test suites.
        int UnitTestImpl::successful_test_suite_count() const {
            return CountIf(test_suites_, TestSuitePassed);
        }

        // Gets the number of failed test suites.
        int UnitTestImpl::failed_test_suite_count() const {
            return CountIf(test_suites_, TestSuiteFailed);
        }

        // Gets the number of all test suites.
        int UnitTestImpl::total_test_suite_count() const {
            return static_cast<int>(test_suites_.size());
        }

        // Gets the number of all test suites that contain at least one test
        // that should run.
        int UnitTestImpl::test_suite_to_run_count() const {
            return CountIf(test_suites_, ShouldRunTestSuite);
        }

        // Gets the number of successful tests.
        int UnitTestImpl::successful_test_count() const {
            return SumOverTestSuiteList(test_suites_, &TestSuite::successful_test_count);
        }

        // Gets the number of skipped tests.
        int UnitTestImpl::skipped_test_count() const {
            return SumOverTestSuiteList(test_suites_, &TestSuite::skipped_test_count);
        }

        // Gets the number of failed tests.
        int UnitTestImpl::failed_test_count() const {
            return SumOverTestSuiteList(test_suites_, &TestSuite::failed_test_count);
        }

        // Gets the number of disabled tests that will be reported in the XML report.
        int UnitTestImpl::reportable_disabled_test_count() const {
            return SumOverTestSuiteList(test_suites_,
                &TestSuite::reportable_disabled_test_count);
        }

        // Gets the number of disabled tests.
        int UnitTestImpl::disabled_test_count() const {
            return SumOverTestSuiteList(test_suites_, &TestSuite::disabled_test_count);
        }

        // Gets the number of tests to be printed in the XML report.
        int UnitTestImpl::reportable_test_count() const {
            return SumOverTestSuiteList(test_suites_, &TestSuite::reportable_test_count);
        }

        // Gets the number of all tests.
        int UnitTestImpl::total_test_count() const {
            return SumOverTestSuiteList(test_suites_, &TestSuite::total_test_count);
        }

        // Gets the number of tests that should run.
        int UnitTestImpl::test_to_run_count() const {
            return SumOverTestSuiteList(test_suites_, &TestSuite::test_to_run_count);
        }


        // Returns the current OS stack trace as an std::string.
        //
        // The maximum number of stack frames to be included is specified by
        // the htest_stack_trace_depth flag.  The skip_count parameter
        // specifies the number of top frames to be skipped, which doesn't
        // count against the number of frames to be included.
        //
        // For example, if Foo() calls Bar(), which in turn calls
        // CurrentOsStackTraceExceptTop(1), Foo() will be included in the
        // trace but Bar() and CurrentOsStackTraceExceptTop() won't.
        std::string UnitTestImpl::CurrentOsStackTraceExceptTop(int skip_count) {
            return os_stack_trace_getter()->CurrentStackTrace(
                FLAGS_htest_stack_trace_depth, skip_count + 1
                // Skips the user-specified number of frames plus this function
                // itself.
            ); // NOLINT
        }

        // This method expands all parameterized tests registered with macros TEST_P
        // and INSTANTIATE_TEST_SUITE_P into regular tests and registers those.
        // This will be done just once during the program runtime.
        void UnitTestImpl::RegisterParameterizedTests() {
            if (!parameterized_tests_registered_) {
                parameterized_test_registry_.RegisterTests();
                type_parameterized_test_registry_.CheckForInstantiations();
                parameterized_tests_registered_ = true;
            }
        }


        // Performs initialization dependent upon flag values obtained in
        // parse_hawk_test_flags_only.  Is called from init_hawk_test after the call to
        // parse_hawk_test_flags_only.  In case a user neglects to call init_hawk_test
        // this function is also called from RunAllTests.  Since this function can be
        // called more than once, it has to be idempotent.
        void UnitTestImpl::PostFlagParsingInit() {
            // Ensures that this function does not execute more than once.
            if (!post_flag_parse_init_performed_) {
                post_flag_parse_init_performed_ = true;

#if defined(HTEST_CUSTOM_TEST_EVENT_LISTENER_)
                // Register to send notifications about key process state changes.
                listeners()->Append(new HTEST_CUSTOM_TEST_EVENT_LISTENER_());
#endif // defined(HTEST_CUSTOM_TEST_EVENT_LISTENER_)

#ifdef HTEST_HAS_DEATH_TEST
                InitDeathTestSubprocessControlInfo();
                SuppressTestEventsIfInSubprocess();
#endif // HTEST_HAS_DEATH_TEST

                // Registers parameterized tests. This makes parameterized tests
                // available to the UnitTest reflection API without running
                // RUN_ALL_TESTS.
                RegisterParameterizedTests();

                // Configures listeners for XML output. This makes it possible for users
                // to shut down the default XML output before invoking RUN_ALL_TESTS.
                ConfigureXmlOutput();

                if (FLAGS_htest_brief) {
                    listeners()->SetDefaultResultPrinter(new BriefUnitTestResultPrinter);
                }

#if HTEST_CAN_STREAM_RESULTS_
                // Configures listeners for streaming test results to the specified server.
                ConfigureStreamingOutput();
#endif // HTEST_CAN_STREAM_RESULTS_
            }
        }


        // Finds and returns a TestSuite with the given name.  If one doesn't
        // exist, creates one and returns it.  It's the CALLER'S
        // RESPONSIBILITY to ensure that this function is only called WHEN THE
        // TESTS ARE NOT SHUFFLED.
        //
        // Arguments:
        //
        //   test_suite_name: name of the test suite
        //   type_param:      the name of the test suite's type parameter, or NULL if
        //                    this is not a typed or a type-parameterized test suite.
        //   set_up_tc:       pointer to the function that sets up the test suite
        //   tear_down_tc:    pointer to the function that tears down the test suite
        TestSuite* UnitTestImpl::GetTestSuite(
            const std::string& test_suite_name, const char* type_param,
            internal::SetUpTestSuiteFunc set_up_tc,
            internal::TearDownTestSuiteFunc tear_down_tc) {
            // During initialization, all TestInfos for a given suite are added in
            // sequence. To optimize this case, see if the most recently added suite is
            // the one being requested now.
            if (!test_suites_.empty() && (*test_suites_.rbegin())->name_ == test_suite_name) {
                return *test_suites_.rbegin();
            }

            // Fall back to searching the collection.
            auto item_it = test_suites_by_name_.find(test_suite_name);
            if (item_it != test_suites_by_name_.end()) {
                return item_it->second;
            }

            // Not found. Create a new instance.
            auto* const new_test_suite = new TestSuite(test_suite_name, type_param, set_up_tc, tear_down_tc);
            test_suites_by_name_.emplace(test_suite_name, new_test_suite);

            const UnitTestFilter death_test_suite_filter{
                std::string(hawk::Constants::kDeathTestSuiteFilter)};
            // Is this a death test suite?
            if (death_test_suite_filter.MatchesName(test_suite_name)) {
                // Yes.  Inserts the test suite after the last death test suite
                // defined so far.  This only works when the test suites haven't
                // been shuffled.  Otherwise we may end up running a death test
                // after a non-death test.
                ++last_death_test_suite_;
                test_suites_.insert(test_suites_.begin() + last_death_test_suite_,
                    new_test_suite);
            } else {
                // No.  Appends to the end of the list.
                test_suites_.push_back(new_test_suite);
            }

            test_suite_indices_.push_back(static_cast<int>(test_suite_indices_.size()));
            return new_test_suite;
        }


        // Compares the name of each test with the user-specified filter to
        // decide whether the test should be run, then records the result in
        // each TestSuite and TestInfo object.
        int UnitTestImpl::FilterTests(ReactionToSharding shard_tests) {
            const int32_t total_shards = shard_tests == HONOR_SHARDING_PROTOCOL
                ? FLAGS_htest_total_shards
                : -1;
            const int32_t shard_index = shard_tests == HONOR_SHARDING_PROTOCOL ? FLAGS_htest_shard_index : -1;

            const PositiveAndNegativeUnitTestFilter htest_flag_filter(
                FLAGS_htest_filter);
            const UnitTestFilter disable_test_filter(hawk::Constants::kDisableTestFilter);
            // num_runnable_tests are the number of tests that will
            // run across all shards (i.e., match filter and are not disabled).
            // num_selected_tests are the number of tests to be run on
            // this shard.
            int num_runnable_tests = 0;
            int num_selected_tests = 0;
            for (auto* test_suite : test_suites_) {
                const std::string& test_suite_name = test_suite->name_;
                test_suite->set_should_run(false);

                for (TestInfo* test_info : test_suite->test_info_list()) {
                    const std::string& test_name = test_info->name_;
                    // A test is disabled if test suite name or test name matches
                    // kDisableTestFilter.
                    const bool is_disabled = disable_test_filter.MatchesName(test_suite_name) || disable_test_filter.MatchesName(test_name);
                    test_info->is_disabled_ = is_disabled;

                    const bool matches_filter = htest_flag_filter.MatchesTest(test_suite_name, test_name);
                    test_info->matches_filter_ = matches_filter;

                    const bool is_runnable = (FLAGS_htest_also_run_disabled_tests || !is_disabled) && matches_filter;

                    const bool is_in_another_shard = shard_tests != IGNORE_SHARDING_PROTOCOL && !ShouldRunTestOnShard(total_shards, shard_index, num_runnable_tests);
                    test_info->is_in_another_shard_ = is_in_another_shard;
                    const bool is_selected = is_runnable && !is_in_another_shard;

                    num_runnable_tests += is_runnable;
                    num_selected_tests += is_selected;

                    test_info->should_run_ = is_selected;
                    test_suite->set_should_run(test_suite->should_run() || is_selected);
                }
            }
            return num_selected_tests;
        }


        // Returns true if a warning should be issued if no tests match the test filter
        // flag. We can't simply count the number of tests that ran because, for
        // instance, test sharding and death tests might mean no tests are expected to
        // run in this process, but will run in another process.
        bool UnitTestImpl::ShouldWarnIfNoTestsMatchFilter() const {
            if (total_test_count() == 0) {
                // No tests were linked in to program.
                // This case is handled by a different warning.
                return false;
            }
            const PositiveAndNegativeUnitTestFilter htest_flag_filter(
                FLAGS_htest_filter);
            for (auto* test_suite : test_suites_) {
                const std::string& test_suite_name = test_suite->name_;
                for (TestInfo* test_info : test_suite->test_info_list()) {
                    const std::string& test_name = test_info->name_;
                    if (htest_flag_filter.MatchesTest(test_suite_name, test_name)) {
                        return false;
                    }
                }
            }
            return true;
        }




        // Prints the given C-string on a single line by replacing all '\n'
        // characters with string "\\n".  If the output takes more than
        // max_length characters, only prints the first max_length characters
        // and "...".
        static void PrintOnOneLine(const char* str, int max_length) {
            if (str != nullptr) {
                for (int i = 0; *str != '\0'; ++str) {
                    if (i >= max_length) {
                        printf("...");
                        break;
                    }
                    if (*str == '\n') {
                        printf("\\n");
                        i += 2;
                    } else {
                        printf("%c", *str);
                        ++i;
                    }
                }
            }
        }

                // Prints the names of the tests matching the user-specified filter flag.
        void UnitTestImpl::ListTestsMatchingFilter() {
            // Print at most this many characters for each type/value parameter.
            const int kMaxParamLength = 250;

            for (auto* test_suite : test_suites_) {
                bool printed_test_suite_name = false;

                for (size_t j = 0; j < test_suite->test_info_list().size(); j++) {
                    const TestInfo* const test_info = test_suite->test_info_list()[j];
                    if (test_info->matches_filter_) {
                        if (!printed_test_suite_name) {
                            printed_test_suite_name = true;
                            printf("%s.", test_suite->name());
                            if (test_suite->type_param() != nullptr) {
                                hawk::stdout_printer().fmt("  # {} = ", hawk::Constants::kTypeParamLabel);
                                // We print the type parameter on a single line to make
                                // the output easy to parse by a program.
                                PrintOnOneLine(test_suite->type_param(), kMaxParamLength);
                            }
                            printf("\n");
                        }
                        printf("  %s", test_info->name());
                        if (test_info->value_param() != nullptr) {
                             hawk::stdout_printer().fmt("  # {} = ", hawk::Constants::kValueParamLabel);
                            // We print the value parameter on a single line to make the
                            // output easy to parse by a program.
                            PrintOnOneLine(test_info->value_param(), kMaxParamLength);
                        }
                        printf("\n");
                    }
                }
            }
            fflush(stdout);
#if HTEST_HAS_FILE_SYSTEM
            const std::string& output_format = UnitTestOptions::GetOutputFormat();
            if (output_format == "xml" || output_format == "json") {
                FILE* fileout = OpenFileForWriting(UnitTestOptions::GetAbsolutePathToOutputFile());
                std::stringstream stream;
                if (output_format == "xml") {
                    XmlUnitTestResultPrinter(
                        UnitTestOptions::GetAbsolutePathToOutputFile().c_str())
                        .PrintXmlTestsList(&stream, test_suites_);
                } else if (output_format == "json") {
                    JsonUnitTestResultPrinter(
                        UnitTestOptions::GetAbsolutePathToOutputFile().c_str())
                        .PrintJsonTestList(&stream, test_suites_);
                }
                fprintf(fileout, "%s", StringStreamToString(&stream).c_str());
                fclose(fileout);
            }
#endif // HTEST_HAS_FILE_SYSTEM
        }

        // Sets the OS stack trace getter.
        //
        // Does nothing if the input and the current OS stack trace getter are
        // the same; otherwise, deletes the old getter and makes the input the
        // current getter.
        void UnitTestImpl::set_os_stack_trace_getter(
            OsStackTraceGetterInterface* getter) {
            if (os_stack_trace_getter_ != getter) {
                delete os_stack_trace_getter_;
                os_stack_trace_getter_ = getter;
            }
        }


        // Returns the current OS stack trace getter if it is not NULL;
        // otherwise, creates an OsStackTraceGetter, makes it the current
        // getter, and returns it.
        OsStackTraceGetterInterface* UnitTestImpl::os_stack_trace_getter() {
            if (os_stack_trace_getter_ == nullptr) {
#ifdef HTEST_OS_STACK_TRACE_GETTER_
                os_stack_trace_getter_ = new HTEST_OS_STACK_TRACE_GETTER_;
#else
                os_stack_trace_getter_ = new OsStackTraceGetter;
#endif // HTEST_OS_STACK_TRACE_GETTER_
            }

            return os_stack_trace_getter_;
        }


        // Returns the most specific TestResult currently running.
        TestResult* UnitTestImpl::current_test_result() {
            if (current_test_info_ != nullptr) {
                return &current_test_info_->result_;
            }
            if (current_test_suite_ != nullptr) {
                return &current_test_suite_->ad_hoc_test_result_;
            }
            return &ad_hoc_test_result_;
        }

        // Shuffles all test suites, and the tests within each test suite,
        // making sure that death tests are still run first.
        void UnitTestImpl::ShuffleTests() {
            // Shuffles the death test suites.
            ShuffleRange(random(), 0, last_death_test_suite_ + 1, &test_suite_indices_);

            // Shuffles the non-death test suites.
            ShuffleRange(random(), last_death_test_suite_ + 1,
                static_cast<int>(test_suites_.size()), &test_suite_indices_);

            // Shuffles the tests inside each test suite.
            for (auto& test_suite : test_suites_) {
                test_suite->ShuffleTests(random());
            }
        }

        // Restores the test suites and tests to their order before the first shuffle.
        void UnitTestImpl::UnshuffleTests() {
            for (size_t i = 0; i < test_suites_.size(); i++) {
                // Unshuffles the tests in each test suite.
                test_suites_[i]->UnshuffleTests();
                // Resets the index of each test suite.
                test_suite_indices_[i] = static_cast<int>(i);
            }
        }

        // If the environment variable TEST_WARNINGS_OUTPUT_FILE was provided, appends
        // `str` to the file, creating the file if necessary.
#if HTEST_HAS_FILE_SYSTEM
        static void AppendToTestWarningsOutputFile(const std::string& str) {
            const std::optional<std::string> filename = hawk::get_env(hawk::Constants::kTestWarningsOutputFile);
            if (!filename.has_value()) {
                return;
            }
            auto* const file = posix::FOpen(filename->c_str(), "a");
            if (file == nullptr) {
                return;
            }
            HTEST_CHECK_(fwrite(str.data(), 1, str.size(), file) == str.size());
            HTEST_CHECK_(posix::FClose(file) == 0);
        }
#endif // HTEST_HAS_FILE_SYSTEM

        // Helpers for setting up / tearing down the given environment.  They
        // are for use in the ForEach() function.
        static void SetUpEnvironment(Environment* env) {
            env->SetUp();
        }
        static void TearDownEnvironment(Environment* env) {
            env->TearDown();
        }

         // Runs all tests in this UnitTest object, prints the result, and
        // returns true if all tests are successful.  If any exception is
        // thrown during a test, the test is considered to be failed, but the
        // rest of the tests will still be run.
        //
        // When parameterized tests are enabled, it expands and registers
        // parameterized tests first in RegisterParameterizedTests().
        // All other functions called from RunAllTests() may safely assume that
        // parameterized tests are ready to be counted and run.
        bool UnitTestImpl::RunAllTests() {
            // True if and only if Hawk Test is initialized before RUN_ALL_TESTS() is
            // called.
            const bool htest_is_initialized_before_run_all_tests = HTestIsInitialized();

            // Do not run any test if the --help flag was specified.
            if (g_help_flag)
                return true;

            // Repeats the call to the post-flag parsing initialization in case the
            // user didn't call init_hawk_test.
            PostFlagParsingInit();

            // Handle the case where the program has no tests linked.
            // Sometimes this is a programmer mistake, but sometimes it is intended.
            if (total_test_count() == 0) {
                constexpr char kNoTestLinkedMessage[] = "This test program does NOT link in any test case.";
                constexpr char kNoTestLinkedFatal[] = "This is INVALID. Please make sure to link in at least one test case.";
                constexpr char kNoTestLinkedWarning[] = "Please make sure this is intended.";
                const bool fail_if_no_test_linked = FLAGS_htest_fail_if_no_test_linked;
                hawk::stdout_printer().fmt_red("{} {}\n", kNoTestLinkedMessage,
                    fail_if_no_test_linked ? kNoTestLinkedFatal : kNoTestLinkedWarning);
                if (fail_if_no_test_linked) {
                    return false;
                }
#if HTEST_HAS_FILE_SYSTEM
                AppendToTestWarningsOutputFile(std::string(kNoTestLinkedMessage) + ' ' + kNoTestLinkedWarning + '\n');
#endif // HTEST_HAS_FILE_SYSTEM
            }

#if HTEST_HAS_FILE_SYSTEM
            // Even if sharding is not on, test runners may want to use the
            // HTEST_SHARD_STATUS_FILE to query whether the test supports the sharding
            // protocol.
            internal::WriteToShardStatusFileIfNeeded();
#endif // HTEST_HAS_FILE_SYSTEM

            // True if and only if we are in a subprocess for running a thread-safe-style
            // death test.
            bool in_subprocess_for_death_test = false;

#ifdef HTEST_HAS_DEATH_TEST
            in_subprocess_for_death_test = (internal_run_death_test_flag_ != nullptr);
#if defined(HTEST_EXTRA_DEATH_TEST_CHILD_SETUP_)
            if (in_subprocess_for_death_test) {
                HTEST_EXTRA_DEATH_TEST_CHILD_SETUP_();
            }
#endif // defined(HTEST_EXTRA_DEATH_TEST_CHILD_SETUP_)
#endif // HTEST_HAS_DEATH_TEST

            const bool should_shard = ShouldShard(in_subprocess_for_death_test);

            // Compares the full test names with the filter to decide which
            // tests to run.
            const bool has_tests_to_run = FilterTests(should_shard ? HONOR_SHARDING_PROTOCOL
                                                                   : IGNORE_SHARDING_PROTOCOL)
                > 0;

            // Lists the tests and exits if the --htest_list_tests flag was specified.
            if (FLAGS_htest_list_tests) {
                // This must be called *after* FilterTests() has been called.
                ListTestsMatchingFilter();
                return true;
            }

            random_seed_ = GetRandomSeedFromFlag(FLAGS_htest_random_seed);

            // True if and only if at least one test has failed.
            bool failed = false;

            TestEventListener* repeater = listeners()->repeater();

            start_timestamp_ = hawk::GetTimeInMillis();
            repeater->OnTestProgramStart(*parent_);

            // How many times to repeat the tests?  We don't want to repeat them
            // when we are inside the subprocess of a death test.
            const int repeat = in_subprocess_for_death_test ? 1 : FLAGS_htest_repeat;

            // Repeats forever if the repeat count is negative.
            const bool htest_repeat_forever = repeat < 0;

            // Should test environments be set up and torn down for each repeat, or only
            // set up on the first and torn down on the last iteration? If there is no
            // "last" iteration because the tests will repeat forever, always recreate the
            // environments to avoid leaks in case one of the environments is using
            // resources that are external to this process. Without this check there would
            // be no way to clean up those external resources automatically.
            const bool recreate_environments_when_repeating = FLAGS_htest_recreate_environments_when_repeating || htest_repeat_forever;

            for (int i = 0; htest_repeat_forever || i != repeat; i++) {
                // We want to preserve failures generated by ad-hoc test
                // assertions executed before RUN_ALL_TESTS().
                ClearNonAdHocTestResult();

                hawk::Timer timer;

                // Shuffles test suites and tests if requested.
                if (has_tests_to_run && FLAGS_htest_shuffle) {
                    random()->Reseed(static_cast<uint32_t>(random_seed_));
                    // This should be done before calling OnTestIterationStart(),
                    // such that a test event listener can see the actual test order
                    // in the event.
                    ShuffleTests();
                }

                // Tells the unit test event listeners that the tests are about to start.
                repeater->OnTestIterationStart(*parent_, i);

                // Runs each test suite if there is at least one test to run.
                if (has_tests_to_run) {
                    // Sets up all environments beforehand. If test environments aren't
                    // recreated for each iteration, only do so on the first iteration.
                    if (i == 0 || recreate_environments_when_repeating) {
                        repeater->OnEnvironmentsSetUpStart(*parent_);
                        ForEach(environments_, SetUpEnvironment);
                        repeater->OnEnvironmentsSetUpEnd(*parent_);
                    }

                    // Runs the tests only if there was no fatal failure or skip triggered
                    // during global set-up.
                    if (Test::IsSkipped()) {
                        // Emit diagnostics when global set-up calls skip, as it will not be
                        // emitted by default.
                        TestResult& test_result = *internal::GetUnitTestImpl()->current_test_result();
                        for (int j = 0; j < test_result.total_part_count(); ++j) {
                            const TestPartResult& test_part_result = test_result.GetTestPartResult(j);
                            if (test_part_result.type() == TestPartResult::kSkip) {
                                const std::string& result = test_part_result.message();
                                printf("%s\n", result.c_str());
                            }
                        }
                        fflush(stdout);
                    } else if (!Test::HasFatalFailure()) {
                        for (int test_index = 0; test_index < total_test_suite_count();
                            test_index++) {
                            GetMutableSuiteCase(test_index)->Run();
                            if (FLAGS_htest_fail_fast && GetMutableSuiteCase(test_index)->Failed()) {
                                for (int j = test_index + 1; j < total_test_suite_count(); j++) {
                                    GetMutableSuiteCase(j)->Skip();
                                }
                                break;
                            }
                        }
                    } else if (Test::HasFatalFailure()) {
                        // If there was a fatal failure during the global setup then we know we
                        // aren't going to run any tests. Explicitly mark all of the tests as
                        // skipped to make this obvious in the output.
                        for (int test_index = 0; test_index < total_test_suite_count();
                            test_index++) {
                            GetMutableSuiteCase(test_index)->Skip();
                        }
                    }

                    // Tears down all environments in reverse order afterwards. If test
                    // environments aren't recreated for each iteration, only do so on the
                    // last iteration.
                    if (i == repeat - 1 || recreate_environments_when_repeating) {
                        repeater->OnEnvironmentsTearDownStart(*parent_);
                        std::for_each(environments_.rbegin(), environments_.rend(),
                            TearDownEnvironment);
                        repeater->OnEnvironmentsTearDownEnd(*parent_);
                    }
                } else if (FLAGS_htest_fail_if_no_test_selected) {
                    // If there were no tests to run, bail if we were requested to be
                    // strict.
                    constexpr char kNoTestsSelectedMessage[] = "No tests ran. Check that tests exist and are not disabled or "
                                                               "filtered out.\n\n"
                                                               "For sharded runs, this error indicates an empty shard. This can "
                                                               "happen if you have more shards than tests, or if --htest_filter "
                                                               "leaves a shard with no tests.\n\n"
                                                               "To permit empty shards (e.g., when debugging with a filter), "
                                                               "specify \n"
                                                               "--htest_fail_if_no_test_selected=false.";
                    hawk::stdout_printer().fmt_red("{}\n", kNoTestsSelectedMessage);
                    return false;
                }

                elapsed_time_ = timer.Elapsed();

                // Tells the unit test event listener that the tests have just finished.
                repeater->OnTestIterationEnd(*parent_, i);

                // Gets the result and clears it.
                if (!Passed()) {
                    failed = true;
                }

                // Restores the original test order after the iteration.  This
                // allows the user to quickly repro a failure that happens in the
                // N-th iteration without repeating the first (N - 1) iterations.
                // This is not enclosed in "if (FLAGS_htest_shuffle) { ... }", in
                // case the user somehow changes the value of the flag somewhere
                // (it's always safe to unshuffle the tests).
                UnshuffleTests();

                if (FLAGS_htest_shuffle) {
                    // Picks a new random seed for each iteration.
                    random_seed_ = GetNextRandomSeed(random_seed_);
                }
            }

            repeater->OnTestProgramEnd(*parent_);
            // Destroy environments in normal code, not in static teardown.
            bool delete_environment_on_teardown = true;
            if (delete_environment_on_teardown) {
                ForEach(environments_, internal::Delete<Environment>);
                environments_.clear();
            }

            // Try to warn the user if no tests matched the test filter.
            if (ShouldWarnIfNoTestsMatchFilter()) {
                const std::string filter_warning = std::string("filter \"") + FLAGS_htest_filter + "\" did not match any test; no tests were run\n";
                hawk::stdout_printer().fmt_red("WARNING: {}", filter_warning);
#if HTEST_HAS_FILE_SYSTEM
                AppendToTestWarningsOutputFile(filter_warning);
#endif // HTEST_HAS_FILE_SYSTEM
            }

            if (!htest_is_initialized_before_run_all_tests) {
                hawk::stdout_printer().print_red(
                    "\nIMPORTANT NOTICE - DO NOT IGNORE:\n"
                    "This test program did NOT call " HTEST_INIT_GOOGLE_TEST_NAME_
                    "() before calling RUN_ALL_TESTS(). This is INVALID. Soon " HTEST_NAME_
                    " will start to enforce the valid usage. "
                    "Please fix it ASAP, or IT WILL START TO FAIL.\n"); // NOLINT
            }

            return !failed;
        }
    }
}