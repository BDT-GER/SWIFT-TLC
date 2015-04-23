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
 * FileCollectionTest.cpp
 *
 *  Created on: Dec 25, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "FileCollectionTest.h"
#include "../FileCollection.h"
#include "../FileCompare.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileCollectionTest );


//testDirectory should be less than testFile in name compare to test
static const string testFolder = "test-folder";
static const string testDirectory = "/test-directory";
static const string testFile0 = "/test-file0";
static const string testFile1 = testDirectory + "/test-file1";
static const string testFile2 = "/test-file2";
static const string testFile3 = testDirectory + "/test-file3";

static const fs::path folderTest = testFolder;
static const fs::path fileTest0 = folderTest / testFile0;
static const fs::path fileTest1 = folderTest / testFile1;
static const fs::path fileTest2 = folderTest / testFile2;
static const fs::path fileTest3 = folderTest / testFile3;
static const fs::path fileNone = folderTest / "NotExistFile";


void
FileCollectionTest::setUp()
{
    fs::remove_all(testFolder);

    fs::create_directory(folderTest);
    fs::create_directory(folderTest/testDirectory);
    {
        ofstream test0(fileTest0.file_string().c_str());
        ofstream test1(fileTest1.file_string().c_str());
        ofstream test2(fileTest2.file_string().c_str());
        ofstream test3(fileTest3.file_string().c_str());
    }

    struct ::utimbuf times;

    times.actime = time(NULL);
    times.modtime = times.actime - 8;
    utime( fileTest0.file_string().c_str(), &times );

    times.actime -= 1;
    times.modtime += 1;
    utime( fileTest1.file_string().c_str(), &times );

    times.actime -= 1;
    times.modtime += 1;
    utime( fileTest2.file_string().c_str(), &times );

    times.actime -= 1;
    times.modtime += 1;
    utime( fileTest3.file_string().c_str(), &times );
}


void
FileCollectionTest::tearDown()
{
    fs::remove_all(testFolder);
}


void
FileCollectionTest::testFileCompare()
{
    auto_ptr<FileCompare> compare;


    compare.reset( new FileCompare(
            FileCompare::COMPARE_TIME_ACCESS,
            FileCompare::DIRECTION_LESS ) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileNone) );

    compare.reset( new FileCompare(
            FileCompare::COMPARE_TIME_ACCESS,
            FileCompare::DIRECTION_LESS_EQUAL ) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileNone) );

    compare.reset( new FileCompare(
            FileCompare::COMPARE_TIME_ACCESS,
            FileCompare::DIRECTION_GREATER ) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileNone) );

    compare.reset( new FileCompare(
            FileCompare::COMPARE_TIME_ACCESS,
            FileCompare::DIRECTION_GREATER_EQUAL ) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileNone) );


    compare.reset( new FileCompare(
            FileCompare::COMPARE_TIME_MODIFY,
            FileCompare::DIRECTION_LESS ) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileNone) );

    compare.reset( new FileCompare(
            FileCompare::COMPARE_TIME_MODIFY,
            FileCompare::DIRECTION_LESS_EQUAL ) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileNone) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileNone) );

    compare.reset( new FileCompare(
            FileCompare::COMPARE_TIME_MODIFY,
            FileCompare::DIRECTION_GREATER ) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileNone,fileNone) );

    compare.reset( new FileCompare(
            FileCompare::COMPARE_TIME_MODIFY,
            FileCompare::DIRECTION_GREATER_EQUAL ) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest0,fileTest0) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest0,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest1,fileTest1) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest1,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest2,fileTest2) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest2,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileTest3,fileTest3) );
    CPPUNIT_ASSERT( false == compare->operator ()(fileTest3,fileNone) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest0) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest1) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest2) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileTest3) );
    CPPUNIT_ASSERT( true == compare->operator ()(fileNone,fileNone) );
}


void
FileCollectionTest::testGetNextFile()
{
    auto_ptr<FileCollection> collection;
    collection.reset(new FileCollection(folderTest));

    fs::path path;

    CPPUNIT_ASSERT( false == collection->GetNextFile(path,fileTest0,false) );
    CPPUNIT_ASSERT( false == collection->GetNextFile(path,fileTest1,false) );
    CPPUNIT_ASSERT( false == collection->GetNextFile(path,fileTest2,false) );
    CPPUNIT_ASSERT( false == collection->GetNextFile(path,fileTest3,false) );

    CPPUNIT_ASSERT( false == collection->GetNextFile(path,testFile0,true) );
    CPPUNIT_ASSERT( false == collection->GetNextFile(path,testFile1,true) );
    CPPUNIT_ASSERT( false == collection->GetNextFile(path,testFile2,true) );
    CPPUNIT_ASSERT( false == collection->GetNextFile(path,testFile3,true) );

    CPPUNIT_ASSERT(
            true == collection->Sort(FileCollection::SORT_TIME_MODIFY) );

    CPPUNIT_ASSERT( true == collection->GetNextFile(path,fileTest0,false) );
    CPPUNIT_ASSERT_MESSAGE( path.file_string(), path == fileTest1 );
    CPPUNIT_ASSERT( true == collection->GetNextFile(path,fileTest1,false) );
    CPPUNIT_ASSERT( path == fileTest2 );
    CPPUNIT_ASSERT( true == collection->GetNextFile(path,fileTest2,false) );
    CPPUNIT_ASSERT( path == fileTest3 );
    CPPUNIT_ASSERT( false == collection->GetNextFile(path,fileTest3,false) );

    CPPUNIT_ASSERT( true == collection->GetNextFile(path,testFile0,true) );
    CPPUNIT_ASSERT( path == testFile1 );
    CPPUNIT_ASSERT( true == collection->GetNextFile(path,testFile1,true) );
    CPPUNIT_ASSERT( path == testFile2 );
    CPPUNIT_ASSERT( true == collection->GetNextFile(path,testFile2,true) );
    CPPUNIT_ASSERT( path == testFile3 );
    CPPUNIT_ASSERT( false == collection->GetNextFile(path,testFile3,true) );
}

