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
 * FileOperationCIFSReadOnlyTest.cpp
 *
 *  Created on: May 15, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../MetaManager.h"
#include "../FileOperation.h"
#include "../FileOperationCIFSReadOnly.h"
#include "FileOperationCIFSReadOnlyTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileOperationCIFSReadOnlyTest );


static const string metaFolder = "meta-folder";
static const string tapeFolder = "tape-folder";
static const string testFile = "/test-file";

static const fs::path folderMeta = metaFolder;
static const fs::path folderTape = tapeFolder;
static const fs::path fileTest = folderTape / testFile;


void
FileOperationCIFSReadOnlyTest::setUp()
{
    fs::create_directory(folderMeta);
    fs::create_directory(folderTape);

    Factory::SetMetaFolder(folderMeta);
}


void
FileOperationCIFSReadOnlyTest::tearDown()
{
    fs::remove_all(folderMeta);
    fs::remove_all(folderTape);
}


void
FileOperationCIFSReadOnlyTest::testFileOperation()
{
    testFileOperationSub(512*1024);
    testFileOperationSub(1024*1024);
    testFileOperationSub(2*1024*1024);
}

void
FileOperationCIFSReadOnlyTest::testFileOperationSub(size_t sizeFile)
{
    MetaManager meta;
    auto_ptr<Inode> inode;
    auto_ptr<FileOperationInterface> oper;

    char bufferWrite[1024] = "hello,world!\n";
    char bufferRead[sizeof(bufferWrite)];
    size_t size;
    boost::posix_time::ptime begin;
    boost::posix_time::ptime current;

    inode.reset(meta.CreateFile(testFile,0600));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == inode->SetSize(sizeFile) );


    oper.reset( new FileOperationCIFSReadOnly( meta.GetInode(testFile),
            new FileOperation(testFile), 10 ) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFile(O_RDWR,0600,false) );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno );


    oper.reset( new FileOperationCIFS( meta.GetInode(testFile),
            new FileOperation(fileTest), 0 ) );
    CPPUNIT_ASSERT( true == oper->CreateFile(O_RDWR,0600,false) );
    for ( off_t offset = 0; offset + sizeof(bufferWrite) <= sizeFile;
            offset += sizeof(bufferWrite) ) {
        size = 0;
        CPPUNIT_ASSERT(
                true == oper->Write(
                    offset, bufferWrite, sizeof(bufferWrite), size ) );
        CPPUNIT_ASSERT( size = sizeof(bufferWrite) );
    }


    oper.reset( new FileOperationCIFSReadOnly( meta.GetInode(testFile),
            new FileOperation(fileTest), 10 ) );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFile(O_RDWR,0600,false) );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFile(O_WRONLY) );
    CPPUNIT_ASSERT_ERRNO_( EROFS == errno );

    CPPUNIT_ASSERT( true == oper->OpenFile(O_RDWR) );

    errno = 0;
    CPPUNIT_ASSERT(
            false == oper->Write(0,bufferWrite,sizeof(bufferWrite),size) );
    CPPUNIT_ASSERT_ERRNO_( EROFS == errno );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->Truncate(2 * sizeFile) );
    CPPUNIT_ASSERT_ERRNO_( EROFS == errno );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->Truncate(sizeFile) );
    CPPUNIT_ASSERT_ERRNO_( EROFS == errno );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->Truncate(0) );
    CPPUNIT_ASSERT_ERRNO_( EROFS == errno );

    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( sizeof(bufferRead) == size );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );

    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(sizeFile),
            true == oper->Read(
                sizeFile - sizeof(bufferRead),
                bufferRead, sizeof(bufferRead), size ) );
    CPPUNIT_ASSERT( sizeof(bufferRead) == size );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );

    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == oper->Read(sizeFile,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( 0 == size );

    CPPUNIT_ASSERT( true == oper->Delete() );
    CPPUNIT_ASSERT( true == meta.DeleteInode(testFile) );
}

