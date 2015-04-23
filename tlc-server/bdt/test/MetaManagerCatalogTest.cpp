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
 * MetaManagerCatalogTest.cpp
 *
 *  Created on: Nov 23, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../MetaManager.h"
#include "../MetaManagerCatalog.h"
#include "MetaManagerCatalogTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( MetaManagerCatalogTest );


static const string metaFolder = "meta-folder";
static const string testFile = "/test-file";
static const string testFolder = "/test-folder";
static const string testFolderFile = testFolder + testFile;
static const string barcode = "01000001";

static const fs::path folderMeta(metaFolder);


void
MetaManagerCatalogTest::setUp()
{
    fs::remove_all(folderMeta);
    fs::create_directory(folderMeta);

    Factory::SetMetaFolder(folderMeta);
}


void
MetaManagerCatalogTest::tearDown()
{
    fs::remove_all(folderMeta);
}


void
MetaManagerCatalogTest::testInode()
{
    auto_ptr<MetaManagerInterface> metaRaw(new MetaManager());
    auto_ptr<MetaManagerInterface> meta;
    meta.reset( new MetaManagerCatalog( metaRaw.get() ) );
    fs::path folderCatalog = Factory::GetMetaCatalogFolder();
    CPPUNIT_ASSERT( fs::is_directory(folderCatalog) );
    off_t size;

    CPPUNIT_ASSERT( NULL == meta->GetInode(testFile) );
    CPPUNIT_ASSERT( NULL == meta->GetInode(testFolder) );
    CPPUNIT_ASSERT( NULL == meta->GetInode(testFolderFile) );

    auto_ptr<Inode> inode(meta->CreateFile(testFile,0644));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFile) );

    CPPUNIT_ASSERT( true == inode->SetSize(0) );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFile) );

    CPPUNIT_ASSERT( true == inode->SetSize(1024) );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFile) );

    CPPUNIT_ASSERT( true == inode->SetTape(barcode) );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / barcode / testFile) );

    CPPUNIT_ASSERT( true == inode->SetSize(0) );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFile) );
    CPPUNIT_ASSERT( fs::exists(folderCatalog / barcode / testFile) );
    CPPUNIT_ASSERT(
            0 == fs::file_size(folderCatalog / barcode / testFile) );

    CPPUNIT_ASSERT( true == inode->SetSize(1024) );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFile) );
    CPPUNIT_ASSERT( fs::exists(folderCatalog / barcode / testFile) );
    size = fs::file_size(folderCatalog / barcode / testFile);
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(size), 1024 == size );


    inode.reset(meta->CreateFolder(testFolder,0755));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFolder) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFolder) );


    inode.reset(meta->CreateFile(testFolderFile,0644));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFolderFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFolderFile) );

    CPPUNIT_ASSERT( true == inode->SetSize(0) );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFolderFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFolderFile) );

    CPPUNIT_ASSERT( true == inode->SetSize(1024) );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFolderFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFolderFile) );

    CPPUNIT_ASSERT( true == inode->SetTape(barcode) );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFolderFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFolderFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / barcode / testFolderFile) );

    CPPUNIT_ASSERT( true == inode->SetSize(0) );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFolderFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFolderFile) );
    CPPUNIT_ASSERT( fs::exists(folderCatalog / barcode / testFolderFile) );
    size = fs::file_size(folderCatalog / barcode / testFolderFile);
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(size), 0 == size );

    CPPUNIT_ASSERT( true == inode->SetSize(1024) );
    CPPUNIT_ASSERT( fs::exists(folderMeta / testFolderFile) );
    CPPUNIT_ASSERT( ! fs::exists(folderCatalog / testFolderFile) );
    CPPUNIT_ASSERT( fs::exists(folderCatalog / barcode / testFolderFile) );
    size = fs::file_size(folderCatalog / barcode / testFolderFile);
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(size), 1024 == size );


    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == inode->IsFile() );

    inode.reset(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( false == inode->IsFile() );

    inode.reset(meta->GetInode(testFolderFile));
    CPPUNIT_ASSERT( NULL != inode.get() );
    CPPUNIT_ASSERT( true == inode->IsFile() );


    CPPUNIT_ASSERT(false == meta->DeleteInode(testFolder));
    CPPUNIT_ASSERT(true == meta->DeleteInode(testFolderFile));
    CPPUNIT_ASSERT(true == meta->DeleteInode(testFolder));
    CPPUNIT_ASSERT(true == meta->DeleteInode(testFile));

    inode.reset(meta->GetInode(testFile));
    CPPUNIT_ASSERT( NULL == inode.get() );

    inode.reset(meta->GetInode(testFolder));
    CPPUNIT_ASSERT( NULL == inode.get() );

    inode.reset(meta->GetInode(testFolderFile));
    CPPUNIT_ASSERT( NULL == inode.get() );
}

