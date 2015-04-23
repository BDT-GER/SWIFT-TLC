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
 * ConfigureTest.cpp
 *
 *  Created on: Aug 31, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ConfigureTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ConfigureTest );


const string configFile = "bdt-vfs.config";

const fs::path fileConfig(configFile);


void
ConfigureTest::setUp()
{
    ofstream config(configFile.c_str());
}


void
ConfigureTest::tearDown()
{
    fs::remove(fileConfig);
}


void
ConfigureTest::testConfigure()
{
    ofstream config(configFile.c_str());
    config << Configure::CacheFreeMinPercent << " : 5" << endl;
    config << Configure::CacheFreeMaxPercent << " : 10" << endl;
    config << Configure::CacheFileSizeRead << " : 1000000000" << endl;
    config << Configure::FileMaxSize << " : 8000000000" << endl;

    Configure configure;
    configure.Refresh(fileConfig);
    CPPUNIT_ASSERT(
            configure.GetValue(Configure::CacheFreeMinPercent) == "5");
    CPPUNIT_ASSERT(
            configure.GetValueSize(Configure::CacheFreeMinPercent) == 5);
    CPPUNIT_ASSERT(
            configure.GetValue(Configure::CacheFreeMaxPercent) == "10");
    CPPUNIT_ASSERT(
            configure.GetValueSize(Configure::CacheFreeMaxPercent) == 10);
    CPPUNIT_ASSERT(
            configure.GetValue(Configure::CacheFileSizeRead) == "1000000000");
    CPPUNIT_ASSERT(
            configure.GetValueSize(Configure::CacheFileSizeRead)
                == 1000000000LL );
    CPPUNIT_ASSERT(
            configure.GetValue(Configure::FileMaxSize) == "8000000000");
    CPPUNIT_ASSERT(
            configure.GetValueSize(Configure::FileMaxSize) == 8000000000LL );
}
