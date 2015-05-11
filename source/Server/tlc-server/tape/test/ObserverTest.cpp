/* Copyright (c) 2012 BDT Media Automation GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ObserverTest.cpp
 *
 *  Created on: Jul 4, 2012
 *      Author: More Zeng
 */

#include "stdafx.h"
#include "ObserverTest.h"

#include "../ObserverSubject.h"
#include "../ObserverListener.h"

#include "ObserverListenerSub.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ObserverTest );


void
ObserverTest::setUp()
{
}


void
ObserverTest::tearDown()
{
}


void
ObserverTest::testObserver()
{
    ObserverListenerSub::UpdateCount = 0;

    auto_ptr<ObserverListenerSub> listener0, listener1;
    auto_ptr<ObserverSubject> subject(new ObserverSubject);
    listener0.reset(new ObserverListenerSub());
    listener1.reset(new ObserverListenerSub());

    subject->Notify();
    CPPUNIT_ASSERT( 0 == ObserverListenerSub::UpdateCount );

    CPPUNIT_ASSERT( true == subject->Attach(listener0.get()) );
    subject->Notify();
    CPPUNIT_ASSERT( 1 == ObserverListenerSub::UpdateCount );
    subject->Notify();
    CPPUNIT_ASSERT( 2 == ObserverListenerSub::UpdateCount );

    CPPUNIT_ASSERT( false == subject->Attach(listener0.get()) );
    subject->Notify();
    CPPUNIT_ASSERT( 3 == ObserverListenerSub::UpdateCount );
    subject->Notify();
    CPPUNIT_ASSERT( 4 == ObserverListenerSub::UpdateCount );

    CPPUNIT_ASSERT( true == subject->Attach(listener1.get()) );
    subject->Notify();
    CPPUNIT_ASSERT( 6 == ObserverListenerSub::UpdateCount );
    subject->Notify();
    CPPUNIT_ASSERT( 8 == ObserverListenerSub::UpdateCount );

    CPPUNIT_ASSERT( true == subject->Detach(listener0.get()) );
    subject->Notify();
    CPPUNIT_ASSERT( 9 == ObserverListenerSub::UpdateCount );
    subject->Notify();
    CPPUNIT_ASSERT( 10 == ObserverListenerSub::UpdateCount );

    CPPUNIT_ASSERT( true == subject->Detach(listener0.get()) );
    subject->Notify();
    CPPUNIT_ASSERT( 11 == ObserverListenerSub::UpdateCount );
    subject->Notify();
    CPPUNIT_ASSERT( 12 == ObserverListenerSub::UpdateCount );

    CPPUNIT_ASSERT( true == subject->Detach(listener1.get()) );
    subject->Notify();
    CPPUNIT_ASSERT( 12 == ObserverListenerSub::UpdateCount );
    subject->Notify();
    CPPUNIT_ASSERT( 12 == ObserverListenerSub::UpdateCount );

    CPPUNIT_ASSERT( true == subject->Detach(listener1.get()) );
    subject->Notify();
    CPPUNIT_ASSERT( 12 == ObserverListenerSub::UpdateCount );
    subject->Notify();
    CPPUNIT_ASSERT( 12 == ObserverListenerSub::UpdateCount );
}

