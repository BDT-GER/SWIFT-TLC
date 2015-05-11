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
 * TestBase.cpp
 *
 *  Created on: Aug 22, 2012
 *      Author: Sam Chen
 */

#include "stdafx.h"
#include "TestBase.h"
#include "../../../lib/ltfs_library/CmnFunc.h"

using namespace ltfs_management;

TestBase::TestBase()
{
}
TestBase::~TestBase()
{
}

void TestBase::setUp()
{
	vector<string> lines = GetCommandOutputLines("cat ./test_data/test_conf.cfg");
	for(unsigned int i = 0; i < lines.size(); i++){
		// comments lines
		regex matchComment("(^\\s*$)|(^\\s*#.*)");
		cmatch match;
		if(regex_match(lines[i].c_str(), match, matchComment)){
			continue;
		}
        istringstream is(lines[i]);
        //#RealEnv changer_num   changer_sgName  changer_stName  drive_num   drive_sgName    drive_stName    tape_num    slot_num
        is >> m_bRealEnv >> m_nChangerNum >> m_changerSgName >> m_changerStName >> m_nDriveNum \
        >> m_driveSgName >> m_driveStName >> m_nTapeNum >> m_nSlotNum >> m_nMailSlotNum;

        break;
	}
}
void TestBase::tearDown()
{
}
