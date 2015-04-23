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
 * MetaManagerTest.h
 *
 *  Created on: Mar 1, 2012
 *      Author: More Zeng
 */


#pragma once


class MetaManagerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( MetaManagerTest );
    CPPUNIT_TEST( testConstructor );
    CPPUNIT_TEST( testDeleteFile );
    CPPUNIT_TEST( testRenameFile );
    CPPUNIT_TEST( testRenameFileOverride );
    CPPUNIT_TEST( testDeleteFolder );
    CPPUNIT_TEST( testRenameFolder );
    CPPUNIT_TEST( testRenameFolderOverride );
    CPPUNIT_TEST( testGetFileOperation );
    CPPUNIT_TEST( testBackup );
    CPPUNIT_TEST( testBackupOnWrite );
    CPPUNIT_TEST( testBackupOnDelete );
    CPPUNIT_TEST( testBackupOnRename );
    CPPUNIT_TEST( testPersist );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testConstructor();
    void testDeleteFile();
    void testRenameFile();
    void testRenameFileOverride();
    void testDeleteFolder();
    void testRenameFolder();
    void testRenameFolderOverride();
    void testGetFileOperation();
    void testBackup();
    void testBackupOnWrite();
    void testBackupOnDelete();
    void testBackupOnRename();
    void testPersist();
};

