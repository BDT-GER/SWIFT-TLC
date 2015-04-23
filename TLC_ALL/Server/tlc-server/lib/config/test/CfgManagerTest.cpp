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
 * CfgManagerTest.cpp
 *
 *  Created on: Nov 6, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "../CfgParser.h"
#include "../CfgSerialization.h"
#include "../CfgManager.h"
#include "CfgManagerTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CfgManagerTest );

using namespace ltfs_config;

CfgManager* manager = NULL;

void CfgManagerTest::setUp()
{
	manager = CfgManager::Instance();
}

void CfgManagerTest::tearDown()
{

}

/*void CfgManagerTest::testGetCfgDetail()
{
	START_TEST(__func__);

	CfgDataDetail detail;
	CPPUNIT_ASSERT(manager->GetCfgDetail(detail));

	cout << "mCacheFreeMinSize:" << detail.mCacheFreeMinSize << endl;
	cout << "mCacheFreeMaxSize:" << detail.mCacheFreeMaxSize << endl;
	cout << "mCacheFreeMinPercent:" << detail.mCacheFreeMinPercent << endl;
	cout << "mCacheFreeMaxPercent:" << detail.mCacheFreeMaxPercent << endl;
	cout << "mCacheFileSizeRead:" << detail.mCacheFileSizeRead << endl;
	cout << "mFileMaxSize:" << detail.mFileMaxSize << endl;
	cout << "mTapeIdleTime:" << detail.mTapeIdleTime << endl;
	cout << "mDigestMD5Enable:" << detail.mDigestMD5Enable << endl;
	cout << "mDigestSHA1Enable:" << detail.mDigestSHA1Enable << endl;
	cout << "mTapeReservedMinFreeSize:" << detail.mTapeReservedMinFreeSize << endl;

	END_TEST(__func__);
}*/

void CfgManagerTest::testGet()
{
	START_TEST(__func__);

	string value;
	string item = "LTFStorConf.CacheFreeMinSize";

	CPPUNIT_ASSERT(manager->Get(item, value));
	cout << "LTFStorConf.CacheFreeMinSize "<<value<<endl;

	item = "LTFStorConf.CacheFreeMaxSize";
	CPPUNIT_ASSERT(manager->Get(item, value));
	cout << "LTFStorConf.CacheFreeMaxSize "<<value<<endl;

	item = "LTFStorConf.CacheFreeMinPercent";
	CPPUNIT_ASSERT(manager->Get(item, value));
	cout << "LTFStorConf.CacheFreeMinPercent "<<value<<endl;

	item = "LTFStorConf.CacheFreeMaxPercent";
	CPPUNIT_ASSERT(manager->Get(item, value));
	cout << "LTFStorConf.CacheFreeMaxPercent "<<value<<endl;

	item = "LTFStorConf.CacheFileSizeRead";
	CPPUNIT_ASSERT(manager->Get(item, value));
	cout << "LTFStorConf.CacheFileSizeRead "<<value<<endl;

	item = "LTFStorConf.FileMaxSize";
	CPPUNIT_ASSERT(manager->Get(item, value));
	cout << "LTFStorConf.FileMaxSize "<<value<<endl;

	item = "LTFStorConf.TapeIdleTime";
	CPPUNIT_ASSERT(manager->Get(item, value));
	cout << "LTFStorConf.TapeIdleTime "<<value<<endl;

	item = "LTFStorConf.DigestMD5Enable";
	CPPUNIT_ASSERT(manager->Get(item, value));
	cout << "LTFStorConf.DigestMD5Enable "<<value<<endl;

	item = "LTFStorConf.DigestSHA1Enable";
	CPPUNIT_ASSERT(manager->Get(item, value));
	cout << "LTFStorConf.DigestSHA1Enable "<<value<<endl;

	item = "LTFStorConf.TapeReservedMinFreeSize";
	CPPUNIT_ASSERT(manager->Get(item, value));
	cout << "LTFStorConf.TapeReservedMinFreeSize "<<value<<endl;

	END_TEST(__func__);
}

void CfgManagerTest::testGetUInt64()
{
	START_TEST(__func__);

	UInt64_t value = 0;
	string item = "LTFStorConf.CacheFreeMinPercent";
	CPPUNIT_ASSERT(manager->GetUInt64(item, value));
	cout << "LTFStorConf.CacheFreeMinPercent "<<value<<endl;

	item = "LTFStorConf.CacheFreeMaxPercent";
	CPPUNIT_ASSERT(manager->GetUInt64(item, value));
	cout << "LTFStorConf.CacheFreeMaxPercent "<<value<<endl;

	END_TEST(__func__);
}

void CfgManagerTest::testGetUInt32()
{
	START_TEST(__func__);

	UInt32_t value = 0;
	string item = "LTFStorConf.CacheFreeMinPercent";
	CPPUNIT_ASSERT(manager->GetUInt32(item, value));
	cout << "LTFStorConf.CacheFreeMinPercent "<<value<<endl;

	item = "LTFStorConf.CacheFreeMaxPercent";
	CPPUNIT_ASSERT(manager->GetUInt32(item, value));
	cout << "LTFStorConf.CacheFreeMaxPercent "<<value<<endl;

	END_TEST(__func__);
	END_TEST(__func__);
}

void CfgManagerTest::testGetUInt16()
{
	START_TEST(__func__);

	UInt16_t value = 0;
	string item = "LTFStorConf.CacheFreeMinPercent";
	CPPUNIT_ASSERT(manager->GetUInt16(item, value));
	cout << "LTFStorConf.CacheFreeMinPercent "<<value<<endl;

	item = "LTFStorConf.CacheFreeMaxPercent";
	CPPUNIT_ASSERT(manager->GetUInt16(item, value));
	cout << "LTFStorConf.CacheFreeMaxPercent "<<value<<endl;

	END_TEST(__func__);
}

void CfgManagerTest::testGetFloat()
{
	START_TEST(__func__);

	END_TEST(__func__);
}

void CfgManagerTest::testGetBool()
{
	START_TEST(__func__);

	bool value;
	string item = "LTFStorConf.DigestMD5Enable";
	CPPUNIT_ASSERT(manager->GetBool(item, value));
	cout << "LTFStorConf.DigestMD5Enable "<<value<<endl;

	item = "LTFStorConf.DigestSHA1Enable";
	CPPUNIT_ASSERT(manager->GetBool(item, value));
	cout << "LTFStorConf.DigestSHA1Enable "<<value<<endl;

	END_TEST(__func__);
}

void CfgManagerTest::testGetSize()
{
	START_TEST(__func__);

	size_t value;
	string item = "LTFStorConf.CacheFreeMinSize";

	CPPUNIT_ASSERT(manager->GetSize(item, value));
	cout << "LTFStorConf.CacheFreeMinSize "<<value<<endl;

	item = "LTFStorConf.CacheFreeMaxSize";
	CPPUNIT_ASSERT(manager->GetSize(item, value));
	cout << "LTFStorConf.CacheFreeMaxSize "<<value<<endl;

	item = "LTFStorConf.CacheFileSizeRead";
	CPPUNIT_ASSERT(manager->GetSize(item, value));
	cout << "LTFStorConf.CacheFileSizeRead "<<value<<endl;

	item = "LTFStorConf.FileMaxSize";
	CPPUNIT_ASSERT(manager->GetSize(item, value));
	cout << "LTFStorConf.FileMaxSize "<<value<<endl;

	item = "LTFStorConf.TapeReservedMinFreeSize";
	CPPUNIT_ASSERT(manager->GetSize(item, value));
	cout << "LTFStorConf.TapeReservedMinFreeSize "<<value<<endl;

	END_TEST(__func__);
}


/*void CfgManagerTest::testSetFileRetentionStartTime()
{
	START_TEST(__func__);
	CPPUNIT_ASSERT(manager->SetFileRetentionStartTime("10:10:10"));
	END_TEST(__func__);
}*/

/*void CfgManagerTest::testSetShareRetention()
{
	START_TEST(__func__);

	ShareRetention shareRetention;
	shareRetention.mGroupID = "c9b94f90-f0c3-11e1-a7bb-000c291584d8";
	shareRetention.mRetentionUnit = "minutes";
	shareRetention.mRetentionValue = 1000;
	CPPUNIT_ASSERT(manager->SetShareRetention(shareRetention));
	END_TEST(__func__);
}*/

void CfgManagerTest::testSetChild()
{
	START_TEST(__func__);
	CPPUNIT_ASSERT(manager->SetChild("LTFStorConf.FileRetentionStartTime", "11:11:11"));
	END_TEST(__func__);
}

void CfgManagerTest::testDeleteNode()
{
	START_TEST(__func__);
	CPPUNIT_ASSERT(manager->DeleteNode(string("LTFStorConf.ShareRetention"),string("7918fb38-4ee6-4aba-a658-609fdd905d6f")));
	END_TEST(__func__);
}

