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
 * TapeManagerSETest.cpp
 *
 *  Created on: Jul 25, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../TapeManagerSE.h"
#include "TapeManagerSETest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( TapeManagerSETest );


const string tapeFolder = "tape.folder/";
const string configFile = "ltfs.config";

const fs::path folderTape(tapeFolder);


void
TapeManagerSETest::setUp()
{
    fs::create_directory(folderTape);

    Factory::ReleaseTapeLibraryManager();
    Factory::SetTapeFolder(folderTape);
    Factory::CreateTapeLibraryManager();
}


void
TapeManagerSETest::tearDown()
{
    fs::remove_all(folderTape);

    Factory::ReleaseTapeLibraryManager();
}


void
TapeManagerSETest::testFolder()
{
    auto_ptr<TapeManager> tapeManager(new TapeManagerSE());
    tape::TapeLibraryManager * changerManager =
            Factory::GetTapeLibraryManager();
    vector<tape::Changer> changers;
    vector<tape::Drive> drives;
    vector<tape::Cartridge> cartridges;
    tape::Error error;

    CPPUNIT_ASSERT( true == changerManager->GetChangerList(changers,error) );

    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus("01000001")
            == TapeManager::STATUS_ONLINE );

    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus("01000002")
            == TapeManager::STATUS_OFFLINE );

    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus("02000001")
            == TapeManager::STATUS_ONLINE );

    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus("02000002")
            == TapeManager::STATUS_OFFLINE );

    CPPUNIT_ASSERT(
            tapeManager->GetTapeStatus("03000001")
            == TapeManager::STATUS_EXPORT );

    CPPUNIT_ASSERT(
            tapeManager->GetPath("01000001","none")
            == folderTape / "01000001" / "none" );
}

