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
 * ReadManagerTest.cpp
 *
 *  Created on: Dec 02, 2014
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../CacheManager.h"
#include "../ReadManager.h"
#include "ReadManagerTest.h"
#include "../FileOperationBitmap.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ReadManagerTest );


static const string cacheFolder = "ReadManagerTest.cache";


void
ReadManagerTest::setUp()
{
    fs::remove_all(cacheFolder);
    fs::create_directory(cacheFolder);
    Factory::SetCacheFolder(cacheFolder);
    Factory::CreateCacheManager();
    Factory::CreateReadManager();
}


void
ReadManagerTest::tearDown()
{
    Factory::ReleaseReadManager();
    Factory::ReleaseCacheManager();
    fs::remove_all(cacheFolder);
}


void
ReadManagerTest::testRead()
{
    CacheManager * cache = Factory::GetCacheManager();
    ReadManager * read = Factory::GetReadManager();
    string tape = "barcode0";
    unsigned long long number;
    boost::posix_time::ptime begin, current;
    int duration;
    size_t bufsize = 512 * 1024;

    CPPUNIT_ASSERT( true == cache->CreateNewFile(number) );
    CPPUNIT_ASSERT( false == read->NeedRead(number) );

    CPPUNIT_ASSERT( true == cache->DeleteFile(number) );
    CPPUNIT_ASSERT( true == read->NeedRead(number) );

    CPPUNIT_ASSERT( true == read->ReadBegin(number,tape,10) );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == read->CheckRead(number,0,bufsize) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration),
            duration >= 10 && duration <= 20 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == read->CheckRead(number,0,bufsize) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 2 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == read->CheckRead(number,bufsize,bufsize) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration),
            duration >= 8 && duration <= 12 );

    Factory::ReleaseReadManager();
    Factory::CreateReadManager();
    read = Factory::GetReadManager();

    CPPUNIT_ASSERT( true == read->NeedRead(number) );

    CPPUNIT_ASSERT( true == read->ReadBegin(number,tape,10) );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == read->CheckRead(number,0,bufsize*2) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 2 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == read->CheckRead(number,bufsize*2,bufsize) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration),
            duration >= 8 && duration <= 12 );
}


void
ReadManagerTest::testPreRead()
{
    CacheManager * cache = Factory::GetCacheManager();
    ReadManager * read = Factory::GetReadManager();
    string tape = "barcode0";
    unsigned long long number;
    const size_t bufsize = 512 * 1024;

    for ( int i = 1; i < 5; ++ i ) {
        CPPUNIT_ASSERT( true == cache->CreateNewFile(number) );
        CPPUNIT_ASSERT( i == (int)number );
    }
    for ( int i = 1; i < 5; ++ i ) {
        CPPUNIT_ASSERT( true == cache->DeleteFile(i) );
    }

    CPPUNIT_ASSERT( true == read->NeedRead(1) );
    CPPUNIT_ASSERT( true == read->ReadBegin(1,tape,10) );
    CPPUNIT_ASSERT( true == read->CheckRead(1,0,1024) );
    CPPUNIT_ASSERT( true == read->ReadEnd(1) );

    boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
    auto_ptr<FileOperationBitmap> file1;
    file1.reset(cache->GetFileOperationBitmap(1,O_RDONLY));
    CPPUNIT_ASSERT( NULL != file1.get() );
    CPPUNIT_ASSERT( true == file1->CheckBitmap(0,bufsize) );
    CPPUNIT_ASSERT( true == file1->CheckBitmap(bufsize,bufsize) );
    off_t offset = bufsize * 2;
    if ( true == file1->CheckBitmap(offset,bufsize) ) {
        offset += bufsize;
    }
    CPPUNIT_ASSERT( false == file1->CheckBitmap(offset,bufsize) );

    boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
    auto_ptr<FileOperationBitmap> file2;
    file2.reset(cache->GetFileOperationBitmap(1,O_RDONLY));
    CPPUNIT_ASSERT( NULL != file2.get() );
    CPPUNIT_ASSERT( true == file2->CheckBitmap(offset,bufsize) );
    offset += bufsize;
    if ( true == file2->CheckBitmap(offset,bufsize) ) {
        offset += bufsize;
    }
    CPPUNIT_ASSERT( false == file2->CheckBitmap(offset,bufsize) );
}

