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
 * TapeManagerTest.cpp
 *
 *  Created on: Mar 2, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../TapeManager.h"
#include "TapeManagerTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( TapeManagerTest );


const string configFile = "test.config";

const fs::path fileConfig(configFile);


void
TapeManagerTest::setUp()
{
    ofstream config(configFile.c_str());
}


void
TapeManagerTest::tearDown()
{
    fs::remove(fileConfig);
    CPPUNIT_ASSERT( ! fs::exists(fileConfig) );
}


void
TapeManagerTest::testConfig()
{
    ofstream config(configFile.c_str());
    config << "# 0 to begin with\n";
    config << "barcode0 0 /test0/\n";
    config << "  # 1 to end with\n";
    config << "barcode1 1 .txt\n";
    config << "\t# 2 to container\n";
    config << "barcode2 2 test\n";
    config << "\n# comment wrong format\n";
    config << "wrong format inputs\n";
    config << "# 3 as default\n";
    config << "barcode4 3\n";
    config << "barcode3 3\n";
    config.close();

    auto_ptr<TapeManagerInterface> tapeManager( new TapeManager() );
    dynamic_cast<TapeManager *>(tapeManager.get())->Refresh(fileConfig);

    string tape;
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test0/",0) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test0/1.txt",0) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test0/test.txt",0) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test0",0) );
    CPPUNIT_ASSERT( tape == "barcode2" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/Test0/",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST0/",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test/1.txt",0) );
    CPPUNIT_ASSERT( tape == "barcode1" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test/test.txt",0) );
    CPPUNIT_ASSERT( tape == "barcode1" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test/txt.TXT",0) );
    CPPUNIT_ASSERT( tape == "barcode2" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/txt.TXT",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test/1.log",0) );
    CPPUNIT_ASSERT( tape == "barcode2" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST/1.log",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/Test/1.log",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/0.test",0) );
    CPPUNIT_ASSERT( tape == "barcode2" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/0.TEST",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/0.Test",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );

    config.open(configFile.c_str());
    config << "barcode0 0 /TEST0/\n";
    config << "barcode1 1 .TXT\n";
    config << "barcode2 2 TEST\n";
    config << "barcode4 3\n";
    config << "barcode3 3\n";
    config << endl;

    CPPUNIT_ASSERT(
            true == dynamic_cast<TapeManager *>(tapeManager.get())->Refresh(
                fileConfig ) );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST0/",0) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST0/1.txt",0) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST0/TEST.txt",0) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST0",0) );
    CPPUNIT_ASSERT( tape == "barcode2" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/Test0/",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test0/",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST/1.TXT",0) );
    CPPUNIT_ASSERT( tape == "barcode1" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST/TEST.TXT",0) );
    CPPUNIT_ASSERT( tape == "barcode1" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST/txt.txt",0) );
    CPPUNIT_ASSERT( tape == "barcode2" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/txt.txt",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST/1.log",0) );
    CPPUNIT_ASSERT( tape == "barcode2" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/test/1.log",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/Test/1.log",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/0.TEST",0) );
    CPPUNIT_ASSERT( tape == "barcode2" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/0.test",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );
    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/0.Test",0) );
    CPPUNIT_ASSERT( tape == "barcode3" );
}


void
TapeManagerTest::testTapeUse()
{
    ofstream config(configFile.c_str());
    config << "barcode0 0 /TEST0/\n";
    config.close();

    auto_ptr<TapeManagerInterface> tapeManager( new TapeManager() );
    dynamic_cast<TapeManager *>(tapeManager.get())->Refresh(fileConfig);
    string tape;
    size_t fileNumber;
    off_t usedSize,freeSize;
    const off_t totalCapacity = 1024LL * 1024 * 1024 * 1024;

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST0/",1024) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 0 );
    CPPUNIT_ASSERT( usedSize == 0 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );
    CPPUNIT_ASSERT( true == tapeManager->SetTapeUse(tape,0,1024,0) );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 0 );
    CPPUNIT_ASSERT( usedSize == 1024 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(tape,"/TEST0/1.txt",4096) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 0 );
    CPPUNIT_ASSERT( usedSize == 1024 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );
    CPPUNIT_ASSERT( true == tapeManager->SetTapeUse(tape,1,4096,0) );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 1 );
    CPPUNIT_ASSERT( usedSize == 5120 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(
            tape, "/TEST0/folder", 1024 ) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 1 );
    CPPUNIT_ASSERT( usedSize == 5120 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );
    CPPUNIT_ASSERT( true == tapeManager->SetTapeUse(tape,0,1024,0) );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 1 );
    CPPUNIT_ASSERT( usedSize == 6144 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(
            tape, "/TEST0/folder/1.txt", 4096 ) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 1 );
    CPPUNIT_ASSERT( usedSize == 6144 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );
    CPPUNIT_ASSERT( true == tapeManager->SetTapeUse(tape,1,4096,0) );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 2 );
    CPPUNIT_ASSERT( usedSize == 10240 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );

    CPPUNIT_ASSERT( true == tapeManager->GetTapeUse(
            tape, "/TEST0/2.txt", freeSize ) );
    CPPUNIT_ASSERT( tape == "barcode0" );
    CPPUNIT_ASSERT( false == tapeManager->GetTapeUse(
            tape, "/TEST0/2.txt", freeSize + 1 ) );

    CPPUNIT_ASSERT( true == tapeManager->SetTapeUse(tape,-1,-4096,0) );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 1 );
    CPPUNIT_ASSERT( usedSize == 6144 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );

    CPPUNIT_ASSERT( true == tapeManager->SetTapeUse(tape,-1,-4096,0) );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 0 );
    CPPUNIT_ASSERT( usedSize == 2048 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );

    CPPUNIT_ASSERT( true == tapeManager->SetTapeUse(tape,-1,-4096,0) );
    CPPUNIT_ASSERT( true == tapeManager->GetCapacity(
            tape, fileNumber, usedSize, freeSize ) );
    CPPUNIT_ASSERT( fileNumber == 0 );
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(usedSize),
            usedSize == 0 );
    CPPUNIT_ASSERT( freeSize == totalCapacity - usedSize );
}

