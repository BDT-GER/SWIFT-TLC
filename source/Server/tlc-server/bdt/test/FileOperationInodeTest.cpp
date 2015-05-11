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
 * FileOperationInodeTest.cpp
 *
 *  Created on: Apr 1, 2013
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../FileOperationInode.h"
#include "../MetaManager.h"
#include "FileOperationInodeTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileOperationInodeTest );


static const string metaFolder = "meta";
static const string testFile = "test-file";
static const string testFolder = "test-folder";

static const fs::path folderMeta(metaFolder);
static const fs::path folderTest(testFolder);
static const fs::path folderRoot("/");
static const fs::path fileTest(testFile);


void
FileOperationInodeTest::setUp()
{
    fs::remove_all(folderMeta);

    fs::create_directory(folderMeta);

    Factory::SetMetaFolder(folderMeta);
}


void
FileOperationInodeTest::tearDown()
{
    fs::remove_all(folderMeta);
}


void
FileOperationInodeTest::testFolderOperation()
{
    auto_ptr<FileOperationInterface> folder;
    auto_ptr<FileOperationInterface> file;
    fs::path path;

    folder.reset( new FileOperationInode(folderRoot / folderTest) );

    CPPUNIT_ASSERT( false == folder->OpenFolder() );

    CPPUNIT_ASSERT( true == folder->CreateFolder(0755,false) );
    CPPUNIT_ASSERT( true == folder->OpenFolder() );

    CPPUNIT_ASSERT( false == folder->CreateFolder(0755,false) );

    CPPUNIT_ASSERT( true == folder->OpenFolder() );
    CPPUNIT_ASSERT( false == folder->ReadFolder(path) );

    file.reset( new FileOperationInode(folderRoot / folderTest / fileTest) );
    CPPUNIT_ASSERT( true == file->CreateFile(O_RDWR,0644,false) );

    CPPUNIT_ASSERT( true == folder->OpenFolder() );
    CPPUNIT_ASSERT( true == folder->ReadFolder(path) );
    CPPUNIT_ASSERT( path == folderRoot / folderTest / fileTest );
    CPPUNIT_ASSERT( false == folder->ReadFolder(path) );

    file.reset( new FileOperationInode(folderRoot / folderTest / folderTest) );
    CPPUNIT_ASSERT( true == file->CreateFolder(0755,false) );

    CPPUNIT_ASSERT( true == folder->OpenFolder() );
    CPPUNIT_ASSERT( true == folder->ReadFolder(path) );
    if ( path == folderRoot / folderTest / fileTest ) {
        CPPUNIT_ASSERT( true == folder->ReadFolder(path) );
        CPPUNIT_ASSERT( path == folderRoot / folderTest / folderTest );
    } else {
        CPPUNIT_ASSERT( path == folderRoot / folderTest / folderTest );
        CPPUNIT_ASSERT( true == folder->ReadFolder(path) );
        CPPUNIT_ASSERT( path == folderRoot / folderTest / fileTest );
    }
    CPPUNIT_ASSERT( false == folder->ReadFolder(path) );
}

