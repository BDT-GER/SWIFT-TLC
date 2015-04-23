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
 * ExtendedAttributeTest.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ExtendedAttributeTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ExtendedAttributeTest );


static const string testFolder = "test-folder";
static const string testFile = "test-file";


void
ExtendedAttributeTest::setUp()
{
    fs::create_directory(testFolder);
    ofstream file(testFile.c_str());
    CPPUNIT_ASSERT( fs::is_directory(testFolder) );
    CPPUNIT_ASSERT( fs::is_regular_file(testFile) );
}


void
ExtendedAttributeTest::tearDown()
{
    fs::remove_all(testFolder);
    fs::remove(testFile);
}


void
ExtendedAttributeTest::testExtendedAttribute()
{
    ExtendedAttribute xattrFolder(testFolder);
    ExtendedAttribute xattrFile(testFile);
    ExtendedAttribute xattrHandle(::open(testFile.c_str(),O_RDWR));
    vector<string> names;

    CPPUNIT_ASSERT( xattrFolder.GetNameList(names) );
    CPPUNIT_ASSERT( 0 == names.size() );
    CPPUNIT_ASSERT( xattrFolder.SetValue("user.more","more",4) );
    CPPUNIT_ASSERT( xattrFolder.GetNameList(names) );
    CPPUNIT_ASSERT( 1 == names.size() );
    CPPUNIT_ASSERT( names[0] == "user.more" );
    CPPUNIT_ASSERT( false == xattrFolder.SetValue("system.more","more",4) );
    CPPUNIT_ASSERT( xattrFolder.GetNameList(names) );
    CPPUNIT_ASSERT( 1 == names.size() );
    CPPUNIT_ASSERT( names[0] == "user.more" );
    CPPUNIT_ASSERT( xattrFolder.SetValue("user.amy","amy",3) );
    CPPUNIT_ASSERT( xattrFolder.GetNameList(names) );
    CPPUNIT_ASSERT( 2 == names.size() );
    CPPUNIT_ASSERT( xattrFolder.SetValue("user.yile","yile",4) );
    CPPUNIT_ASSERT( xattrFolder.GetNameList(names) );
    CPPUNIT_ASSERT( 3 == names.size() );
    char buffer[1024];
    int size;
    CPPUNIT_ASSERT(
            xattrFolder.GetValue("user.more",buffer,sizeof(buffer),size) );
    buffer[size] = '\0';
    CPPUNIT_ASSERT( "more" == string(buffer) );
    CPPUNIT_ASSERT(
            xattrFolder.GetValue("user.amy",buffer,sizeof(buffer),size) );
    buffer[size] = '\0';
    CPPUNIT_ASSERT( "amy" == string(buffer) );
    CPPUNIT_ASSERT(
            xattrFolder.GetValue("user.yile",buffer,sizeof(buffer),size) );
    buffer[size] = '\0';
    CPPUNIT_ASSERT( "yile" == string(buffer) );

    CPPUNIT_ASSERT( xattrFile.GetNameList(names) );
    CPPUNIT_ASSERT( 0 == names.size() );
    for ( int i = 0; i < 100; ++i ) {
        string name = "user." + boost::lexical_cast<string>(i);
        CPPUNIT_ASSERT( xattrFile.SetValue(name,&i,sizeof(i)) );
    }
    CPPUNIT_ASSERT( xattrFile.GetNameList(names) );
    CPPUNIT_ASSERT( 100 == names.size() );
    CPPUNIT_ASSERT( xattrHandle.GetNameList(names) );
    CPPUNIT_ASSERT( 100 == names.size() );
    for ( int i = 0; i < 100; ++i ) {
        string name = "user." + boost::lexical_cast<string>(i);
        int value, size;
        CPPUNIT_ASSERT( xattrFile.GetValue(name,&value,sizeof(value),size) );
        CPPUNIT_ASSERT( size == sizeof(value) );
        CPPUNIT_ASSERT( i == value );
        CPPUNIT_ASSERT( xattrHandle.GetValue(name,&value,sizeof(value),size) );
        CPPUNIT_ASSERT( size == sizeof(value) );
        CPPUNIT_ASSERT( i == value );
    }
}
