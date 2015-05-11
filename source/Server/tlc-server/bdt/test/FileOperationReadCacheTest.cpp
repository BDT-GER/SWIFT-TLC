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
 * FileOperationReadCacheTest.cpp
 *
 *  Created on: Aug 31, 2012
 *      Author: More Zeng
 */

#include "stdafx.h"
#include "../ReadCacheManager.h"
#include "ReadTapeFileSimulator.h"
#include "FileOperationReadCacheTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( FileOperationReadCacheTest );

void
FileOperationReadCacheTest::setUp()
{
	//fs::path path = "/usr/LTFStor/Cache/boost_1_44_0.tar";
	//fs::path simPath = "/root/Desktop/boost_1_44_0.tar";
	pReadCacheManager = new ReadCacheManager("/usr/LTFStor/Cache");
	//if(pReadCacheManager)
	//	pInterface = pReadCacheManager->GetFileOperation(new ReadTapeFileSimulator(simPath),  path);
}


void
FileOperationReadCacheTest::tearDown()
{
	if(pReadCacheManager)
		delete pReadCacheManager;
	pReadCacheManager = NULL;

	if(pInterface)
		delete pInterface;
	pInterface = NULL;
}

void
FileOperationReadCacheTest::testOpenFile()
{
	START_TEST(__func__);

	CPPUNIT_ASSERT(pReadCacheManager!=NULL);
	//CPPUNIT_ASSERT(pInterface!=NULL);
	//CPPUNIT_ASSERT(pInterface->OpenFile(O_RDONLY));
	END_TEST(__func__);
}

void
FileOperationReadCacheTest::testRead()
{
	START_TEST(__func__);
	fs::path path = "/usr/LTFStor/Cache/boost_1_44_0.tar";
	fs::path simPath = "/root/Desktop/boost_1_44_0.tar";
	#define BUFSIZE (1024*1024*2)
	char recvBuf[BUFSIZE];
	off_t  readSize = 0;
	size_t recvSize = 0;

	CPPUNIT_ASSERT(pReadCacheManager!=NULL);
	pInterface = pReadCacheManager->GetFileOperation(path);
	CPPUNIT_ASSERT(pInterface==NULL);
	pInterface = pReadCacheManager->GetFileOperation(new ReadTapeFileSimulator(simPath),  path);
	CPPUNIT_ASSERT(pInterface!=NULL);
	CPPUNIT_ASSERT(pReadCacheManager->GetFileOperation(path)!=NULL);

	CPPUNIT_ASSERT(pInterface->OpenFile(O_RDONLY));

	do
	{
		CPPUNIT_ASSERT(pInterface!=NULL);
		CPPUNIT_ASSERT(pInterface->Read(readSize,recvBuf, BUFSIZE, recvSize));

		if(recvSize==0)
			break;
		readSize += recvSize;
		//boost::this_thread::sleep(boost::posix_time::seconds(1));
	}while(true);

	fprintf(stderr, "Go to sleep...\n");
	boost::this_thread::sleep(boost::posix_time::seconds(15));

	END_TEST(__func__);
}
