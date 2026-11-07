#!/usr/bin/env python
#
# Copyright 2006, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Unit test for the htest_xml_output module."""

import datetime
import errno
import os
import re
import sys
from xml.dom import minidom

from tests.test import htest_test_utils
from tests.test import htest_xml_test_utils

HTEST_FILTER_FLAG = '--htest_filter'
HTEST_LIST_TESTS_FLAG = '--htest_list_tests'
HTEST_OUTPUT_FLAG = '--htest_output'
HTEST_DEFAULT_OUTPUT_FILE = 'test_detail.xml'
HTEST_PROGRAM_NAME = 'htest_xml_output_unittest_'

# The flag indicating stacktraces are not supported
NO_STACKTRACE_SUPPORT_FLAG = '--no_stacktrace_support'

# The environment variables for test sharding.
TOTAL_SHARDS_ENV_VAR = 'HTEST_TOTAL_SHARDS'
SHARD_INDEX_ENV_VAR = 'HTEST_SHARD_INDEX'
SHARD_STATUS_FILE_ENV_VAR = 'HTEST_SHARD_STATUS_FILE'

SUPPORTS_STACK_TRACES = NO_STACKTRACE_SUPPORT_FLAG not in sys.argv

if SUPPORTS_STACK_TRACES:
  STACK_TRACE_TEMPLATE = '\nStack trace:\n*'
  STACK_TRACE_ENTITY_TEMPLATE = ''
else:
  STACK_TRACE_TEMPLATE = '\n'
  STACK_TRACE_ENTITY_TEMPLATE = '&#x0A;'
  # unittest.main() can't handle unknown flags
  sys.argv.remove(NO_STACKTRACE_SUPPORT_FLAG)

EXPECTED_NON_EMPTY_XML = """<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="28" failures="5" disabled="2" errors="0" time="*" timestamp="*" name="AllTests">
  <properties>
    <property name="ad_hoc_property" value="42"/>
  </properties>
  <testsuite name="SuccessfulTest" tests="1" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="Succeeds" file="htest_xml_output_unittest_.cc" line="56" status="run" result="completed" time="*" timestamp="*" classname="SuccessfulTest"/>
  </testsuite>
  <testsuite name="FailedTest" tests="1" failures="1" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="Fails" file="htest_xml_output_unittest_.cc" line="64" status="run" result="completed" time="*" timestamp="*" classname="FailedTest">
      <failure message="htest_xml_output_unittest_.cc:*&#x0A;Expected equality of these values:&#x0A;  1&#x0A;  2%(stack_entity)s" type=""><![CDATA[htest_xml_output_unittest_.cc:*
Expected equality of these values:
  1
  2%(stack)s]]></failure>
    </testcase>
  </testsuite>
  <testsuite name="MixedResultTest" tests="3" failures="1" disabled="1" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="Succeeds" file="htest_xml_output_unittest_.cc" line="91" status="run" result="completed" time="*" timestamp="*" classname="MixedResultTest"/>
    <testcase name="Fails" file="htest_xml_output_unittest_.cc" line="96" status="run" result="completed" time="*" timestamp="*" classname="MixedResultTest">
      <failure message="htest_xml_output_unittest_.cc:*&#x0A;Expected equality of these values:&#x0A;  1&#x0A;  2%(stack_entity)s" type=""><![CDATA[htest_xml_output_unittest_.cc:*
Expected equality of these values:
  1
  2%(stack)s]]></failure>
      <failure message="htest_xml_output_unittest_.cc:*&#x0A;Expected equality of these values:&#x0A;  2&#x0A;  3%(stack_entity)s" type=""><![CDATA[htest_xml_output_unittest_.cc:*
Expected equality of these values:
  2
  3%(stack)s]]></failure>
    </testcase>
    <testcase name="DISABLED_test" file="htest_xml_output_unittest_.cc" line="101" status="notrun" result="suppressed" time="*" timestamp="*" classname="MixedResultTest"/>
  </testsuite>
  <testsuite name="XmlQuotingTest" tests="1" failures="1" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="OutputsCData" file="htest_xml_output_unittest_.cc" line="105" status="run" result="completed" time="*" timestamp="*" classname="XmlQuotingTest">
      <failure message="htest_xml_output_unittest_.cc:*&#x0A;Failed&#x0A;XML output: &lt;?xml encoding=&quot;utf-8&quot;&gt;&lt;top&gt;&lt;![CDATA[cdata text]]&gt;&lt;/top&gt;%(stack_entity)s" type=""><![CDATA[htest_xml_output_unittest_.cc:*
Failed
XML output: <?xml encoding="utf-8"><top><![CDATA[cdata text]]>]]&gt;<![CDATA[</top>%(stack)s]]></failure>
    </testcase>
  </testsuite>
  <testsuite name="InvalidCharactersTest" tests="1" failures="1" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="InvalidCharactersInMessage" file="htest_xml_output_unittest_.cc" line="112" status="run" result="completed" time="*" timestamp="*" classname="InvalidCharactersTest">
      <failure message="htest_xml_output_unittest_.cc:*&#x0A;Failed&#x0A;Invalid characters in brackets []%(stack_entity)s" type=""><![CDATA[htest_xml_output_unittest_.cc:*
Failed
Invalid characters in brackets []%(stack)s]]></failure>
    </testcase>
  </testsuite>
  <testsuite name="DisabledTest" tests="1" failures="0" disabled="1" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="DISABLED_test_not_run" file="htest_xml_output_unittest_.cc" line="71" status="notrun" result="suppressed" time="*" timestamp="*" classname="DisabledTest"/>
  </testsuite>
  <testsuite name="SkippedTest" tests="3" failures="1" disabled="0" skipped="2" errors="0" time="*" timestamp="*">
    <testcase name="Skipped" status="run" file="htest_xml_output_unittest_.cc" line="78" result="skipped" time="*" timestamp="*" classname="SkippedTest">
      <skipped message="htest_xml_output_unittest_.cc:*&#x0A;&#x0A;"><![CDATA[htest_xml_output_unittest_.cc:*

]]></skipped>
    </testcase>
    <testcase name="SkippedWithMessage" file="htest_xml_output_unittest_.cc" line="82" status="run" result="skipped" time="*" timestamp="*" classname="SkippedTest">
      <skipped message="htest_xml_output_unittest_.cc:*&#x0A;It is good practice to tell why you skip a test.&#x0A;"><![CDATA[htest_xml_output_unittest_.cc:*
It is good practice to tell why you skip a test.
]]></skipped>
    </testcase>
    <testcase name="SkippedAfterFailure" file="htest_xml_output_unittest_.cc" line="86" status="run" result="completed" time="*" timestamp="*" classname="SkippedTest">
      <failure message="htest_xml_output_unittest_.cc:*&#x0A;Expected equality of these values:&#x0A;  1&#x0A;  2%(stack_entity)s" type=""><![CDATA[htest_xml_output_unittest_.cc:*
Expected equality of these values:
  1
  2%(stack)s]]></failure>
      <skipped message="htest_xml_output_unittest_.cc:*&#x0A;It is good practice to tell why you skip a test.&#x0A;"><![CDATA[htest_xml_output_unittest_.cc:*
It is good practice to tell why you skip a test.
]]></skipped>
    </testcase>

  </testsuite>
  <testsuite name="PropertyRecordingTest" tests="4" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <properties>
      <property name="SetUpTestSuite" value="yes"/>
      <property name="SetUpTestSuite (with whitespace)" value="yes and yes"/>
      <property name="TearDownTestSuite" value="aye"/>
      <property name="TearDownTestSuite (with whitespace)" value="aye and aye"/>
    </properties>
    <testcase name="OneProperty" file="htest_xml_output_unittest_.cc" line="128" status="run" result="completed" time="*" timestamp="*" classname="PropertyRecordingTest">
      <properties>
        <property name="key_1" value="1"/>
      </properties>
    </testcase>
    <testcase name="IntValuedProperty" file="htest_xml_output_unittest_.cc" line="132" status="run" result="completed" time="*" timestamp="*" classname="PropertyRecordingTest">
      <properties>
        <property name="key_int" value="1"/>
      </properties>
    </testcase>
    <testcase name="ThreeProperties" file="htest_xml_output_unittest_.cc" line="136" status="run" result="completed" time="*" timestamp="*" classname="PropertyRecordingTest">
      <properties>
        <property name="key_1" value="1"/>
        <property name="key_2" value="2"/>
        <property name="key_3" value="3"/>
      </properties>
    </testcase>
    <testcase name="TwoValuesForOneKeyUsesLastValue" file="htest_xml_output_unittest_.cc" line="142" status="run" result="completed" time="*" timestamp="*" classname="PropertyRecordingTest">
      <properties>
        <property name="key_1" value="2"/>
      </properties>
    </testcase>
  </testsuite>
  <testsuite name="NoFixtureTest" tests="3" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
     <testcase name="RecordProperty" file="htest_xml_output_unittest_.cc" line="147" status="run" result="completed" time="*" timestamp="*" classname="NoFixtureTest">
       <properties>
         <property name="key" value="1"/>
       </properties>
     </testcase>
     <testcase name="ExternalUtilityThatCallsRecordIntValuedProperty" file="htest_xml_output_unittest_.cc" line="160" status="run" result="completed" time="*" timestamp="*" classname="NoFixtureTest">
       <properties>
         <property name="key_for_utility_int" value="1"/>
       </properties>
     </testcase>
     <testcase name="ExternalUtilityThatCallsRecordStringValuedProperty" file="htest_xml_output_unittest_.cc" line="164" status="run" result="completed" time="*" timestamp="*" classname="NoFixtureTest">
       <properties>
         <property name="key_for_utility_string" value="1"/>
       </properties>
     </testcase>
  </testsuite>
  <testsuite name="SetupFailTest" tests="1" failures="0" disabled="0" skipped="1" errors="0" time="*" timestamp="*">
    <testcase name="NoopPassingTest" file="htest_xml_output_unittest_.cc" line="175" status="run" result="skipped" time="*" timestamp="*" classname="SetupFailTest">
      <skipped message="htest_xml_output_unittest_.cc:*&#x0A;"><![CDATA[htest_xml_output_unittest_.cc:*
]]></skipped>
    </testcase>
    <testcase name="" status="run" result="completed" classname="" time="*" timestamp="*">
      <failure message="htest_xml_output_unittest_.cc:*&#x0A;Expected equality of these values:&#x0A;  1&#x0A;  2%(stack_entity)s" type=""><![CDATA[htest_xml_output_unittest_.cc:*
Expected equality of these values:
  1
  2%(stack)s]]></failure>
    </testcase>
  </testsuite>
  <testsuite name="TearDownFailTest" tests="1" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="NoopPassingTest" file="htest_xml_output_unittest_.cc" line="182" status="run" result="completed" time="*" timestamp="*" classname="TearDownFailTest"/>
    <testcase name="" status="run" result="completed" classname="" time="*" timestamp="*">
      <failure message="htest_xml_output_unittest_.cc:*&#x0A;Expected equality of these values:&#x0A;  1&#x0A;  2%(stack_entity)s" type=""><![CDATA[htest_xml_output_unittest_.cc:*
Expected equality of these values:
  1
  2%(stack)s]]></failure>
    </testcase>
  </testsuite>
  <testsuite name="Single/ValueParamTest" tests="4" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="HasValueParamAttribute/0" file="htest_xml_output_unittest_.cc" line="187" value_param="33" status="run" result="completed" time="*" timestamp="*" classname="Single/ValueParamTest" />
    <testcase name="HasValueParamAttribute/1" file="htest_xml_output_unittest_.cc" line="187" value_param="42" status="run" result="completed" time="*" timestamp="*" classname="Single/ValueParamTest" />
    <testcase name="AnotherTestThatHasValueParamAttribute/0" file="htest_xml_output_unittest_.cc" line="188" value_param="33" status="run" result="completed" time="*" timestamp="*" classname="Single/ValueParamTest" />
    <testcase name="AnotherTestThatHasValueParamAttribute/1" file="htest_xml_output_unittest_.cc" line="188" value_param="42" status="run" result="completed" time="*" timestamp="*" classname="Single/ValueParamTest" />
  </testsuite>
  <testsuite name="TypedTest/0" tests="1" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="HasTypeParamAttribute" file="htest_xml_output_unittest_.cc" line="196" type_param="*" status="run" result="completed" time="*" timestamp="*" classname="TypedTest/0" />
  </testsuite>
  <testsuite name="TypedTest/1" tests="1" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="HasTypeParamAttribute" file="htest_xml_output_unittest_.cc" line="196" type_param="*" status="run" result="completed" time="*" timestamp="*" classname="TypedTest/1" />
  </testsuite>
  <testsuite name="Single/TypeParameterizedTestSuite/0" tests="1" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="HasTypeParamAttribute" file="htest_xml_output_unittest_.cc" line="203" type_param="*" status="run" result="completed" time="*" timestamp="*" classname="Single/TypeParameterizedTestSuite/0" />
  </testsuite>
  <testsuite name="Single/TypeParameterizedTestSuite/1" tests="1" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="HasTypeParamAttribute" file="htest_xml_output_unittest_.cc" line="203" type_param="*" status="run" result="completed" time="*" timestamp="*" classname="Single/TypeParameterizedTestSuite/1" />
  </testsuite>
</testsuites>""" % {
    'stack': STACK_TRACE_TEMPLATE,
    'stack_entity': STACK_TRACE_ENTITY_TEMPLATE,
}

EXPECTED_FILTERED_TEST_XML = """<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="1" failures="0" disabled="0" errors="0" time="*" timestamp="*" name="AllTests">
  <properties>
    <property name="ad_hoc_property" value="42"/>
  </properties>
  <testsuite name="SuccessfulTest" tests="1" failures="0" disabled="0" skipped="0"
             errors="0" time="*" timestamp="*">
    <testcase name="Succeeds" file="htest_xml_output_unittest_.cc" line="56" status="run" result="completed" time="*" timestamp="*" classname="SuccessfulTest"/>
  </testsuite>
</testsuites>"""

EXPECTED_SHARDED_TEST_XML = """<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="3" failures="0" disabled="0" errors="0" time="*" timestamp="*" name="AllTests">
  <properties>
    <property name="ad_hoc_property" value="42"/>
  </properties>
  <testsuite name="SuccessfulTest" tests="1" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="Succeeds" file="htest_xml_output_unittest_.cc" line="56" status="run" result="completed" time="*" timestamp="*" classname="SuccessfulTest"/>
  </testsuite>
  <testsuite name="PropertyRecordingTest" tests="1" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <properties>
      <property name="SetUpTestSuite" value="yes"/>
      <property name="SetUpTestSuite (with whitespace)" value="yes and yes"/>
      <property name="TearDownTestSuite" value="aye"/>
      <property name="TearDownTestSuite (with whitespace)" value="aye and aye"/>
    </properties>
    <testcase name="IntValuedProperty" file="htest_xml_output_unittest_.cc" line="132" status="run" result="completed" time="*" timestamp="*" classname="PropertyRecordingTest">
      <properties>
        <property name="key_int" value="1"/>
      </properties>
    </testcase>
  </testsuite>
  <testsuite name="Single/TypeParameterizedTestSuite/0" tests="1" failures="0" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="HasTypeParamAttribute" type_param="*" file="htest_xml_output_unittest_.cc" line="203" status="run" result="completed" time="*" timestamp="*" classname="Single/TypeParameterizedTestSuite/0" />
  </testsuite>
</testsuites>"""

EXPECTED_NO_TEST_XML = """<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="0" failures="0" disabled="0" errors="0" time="*"
            timestamp="*" name="AllTests">
  <testsuite name="NonTestSuiteFailure" tests="1" failures="1" disabled="0" skipped="0" errors="0" time="*" timestamp="*">
    <testcase name="" status="run" result="completed" time="*" timestamp="*" classname="">
      <failure message="htest_no_test_unittest.cc:*&#x0A;Expected equality of these values:&#x0A;  1&#x0A;  2%(stack_entity)s" type=""><![CDATA[htest_no_test_unittest.cc:*
Expected equality of these values:
  1
  2%(stack)s]]></failure>
    </testcase>
  </testsuite>
</testsuites>""" % {
    'stack': STACK_TRACE_TEMPLATE,
    'stack_entity': STACK_TRACE_ENTITY_TEMPLATE,
}

HTEST_PROGRAM_PATH = htest_test_utils.GetTestExecutablePath(HTEST_PROGRAM_NAME)

SUPPORTS_TYPED_TESTS = (
    'TypedTest'
    in htest_test_utils.Subprocess(
        [HTEST_PROGRAM_PATH, HTEST_LIST_TESTS_FLAG], capture_stderr=False
    ).output
)


class HTestXMLOutputUnitTest(htest_xml_test_utils.HTestXMLTestCase):
  """Unit test for Hawk Test's XML output functionality."""

  # This test currently breaks on platforms that do not support typed and
  # type-parameterized tests, so we don't run it under them.
  if SUPPORTS_TYPED_TESTS:

    def testNonEmptyXmlOutput(self):
      """Generates non-empty XML and verifies it matches the expected output.

      Runs a test program that generates a non-empty XML output, and
      tests that the XML output is expected.
      """
      self._TestXmlOutput(HTEST_PROGRAM_NAME, EXPECTED_NON_EMPTY_XML, 1)

  def testNoTestXmlOutput(self):
    """Verifies XML output for a Hawk Test binary without actual tests.

    Runs a test program that generates an XML output for a binary without tests,
    and tests that the XML output is expected.
    """

    self._TestXmlOutput('ut_htest_no_test_unittest', EXPECTED_NO_TEST_XML, 0)

  def testTimestampValue(self):
    """Checks whether the timestamp attribute in the XML output is valid.

    Runs a test program that generates an empty XML output, and checks if
    the timestamp attribute in the testsuites tag is valid.
    """
    actual = self._GetXmlOutput('ut_htest_no_test_unittest', [], {}, 0)
    date_time_str = actual.documentElement.getAttributeNode('timestamp').value
    # datetime.strptime() is only available in Python 2.5+ so we have to
    # parse the expected datetime manually.
    match = re.match(r'(\d+)-(\d\d)-(\d\d)T(\d\d):(\d\d):(\d\d)', date_time_str)
    self.assertTrue(
        re.match, 'XML datettime string %s has incorrect format' % date_time_str
    )
    date_time_from_xml = datetime.datetime(
        year=int(match.group(1)),
        month=int(match.group(2)),
        day=int(match.group(3)),
        hour=int(match.group(4)),
        minute=int(match.group(5)),
        second=int(match.group(6)),
    )

    time_delta = abs(datetime.datetime.now() - date_time_from_xml)
    # timestamp value should be near the current local time
    self.assertLess(time_delta, datetime.timedelta(seconds=600))
    actual.unlink()

  def testDefaultOutputFile(self):
    """Tests XML file with default name is created when name is not specified.

    Confirms that Hawk Test produces an XML output file with the expected
    default name if no name is explicitly specified.
    """
    output_file = os.path.join(
        htest_test_utils.GetTempDir(), HTEST_DEFAULT_OUTPUT_FILE
    )
    htest_prog_path = htest_test_utils.GetTestExecutablePath(
        'ut_htest_no_test_unittest'
    )
    try:
      os.remove(output_file)
    except OSError:
      e = sys.exc_info()[1]
      if e.errno != errno.ENOENT:
        raise

    p = htest_test_utils.Subprocess(
        [htest_prog_path, '%s=xml' % HTEST_OUTPUT_FLAG],
        working_dir=htest_test_utils.GetTempDir(),
    )
    self.assertTrue(p.exited)
    self.assertEqual(0, p.exit_code)
    self.assertTrue(os.path.isfile(output_file))

  def testSuppressedXmlOutput(self):
    """Verifies XML output is suppressed if default listener is shut down.

    Tests that no XML file is generated if the default XML listener is
    shut down before RUN_ALL_TESTS is invoked.
    """

    xml_path = os.path.join(
        htest_test_utils.GetTempDir(), HTEST_PROGRAM_NAME + 'out.xml'
    )
    if os.path.isfile(xml_path):
      os.remove(xml_path)

    command = [
        HTEST_PROGRAM_PATH,
        '%s=xml:%s' % (HTEST_OUTPUT_FLAG, xml_path),
        '--shut_down_xml',
    ]
    p = htest_test_utils.Subprocess(command)
    if p.terminated_by_signal:
      # p.signal is available only if p.terminated_by_signal is True.
      self.assertFalse(
          p.terminated_by_signal,
          '%s was killed by signal %d' % (HTEST_PROGRAM_NAME, p.signal),
      )
    else:
      self.assertTrue(p.exited)
      self.assertEqual(
          1,
          p.exit_code,
          "'%s' exited with code %s, which doesn't match "
          'the expected exit code %s.' % (command, p.exit_code, 1),
      )

    self.assertFalse(os.path.isfile(xml_path))

  def testFilteredTestXmlOutput(self):
    """Verifies XML output when a filter is applied.

    Runs a test program that executes only some tests and verifies that
    non-selected tests do not show up in the XML output.
    """

    self._TestXmlOutput(
        HTEST_PROGRAM_NAME,
        EXPECTED_FILTERED_TEST_XML,
        0,
        extra_args=['%s=SuccessfulTest.*' % HTEST_FILTER_FLAG],
    )

  def testShardedTestXmlOutput(self):
    """Verifies XML output when run using multiple shards.

    Runs a test program that executes only one shard and verifies that tests
    from other shards do not show up in the XML output.
    """

    self._TestXmlOutput(
        HTEST_PROGRAM_NAME,
        EXPECTED_SHARDED_TEST_XML,
        0,
        extra_env={SHARD_INDEX_ENV_VAR: '0', TOTAL_SHARDS_ENV_VAR: '10'},
    )

  def _GetXmlOutput(
      self, htest_prog_name, extra_args, extra_env, expected_exit_code
  ):
    """Returns the XML output generated by running the program htest_prog_name.

    Furthermore, the program's exit code must be expected_exit_code.

    Args:
      htest_prog_name: Program to run.
      extra_args: Optional arguments to pass to program.
      extra_env: Optional environment variables to set.
      expected_exit_code: Expected exit code from running htest_prog_name.
    """
    xml_path = os.path.join(
        htest_test_utils.GetTempDir(), htest_prog_name + 'out.xml'
    )
    htest_prog_path = htest_test_utils.GetTestExecutablePath(htest_prog_name)

    command = [
        htest_prog_path,
        '%s=xml:%s' % (HTEST_OUTPUT_FLAG, xml_path),
    ] + extra_args
    environ_copy = os.environ.copy()
    if extra_env:
      environ_copy.update(extra_env)
    p = htest_test_utils.Subprocess(command, env=environ_copy)

    if p.terminated_by_signal:
      self.assertTrue(
          False, '%s was killed by signal %d' % (htest_prog_name, p.signal)
      )
    else:
      self.assertTrue(p.exited)
      self.assertEqual(
          expected_exit_code,
          p.exit_code,
          "'%s' exited with code %s, which doesn't match "
          'the expected exit code %s.'
          % (command, p.exit_code, expected_exit_code),
      )
    actual = minidom.parse(xml_path)
    return actual

  def _TestXmlOutput(
      self,
      htest_prog_name,
      expected_xml,
      expected_exit_code,
      extra_args=None,
      extra_env=None,
  ):
    """Asserts that the XML document matches.

    Asserts that the XML document generated by running the program
    htest_prog_name matches expected_xml, a string containing another
    XML document.  Furthermore, the program's exit code must be
    expected_exit_code.

    Args:
      htest_prog_name: Program to run.
      expected_xml: Path to XML document to match.
      expected_exit_code: Expected exit code from running htest_prog_name.
      extra_args: Optional arguments to pass to program.
      extra_env: Optional environment variables to set.
    """

    actual = self._GetXmlOutput(
        htest_prog_name, extra_args or [], extra_env or {}, expected_exit_code
    )
    expected = minidom.parseString(expected_xml)
    self.NormalizeXml(actual.documentElement)
    self.AssertEquivalentNodes(expected.documentElement, actual.documentElement)
    expected.unlink()
    actual.unlink()


if __name__ == '__main__':
  os.environ['HTEST_STACK_TRACE_DEPTH'] = '1'
  htest_test_utils.Main()
