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
 * CfgfileParserTest.cpp
 *
 *  Created on: Oct 15, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "../SimChanger.h"
#include "../CfgfileParser.h"
#include "CfgfileParserTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CfgfileParserTest );

using namespace ltfs_management;

void
CfgfileParserTest::setUp()
{
	// TODO Auto-generated constructor stub

}

void
CfgfileParserTest::tearDown()
{
	// TODO Auto-generated destructor stub

}

void
CfgfileParserTest::testParserElement()
{
	START_TEST(__func__);

	struct CfgDetail detail;
	CfgfileParser parser(CFG_FILE.c_str());

	CPPUNIT_ASSERT(parser.ParserElement(detail));

	cout << "size_for_trigger_read_error "<<detail.m_SizeReadErr<<endl;
	cout<<"number_of_read_errors "<<detail.m_NumReadErr<<endl;
	cout<<"size_for_trigger_write_error "<<detail.m_SizeWriteErr<<endl;
	cout<<"number_of_write_errors "<<detail.m_NumWriteErr<<endl;
	cout<<"read_delay "<<detail.m_ReadDelay<<endl;
	cout<<"write_delay "<<detail.m_WriteDelay<<endl;
	cout<<"move_tape_delay "<<detail.m_MoveDelay<<endl;
	cout<<"enable_move_tape_failure "<<detail.m_MoveFailure<< endl;
	cout<<""<<endl;
	cout<<""<<endl;
	cout<<""<<endl;
	cout<<""<<endl;
	cout<<""<<endl;
	cout<<""<<endl;

	END_TEST(__func__);
}

void
CfgfileParserTest::testParserChangers()
{
	START_TEST(__func__);

	vector<SimChanger> changers;
	CfgfileParser parser(CFG_FILE.c_str());
	CPPUNIT_ASSERT(parser.ParserChangers(changers));


	END_TEST(__func__);
}
