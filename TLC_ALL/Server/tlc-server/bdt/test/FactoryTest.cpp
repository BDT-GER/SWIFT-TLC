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
 * FactoryTest.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: More Zeng
 */

#include "stdafx.h"
#include "../Factory.h"
#include "FactoryTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FactoryTest );


fs::path metaFolder;
fs::path cacheFolder;

void
FactoryTest::setUp()
{
    metaFolder = fs::system_complete(fs::path("meta"));
    cacheFolder = fs::system_complete(fs::path("cache"));
    Factory::SetMetaFolder(metaFolder);
    Factory::SetCacheFolder(cacheFolder);
}


void
FactoryTest::tearDown()
{
    Factory::SetMetaFolder("");
    Factory::SetCacheFolder("");
}


void
FactoryTest::testPath()
{
    const string name = "test.test";
    fs::path pathMeta = metaFolder / name;
    fs::path pathCache;
    CPPUNIT_ASSERT(
            true == Factory::GetCachePathFromMetaPath(pathMeta,pathCache) );
    CPPUNIT_ASSERT( pathCache == cacheFolder / name );
    pathMeta = "";
    CPPUNIT_ASSERT(
            true == Factory::GetMetaPathFromCachePath(pathCache,pathMeta) );
    CPPUNIT_ASSERT( pathMeta == metaFolder / name );
}
