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
 * TapeManagerLETest.cpp
 *
 *  Created on: Mar 6, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../TapeManagerLE.h"
#include "TapeManagerLETest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( TapeManagerLETest );


const string testFolder = "test.folder/";
const string testTape0 = "barcode0";
const string testTape1 = "barcode1";
const string testTape2 = "barcode2";
const string testTape3 = "barcode3";
const string configFile = "ltfs.config";

const fs::path folderTest(testFolder);
const fs::path folderTape0(testFolder+testTape0);
const fs::path folderTape1(testFolder+testTape1);
const fs::path folderTape2(testFolder+testTape2);
const fs::path folderTape3(testFolder+testTape3);
const fs::path fileConfig(testFolder+configFile);


void
TapeManagerLETest::setUp()
{
    fs::create_directory(folderTest);
    fs::create_directory(folderTape0);
    fs::create_directory(folderTape1);
    fs::create_directory(folderTape2);
    fs::create_directory(folderTape3);

    ofstream file(fileConfig.file_string().c_str());
    file << testTape0 << " 0 /test0/\n";
    file << testTape1 << " 1 .test1\n";
    file << testTape2 << " 2 test2\n";
    file << testTape3 << " 3\n";
    file << endl;

    Factory::SetTapeFolder(folderTest);
}


void
TapeManagerLETest::tearDown()
{
    fs::remove_all(folderTest);
    CPPUNIT_ASSERT( ! fs::exists(folderTest) );
}


void
TapeManagerLETest::testFolder()
{
    auto_ptr<TapeManagerInterface> tapeManager(new TapeManagerLE());
    dynamic_cast<TapeManagerLE *>(tapeManager.get())->Refresh(fileConfig);
    string tape;

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test0/1.txt",0) );
    CPPUNIT_ASSERT( tape == testTape0 );
    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus(testTape0)
            == TapeManager::STATUS_ONLINE );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test/1.test1",0) );
    CPPUNIT_ASSERT( tape == testTape1 );
    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus(testTape1)
            == TapeManager::STATUS_ONLINE );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test/test2.txt",0) );
    CPPUNIT_ASSERT( tape == testTape2 );
    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus(testTape2)
            == TapeManager::STATUS_ONLINE );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test3.txt",0) );
    CPPUNIT_ASSERT( tape == testTape3 );
    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus(testTape3)
            == TapeManager::STATUS_ONLINE );

    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus("none")
            == TapeManager::STATUS_EXPORT );
    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus(configFile)
            == TapeManager::STATUS_EXPORT );

    CPPUNIT_ASSERT(
            tapeManager->GetPath(testTape1,"none")
            == folderTest/testTape1/"none" );
}

