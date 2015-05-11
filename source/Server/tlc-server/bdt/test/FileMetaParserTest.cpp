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
 * FileMetaParserTest.cpp
 *
 *  Created on: Mar 27, 2015
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../FileMetaParser.h"
#include "../MetaManager.h"
#include "FileMetaParserTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileMetaParserTest );


static const string testFile = "/test-file";
static const string metaFolder = "meta.folder";
static const string cacheFolder = "cache.folder";

void
FileMetaParserTest::setUp()
{
    fs::create_directory(cacheFolder);
    Factory::SetCacheFolder(cacheFolder);
    Factory::CreateCacheManager();

    fs::create_directory(metaFolder);
    Factory::SetMetaFolder(metaFolder);
    Factory::CreateMetaManager();
}


void
FileMetaParserTest::tearDown()
{
    Factory::ReleaseMetaManager();
    fs::remove_all(metaFolder);

    Factory::ReleaseCacheManager();
    fs::remove_all(cacheFolder);
}


void
FileMetaParserTest::testMetaSingle()
{
    MetaManager * meta = Factory::GetMetaManager();
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0600) );
    auto_ptr<Inode> inode(meta->GetInode(testFile));

    char buffer[1024];
    ifstream input("single.xattr", std::ios::in|std::ios::binary);
    size_t length = input.readsome(buffer,sizeof(buffer)-1);
    buffer[length] = '\0';
    string content = buffer;

    CPPUNIT_ASSERT(inode->SetExtendedAttribute("user.swift.metadata", buffer, 10));
    CPPUNIT_ASSERT(inode->SetExtendedAttribute("user.swift.metadata1", buffer + 10, 10));
    CPPUNIT_ASSERT(inode->SetExtendedAttribute("user.swift.metadata2", buffer + 20, length - 20));

    CPPUNIT_ASSERT(parser_.ParseSwiftMeta(testFile));
    CPPUNIT_ASSERT(parser_.GetName() == "/AUTH_a7b3d7d404a14df993313de181b794e2/ss/etc/swift/swift.conf");
    CPPUNIT_ASSERT(! parser_.IsMultiple());
    CPPUNIT_ASSERT(! parser_.IsManifest());
    CPPUNIT_ASSERT(parser_.GetNumber() == -1);
    CPPUNIT_ASSERT(parser_.GetTotal() == 1);
}


void
FileMetaParserTest::testMetaMultiple()
{
    MetaManager * meta = Factory::GetMetaManager();

    for (int i=0; i<5; ++i) {
        testMetaMultipleSub(i);
    }
}


void
FileMetaParserTest::testMetaMultipleSub(int number)
{
    MetaManager * meta = Factory::GetMetaManager();
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0600) );
    auto_ptr<Inode> inode(meta->GetInode(testFile));

    char buffer[1024];
    string path = boost::lexical_cast<string>(number) + ".attr";
    ifstream input(path.c_str(), std::ios::in|std::ios::binary);
    size_t length = input.readsome(buffer,sizeof(buffer)-1);
    buffer[length] = '\0';
    string content = buffer;

    CPPUNIT_ASSERT(inode->SetExtendedAttribute("user.swift.metadata", buffer, 40));
    CPPUNIT_ASSERT(inode->SetExtendedAttribute("user.swift.metadata1", buffer + 40, length - 40));

    CPPUNIT_ASSERT(parser_.ParseSwiftMeta(testFile));
    string name = "/AUTH_7b4e0995cdb5423d94adf4d7bc93b89a/CC_segments/large/l_3twrhUYtQWN6Rji.bin/1422305129.671900/5000000000/1000000000/0000000";
    name += boost::lexical_cast<string>(number);
    CPPUNIT_ASSERT(parser_.GetName() == name);
    CPPUNIT_ASSERT(parser_.IsMultiple());
    CPPUNIT_ASSERT(! parser_.IsManifest());
    CPPUNIT_ASSERT_MESSAGE( parser_.GetManifest(),
            parser_.GetManifest() == "/1422305129.671900/5000000000/1000000000/");
    CPPUNIT_ASSERT(parser_.GetNumber() == number);
    CPPUNIT_ASSERT(parser_.GetTotal() == 5);

    inode.reset();
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFile) );
}


void
FileMetaParserTest::testMetaMultipleManifest()
{
    MetaManager * meta = Factory::GetMetaManager();
    CPPUNIT_ASSERT( true == meta->CreateFile(testFile,0600) );
    auto_ptr<Inode> inode(meta->GetInode(testFile));

    char buffer[1024];
    ifstream input("manifest.attr", std::ios::in|std::ios::binary);
    size_t length = input.readsome(buffer,sizeof(buffer)-1);
    buffer[length] = '\0';
    string content = buffer;

    CPPUNIT_ASSERT(inode->SetExtendedAttribute("user.swift.metadata", buffer, 40));
    CPPUNIT_ASSERT(inode->SetExtendedAttribute("user.swift.metadata1", buffer + 40, length - 40));

    CPPUNIT_ASSERT(parser_.ParseSwiftMeta(testFile));
    string name = "/AUTH_7b4e0995cdb5423d94adf4d7bc93b89a/CC/large/l_3twrhUYtQWN6Rji.bin";
    CPPUNIT_ASSERT(parser_.GetName() == name);
    CPPUNIT_ASSERT(parser_.IsMultiple());
    CPPUNIT_ASSERT(parser_.IsManifest());
    CPPUNIT_ASSERT_MESSAGE( parser_.GetManifest(),
            parser_.GetManifest() == "/1422305129.671900/5000000000/1000000000/");
    CPPUNIT_ASSERT(parser_.GetNumber() == -1);
    CPPUNIT_ASSERT(parser_.GetTotal() == 5);

    inode.reset();
    CPPUNIT_ASSERT( true == meta->DeleteInode(testFile) );
}

