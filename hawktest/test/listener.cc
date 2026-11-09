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

#include <hawktest/test/listener.h>
#include <hawktest/test/internal/test_event_repeater.h>

namespace testing {


    TestEventListeners::TestEventListeners()
        : repeater_(new internal::TestEventRepeater())
        , default_result_printer_(nullptr)
        , default_xml_generator_(nullptr) { }

    TestEventListeners::~TestEventListeners() {
        delete repeater_;
    }


    // Returns the standard listener responsible for the default console
    // output.  Can be removed from the listeners list to shut down default
    // console output.  Note that removing this object from the listener list
    // with Release transfers its ownership to the user.
    void TestEventListeners::Append(TestEventListener* listener) {
        repeater_->Append(listener);
    }


    // Removes the given event listener from the list and returns it.  It then
    // becomes the caller's responsibility to delete the listener. Returns
    // NULL if the listener is not found in the list.
    TestEventListener* TestEventListeners::Release(TestEventListener* listener) {
        if (listener == default_result_printer_)
            default_result_printer_ = nullptr;
        else if (listener == default_xml_generator_)
            default_xml_generator_ = nullptr;
        return repeater_->Release(listener);
    }

    // Returns repeater that broadcasts the TestEventListener events to all
    // subscribers.
    TestEventListener* TestEventListeners::repeater() {
        return repeater_;
    }


    // Sets the default_result_printer attribute to the provided listener.
    // The listener is also added to the listener list and previous
    // default_result_printer is removed from it and deleted. The listener can
    // also be NULL in which case it will not be added to the list. Does
    // nothing if the previous and the current listener objects are the same.
    void TestEventListeners::SetDefaultResultPrinter(TestEventListener* listener) {
        if (default_result_printer_ != listener) {
            // It is an error to pass this method a listener that is already in the
            // list.
            delete Release(default_result_printer_);
            default_result_printer_ = listener;
            if (listener != nullptr)
                Append(listener);
        }
    }


    // Sets the default_xml_generator attribute to the provided listener.  The
    // listener is also added to the listener list and previous
    // default_xml_generator is removed from it and deleted. The listener can
    // also be NULL in which case it will not be added to the list. Does
    // nothing if the previous and the current listener objects are the same.
    void TestEventListeners::SetDefaultXmlGenerator(TestEventListener* listener) {
        if (default_xml_generator_ != listener) {
            // It is an error to pass this method a listener that is already in the
            // list.
            delete Release(default_xml_generator_);
            default_xml_generator_ = listener;
            if (listener != nullptr)
                Append(listener);
        }
    }

    // Controls whether events will be forwarded by the repeater to the
    // listeners in the list.
    bool TestEventListeners::EventForwardingEnabled() const {
        return repeater_->forwarding_enabled();
    }

    void TestEventListeners::SuppressEventForwarding(bool suppress) {
        repeater_->set_forwarding_enabled(!suppress);
    }


}