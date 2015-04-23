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
 * FileDigestTest.cpp
 *
 *  Created on: Sep 26, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "FileDigestTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileDigestTest );


const string digest0KMD5 = "d41d8cd98f00b204e9800998ecf8427e";
const string digest0KSHA1 = "da39a3ee5e6b4b0d3255bfef95601890afd80709";
const string digest1KSHA1 = "60cacbf3d72e1e7834203da608037b1bf83b40e8";
const string digest1KMD5 = "0f343b0931126a20f133d67c2b018a3b";
const string digest4KSHA1 = "1ceaf73df40e531df3bfb26b4fb7cd95fb7bff1d";
const string digest4KMD5 = "620f0b67a91f7f74151bc5be745b7110";


void
FileDigestTest::setUp()
{
}


void
FileDigestTest::tearDown()
{
}


void
FileDigestTest::testFileDigest()
{
    auto_ptr<FileDigest> object;
    char buffer[4096];
    memset(buffer,0,sizeof(buffer));
    string digest;

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest) );
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest) );
    CPPUNIT_ASSERT(true==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(false==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest) );
    CPPUNIT_ASSERT_MESSAGE(digest,digest==digest1KMD5);
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest) );
    CPPUNIT_ASSERT(true==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(false==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT_MESSAGE(digest,digest==digest1KSHA1);
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(true==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest0KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest0KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest1KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest1KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,4096));
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest4KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest4KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());

#if 0
    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->DisableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest1KMD5);
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest));

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->DisableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest1KSHA1);

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->DisableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(false==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(true==object->DisableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(false==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
#endif

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->UpdateContent(1024,buffer,3072));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest4KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest4KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(1024,buffer,3072));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest4KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest4KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateSize(4096));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->UpdateContent(3072,buffer,1024));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest4KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest4KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateSize(4096));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest4KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest4KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateSize(4096));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->UpdateSize(1024));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest1KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest1KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateSize(4096));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->UpdateContent(1024,buffer,3072));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateSize(1024));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(false==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateContent(1024,buffer,3072));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(false==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,4096));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateSize(1024));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateSize(4096));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(false==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateSize(4096));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(false==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateSize(0));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateContent(1024,buffer,3072));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(false==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest1KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest1KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateContent(1024,buffer,3072));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateSize(4096));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(false==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest1KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest1KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateSize(4096));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->UpdateContent(1024,buffer,3072));
    CPPUNIT_ASSERT(false==object->IsValid());
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(false==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(false==object->IsValid());

    object.reset(new FileDigest());
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_SHA1));
    CPPUNIT_ASSERT(true==object->EnableDigest(FileDigest::DIGEST_MD5));
    CPPUNIT_ASSERT(true==object->UpdateContent(0,buffer,1024));
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest1KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest1KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_MD5,digest));
    CPPUNIT_ASSERT(digest==digest1KMD5);
    CPPUNIT_ASSERT(true==object->GetDigest(FileDigest::DIGEST_SHA1,digest));
    CPPUNIT_ASSERT(digest==digest1KSHA1);
    CPPUNIT_ASSERT(true==object->IsValid());
}

