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
 * FileOperationMetaTest.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../FileOperationCache.h"
#include "../FileOperationMeta.h"
#include "../MetaManager.h"
#include "FileOperationMetaTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileOperationMetaTest );


static const string metaFolder("meta");
static const string cacheFolder("cache");
static const string testFile("/test-file");
static const string testFolder("/test-folder");
static const string testFolder2("/test-folder2");

static const fs::path folderMeta(metaFolder);
static const fs::path folderCache(cacheFolder);

static const string barcode0("barcode0");
static const string barcode1("barcode1");
static const string barcode2("barcode2");


void
FileOperationMetaTest::setUp()
{
    fs::remove_all(folderMeta);
    fs::remove_all(folderCache);

    fs::create_directory(folderMeta);
    fs::create_directory(folderCache);

    Factory::SetMetaFolder(folderMeta);
    Factory::SetCacheFolder(folderCache);
}


void
FileOperationMetaTest::tearDown()
{
    fs::remove_all(folderMeta);
    fs::remove_all(folderCache);
}


void
FileOperationMetaTest::testFileOperation()
{
    auto_ptr<MetaManager> meta(new MetaManager());
    auto_ptr<FileOperationInterface> oper;
    oper.reset( new FileOperationMeta(
            meta.get(),
            testFolder,
            NULL,
            "" ) );
    oper->CreateFolder(0700,false);
    oper.reset( new FileOperationMeta(
            meta.get(),
            testFolder2,
            NULL,
            "" ) );
    oper->CreateFolder(0700,false);

    testFileOperationSub(testFile);
    testFileOperationSub(testFolder + testFile);
    testFileOperationSub(testFolder2 + testFile);
}


void
FileOperationMetaTest::testFileOperationSubNone(
        auto_ptr<FileOperationInterface> & oper, const fs::path & fileMeta,
        const fs::path & fileEntity,
        const string & tapeName)
{
    string tape;
    char buffer[1024] = "hello,world!\n";
    char bufferRead[sizeof(buffer)];
    size_t size;
    struct stat stat;

    errno = 0;
    CPPUNIT_ASSERT( false == oper->GetStat(stat) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);
    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);
    CPPUNIT_ASSERT_MESSAGE( fileEntity.file_string(),
            true == oper->CreateFile(O_RDWR,0644,false) );
    CPPUNIT_ASSERT( fs::exists(fileMeta) );
    CPPUNIT_ASSERT( fs::exists(fileEntity) );
    CPPUNIT_ASSERT( true == oper->Delete() );
    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    CPPUNIT_ASSERT( true == oper->GetTape(tape) );
    CPPUNIT_ASSERT( tape == tapeName );
}


void
FileOperationMetaTest::testFileOperationSubCreate(
        auto_ptr<MetaManager> & meta,
        const fs::path & path,
        auto_ptr<FileOperationInterface> & oper,
        const fs::path & fileMeta,
        const fs::path & fileEntity,
        const string & tapeName)
{
    string tape;
    auto_ptr<Inode> inode;
    char buffer[1024] = "hello,world!\n";
    char bufferRead[sizeof(buffer)];
    size_t size;
    struct stat stat;

    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->GetStat(stat) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);
    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);
    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->GetStat(stat) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);
    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    CPPUNIT_ASSERT( true == oper->CreateFile(O_RDWR,0644,false) );
    CPPUNIT_ASSERT( fs::exists(fileMeta) );
    CPPUNIT_ASSERT( fs::exists(fileEntity) );
    inode.reset(meta->GetInode(path));
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );

    tape.clear();
    CPPUNIT_ASSERT( true == oper->GetTape(tape) );
    CPPUNIT_ASSERT( tape == tapeName );
    tape.clear();
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );
    CPPUNIT_ASSERT( tape == tapeName );

    CPPUNIT_ASSERT( false == oper->CreateFile(O_RDWR,0644,false) );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno);

    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( 0 == stat.st_size );

    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( 0 == size );

    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( 0 == stat.st_size );
}


void
FileOperationMetaTest::testFileOperationSubOpenDelete(
        auto_ptr<MetaManager> & meta,
        const fs::path & path,
        auto_ptr<FileOperationInterface> & oper,
        const fs::path & fileMeta,
        const fs::path & fileEntity,
        const string & tapeName)
{
    string tape;
    auto_ptr<Inode> inode;
    char buffer[1024] = "hello,world!\n";
    char bufferRead[sizeof(buffer)];
    size_t size;
    struct stat stat;

    CPPUNIT_ASSERT( fs::exists(fileMeta) );
    CPPUNIT_ASSERT( fs::exists(fileEntity) );
    inode.reset(meta->GetInode(path));
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );

    tape.clear();
    CPPUNIT_ASSERT( true == oper->GetTape(tape) );
    CPPUNIT_ASSERT( tape == tapeName );
    tape.clear();
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );
    CPPUNIT_ASSERT( tape == tapeName );

    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);

    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFile(O_RDWR,0644,false) );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno);

    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);

    CPPUNIT_ASSERT( true == oper->OpenFile(O_RDWR) );

    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( 0 == stat.st_size );

    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( 0 == size );

    tape.clear();
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );
    CPPUNIT_ASSERT( tape == tapeName );

    CPPUNIT_ASSERT( true == oper->Write(0,buffer,sizeof(buffer),size) );
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( sizeof(buffer) == size );
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( sizeof(buffer) == stat.st_size );

    tape.clear();
    CPPUNIT_ASSERT( true == oper->GetTape(tape) );
    CPPUNIT_ASSERT( tape == tapeName );
    tape.clear();
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );
    CPPUNIT_ASSERT( tape == tapeName );

    CPPUNIT_ASSERT( true == oper->Delete() );
    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );
}


void
FileOperationMetaTest::testFileOperationSubCreateTruncate(
        auto_ptr<MetaManager> & meta,
        const fs::path & path,
        auto_ptr<FileOperationInterface> & oper,
        const fs::path & fileMeta,
        const fs::path & fileEntity,
        const string & tapeName)
{
    string tape;
    auto_ptr<Inode> inode;
    char buffer[1024] = "hello,world!\n";
    char bufferRead[sizeof(buffer)];
    size_t size;
    struct stat stat;

    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->GetStat(stat) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno);
    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);
    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    CPPUNIT_ASSERT( true == oper->CreateFile(O_RDWR,0644,false) );
    CPPUNIT_ASSERT( fs::exists(fileMeta) );
    CPPUNIT_ASSERT_MESSAGE( fileEntity.file_string(), fs::exists(fileEntity) );
    inode.reset(meta->GetInode(path));
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFile(O_RDWR,0644,false) );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno);

    tape.clear();
    CPPUNIT_ASSERT( true == oper->GetTape(tape) );
    CPPUNIT_ASSERT( tapeName == tape );
    tape.clear();
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );
    CPPUNIT_ASSERT( tapeName == tape );

    CPPUNIT_ASSERT( true == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT( fs::exists(fileMeta) );
    CPPUNIT_ASSERT( fs::exists(fileEntity) );
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( sizeof(buffer) == stat.st_size );
    CPPUNIT_ASSERT( true == oper->Truncate( sizeof(buffer) - 5 ) );
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( sizeof(buffer) - 5 == stat.st_size );
    CPPUNIT_ASSERT( true == oper->Truncate( sizeof(buffer) + 5 ) );
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( sizeof(buffer) + 5 == stat.st_size );
    CPPUNIT_ASSERT( true == oper->Truncate( sizeof(buffer) ) );
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( sizeof(buffer) == stat.st_size );

    tape.clear();
    CPPUNIT_ASSERT( true == oper->GetTape(tape) );
    CPPUNIT_ASSERT( tapeName == tape );
    tape.clear();
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );
    CPPUNIT_ASSERT( tape == tapeName );
}


void
FileOperationMetaTest::testFileOperationSubCheckTruncate(
        auto_ptr<MetaManager> & meta,
        const fs::path & path,
        auto_ptr<FileOperationInterface> & oper,
        const fs::path & fileMeta,
        const fs::path & fileEntity,
        const string & tapeName)
{
    string tape;
    auto_ptr<Inode> inode;
    struct stat stat;
    char buffer[1024] = "hello,world!\n";

    CPPUNIT_ASSERT( fs::exists(fileMeta) );
    CPPUNIT_ASSERT( fs::exists(fileEntity) );
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( sizeof(buffer) == stat.st_size );
    inode.reset(meta->GetInode(path));
    CPPUNIT_ASSERT( true == inode->GetTape(tape) );
    CPPUNIT_ASSERT( tape == tapeName );

    CPPUNIT_ASSERT( true == oper->Delete() );
}


void
FileOperationMetaTest::testFileOperationSubCreateClose(
        auto_ptr<MetaManager> & meta,
        const fs::path & path,
        auto_ptr<FileOperationInterface> & oper,
        const fs::path & fileMeta,
        const fs::path & fileEntity,
        const string & tapeName)
{
    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);
    CPPUNIT_ASSERT( ! fs::exists(fileMeta) );
    CPPUNIT_ASSERT( ! fs::exists(fileEntity) );

    CPPUNIT_ASSERT( true == oper->CreateFile(O_RDWR,0644,false) );
    CPPUNIT_ASSERT( fs::exists(fileMeta) );
    CPPUNIT_ASSERT( fs::exists(fileEntity) );

    oper.reset();

    CPPUNIT_ASSERT( fs::exists(fileMeta) );
    CPPUNIT_ASSERT( fs::exists(fileEntity) );
}


void
FileOperationMetaTest::testFileOperationSub(const fs::path & path)
{
    Factory::ReleaseTapeManager();
    Factory::CreateTapeManager();
    Factory::ReleaseCacheManager();
    Factory::CreateCacheManager();
    auto_ptr<MetaManager> meta( new MetaManager() );
    CacheManager * cache = Factory::GetCacheManager();
    fs::path fileMeta = folderMeta / path;
    fs::path fileEntity = folderCache / path;
    auto_ptr<FileOperationInterface> oper;
    const int actCreate = FileOperationInterface::ActionCreate;
    const int actWrite = FileOperationInterface::ActionWrite;
    const int actRead = FileOperationInterface::ActionRead;


    oper.reset( new FileOperationMeta(
            meta.get(),
            path,
            new FileOperationCache(fileEntity,true,actCreate,cache),
            barcode0 ) );
    testFileOperationSubNone(oper,fileMeta,fileEntity,barcode0);


    oper.reset( new FileOperationMeta(
            meta.get(),
            path,
            new FileOperationCache(fileEntity,true,actCreate,cache),
            barcode1 ) );
    testFileOperationSubCreate(meta,path,oper,fileMeta,fileEntity,barcode1);


    oper.reset( new FileOperationMeta(
            meta.get(),
            path,
            new FileOperationCache(fileEntity,true,actWrite,cache),
            barcode1 ) );
    testFileOperationSubOpenDelete(meta,path,oper,fileMeta,fileEntity,barcode1);


    oper.reset( meta->GetFileOperation(path,true,actCreate,barcode1) );
    fs::path fileTapeEntity = folderCache / path;
    testFileOperationSubCreate(meta,path,oper,fileMeta,fileTapeEntity,barcode1);


    oper.reset( meta->GetFileOperation(path,true,actWrite,barcode1) );
    testFileOperationSubOpenDelete(
            meta, path, oper, fileMeta, fileTapeEntity, barcode1 );


    oper.reset( new FileOperationMeta(
            meta.get(),
            path,
            new FileOperationCache(fileEntity,true,actCreate,cache),
            barcode2 ) );
    testFileOperationSubCreateTruncate(
            meta, path, oper, fileMeta, fileEntity, barcode2 );


    oper.reset();
    oper.reset( new FileOperationMeta(
            meta.get(),
            path,
            new FileOperationCache(fileEntity,true,actRead,cache),
            barcode2 ) );
    testFileOperationSubCheckTruncate(
            meta, path, oper, fileMeta, fileEntity, barcode2);
    CPPUNIT_ASSERT( true == oper->Delete() );


    oper.reset( new FileOperationMeta(
            meta.get(),
            path,
            new FileOperationCache(fileEntity,true,actCreate,cache),
            barcode2 ) );
    testFileOperationSubCreateClose(
            meta, path, oper, fileMeta, fileEntity, barcode2 );

    oper.reset( new FileOperationMeta(
            meta.get(),
            path,
            new FileOperationCache(fileEntity,true,actWrite,cache),
            barcode2 ) );
    CPPUNIT_ASSERT( true == oper->Delete() );


    oper.reset( meta->GetFileOperation(path,true,actCreate,barcode2) );
    fileTapeEntity = folderCache / path;
    testFileOperationSubCreateTruncate(
            meta, path, oper, fileMeta, fileTapeEntity, barcode2 );


    oper.reset( meta->GetFileOperation(path,true,actWrite,barcode2) );
    testFileOperationSubCheckTruncate(
            meta, path, oper, fileMeta, fileTapeEntity, barcode2);
    CPPUNIT_ASSERT( true == oper->Delete() );


    oper.reset( meta->GetFileOperation(path,true,actCreate,barcode2) );
    testFileOperationSubCreateClose(
            meta, path, oper, fileMeta, fileTapeEntity, barcode2);
}

