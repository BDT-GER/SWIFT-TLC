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
 * MetaManagerTapeStatusTest.cpp
 *
 *  Created on: Mar 30, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../TapeManagerLE.h"
#include "../MetaManagerTapeStatus.h"
#include "MetaManagerTapeStatusTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( MetaManagerTapeStatusTest );


static const string metaFolder("MetaManagerTapeStatusTest");
static const string tapeFolder("tape-folder");
static const string cacheFolder("cache-folder");
static const string testFile("/test-file");
static const string testFileSystem
        = MetaManagerTapeStatus::SYSTEM_FOLDER + testFile;
static const string testFolder("/test-folder");
static const string testSystemFolder
        = MetaManagerTapeStatus::SYSTEM_FOLDER + testFolder;
static const string testFolderSystem
        = testFolder + "/" + MetaManagerTapeStatus::SYSTEM_FOLDER;
static const string testFolderFile = testFolder + testFile;
static const string testFolderFileSystem
        = testFolder
            + "/" + MetaManagerTapeStatus::SYSTEM_FOLDER
            + "/" + testFile;

static const fs::path folderMeta(metaFolder);
static const fs::path folderTape(tapeFolder);
static const fs::path folderCache(cacheFolder);
static const fs::path fileTest(metaFolder + testFile);
static const fs::path folderTest(metaFolder + testFolder);
static const fs::path fileFolderTest(metaFolder + testFolderFile);


void
MetaManagerTapeStatusTest::setUp()
{
    fs::create_directory(folderMeta);
    fs::create_directory(folderTape);
    fs::create_directory(folderCache);

    Factory::SetCacheFolder(folderCache);
    Factory::SetTapeFolder(folderTape);
    Factory::SetMetaFolder(folderMeta);
}


void
MetaManagerTapeStatusTest::tearDown()
{
    fs::remove_all(folderMeta);
    fs::remove_all(folderTape);
    fs::remove_all(folderCache);
}


void
MetaManagerTapeStatusTest::testInodeEqual(
        int line, Inode * inode1, Inode * inode2 )
{
    struct stat stat1;
    struct stat stat2;
    memset(&stat1,0,sizeof(stat1));
    memset(&stat2,0,sizeof(stat2));
    CPPUNIT_ASSERT( true == inode1->GetStat(stat1) );
    CPPUNIT_ASSERT( true == inode2->GetStat(stat2) );
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(line),
            0 == memcmp(&stat1,&stat2,sizeof(stat1)) );
}


void
MetaManagerTapeStatusTest::testInode()
{
    Factory::ReleaseTapeManager();
    Factory::CreateTapeManager();
    Factory::ReleaseCacheManager();
    Factory::CreateCacheManager();
    auto_ptr<MetaManager> meta(
            new MetaManagerTapeStatus() );
    auto_ptr<MetaManager> metaRaw(
            new MetaManager() );

    auto_ptr<Inode> inode;
    auto_ptr<Inode> inodeRaw;

    inode.reset(meta->CreateFolder(MetaManagerTapeStatus::SYSTEM_FOLDER,0755));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFile(MetaManagerTapeStatus::SYSTEM_FOLDER,0644));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(MetaManagerTapeStatus::SYSTEM_FOLDER));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( false == inode->IsFile() );
    errno = 0;
    CPPUNIT_ASSERT(
            false == meta->DeleteInode(MetaManagerTapeStatus::SYSTEM_FOLDER) );
    CPPUNIT_ASSERT_ERRNO_( EPERM == errno);


    inode.reset(meta->GetInode("/"));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( false == inode->IsFile() );


    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(testFileSystem));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFolder(testFileSystem,0755));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFile(testFileSystem,0644));
    CPPUNIT_ASSERT( NULL == inode.get() );

    inode.reset( meta->CreateFile(testFile,0644) );
    CPPUNIT_ASSERT( NULL != inode.get() );
    inode.reset( meta->GetInode(testFile) );
    CPPUNIT_ASSERT( NULL != inode.get() );
    inodeRaw.reset( metaRaw->GetInode(testFile) );
    CPPUNIT_ASSERT( NULL != inodeRaw.get() );
    testInodeEqual( __LINE__, inode.get(), inodeRaw.get() );
    inode.reset( meta->GetInode(testFile) );
    testInodeEqual( __LINE__, inode.get(), inodeRaw.get() );

    inode.reset(meta->GetInode(testFileSystem));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == inode->IsFile() );
    inode.reset(meta->CreateFolder(testFileSystem,0755));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFile(testFileSystem,0644));
    CPPUNIT_ASSERT( NULL == inode.get() );

    errno = 0;
    CPPUNIT_ASSERT( false == meta->DeleteInode(testFileSystem) );
    CPPUNIT_ASSERT_ERRNO_( EPERM == errno);
    inode.reset(meta->GetInode(testFileSystem));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFile) );
    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(testFileSystem));
    CPPUNIT_ASSERT( NULL == inode.get() );


    inode.reset(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(testSystemFolder));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFolder(testSystemFolder,0755));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFile(testSystemFolder,0644));
    CPPUNIT_ASSERT( NULL == inode.get() );

    inode.reset( meta->CreateFolder(testFolder,0755) );
    CPPUNIT_ASSERT( NULL != inode.get() );
    inode.reset( meta->GetInode(testFolder) );
    CPPUNIT_ASSERT( NULL != inode.get() );
    inodeRaw.reset( metaRaw->GetInode(testFolder) );
    CPPUNIT_ASSERT( NULL != inodeRaw.get() );
    testInodeEqual( __LINE__, inode.get(), inodeRaw.get() );
    inode.reset( meta->GetInode(testFolder) );
    testInodeEqual( __LINE__, inode.get(), inodeRaw.get() );

    inode.reset(meta->GetInode(testSystemFolder));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFolder(testSystemFolder,0755));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFile(testSystemFolder,0644));
    CPPUNIT_ASSERT( NULL == inode.get() );

    inode.reset(meta->CreateFolder(testFolderSystem,0755));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFile(testFolderSystem,0644));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(testFolderSystem));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( false == inode->IsFile() );

    errno = 0;
    CPPUNIT_ASSERT( false == meta->DeleteInode(testFolderSystem) );
    CPPUNIT_ASSERT_ERRNO_( EPERM == errno);
    inode.reset(meta->GetInode(testFolderSystem));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFolder) );
    inode.reset(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(testFolderSystem));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset( meta->CreateFolder(testFolder,0755) );
    CPPUNIT_ASSERT( NULL != inode.get() );


    inode.reset(meta->GetInode(testFolderFile));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(testFolderFileSystem));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFolder(testFolderFileSystem,0755));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFile(testFolderFileSystem,0644));
    CPPUNIT_ASSERT( NULL == inode.get() );

    inode.reset( meta->CreateFile(testFolderFile,0644) );
    CPPUNIT_ASSERT( NULL != inode.get() );
    inode.reset( meta->GetInode(testFolderFile) );
    CPPUNIT_ASSERT( NULL != inode.get() );
    inodeRaw.reset( metaRaw->GetInode(testFolderFile) );
    CPPUNIT_ASSERT( NULL != inodeRaw.get() );
    testInodeEqual( __LINE__, inode.get(), inodeRaw.get() );
    inode.reset( meta->GetInode(testFolderFile) );
    testInodeEqual( __LINE__, inode.get(), inodeRaw.get() );

    inode.reset(meta->GetInode(testFolderFileSystem));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == inode->IsFile() );
    inode.reset(meta->CreateFolder(testFolderFileSystem,0755));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->CreateFile(testFolderFileSystem,0644));
    CPPUNIT_ASSERT( NULL == inode.get() );

    errno = 0;
    CPPUNIT_ASSERT( false == meta->DeleteInode(testFolderFileSystem) );
    CPPUNIT_ASSERT_ERRNO_( EPERM == errno);
    inode.reset(meta->GetInode(testFolderFileSystem));
    CPPUNIT_ASSERT( NULL != inode.get() );
    errno = 0;
    CPPUNIT_ASSERT( false == meta->DeleteInode(testFolder) );
    CPPUNIT_ASSERT_ERRNO_( ENOTEMPTY == errno);
    inode.reset(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFolderFile) );
    inode.reset(meta->GetInode(testFolderFile));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(testFolderFileSystem));
    CPPUNIT_ASSERT( NULL == inode.get() );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFolder) );
    inode.reset(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(testFolderSystem));
    CPPUNIT_ASSERT( NULL == inode.get() );
}

