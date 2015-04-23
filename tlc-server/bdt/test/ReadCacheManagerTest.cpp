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
 * ReadCacheManagerTest.cpp
 *
 *  Created on: Sep 5, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ReadCacheManagerTest.h"
#include "../ReadCacheManager.h"
#include "../FileOperationDelay.h"
#include "../FileOperationBitmap.h"
#include "../FileOperationReadWriteCache.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ReadCacheManagerTest );


const string cacheFolder = "test.cache";
const string testFile = "test.file";
const string testFileOrig = "test.file.orig";
static const string metaFolder = "meta-folder";

const fs::path folderCache(cacheFolder);
const fs::path fileTest(cacheFolder+"/"+testFile);
const fs::path fileTestOrig(cacheFolder+"/"+testFileOrig);
static const fs::path folderMeta(metaFolder);


void
ReadCacheManagerTest::setUp()
{
    fs::create_directory(folderCache);
    Factory::SetCacheFolder(folderCache);

    fs::create_directory(folderMeta);
    Factory::SetMetaFolder(folderMeta);
    Factory::CreateMetaManager();
}


void
ReadCacheManagerTest::tearDown()
{
    Factory::ReleaseMetaManager();
    fs::remove_all(folderMeta);

    fs::remove_all(folderCache);
}


void
ReadCacheManagerTest::testFileOperation()
{
    boost::posix_time::ptime begin;
    boost::posix_time::ptime end;
    int duration;

    auto_ptr<ReadCacheManager> manager(new ReadCacheManager());

    const int bufsize = 512 * 1024;
    char bufCmp[bufsize];
    char buf[bufsize];
    {
        ofstream file(fileTestOrig.file_string().c_str());
        for ( int i = 1; i <= 4; ++i ) {
            memset(buf,0,sizeof(buf));
            memset(buf,'a'+i,sizeof(buf)-1);
            file << buf << endl;
        }
        file << "end" << endl;
    }

    CPPUNIT_ASSERT( ! fs::exists(fileTest) );

    CPPUNIT_ASSERT( NULL == manager->GetFileOperationRead(fileTestOrig) );
    CPPUNIT_ASSERT( NULL == manager->GetFileOperationRead(fileTest) );

    CPPUNIT_ASSERT( ! fs::exists(fileTest) );

    auto_ptr<FileOperationInterface> fileOrig(
            new FileOperationDelay(fileTestOrig,20) );

    begin = boost::posix_time::microsec_clock::local_time();
    off_t offset;

    auto_ptr<FileOperationInterface> file(
            manager->GetFileOperationRead(fileOrig.release(),fileTest) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == file->OpenFile(O_RDONLY) );

    boost::this_thread::sleep(
            boost::posix_time::milliseconds(5) );

    CPPUNIT_ASSERT( fs::exists(fileTest) );
    CPPUNIT_ASSERT( 0 == fs::file_size(fileTest) );

    auto_ptr<FileOperationInterface> file1(
            manager->GetFileOperationRead(fileTest) );
    CPPUNIT_ASSERT( NULL != file1.get() );
    CPPUNIT_ASSERT( true == file1->OpenFile(O_RDONLY) );

    CPPUNIT_ASSERT( fs::exists(fileTest) );
    CPPUNIT_ASSERT( 0 == fs::file_size(fileTest) );

    auto_ptr<FileOperationInterface> file2(
            manager->GetFileOperationRead(fileTest) );
    CPPUNIT_ASSERT( NULL != file2.get() );
    CPPUNIT_ASSERT( true == file2->OpenFile(O_RDONLY) );

    CPPUNIT_ASSERT( fs::exists(fileTest) );
    CPPUNIT_ASSERT( 0 == fs::file_size(fileTest) );

    offset = 0;
    size_t size;
    for ( int i = 1; i <= 4; ++i ) {
        CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(offset),
                true == file->Read(offset,buf,bufsize,size) );
        CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(size),
                size == bufsize );
        end = boost::posix_time::microsec_clock::local_time();
        duration = (end - begin).total_milliseconds();
        CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
                duration >= 10 );
        memset(bufCmp,'\n',sizeof(buf));
        memset(bufCmp,'a'+i,sizeof(buf)-1);
        CPPUNIT_ASSERT( 0 == memcmp(buf,bufCmp,sizeof(bufCmp)) );

        CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(offset),
                true == file1->Read(offset,buf,bufsize,size) );
        CPPUNIT_ASSERT( size == bufsize );
        CPPUNIT_ASSERT( 0 == memcmp(buf,bufCmp,sizeof(bufCmp)) );

        offset += size;
    }
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(offset),
            true == file->Read(offset,buf,bufsize,size) );
    CPPUNIT_ASSERT( size == 4 );
    CPPUNIT_ASSERT( 0 == memcmp(buf,"end\n",size) );
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(offset),
            true == file1->Read(offset,buf,bufsize,size) );
    CPPUNIT_ASSERT( size == 4 );
    CPPUNIT_ASSERT( 0 == memcmp(buf,"end\n",size) );

    CPPUNIT_ASSERT( fs::exists(fileTest) );
    CPPUNIT_ASSERT( 4 * bufsize + 4 == fs::file_size(fileTest) );

    offset = 0;
    for ( int i = 1; i <= 4; ++i ) {
        CPPUNIT_ASSERT( true == file2->Read(offset,buf,bufsize,size) );
        CPPUNIT_ASSERT( size == bufsize );
        memset(bufCmp,'\n',sizeof(buf));
        memset(bufCmp,'a'+i,sizeof(buf)-1);
        CPPUNIT_ASSERT( 0 == memcmp(buf,bufCmp,sizeof(buf)) );
        offset += size;
    }
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(offset),
            true == file2->Read(offset,buf,bufsize,size) );
    CPPUNIT_ASSERT( size == 4 );
    CPPUNIT_ASSERT( 0 == memcmp(buf,"end\n",size) );

    file2.reset();

    CPPUNIT_ASSERT( fs::exists(fileTest) );
    CPPUNIT_ASSERT( 4 * bufsize + 4 == fs::file_size(fileTest) );

    file1.reset();

    CPPUNIT_ASSERT( fs::exists(fileTest) );
    CPPUNIT_ASSERT( 4 * bufsize + 4 == fs::file_size(fileTest) );

    file.reset();

    CPPUNIT_ASSERT( fs::exists(fileTest) );
    CPPUNIT_ASSERT( 4 * bufsize + 4 == fs::file_size(fileTest) );

    file.reset( manager->GetFileOperationRead(fileTest) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == file->OpenFile(O_RDONLY) );

    offset = 0;
    for ( int i = 1; i <= 4; ++i ) {
        CPPUNIT_ASSERT( true == file->Read(offset,buf,bufsize,size) );
        CPPUNIT_ASSERT( size == bufsize );
        offset += size;
        memset(bufCmp,'\n',sizeof(buf));
        memset(bufCmp,'a'+i,sizeof(buf)-1);
        CPPUNIT_ASSERT( 0 == memcmp(buf,bufCmp,sizeof(buf)) );
    }
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(offset),
            true == file->Read(offset,buf,bufsize,size) );
    CPPUNIT_ASSERT( size == 4 );
    CPPUNIT_ASSERT( 0 == memcmp(buf,"end\n",size) );
}


void
ReadCacheManagerTest::testFileOperationRead()
{
    auto_ptr<ReadCacheManager> manager(new ReadCacheManager());

    const int bufsize = 512 * 1024;
    char buf[bufsize];
    {
        ofstream file(fileTestOrig.file_string().c_str());
        for ( int i = 1; i <= 10; ++i ) {
            memset(buf,0,sizeof(buf));
            memset(buf,'a'+i,sizeof(buf)-1);
            file << buf << endl;
        }
        file << "end" << endl;
    }

    CPPUNIT_ASSERT( ! fs::exists(fileTest) );

    CPPUNIT_ASSERT( NULL == manager->GetFileOperationRead(fileTestOrig) );
    CPPUNIT_ASSERT( NULL == manager->GetFileOperationRead(fileTest) );

    CPPUNIT_ASSERT( ! fs::exists(fileTest) );

    auto_ptr<FileOperationInterface> fileOrig(new FileOperation(fileTestOrig));
    auto_ptr<FileOperationInterface> file(
            manager->GetFileOperationRead(fileOrig.release(),fileTest) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == file->OpenFile(O_RDONLY) );
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(2) );
    file.reset();
    off_t offset = fs::file_size(fileTest);
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(offset),
            offset == 0 );

    fileOrig.reset(new FileOperation(fileTestOrig));
    file.reset(manager->GetFileOperationRead(fileOrig.release(),fileTest));
    CPPUNIT_ASSERT( true == file->OpenFile(O_RDONLY) );
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(2) );
    file.reset();
    off_t offsetOld = offset;
    offset = fs::file_size(fileTest);
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(offset),
            offset == offsetOld );
    offsetOld = offset;

    fileOrig.reset(new FileOperation(fileTestOrig));
    file.reset(manager->GetFileOperationRead(fileOrig.release(),fileTest));
    CPPUNIT_ASSERT( true == file->OpenFile(O_RDONLY) );
    size_t size;
    CPPUNIT_ASSERT( true == file->Read(0,buf,bufsize,size) );
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(2) );
    file.reset();
    offsetOld = offset;
    offset = fs::file_size(fileTest);
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(offset),
            offset >= offsetOld + bufsize );
}


void
ReadCacheManagerTest::testFileOperationReadPartial()
{
    auto_ptr<ReadCacheManager> manager(new ReadCacheManager());

    const int bufsize = 512 * 1024;
    char bufCmp[bufsize];
    char buf[bufsize];
    size_t size;
    {
        ofstream file(fileTestOrig.file_string().c_str());
        for ( int i = 1; i <= 10; ++i ) {
            memset(buf,0,sizeof(buf));
            memset(buf,'a'+i,sizeof(buf)-1);
            file << buf << endl;
        }
        file << "end" << endl;
    }

    CPPUNIT_ASSERT( ! fs::exists(fileTest) );

    CPPUNIT_ASSERT( NULL == manager->GetFileOperationRead(fileTestOrig) );
    CPPUNIT_ASSERT( NULL == manager->GetFileOperationRead(fileTest) );

    CPPUNIT_ASSERT( ! fs::exists(fileTest) );

    auto_ptr<FileOperationInterface> fileOrig(new FileOperation(fileTestOrig));
    auto_ptr<FileOperationInterface> file(
            manager->GetFileOperationRead(fileOrig.release(),fileTest) );
    auto_ptr<FileOperationBitmap> fileBitmap;
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == file->OpenFile(O_RDONLY) );
    CPPUNIT_ASSERT( true == file->Read(4*bufsize,buf,bufsize,size) );
    file.reset();
    memset(bufCmp,'\n',sizeof(buf));
    memset(bufCmp,'a'+5,sizeof(buf)-1);
    CPPUNIT_ASSERT( 0 == memcmp(buf,bufCmp,sizeof(buf)) );
    fileBitmap.reset(new FileOperationBitmap(fileTest,bufsize));
    CPPUNIT_ASSERT( true == fileBitmap->OpenFile(O_RDONLY) );
    CPPUNIT_ASSERT( false == fileBitmap->CheckBitmap(3*bufsize,bufsize) );
    CPPUNIT_ASSERT( true == fileBitmap->CheckBitmap(4*bufsize,bufsize) );
    fileBitmap.reset();

    fileOrig.reset(new FileOperation(fileTestOrig));
    file.reset(manager->GetFileOperationRead(fileOrig.release(),fileTest));
    CPPUNIT_ASSERT( true == file->OpenFile(O_RDONLY) );
    CPPUNIT_ASSERT( true == file->Read(8*bufsize,buf,bufsize,size) );
    memset(bufCmp,'\n',sizeof(buf));
    memset(bufCmp,'a'+9,sizeof(buf)-1);
    CPPUNIT_ASSERT( 0 == memcmp(buf,bufCmp,sizeof(buf)) );
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(50) );
    file.reset();
    fileBitmap.reset(new FileOperationBitmap(fileTest,bufsize));
    CPPUNIT_ASSERT( true == fileBitmap->OpenFile(O_RDONLY) );
    CPPUNIT_ASSERT( true == fileBitmap->IsFull() );
    fileBitmap.reset();
}


void
ReadCacheManagerTest::testFileOperationReadWrite()
{
    auto_ptr<ReadCacheManager> manager(new ReadCacheManager());

    const int bufsize = 512 * 1024;
    char buf[bufsize];
    {
        ofstream file(fileTestOrig.file_string().c_str());
        for ( int i = 1; i <= 10; ++i ) {
            memset(buf,0,sizeof(buf));
            memset(buf,'a'+i,sizeof(buf)-1);
            file << buf << endl;
        }
        file << "end" << endl;
    }
    size_t size;

    CPPUNIT_ASSERT( ! fs::exists(fileTest) );

    auto_ptr<FileOperationInterface> fileOrig(new FileOperation(fileTestOrig));
    auto_ptr<FileOperationInterface> file(
            manager->GetFileOperationReadWrite(fileOrig.release(),fileTest) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == file->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT( true == file->Write(bufsize,buf,bufsize,size) );
    CPPUNIT_FAIL("TODO");
}


void
ReadCacheManagerTest::testOldName()
{
    auto_ptr<ofstream> file(new ofstream(fileTest.file_string().c_str()));
    file.reset();
    CPPUNIT_ASSERT( fs::exists(fileTest) );

    fs::path name;
    const string oldName = "oldName";

    CPPUNIT_ASSERT( ! ReadCacheManager::GetFileOldName(fileTest,name) );
    CPPUNIT_ASSERT( ReadCacheManager::SetFileOldName(fileTest,oldName) );
    CPPUNIT_ASSERT( ReadCacheManager::GetFileOldName(fileTest,name) );
    CPPUNIT_ASSERT( name == oldName );
    CPPUNIT_ASSERT( ! ReadCacheManager::SetFileOldName(fileTest,oldName) );
    CPPUNIT_ASSERT( ! ReadCacheManager::SetFileOldName(fileTest,oldName+"a") );
    name.clear();
    CPPUNIT_ASSERT( ReadCacheManager::GetFileOldName(fileTest,name) );
    CPPUNIT_ASSERT( name == oldName );
}

