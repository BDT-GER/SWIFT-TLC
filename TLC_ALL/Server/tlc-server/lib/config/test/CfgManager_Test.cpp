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
 * CfgManager_Test.cpp
 *
 *  Created on: Dec 18th, 2013
 *      Author: Sam Chen
 */

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "stdafx.h"
#include "CfgManagerTest.h"

int main(int argc, char * argv[]) {
	CppUnit::Test * suite =
			CppUnit::TestFactoryRegistry::getRegistry().makeTest();
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(suite);

	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
	bool ret = runner.run();
	return ret ? 0 : 1;
}
