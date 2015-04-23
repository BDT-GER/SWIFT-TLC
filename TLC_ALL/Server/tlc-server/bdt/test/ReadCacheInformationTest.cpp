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
 * ReadCacheInformationTest.cpp
 *
 *  Created on: Aug 31, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "../ReadCacheInformation.h"
#include "ReadCacheInformationTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( ReadCacheInformationTest );

void
ReadCacheInformationTest::setUp()
{

}


void
ReadCacheInformationTest::tearDown()
{

}

void
ReadCacheInformationTest::testGetTempPath()
{
	START_TEST(__func__);
	auto_ptr<ReadCacheInformation> information(new ReadCacheInformation());
	fs::path cachePath = "/usr/LTFStor/cache/aa.txt";
	fs::path tmpCache1 = "/usr/LTFStor/cache/Temp/aa.txt";
	fs::path tmpCache2 = information->GetTempPath(cachePath);
	fprintf(stderr, "tmpCache1 is %s\n", tmpCache1.file_string().c_str());
	fprintf(stderr, "tmpCache2 is %s\n", tmpCache2.file_string().c_str());
	CPPUNIT_ASSERT(tmpCache2==tmpCache1);

	END_TEST(__func__);
}
