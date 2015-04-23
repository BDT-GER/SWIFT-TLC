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
 * FileOperationTapeStatusTest.cpp
 *
 *  Created on: Mar 22, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../FileOperationTapeStatus.h"
#include "../MetaManagerTapeStatus.h"
#include "FileOperationTapeStatusTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileOperationTapeStatusTest );


static const string metaFolder("FileOperationTapeStatusTest");
static const string tapeFolder("tape-folder");
static const string cacheFolder("cache-folder");
static const string testFile("/test-file");
static const string testFileSystem
        = MetaManagerTapeStatus::SYSTEM_FOLDER + testFile;
static const string testFolder("/test-folder");
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

const string barcodeOnline("01000001");
const string barcodeExport("01000017");


void
FileOperationTapeStatusTest::setUp()
{
    fs::create_directory(folderMeta);
    fs::create_directory(folderTape);
    fs::create_directory(folderTape / barcodeOnline);
    fs::create_directory(folderCache);

    Factory::SetCacheFolder(folderCache);
    Factory::SetTapeFolder(folderTape);
    Factory::SetMetaFolder(folderMeta);
}


void
FileOperationTapeStatusTest::tearDown()
{
    fs::remove_all(folderMeta);
    fs::remove_all(folderTape);
    fs::remove_all(folderCache);
}


void
FileOperationTapeStatusTest::testFileOperation()
{
    Factory::ReleaseTapeLibraryManager();
    Factory::CreateTapeLibraryManager();
    Factory::ReleaseTapeManager();
    Factory::CreateTapeManager();
    Factory::ReleaseCacheManager();
    Factory::CreateCacheManager();
    auto_ptr<MetaManager> meta(
            new MetaManagerTapeStatus() );
    auto_ptr<MetaManager> metaRaw(
            new MetaManager() );

    auto_ptr<FileOperationInterface> oper;
    auto_ptr<FileOperationInterface> operRaw;
    fs::path entry;

    char buffer[1024] = "hello,world!\n";
    char bufferRead[sizeof(buffer)];
    string content;
    size_t size;
    struct stat stat;

    const int actCreate = FileOperationInterface::ActionCreate;
    const int actWrite = FileOperationInterface::ActionWrite;
    const int actRead = FileOperationInterface::ActionRead;


    oper.reset( meta->GetFileOperation(
            MetaManagerTapeStatus::SYSTEM_FOLDER,false,actRead,barcodeOnline) );
    CPPUNIT_ASSERT( NULL != oper.get() );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFile(O_RDWR,0644,true) );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFolder(0755,true) );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT_ERRNO_( EISDIR == errno);
    CPPUNIT_ASSERT( true == oper->OpenFolder() );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);


    oper.reset( meta->GetFileOperation( "/", false, actRead, barcodeOnline ) );
    CPPUNIT_ASSERT( NULL != oper.get() );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFile(O_RDWR,0644,true) );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFolder(0755,true) );
    CPPUNIT_ASSERT_ERRNO_( EINVAL == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT_ERRNO_( EISDIR == errno);
    CPPUNIT_ASSERT( true == oper->OpenFolder() );
    CPPUNIT_ASSERT( true == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT( entry.filename() == MetaManagerTapeStatus::SYSTEM_FOLDER );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);


    oper.reset( meta->GetFileOperation(testFile,true,actCreate,barcodeOnline) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFolder() );
    CPPUNIT_ASSERT_ERRNO_( EINVAL == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);
    CPPUNIT_ASSERT( true == oper->CreateFile(O_RDWR,0644,false) );
    CPPUNIT_ASSERT( true == oper->Write(0,buffer,sizeof(buffer),size) );
    memset(bufferRead,0,sizeof(bufferRead));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( sizeof(bufferRead) == size );
    CPPUNIT_ASSERT( 0 == memcmp(buffer,bufferRead,sizeof(buffer)) );

    oper.reset( meta->GetFileOperation(testFile,true,actRead,barcodeOnline) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFile(O_RDWR,0644,true) );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFolder(0755,true) );
    CPPUNIT_ASSERT_ERRNO_( EINVAL == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFolder() );
    CPPUNIT_ASSERT_ERRNO_( ENOTDIR == errno);
    CPPUNIT_ASSERT( true == oper->OpenFile(O_RDWR) );
    memset(bufferRead,0,sizeof(bufferRead));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( sizeof(bufferRead) == size );
    CPPUNIT_ASSERT( 0 == memcmp(buffer,bufferRead,sizeof(buffer)) );

    operRaw.reset( metaRaw->GetFileOperation(
            testFile, true, actRead, barcodeOnline ) );
    CPPUNIT_ASSERT( true == operRaw->OpenFile(O_RDWR) );
    memset(bufferRead,0,sizeof(bufferRead));
    size = 0;
    CPPUNIT_ASSERT(
            true == operRaw->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( sizeof(bufferRead) == size );
    CPPUNIT_ASSERT( 0 == memcmp(buffer,bufferRead,sizeof(buffer)) );

    oper.reset( meta->GetFileOperation(
            MetaManagerTapeStatus::SYSTEM_FOLDER,false,actRead,barcodeOnline) );
    CPPUNIT_ASSERT( NULL != oper.get() );
    CPPUNIT_ASSERT( true == oper->OpenFolder() );
    CPPUNIT_ASSERT( true == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT( entry.filename() == testFile.substr(1) );

    oper.reset( meta->GetFileOperation(
            testFileSystem, true, actRead, barcodeOnline ) );
    CPPUNIT_ASSERT( true == oper->OpenFile(O_RDWR) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_ERRNO_( EINVAL == errno);
    memset(bufferRead,0,sizeof(bufferRead));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( InodeTapeStatus::FILE_SIZE == size );
    content = barcodeOnline + "#Online";
    CPPUNIT_ASSERT( 0 == memcmp(content.c_str(),bufferRead,content.size()) );

    oper.reset( meta->GetFileOperation( "/", false, actRead, barcodeOnline ) );
    CPPUNIT_ASSERT( NULL != oper.get() );
    CPPUNIT_ASSERT( true == oper->OpenFolder() );
    CPPUNIT_ASSERT( true == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT( entry.filename() == MetaManagerTapeStatus::SYSTEM_FOLDER );
    CPPUNIT_ASSERT( true == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT( entry.filename() == testFile.substr(1) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);


    oper.reset( meta->GetFileOperation(
            testFolder, false, actCreate, barcodeOnline ) );
    CPPUNIT_ASSERT( true == oper->CreateFolder(0755,true) );
    oper.reset(meta->GetFileOperation(testFolder,false,actRead,barcodeOnline));
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFolder(0755,true) );
    CPPUNIT_ASSERT_ERRNO_( EINVAL == errno);
    CPPUNIT_ASSERT( true == oper->OpenFolder() );
    CPPUNIT_ASSERT( true == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT( entry.filename() == MetaManagerTapeStatus::SYSTEM_FOLDER );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);

    operRaw.reset( metaRaw->GetFileOperation(
            testFolder, false, actRead, barcodeOnline ) );
    CPPUNIT_ASSERT( true == operRaw->OpenFolder() );
    errno = 0;
    CPPUNIT_ASSERT( false == operRaw->ReadFolder(entry) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);

    oper.reset( meta->GetFileOperation(
            MetaManagerTapeStatus::SYSTEM_FOLDER,false,actRead,barcodeOnline) );
    CPPUNIT_ASSERT( true == oper->OpenFolder() );
    CPPUNIT_ASSERT( true == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT( entry.filename() == testFile.substr(1) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);


    oper.reset( meta->GetFileOperation(
            testFolderFile, true, actCreate, barcodeExport ) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFolder() );
    CPPUNIT_ASSERT_ERRNO_( EINVAL == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);
    CPPUNIT_ASSERT( true == oper->CreateFile(O_RDWR,0644,false) );
#if 0
    CPPUNIT_ASSERT( false == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT( false == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( fs::create_directory( folderTape / barcodeExport ) );
#endif
    CPPUNIT_ASSERT( true == oper->Write(0,buffer,sizeof(buffer),size) );
    memset(bufferRead,0,sizeof(bufferRead));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( sizeof(bufferRead) == size );
    CPPUNIT_ASSERT( 0 == memcmp(buffer,bufferRead,sizeof(buffer)) );

    oper.reset( meta->GetFileOperation(
            testFolderFile, true, actRead, barcodeExport ) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFile(O_RDWR,0644,true) );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->CreateFolder(0755,true) );
    CPPUNIT_ASSERT_ERRNO_( EINVAL == errno);
    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFolder() );
    CPPUNIT_ASSERT_ERRNO_( ENOTDIR == errno);
    CPPUNIT_ASSERT( true == oper->OpenFile(O_RDWR) );
    memset(bufferRead,0,sizeof(bufferRead));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( 0 == memcmp(buffer,bufferRead,sizeof(buffer)) );

    operRaw.reset(metaRaw->GetFileOperation(
            testFolderFile,true,actRead,barcodeExport) );
    CPPUNIT_ASSERT( true == operRaw->OpenFile(O_RDWR) );
    memset(bufferRead,0,sizeof(bufferRead));
    size = 0;
    CPPUNIT_ASSERT(
            true == operRaw->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( 0 == memcmp(buffer,bufferRead,sizeof(buffer)) );

    oper.reset(meta->GetFileOperation(
            testFolderSystem, false, actRead, barcodeOnline ) );
    CPPUNIT_ASSERT( NULL != oper.get() );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT_ERRNO_( EISDIR == errno);
    CPPUNIT_ASSERT( true == oper->OpenFolder() );
    CPPUNIT_ASSERT( true == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT( entry.filename() == testFile.substr(1) );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->ReadFolder(entry) );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno);

    oper.reset( meta->GetFileOperation(
            testFolderFileSystem,true,actRead,barcodeExport) );
    stat.st_size = 0;
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( InodeTapeStatus::FILE_SIZE == stat.st_size );
    CPPUNIT_ASSERT( true == oper->OpenFile(O_RDWR) );
    stat.st_size = 0;
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT( InodeTapeStatus::FILE_SIZE == stat.st_size );
    errno = 0;
    CPPUNIT_ASSERT( false == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_ERRNO_( EINVAL == errno);
    memset(bufferRead,0,sizeof(bufferRead));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( InodeTapeStatus::FILE_SIZE == size );
    content = barcodeExport + "#Export";
    CPPUNIT_ASSERT( 0 == memcmp(content.c_str(),bufferRead,content.size()) );
    fs::remove_all( folderTape / barcodeExport );
    memset(bufferRead,0,sizeof(bufferRead));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( InodeTapeStatus::FILE_SIZE == size );
    content = barcodeExport + "#Export";
    CPPUNIT_ASSERT( 0 == memcmp(content.c_str(),bufferRead,content.size()) );
}

