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
 * MetaManagerTest.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../MetaManager.h"
#include "../CacheManager.h"
#include "../FileOperationDelay.h"
#include "MetaManagerTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( MetaManagerTest );


static const string metaFolder = "meta.folder";
static const string metaFolderNone = "meta.folder-none";
static const string metaFile = "meta.file";
static const string cacheFolder = "cache.folder";
static const string backupFile = "backup.file";


void
MetaManagerTest::setUp()
{
    fs::create_directory(metaFolder);
    fs::create_directory(cacheFolder);
    ofstream f(metaFile.c_str());

    Factory::SetCacheFolder(cacheFolder);
    Factory::CreateCacheManager();
    Factory::CreateReadManager();
    Factory::SetMetaFolder(metaFolder);
    Factory::CreateMetaManager();
}


void
MetaManagerTest::tearDown()
{
    Factory::ReleaseMetaManager();
    Factory::ReleaseReadManager();
    Factory::ReleaseCacheManager();

    fs::remove_all(metaFolder);
    fs::remove_all(metaFolderNone);
    fs::remove(metaFile);
    fs::remove_all(cacheFolder);
    fs::remove(backupFile);
}


void
MetaManagerTest::testConstructor()
{
    try {
        Factory::SetMetaFolder(metaFolder);
        MetaManager meta;
    } catch (const std::exception & e) {
        CPPUNIT_FAIL(e.what());
    }
    CPPUNIT_ASSERT( fs::is_directory(metaFolder) );

    CPPUNIT_ASSERT( ! fs::exists(metaFolderNone) );
    try {
        Factory::SetMetaFolder(metaFolderNone);
        MetaManager meta;
    } catch (const std::exception & e) {
        CPPUNIT_FAIL(e.what());
    }
    CPPUNIT_ASSERT( fs::is_directory(metaFolderNone) );

    CPPUNIT_ASSERT( fs::is_regular_file(metaFile) );
    try {
        Factory::SetMetaFolder(metaFile);
        MetaManager meta;
    } catch (const PathNotExistException & e) {
        CPPUNIT_ASSERT( fs::is_regular_file(metaFile) );
        CPPUNIT_ASSERT( e.what() == metaFile );
        return;
    } catch (const std::exception & e) {
        CPPUNIT_FAIL(e.what());
    }
    CPPUNIT_FAIL("Exception should be raised for file as meta folder");
}


void
MetaManagerTest::testDeleteFile()
{
    MetaManager * meta = Factory::GetMetaManager();
    CacheManager * cache = Factory::GetCacheManager();
    auto_ptr<Inode> inode;
    auto_ptr<FileOperationInterface> file;

    string testFile = "MetaManagerTest_testDeleteFile_0";
    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL == inode.get() );
    file.reset(meta->GetFileOperation(testFile,O_RDWR));
    CPPUNIT_ASSERT( NULL == file.get() );
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0600) );
    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( false == cache->ExistFile(1) );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFile) );
    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL == inode.get() );

    testFile = "MetaManagerTest_testDeleteFile_1";
    CPPUNIT_ASSERT( NULL == meta->GetFileOperation(testFile,O_RDWR) );
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0600) );
    CPPUNIT_ASSERT( false == cache->ExistFile(1) );
    file.reset(meta->GetFileOperation(testFile,O_RDWR));
    CPPUNIT_ASSERT( NULL != file.get() );
    file.reset();
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFile) );
    CPPUNIT_ASSERT( false == cache->ExistFile(1) );

    testFile = "MetaManagerTest_testDeleteFile_2";
    CPPUNIT_ASSERT( NULL == meta->GetFileOperation(testFile,O_RDWR) );
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0600) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
    file.reset(meta->GetFileOperation(testFile,O_RDWR));
    CPPUNIT_ASSERT( true == cache->ExistFile(2) );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFile) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
}


void
MetaManagerTest::testRenameFile()
{
    MetaManager * meta = Factory::GetMetaManager();
    CacheManager * cache = Factory::GetCacheManager();

    string testFile = "MetaManagerTest_testRenameFile_0";
    string testFileNew = "MetaManagerTest_testRenameFile_0_new";
    auto_ptr<Inode> inode(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL == inode.get() );
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0600) );
    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == meta->RenameInode(testFile,testFileNew) );
    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(testFileNew));
    CPPUNIT_ASSERT( NULL != inode.get() );

    testFile = "MetaManagerTest_testRenameFile_1";
    testFileNew = "MetaManagerTest_testRenameFile_1_new";
    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL == inode.get() );
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0600) );
    CPPUNIT_ASSERT( false == cache->ExistFile(1) );
    auto_ptr<FileOperationInterface> file;
    file.reset( meta->GetFileOperation(testFile,O_RDWR) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( NULL != file.get() );
    file.reset();
    CPPUNIT_ASSERT( true == meta->RenameInode(testFile,testFileNew) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
    file.reset( meta->GetFileOperation(testFile,O_RDWR) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );

    testFile = "MetaManagerTest_testRenameFile_2";
    testFileNew = "MetaManagerTest_testRenameFile_2_new";
    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL == inode.get() );
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0600) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
    file.reset( meta->GetFileOperation(testFile,O_RDWR) );
    CPPUNIT_ASSERT( true == cache->ExistFile(2) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == meta->RenameInode(testFile,testFileNew) );
    CPPUNIT_ASSERT( true == cache->ExistFile(2) );
    CPPUNIT_ASSERT( false == cache->ExistFile(3) );
    file.reset( meta->GetFileOperation(testFile,O_RDWR) );
    CPPUNIT_ASSERT( true == cache->ExistFile(2) );
    CPPUNIT_ASSERT( false == cache->ExistFile(3) );
}


void
MetaManagerTest::testRenameFileOverride()
{
    CPPUNIT_FAIL("TODO");
}


void
MetaManagerTest::testDeleteFolder()
{
    MetaManager * meta = Factory::GetMetaManager();

    string testFolder = "MetaManagerTest_testDeleteFolder_0";
    CPPUNIT_ASSERT( true == meta->CreateFolder(testFolder,0755) );
    auto_ptr<Inode> inode(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFolder) );
    inode.reset(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL == inode.get() );

    testFolder = "MetaManagerTest_testDeleteFolder_1";
    string testFile = "MetaManagerTest_testDeleteFolder_file";
    CPPUNIT_ASSERT( true == meta->CreateFolder(testFolder,0755) );
    CPPUNIT_ASSERT( true == meta->CreateFile(
            fs::path(testFolder) / testFile, 0755 ) );
    inode.reset(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL != inode.get() );
    inode.reset( meta->GetInode( fs::path(testFolder) / testFile ) );
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( false == meta->DeleteInode(testFolder) );
    CPPUNIT_ASSERT( true == meta->DeleteInode(
            fs::path(testFolder) / testFile ) );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFolder) );
}


void
MetaManagerTest::testRenameFolder()
{
    MetaManager * meta = Factory::GetMetaManager();
    CacheManager * cache = Factory::GetCacheManager();

    string testFolder = "MetaManagerTest_testRenameFolder_0";
    string testFolderNew = "MetaManagerTest_testRenameFolder_0_new";
    auto_ptr<Inode> inode(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL == inode.get() );
    CPPUNIT_ASSERT( true == meta->CreateFolder(testFolder,0755) );
    inode.reset(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == meta->RenameInode(testFolder,testFolderNew) );
    inode.reset(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL == inode.get() );
    inode.reset(meta->GetInode(testFolderNew));
    CPPUNIT_ASSERT( NULL != inode.get() );

    testFolder = "MetaManagerTest_testRenameFolder_1";
    testFolderNew = "MetaManagerTest_testRenameFolder_1_new";
    string testFile1 = "MetaManagerTest_testRenameFolder_file1";
    string testFile2 = "MetaManagerTest_testRenameFolder_file2";
    string testFile3 = "MetaManagerTest_testRenameFolder_file3";
    auto_ptr<FileOperationInterface> file;
    CPPUNIT_ASSERT( true == meta->CreateFolder(testFolder,0755) );
    CPPUNIT_ASSERT( true == meta->CreateFile(
            fs::path(testFolder) / testFile1, 0600 ) );
    CPPUNIT_ASSERT( true == meta->CreateFile(
            fs::path(testFolder) / testFile2, 0600 ) );
    CPPUNIT_ASSERT( false == cache->ExistFile(1) );
    file.reset( meta->GetFileOperation(
            fs::path(testFolder) / testFile2, O_RDWR ) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == meta->CreateFile(
            fs::path(testFolder) / testFile3, 0600 ) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
    file.reset( meta->GetFileOperation(
            fs::path(testFolder) / testFile3, O_RDWR ) );
    CPPUNIT_ASSERT( true == cache->ExistFile(2) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( true == meta->RenameInode(testFolder,testFolderNew) );
    CPPUNIT_ASSERT( false == cache->ExistFile(3) );
    file.reset( meta->GetFileOperation(
            fs::path(testFolderNew) / testFile1, O_RDWR ) );
    CPPUNIT_ASSERT( true == cache->ExistFile(3) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( false == cache->ExistFile(4) );
    file.reset( meta->GetFileOperation(
            fs::path(testFolderNew) / testFile2, O_RDWR ) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( false == cache->ExistFile(4) );
    file.reset( meta->GetFileOperation(
            fs::path(testFolderNew) / testFile3, O_RDWR ) );
    CPPUNIT_ASSERT( NULL != file.get() );
    CPPUNIT_ASSERT( false == cache->ExistFile(4) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( true == cache->ExistFile(2) );
    CPPUNIT_ASSERT( true == cache->ExistFile(3) );
}


void
MetaManagerTest::testRenameFolderOverride()
{
    CPPUNIT_FAIL("TODO");
}


void
MetaManagerTest::testGetFileOperation()
{
    MetaManager * meta = Factory::GetMetaManager();
    CacheManager * cache = Factory::GetCacheManager();
    char buffer[1024] = "Hello,World!\n";
    char bufferRead[1024];
    size_t size;

    string testFile = "MetaManagerTest_testGetFileOperation";
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0644) );
    auto_ptr<FileOperationInterface> file0, file1, file2;
    file0.reset( meta->GetFileOperation(testFile,O_RDWR) );
    file1.reset( meta->GetFileOperation(testFile,O_RDWR) );
    CPPUNIT_ASSERT( true == file0->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT( sizeof(buffer) == size );
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(true == file0->Read(0,bufferRead,sizeof(buffer),size));
    CPPUNIT_ASSERT( 0 == memcmp(buffer,bufferRead,sizeof(buffer)) );
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(true == file1->Read(0,bufferRead,sizeof(buffer),size));
    CPPUNIT_ASSERT( 0 == memcmp(buffer,bufferRead,sizeof(buffer)) );
    file2.reset( meta->GetFileOperation(testFile,O_RDWR) );
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(true == file2->Read(0,bufferRead,sizeof(buffer),size));
    CPPUNIT_ASSERT( 0 == memcmp(buffer,bufferRead,sizeof(buffer)) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
}


void
MetaManagerTest::testBackup()
{
    MetaManager * meta = Factory::GetMetaManager();
    string tape = "barcode0";

    string testFile0 = "MetaManagerTest_testBackup0";
    string testFile1 = "MetaManagerTest_testBackup1";
    string testFile2 = "MetaManagerTest_testBackup2";
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile0,0644) );
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile1,0644) );
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile2,0644) );
    auto_ptr<FileOperationInterface> file0, file1, file2;
    auto_ptr<FileOperationInterface> backup;
    fs::path path;

    vector<BackupItem> list;
    meta->GetBackupList(list);
    CPPUNIT_ASSERT_EQUAL((size_t)0, list.size());

    file0.reset( meta->GetFileOperation(testFile0,O_RDWR) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT_EQUAL((size_t)1, list.size());
    CPPUNIT_ASSERT_EQUAL(testFile0, list[0].path.string());

    file0.reset();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT_EQUAL((size_t)1, list.size());
    CPPUNIT_ASSERT_EQUAL(testFile0, list[0].path.string());

    file1.reset( meta->GetFileOperation(testFile1,O_RDWR) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT_EQUAL((size_t)2, list.size());
    CPPUNIT_ASSERT_EQUAL(testFile0, list[0].path.string());
    CPPUNIT_ASSERT_EQUAL(testFile1, list[1].path.string());

    file2.reset( meta->GetFileOperation(testFile2,O_RDWR) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT_EQUAL((size_t)3, list.size());
    CPPUNIT_ASSERT_EQUAL(testFile0, list[0].path.string());
    CPPUNIT_ASSERT_EQUAL(testFile1, list[1].path.string());
    CPPUNIT_ASSERT_EQUAL(testFile2, list[2].path.string());

    backup.reset(new FileOperation(backupFile,0644,O_RDWR));
    bool write = false;
    CPPUNIT_ASSERT( meta->Backup(testFile1,backup.get(),tape,path,write) );
    CPPUNIT_ASSERT_EQUAL(testFile1, path.string());
    CPPUNIT_ASSERT( write );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT_EQUAL((size_t)2, list.size());
    CPPUNIT_ASSERT_EQUAL(testFile0, list[0].path.string());
    CPPUNIT_ASSERT_EQUAL(testFile2, list[1].path.string());
    write = false;
    CPPUNIT_ASSERT( meta->Backup(testFile0,backup.get(),tape,path,write) );
    CPPUNIT_ASSERT_EQUAL(testFile0, path.string());
    CPPUNIT_ASSERT( write );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT_EQUAL((size_t)1, list.size());
    CPPUNIT_ASSERT_EQUAL(testFile2, list[0].path.string());
    write = false;
    CPPUNIT_ASSERT( meta->Backup(testFile2,backup.get(),tape,path,write) );
    CPPUNIT_ASSERT_EQUAL(testFile2, path.string());
    CPPUNIT_ASSERT( write );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT_EQUAL((size_t)0, list.size());
    CPPUNIT_ASSERT( ! meta->Backup(testFile0,backup.get(),tape,path,write) );
    CPPUNIT_ASSERT( ! meta->Backup(testFile1,backup.get(),tape,path,write) );
    CPPUNIT_ASSERT( ! meta->Backup(testFile2,backup.get(),tape,path,write) );
}


void
MetaManagerTest::testBackupOnWrite()
{
    MetaManager * meta = Factory::GetMetaManager();
    const string tape = "barcode0";
    string testFile = "MetaManagerTest_testBackupOnWrite";
    string testBackup = "MetaManagerTest_testBackupOnWrite_backup";
    char buffer[1024] = "Hello,World!\n";
    const size_t bufsize = sizeof(buffer);
    const off_t filesize = 2 * 1024 * 1024;
    auto_ptr<FileOperationInterface> file;
    auto_ptr<FileOperationInterface> backup;
    fs::path path;
    size_t size;

    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0644) );
    file.reset(meta->GetFileOperation(testFile,O_RDWR));
    CPPUNIT_ASSERT( NULL != file.get() );
    for (off_t offset=0; offset<filesize; offset += bufsize) {
        size = 0;
        CPPUNIT_ASSERT( true == file->Write(offset,buffer,bufsize,size) );
        CPPUNIT_ASSERT( size == bufsize );
    }
    vector<BackupItem> list;
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    file.reset();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );

    backup.reset(new FileOperation(backupFile,0644,O_RDWR));
    bool write;
    CPPUNIT_ASSERT( true == meta->Backup(testFile,backup.get(),tape,path,write) );
    CPPUNIT_ASSERT( testFile == path );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    struct stat stat;
    CPPUNIT_ASSERT( true == backup->GetStat(stat) );
    CPPUNIT_ASSERT( filesize == stat.st_size );

    file.reset(meta->GetFileOperation(testFile,O_RDWR));
    CPPUNIT_ASSERT( NULL != file.get() );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    CPPUNIT_ASSERT( true == file->Write(0,buffer,bufsize,size) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    file.reset();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );

    backup.reset( new FileOperationDelay(backupFile,O_WRONLY,0,0,10) );
    boost::thread thread(&MetaManager::Backup,meta,testFile,backup.get(),tape,path,write);
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    boost::this_thread::sleep(boost::posix_time::milliseconds(2));
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    file.reset(meta->GetFileOperation(testFile,O_WRONLY));
    CPPUNIT_ASSERT( NULL != file.get() );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    CPPUNIT_ASSERT( true == file->Write(0,buffer,bufsize,size) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    CPPUNIT_ASSERT( false == thread.timed_join(
            boost::posix_time::milliseconds(2) ) );
    CPPUNIT_ASSERT( true == thread.timed_join(
            boost::posix_time::milliseconds(10) ) );
}


void
MetaManagerTest::testBackupOnDelete()
{
    MetaManager * meta = Factory::GetMetaManager();
    CacheManager * cache = Factory::GetCacheManager();
    string tape = "barcode0";
    string testFile = "MetaManagerTest_testBackupOnDelete";
    string testBackup = "MetaManagerTest_testBackupOnDelete_backup";
    char buffer[1024] = "Hello,World!\n";
    const size_t bufsize = sizeof(buffer);
    const off_t filesize = 2 * 1024 * 1024;
    auto_ptr<FileOperationInterface> file;
    auto_ptr<FileOperationInterface> backup;
    fs::path path;
    size_t size;
    auto_ptr<boost::thread> thread;

    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0644) );
    CPPUNIT_ASSERT( false == cache->ExistFile(1) );
    file.reset( meta->GetFileOperation(testFile,O_WRONLY) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( NULL != file.get() );
    for (off_t offset=0; offset<filesize; offset += bufsize) {
        size = 0;
        CPPUNIT_ASSERT( true == file->Write(offset,buffer,bufsize,size) );
        CPPUNIT_ASSERT( size == bufsize );
    }
    vector<BackupItem> list;
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    file.reset();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );

    backup.reset( new FileOperationDelay(backupFile,0644,O_WRONLY,0,0,10) );
    bool write = false;
    thread.reset( new boost::thread(
            &MetaManager::Backup,meta,testFile,backup.get(),tape,path,write) );
    boost::this_thread::sleep(boost::posix_time::milliseconds(2));
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFile) );
    CPPUNIT_ASSERT( false == cache->ExistFile(1) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    CPPUNIT_ASSERT( false == thread->timed_join(
            boost::posix_time::milliseconds(2) ) );
    CPPUNIT_ASSERT( true == thread->timed_join(
            boost::posix_time::milliseconds(10) ) );

    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0644) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
    file.reset( meta->GetFileOperation(testFile,O_WRONLY) );
    CPPUNIT_ASSERT( true == cache->ExistFile(2) );
    CPPUNIT_ASSERT( NULL != file.get() );
    for (off_t offset=0; offset<filesize; offset += bufsize) {
        size = 0;
        CPPUNIT_ASSERT( true == file->Write(offset,buffer,bufsize,size) );
        CPPUNIT_ASSERT( size == bufsize );
    }
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    file.reset();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );

    backup.reset( new FileOperationDelay(backupFile,O_WRONLY,0,0,10) );
    write = false;
    thread.reset( new boost::thread(
            &MetaManager::Backup,meta,testFile,backup.get(),tape,path,write) );
    boost::this_thread::sleep(boost::posix_time::milliseconds(2));
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    file.reset( meta->GetFileOperation(testFile,O_WRONLY) );
    CPPUNIT_ASSERT( NULL != file.get() );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    CPPUNIT_ASSERT( true == cache->ExistFile(2) );
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFile) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    CPPUNIT_ASSERT( false == thread->timed_join(
            boost::posix_time::milliseconds(2) ) );
    CPPUNIT_ASSERT( true == file->Write(0,buffer,bufsize,size) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
    CPPUNIT_ASSERT( true == thread->timed_join(
            boost::posix_time::milliseconds(10) ) );
    CPPUNIT_ASSERT( true == file->Write(0,buffer,bufsize,size) );
    CPPUNIT_ASSERT( false == cache->ExistFile(2) );
}


void
MetaManagerTest::testBackupOnRename()
{
    MetaManager * meta = Factory::GetMetaManager();
    CacheManager * cache = Factory::GetCacheManager();
    string tape = "barcode0";
    string testFile = "MetaManagerTest_testBackupOnRename";
    string testFileNew = "MetaManagerTest_testBackupOnRename_new";
    string testBackup = "MetaManagerTest_testBackupOnRename_backup";
    char buffer[1024] = "Hello,World!\n";
    const size_t bufsize = sizeof(buffer);
    const off_t filesize = 2 * 1024 * 1024;
    auto_ptr<FileOperationInterface> file;
    auto_ptr<FileOperationInterface> backup;
    fs::path path;
    size_t size;
    auto_ptr<boost::thread> thread;

    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0644) );
    CPPUNIT_ASSERT( false == cache->ExistFile(1) );
    file.reset( meta->GetFileOperation(testFile,O_WRONLY) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( NULL != file.get() );
    for (off_t offset=0; offset<filesize; offset += bufsize) {
        size = 0;
        CPPUNIT_ASSERT( true == file->Write(offset,buffer,bufsize,size) );
        CPPUNIT_ASSERT( size == bufsize );
    }
    vector<BackupItem> list;
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    file.reset();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );

    backup.reset( new FileOperationDelay(backupFile,0644,O_WRONLY,0,0,10) );
    path = testFile;
    bool write = false;
    thread.reset( new boost::thread(
            &MetaManager::Backup,meta,testFile,backup.get(),tape,path,write) );
    boost::this_thread::sleep(boost::posix_time::milliseconds(2));
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    CPPUNIT_ASSERT( true == meta->RenameInode(testFile,testFileNew) );
    CPPUNIT_ASSERT( true == cache->ExistFile(1) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    CPPUNIT_ASSERT( testFileNew == list[0].path );
    CPPUNIT_ASSERT( false == thread->timed_join(
            boost::posix_time::milliseconds(10) ) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    CPPUNIT_ASSERT( testFileNew == list[0].path );
    thread->join();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
}

void
MetaManagerTest::testPersist()
{
    MetaManager * meta = Factory::GetMetaManager();
    string testFile1 = "MetaManagerTest_testPersist_file1";
    string testFile2 = "MetaManagerTest_testPersist_file2";
    string testFile3 = "MetaManagerTest_testPersist_file3";
    auto_ptr<FileOperationInterface> file1, file2, file3;
    vector<BackupItem> list;
    char buffer[1024] = "Hello,World!\n";
    size_t size;

    CPPUNIT_ASSERT( true == meta->CreateFile(testFile1,0600) );
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile2,0600) );
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile3,0600) );
    file1.reset(meta->GetFileOperation(testFile1,O_RDWR));
    file2.reset(meta->GetFileOperation(testFile2,O_RDWR));
    file3.reset(meta->GetFileOperation(testFile3,O_RDWR));
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 3 == list.size() );
    CPPUNIT_ASSERT( true == file1->Write(0,buffer,sizeof(buffer),size) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 2 == list.size() );
    CPPUNIT_ASSERT( true == file2->Write(0,buffer,sizeof(buffer),size) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    CPPUNIT_ASSERT( true == file3->Write(0,buffer,sizeof(buffer),size) );
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 0 == list.size() );
    file1.reset();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 1 == list.size() );
    file2.reset();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 2 == list.size() );
    file3.reset();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 3 == list.size() );

    Factory::ReleaseMetaManager();
    Factory::CreateMetaManager();
    meta = Factory::GetMetaManager();

    list.clear();
    meta->GetBackupList(list);
    if ( 3 != list.size() ) {
        cout <<  "actual size: " << list.size() << endl;
        abort();
    }
    CPPUNIT_ASSERT( 3 == list.size() );

    Factory::ReleaseMetaManager();
    fs::remove(metaFolder + "/.save");
    Factory::CreateMetaManager();
    meta = Factory::GetMetaManager();

    list.clear();
    meta->GetBackupList(list);
    CPPUNIT_ASSERT( 3 == list.size() );
}
