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
 * loggerManager.cpp
 *
 *  Created on: Sep 20, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "loggerManager.h"

namespace ltfs_logger
{
LoggerManager LoggerManager::Manager;

LoggerManager::LoggerManager():
		cfgFileExists(false)
{
	// TODO Auto-generated constructor stub
	try
	{
		fs::path pFile(COMM_APP_CONF_PATH + "/log4cplus.cfg");
		Logger root = Logger::getRoot();
		loggerList.insert(ListLogType::value_type("root", root));

		if(exists(pFile))
		{
			PropertyConfigurator::doConfigure(pFile.string());
			cfgFileExists = true;
		}
	}
	catch(...)
	{

	}

}

LoggerManager::~LoggerManager()
{
	// TODO Auto-generated destructor stub
}

Logger LoggerManager::GetLogger(string name, bool useDef)
{

#if 1
	return Logger::getInstance(name);

#else
	if(name.empty())
	{
		name = "ltfs";
	}
	ListLogType::iterator i = Manager.loggerList.find(name);
	if(i == Manager.loggerList.end())
	{

		Logger tmpLog = Logger::getInstance(name);
		if(!Manager.cfgFileExists)
		{

		}
		Manager.loggerList.insert(ListLogType::value_type(name, tmpLog));
		return tmpLog;
	}
	return i->second;
#endif
}


FunctionLogger::FunctionLogger(const string& file, const string& function, int line, const string& logName)
{
	file_ = file;
	func_ = function;
	line_ = line;
	logName_= logName;
	LgDebug(logName_, "" << file_ << "::" << func_ << ":" << line_ << " start.");
}
FunctionLogger::~FunctionLogger()
{
	LgDebug(logName_, "" << file_ << "::" << func_ << ":" << line_ << " end.");
}

} /* namespace ltfs_logger */
