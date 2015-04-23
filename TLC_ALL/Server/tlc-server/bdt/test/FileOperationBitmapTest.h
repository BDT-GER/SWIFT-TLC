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
 * FileOperationBitmapTest.h
 *
 *  Created on: Mar 12, 2013
 *      Author: More Zeng
 */


#pragma once


class FileOperationBitmapTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( FileOperationBitmapTest );
    CPPUNIT_TEST( testConstructor );
    CPPUNIT_TEST( testTruncateBitmap );
    CPPUNIT_TEST( testBitmap );
    CPPUNIT_TEST( testBitmapPartial );
    CPPUNIT_TEST( testClose );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testConstructor();
    void testTruncateBitmap();
    void testBitmap();
    void testBitmapPartial();
    void testClose();
};

