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
 * FileOperationMetaTest.h
 *
 *  Created on: Mar 20, 2012
 *      Author: More Zeng
 */


#pragma once


class FileOperationMetaTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( FileOperationMetaTest );
    CPPUNIT_TEST( testFileOperation );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testFileOperation();

private:
    void testFileOperationSub(const fs::path & path);
    void testFileOperationSubNone(
            auto_ptr<FileOperationInterface> & oper,
            const fs::path & fileMeta,
            const fs::path & fileEntity,
            const string & tapeName);
    void testFileOperationSubCreate(
            auto_ptr<MetaManager> & meta,
            const fs::path & path,
            auto_ptr<FileOperationInterface> & oper,
            const fs::path & fileMeta,
            const fs::path & fileEntity,
            const string & tapeName);
    void testFileOperationSubOpenDelete(
            auto_ptr<MetaManager> & meta,
            const fs::path & path,
            auto_ptr<FileOperationInterface> & oper,
            const fs::path & fileMeta,
            const fs::path & fileEntity,
            const string & tapeName);
    void testFileOperationSubCreateTruncate(
            auto_ptr<MetaManager> & meta,
            const fs::path & path,
            auto_ptr<FileOperationInterface> & oper,
            const fs::path & fileMeta,
            const fs::path & fileEntity,
            const string & tapeName);
    void testFileOperationSubCheckTruncate(
            auto_ptr<MetaManager> & meta,
            const fs::path & path,
            auto_ptr<FileOperationInterface> & oper,
            const fs::path & fileMeta,
            const fs::path & fileEntity,
            const string & tapeName);
    void testFileOperationSubCreateClose(
            auto_ptr<MetaManager> & meta,
            const fs::path & path,
            auto_ptr<FileOperationInterface> & oper,
            const fs::path & fileMeta,
            const fs::path & fileEntity,
            const string & tapeName);
};

