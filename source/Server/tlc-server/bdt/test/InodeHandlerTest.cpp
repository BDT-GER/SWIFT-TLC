/* Copyright (c) 2014 BDT Media Automation GmbH
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
 * InodeHandlerTest.cpp
 *
 *  Created on: Nov 14, 2014
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../InodeHandler.h"
#include "InodeHandlerTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( InodeHandlerTest );


static const string testFile = "test-file";
static const fs::path fileTest(testFile);


void
InodeHandlerTest::setUp()
{
}


void
InodeHandlerTest::tearDown()
{
    fs::remove(fileTest);
    CPPUNIT_ASSERT( ! fs::exists(fileTest) );
}


void
InodeHandlerTest::testInodeHandler()
{
    CPPUNIT_FAIL("TODO");
}

