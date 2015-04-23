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
 * ServiceDiagnoseTest.cpp
 *
 *  Created on: Apr 17, 2013
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ServiceDiagnoseTest.h"
#include "../MetaManager.h"
#include "../CacheManager.h"
#include "../ReadCacheManager.h"
#include "../ServiceDiagnose.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ServiceDiagnoseTest );


static const string metaFolder = "meta-folder";
static const string cacheFolder = "cache-folder";
static const string tapeFolder = "tape-folder";
static const string service = "service";
static const string barcode = "barcode";
static const string testFile = "/test-file";

static const fs::path folderMeta(metaFolder);
static const fs::path folderCache(cacheFolder);
static const fs::path folderTape(tapeFolder);
static const fs::path folderBarcode(folderTape/barcode);


void
ServiceDiagnoseTest::setUp()
{
    fs::remove_all(folderMeta);
    fs::remove_all(folderCache);
    fs::remove_all(folderTape);

    fs::create_directory(folderCache);
    Factory::SetCacheFolder(folderCache);

    fs::create_directory(folderMeta);
    Factory::SetMetaFolder(folderMeta);

    Factory::SetService(service);
    fs::create_directory(folderCache / service);
    fs::create_directory(folderMeta / service);

    fs::create_directory(folderTape);
    fs::create_directory(folderBarcode);
}


void
ServiceDiagnoseTest::tearDown()
{
    fs::remove_all(folderMeta);
    fs::remove_all(folderCache);
    fs::remove_all(folderTape);
}


//  Case: the file in meta and cache as backup, while not in tape
//  Action: mark as not backup in meta and cache
void
ServiceDiagnoseTest::testDiagnoseNotInTape()
{
    auto_ptr<CacheManager> cache(new CacheManager());
    auto_ptr<ReadCacheManager> cacheRead(new ReadCacheManager());
    auto_ptr<MetaManager> meta(new MetaManager());
    auto_ptr<ServiceDiagnose> diagnose(
            new ServiceDiagnose(service,barcode,folderBarcode) );
    auto_ptr<Inode> inode;
    string tape;
    auto_ptr<FileOperationInterface> file;
    int status;
    vector<fs::path> files;


    inode.reset(meta->CreateFile(testFile,0777));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( false == inode->GetTape(tape) );
    CPPUNIT_ASSERT( true == inode->SetTape(barcode) );
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );
    CPPUNIT_ASSERT( barcode == tape );

    file.reset( cache->GetFileOperation(
            testFile, FileOperationInterface::ActionCreate ) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == file->CreateFile(O_RDWR,0777,false) );
    CPPUNIT_ASSERT( true == cacheRead->GetFileStatus(
            folderCache/service/testFile, status ) );
    CPPUNIT_ASSERT( status == ReadCacheManager::StatusWrite );
    file.reset();
    status = ReadCacheManager::StatusClose;
    CPPUNIT_ASSERT( true == cacheRead->GetFileStatus(
            folderCache/service/testFile, status ) );
    CPPUNIT_ASSERT( status == ReadCacheManager::StatusWrite );
    CPPUNIT_ASSERT( true == cacheRead->SetFileStatus(
            folderCache/service/testFile, ReadCacheManager::StatusClose) );
    CPPUNIT_ASSERT( true == cacheRead->GetFileStatus(
            folderCache/service/testFile, status ) );
    CPPUNIT_ASSERT( status == ReadCacheManager::StatusClose );


    CPPUNIT_ASSERT( true == diagnose->GetCorruptFileList(files) );
    CPPUNIT_ASSERT( files.empty() );

    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL != inode.get() );
    tape.clear();
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );
    CPPUNIT_ASSERT( barcode == tape );

    status = ReadCacheManager::StatusWrite;
    CPPUNIT_ASSERT( true == cacheRead->GetFileStatus(
            folderCache/service/testFile, status ) );
    CPPUNIT_ASSERT( status == ReadCacheManager::StatusClose );


    CPPUNIT_ASSERT( true == diagnose->Diagnose() );
    CPPUNIT_ASSERT( true == diagnose->GetCorruptFileList(files) );
    CPPUNIT_ASSERT( files.empty() );

    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == inode->GetTape(tape) && tape.empty() );

    status = ReadCacheManager::StatusClose;
    CPPUNIT_ASSERT( true == cacheRead->GetFileStatus(
            folderCache/service/testFile, status ) );
    CPPUNIT_ASSERT( status == ReadCacheManager::StatusWrite );


    CPPUNIT_ASSERT( true == diagnose->Diagnose() );
    CPPUNIT_ASSERT( true == diagnose->GetCorruptFileList(files) );
    CPPUNIT_ASSERT( files.empty() );

    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == inode->GetTape(tape) && tape.empty() );

    status = ReadCacheManager::StatusClose;
    CPPUNIT_ASSERT( true == cacheRead->GetFileStatus(
            folderCache/service/testFile, status ) );
    CPPUNIT_ASSERT( status == ReadCacheManager::StatusWrite );
}


//  Case: the file in meta, while not in cache or tape
//  Action: add the file to corrupt file list
void
ServiceDiagnoseTest::testDiagnoseNotInCache()
{
    auto_ptr<CacheManager> cache(new CacheManager());
    auto_ptr<MetaManager> meta(new MetaManager());
    auto_ptr<ServiceDiagnose> diagnose(
            new ServiceDiagnose(service,barcode,folderBarcode) );
    auto_ptr<Inode> inode;
    vector<fs::path> files;


    inode.reset(meta->CreateFile(testFile,0777));
    CPPUNIT_ASSERT( NULL != inode.get() );
    inode->SetTape(barcode);


    CPPUNIT_ASSERT( true == diagnose->Diagnose() );
    CPPUNIT_ASSERT( true == diagnose->Diagnose() );
    files.clear();
    CPPUNIT_ASSERT( true == diagnose->GetCorruptFileList(files) );
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(files.size()),
            1 == files.size() );
    CPPUNIT_ASSERT( testFile == files[0] );

    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL != inode.get() );

    CPPUNIT_ASSERT( ! fs::exists(folderCache/service/testFile) );


    CPPUNIT_ASSERT( true == diagnose->Diagnose() );
    files.clear();
    CPPUNIT_ASSERT( true == diagnose->GetCorruptFileList(files) );
    CPPUNIT_ASSERT( 1 == files.size() );
    CPPUNIT_ASSERT( testFile == files[0] );

    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL != inode.get() );

    CPPUNIT_ASSERT( ! fs::exists(folderCache/service/testFile) );
}


//  Case: the file in cache, while not in meta
//  Action: delete the file in cache
void
ServiceDiagnoseTest::testDiagnoseNotInMeta()
{
    auto_ptr<CacheManager> cache(new CacheManager());
    auto_ptr<MetaManager> meta(new MetaManager());
    auto_ptr<ServiceDiagnose> diagnose(
            new ServiceDiagnose(service,barcode,folderBarcode) );
    auto_ptr<Inode> inode;
    auto_ptr<FileOperationInterface> file;
    vector<fs::path> files;


    file.reset( cache->GetFileOperation(
            testFile, FileOperationInterface::ActionCreate ) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == file->CreateFile(O_RDWR,0777,false) );
    file.reset();
    CPPUNIT_ASSERT( fs::exists(folderCache/service/testFile) );


    CPPUNIT_ASSERT( true == diagnose->Diagnose() );
    CPPUNIT_ASSERT( true == diagnose->GetCorruptFileList(files) );
    CPPUNIT_ASSERT( files.empty() );

    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL == inode.get() );

    CPPUNIT_ASSERT( ! fs::exists(folderCache/service/testFile) );


    CPPUNIT_ASSERT( true == diagnose->Diagnose() );
    CPPUNIT_ASSERT( true == diagnose->GetCorruptFileList(files) );
    CPPUNIT_ASSERT( files.empty() );

    CPPUNIT_ASSERT( NULL == inode.get() );

    CPPUNIT_ASSERT( ! fs::exists(folderCache/service/testFile) );
}

