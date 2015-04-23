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
 * CacheManagerTest.cpp
 *
 *  Created on: Apr 10, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../CacheManager.h"
#include "CacheManagerTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CacheManagerTest );


static const string cacheFolder("cache.folder");
static const string cacheFolderNone("cache.folder-none");
static const string cacheFile("cache.file");


void
CacheManagerTest::setUp()
{
    fs::create_directory(cacheFolder);
    ofstream file(cacheFile.c_str());

    Factory::SetCacheFolder(cacheFolder);
    Factory::CreateCacheManager();
}


void
CacheManagerTest::tearDown()
{
    Factory::ReleaseCacheManager();

    fs::remove_all(cacheFolder);
    fs::remove_all(cacheFolderNone);
    fs::remove(cacheFile);
}


void
CacheManagerTest::testConstructor()
{
    auto_ptr<CacheManager> cache;

    try {
        Factory::SetCacheFolder(cacheFolder);
        cache.reset( new CacheManager() );
    } catch (const std::exception & e) {
        CPPUNIT_FAIL(e.what());
    }
    CPPUNIT_ASSERT( fs::is_directory(cacheFolder) );

    CPPUNIT_ASSERT( ! fs::exists(cacheFolderNone) );
    try {
        Factory::SetCacheFolder(cacheFolderNone);
        cache.reset( new CacheManager() );
    } catch (const std::exception & e) {
        CPPUNIT_FAIL(e.what());
    }
    CPPUNIT_ASSERT( fs::is_directory(cacheFolderNone) );

    try {
        Factory::SetCacheFolder(cacheFile);
        cache.reset( new CacheManager() );
    } catch (const PathNotExistException & e) {
        CPPUNIT_ASSERT( fs::is_regular_file(cacheFile) );
        CPPUNIT_ASSERT( e.what() == cacheFile );
        return;
    } catch (const std::exception & e) {
        CPPUNIT_FAIL(e.what());
    }
    CPPUNIT_FAIL( "Exception should be raised for file as cache folder" );
}


void
CacheManagerTest::testFile()
{
    CacheManager * cache = Factory::GetCacheManager();
    const unsigned long long bigNumber = 1024LL * 1024 * 1024 * 1024;
    unsigned long long number = 0;

    CPPUNIT_ASSERT( false == cache->ExistFile(0) );
    CPPUNIT_ASSERT( false == cache->ExistFile(1) );
    CPPUNIT_ASSERT( true == cache->CreateNewFile(number) );
    CPPUNIT_ASSERT( 1 == number );
    CPPUNIT_ASSERT( true == cache->CreateNewFile(number) );
    CPPUNIT_ASSERT( 2 == number );

    Factory::ReleaseCacheManager();
    Factory::CreateCacheManager();
    cache = Factory::GetCacheManager();

    CPPUNIT_ASSERT( false == cache->ExistFile(0) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( true == cache->ExistFile(2) );
    CPPUNIT_ASSERT( false == cache->ExistFile(3) );
    CPPUNIT_ASSERT( true == cache->CreateFile(bigNumber) );
    CPPUNIT_ASSERT( false == cache->ExistFile(bigNumber - 1) );
    CPPUNIT_ASSERT( true == cache->ExistFile(bigNumber) );
    CPPUNIT_ASSERT( true == cache->CreateNewFile(number) );
    CPPUNIT_ASSERT( bigNumber + 1 == number );
    CPPUNIT_ASSERT( true == cache->ExistFile(bigNumber + 1) );

    Factory::ReleaseCacheManager();
    Factory::CreateCacheManager();
    cache = Factory::GetCacheManager();

    CPPUNIT_ASSERT( false == cache->ExistFile(0) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( true == cache->ExistFile(2) );
    CPPUNIT_ASSERT( false == cache->ExistFile(3) );
    CPPUNIT_ASSERT( false == cache->ExistFile(bigNumber - 1) );
    CPPUNIT_ASSERT( true == cache->ExistFile(bigNumber) );
    CPPUNIT_ASSERT( true == cache->ExistFile(bigNumber + 1) );
    CPPUNIT_ASSERT( true == cache->CreateNewFile(number) );
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(number),
            bigNumber + 2 == number );
    CPPUNIT_ASSERT( true == cache->ExistFile(bigNumber + 2) );

    for ( unsigned long long i = 0; i < 4; ++ i ) {
        CPPUNIT_ASSERT( true == cache->DeleteFile(i) );
        CPPUNIT_ASSERT( false == cache->ExistFile(i) );
    }
    for ( unsigned long long i = bigNumber - 1; i < bigNumber + 3; ++ i ) {
        CPPUNIT_ASSERT( true == cache->DeleteFile(i) );
        CPPUNIT_ASSERT( false == cache->ExistFile(i) );
    }
    CPPUNIT_ASSERT( true == cache->CreateNewFile(number) );
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(number),
            bigNumber + 3 == number );
    CPPUNIT_ASSERT( true == cache->ExistFile(bigNumber + 3) );
}


void
CacheManagerTest::testFileOperation()
{
    CPPUNIT_FAIL("TODO");
}

