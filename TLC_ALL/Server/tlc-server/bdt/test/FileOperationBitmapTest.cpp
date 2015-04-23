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
 * FileOperationBitmapTest.cpp
 *
 *  Created on: Mar 12, 2013
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../FileOperationBitmap.h"
#include "FileOperationBitmapTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileOperationBitmapTest );


static const string testFile = "FileOperationBitmapTest";

static const fs::path fileTest(testFile);


void
FileOperationBitmapTest::setUp()
{
}


void
FileOperationBitmapTest::tearDown()
{
    if ( fs::exists(fileTest) ) {
        fs::remove(fileTest);
    }
}


void
FileOperationBitmapTest::testConstructor()
{
    auto_ptr<FileOperationBitmap> file;

    CPPUNIT_ASSERT_THROW(
            file.reset(new FileOperationBitmap(fileTest,O_RDWR,1024)),
            ExceptionFileNonExist );

    file.reset(new FileOperationBitmap(fileTest,0644,O_RDWR,1024)),
    CPPUNIT_ASSERT( NULL != file.get() );

    CPPUNIT_ASSERT_THROW(
            file.reset(new FileOperationBitmap(fileTest,0644,O_RDWR,1024)),
            ExceptionFileExistedAlready );

    file.reset(new FileOperationBitmap(fileTest,O_RDWR,1024)),
    CPPUNIT_ASSERT( NULL != file.get() );
}


void
FileOperationBitmapTest::testTruncateBitmap()
{
    auto_ptr<FileOperationBitmap> file;
    char buffer[1024];
    int bufsize = sizeof(buffer);
    memset(buffer,'C',bufsize);
    size_t size;

    file.reset(new FileOperationBitmap(fileTest,0644,O_RDWR,bufsize)),
    CPPUNIT_ASSERT( NULL != file.get() );

    CPPUNIT_ASSERT( true == file->IsFull() );

    CPPUNIT_ASSERT( true == file->Truncate(bufsize) );
    CPPUNIT_ASSERT( true == file->IsFull() );

    CPPUNIT_ASSERT( true == file->TruncateBitmap(bufsize*2) );
    CPPUNIT_ASSERT( false == file->IsFull() );

    CPPUNIT_ASSERT( true == file->Truncate(bufsize*2) );
    CPPUNIT_ASSERT( false == file->IsFull() );

    CPPUNIT_ASSERT( true == file->Write(bufsize,buffer,bufsize,size) );
    CPPUNIT_ASSERT( true == file->IsFull() );

    CPPUNIT_ASSERT( true == file->TruncateBitmap(bufsize*3) );
    CPPUNIT_ASSERT( false == file->IsFull() );

    CPPUNIT_ASSERT( true == file->TruncateBitmap(bufsize*2) );
    CPPUNIT_ASSERT( true == file->IsFull() );
}


void
FileOperationBitmapTest::testBitmap()
{
    auto_ptr<FileOperationBitmap> file;
    char buffer[1024];
    memset(buffer,'A',sizeof(buffer));
    size_t bufsize = sizeof(buffer);
    size_t size;

    file.reset(new FileOperationBitmap(fileTest,0644,O_RDWR,bufsize));
    CPPUNIT_ASSERT( NULL != file.get() );

    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, 0 ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( 0, 1 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( 1, 0 ) );
    CPPUNIT_ASSERT( true == file->IsFull() );

    size = 0;
    CPPUNIT_ASSERT( true == file->Write(bufsize,buffer,bufsize,size) );
    CPPUNIT_ASSERT( bufsize == size );

    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, 0 ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( 0, bufsize ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( 0, bufsize + 1 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( bufsize, 0 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( bufsize, bufsize ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( bufsize, bufsize + 1 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( bufsize + 1, bufsize - 1 ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( bufsize + 1, bufsize ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( bufsize + bufsize, 0 ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( bufsize + bufsize, 1 ) );
    CPPUNIT_ASSERT( false == file->IsFull() );

    size = 0;
    CPPUNIT_ASSERT( true == file->Write(0,buffer,bufsize,size) );
    CPPUNIT_ASSERT( bufsize == size );

    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, 0 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, bufsize ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, bufsize + bufsize ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( 0, bufsize + bufsize + 1 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( bufsize, 0 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( bufsize, bufsize ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( bufsize, bufsize + 1 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( bufsize + 1, bufsize - 1 ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( bufsize + 1, bufsize ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( bufsize + bufsize, 0 ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( bufsize + bufsize, 1 ) );
    CPPUNIT_ASSERT( true == file->IsFull() );
}


void
FileOperationBitmapTest::testBitmapPartial()
{
    auto_ptr<FileOperationBitmap> file;
    char buffer[512];
    memset(buffer,'B',sizeof(buffer));
    size_t bufsize = sizeof(buffer);
    size_t size;

    file.reset(new FileOperationBitmap(fileTest,0644,O_RDWR,bufsize*2));
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == file->TruncateBitmap(0) );

    size = 0;
    CPPUNIT_ASSERT( true == file->Write(bufsize*2,buffer,bufsize,size) );
    CPPUNIT_ASSERT( bufsize == size );
    CPPUNIT_ASSERT( true == file->CheckBitmap( bufsize*2, bufsize ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( bufsize*2, bufsize + 1 ) );

    size = 0;
    CPPUNIT_ASSERT( true == file->Write(0,buffer,bufsize,size) );
    CPPUNIT_ASSERT( bufsize == size );

    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, 0 ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( 0, 1 ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( 0, bufsize ) );

    size = 0;
    CPPUNIT_ASSERT( true == file->Write(bufsize,buffer,bufsize,size) );
    CPPUNIT_ASSERT( bufsize == size );

    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, 0 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, bufsize ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, bufsize * 2 ) );

    size = 0;
    CPPUNIT_ASSERT( true == file->Write(bufsize*3,buffer,bufsize,size) );
    CPPUNIT_ASSERT( bufsize == size );
    CPPUNIT_ASSERT( true == file->CheckBitmap( bufsize*2, bufsize*2 ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( bufsize*2, bufsize*2 + 1 ) );

    size = 0;
    CPPUNIT_ASSERT( true == file->Write(bufsize*5,buffer,bufsize,size) );
    CPPUNIT_ASSERT( bufsize == size );
    CPPUNIT_ASSERT( false == file->CheckBitmap( bufsize*5, bufsize ) );
    CPPUNIT_ASSERT( false == file->CheckBitmap( bufsize*4, bufsize*2 ) );
}


void
FileOperationBitmapTest::testClose()
{
    auto_ptr<FileOperationBitmap> file;
    char buffer[1024];
    memset(buffer,'C',sizeof(buffer));
    size_t bufsize = sizeof(buffer);
    size_t size;


    file.reset(new FileOperationBitmap(fileTest,0644,O_RDWR,bufsize));
    CPPUNIT_ASSERT( NULL != file.get() );

    CPPUNIT_ASSERT( true == file->TruncateBitmap(0) );

    size = 0;
    CPPUNIT_ASSERT( true == file->Write(0,buffer,bufsize,size) );
    CPPUNIT_ASSERT( bufsize == size );

    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, 0 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, bufsize ) );


    file.reset(new FileOperationBitmap(fileTest,O_RDWR,bufsize));
    CPPUNIT_ASSERT( NULL != file.get() );

    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, 0 ) );
    CPPUNIT_ASSERT( true == file->CheckBitmap( 0, bufsize ) );
}

