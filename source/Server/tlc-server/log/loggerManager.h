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
 * loggerManager.h
 *
 *  Created on: Sep 20, 2012
 *      Author: chento
 */

#ifndef __LOGGERMANAGER_H__
#define __LOGGERMANAGER_H__

#include "stdafx.h"
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;


namespace ltfs_logger
{
#if 1
	#define Lg4CPLUS(log_level, name, msg)\
	{\
		Logger lgger = LoggerManager::GetLogger(name); \
		pid_t pid = getpid();\
		 pid_t tid = syscall(SYS_gettid);\
		log_level(lgger, "pid/tid:" << pid << ":" << tid << " " << msg); \
	}
	#define LgDebug(name, msg); \
			{ \
					Lg4CPLUS(LOG4CPLUS_DEBUG, name, msg); \
			}

	#define LgInfo(name, msg); \
			{ \
					Lg4CPLUS(LOG4CPLUS_INFO, name, msg); \
			}
#else
	#define LgDebug(name, msg);
	#define LgInfo(name, msg);
#endif

	#define LgWarn(name, msg) \
			{ \
					Lg4CPLUS(LOG4CPLUS_WARN, name, msg); \
			};

	#define LgError(name, msg) \
			{ \
					Lg4CPLUS(LOG4CPLUS_ERROR, name, msg); \
			};

	#define LgFatal(name, msg) \
			{ \
					Lg4CPLUS(LOG4CPLUS_FATAL, name, msg); \
			};



	const string EVENT_LEVEL_DEBUG 		= "debug";
	const string EVENT_LEVEL_INFO 		= "info";
	const string EVENT_LEVEL_WARNING 	= "warning";
	const string EVENT_LEVEL_ERR 		= "err";
	const string EVENT_LEVEL_CRITICAL	= "crit";
	const string EVENT_LEVEL_ALL	 	= "all";

#if 1
		#define CmnLog(tag, LogLevel, msg);\
				LogLevel(tag, msg);
#else
		#define CmnLog(tag, LogLevel, msg);
#endif

	#define CmnLogDebug(tag, msg);  CmnLog(tag, LgDebug, msg);
	#define CmnLogInfo(tag, msg);  CmnLog(tag, LgInfo, msg);
	#define CmnLogWarn(tag, msg);  CmnLog(tag, LgWarn, msg);
	#define CmnLogError(tag, msg); CmnLog(tag, LgError, msg);
	#define CmnLogFatal(tag, msg); CmnLog(tag, LgFatal, msg);

	#define CmnEvent(tag, level, eventId, msg);\
	{\
		string ___event_prefix___ = "";\
		if(string(eventId) != ""){\
			___event_prefix___ = string("EVENT[") + eventId + string("]:");\
		}\
		if(level == EVENT_LEVEL_DEBUG){\
			CmnLog(tag, LgDebug, msg);\
		}\
		else if(level == EVENT_LEVEL_INFO){\
			CmnLog(tag, LgInfo, ___event_prefix___ << msg);\
		}\
		else if(level == EVENT_LEVEL_WARNING){\
			CmnLog(tag, LgWarn, ___event_prefix___ << msg);\
		}\
		else if(level ==EVENT_LEVEL_ERR){\
			CmnLog(tag, LgError, ___event_prefix___ << msg);\
		}\
		else if(level == EVENT_LEVEL_CRITICAL){\
			CmnLog(tag, LgFatal, ___event_prefix___ << msg);\
		}else{\
			CmnLog(tag, LgDebug, ___event_prefix___ << msg);\
		}\
	}

	class LoggerManager
	{
	public:
		static Logger GetLogger(string name, bool useDef=false);

	private:
		LoggerManager();
		virtual ~LoggerManager();
	private:
		static LoggerManager Manager;
		typedef map<string, Logger> ListLogType;
		ListLogType loggerList;
		bool cfgFileExists;
	};

	class FunctionLogger
	{
	public:
		FunctionLogger(const string& file, const string& function, int line, const string& logName);
		~FunctionLogger();
	private:
		string file_;
		string func_;
		int line_;
		string logName_;
	};

#ifdef DEBUG
#define VS_DBG_LOG_FUNCTION \
	FunctionLogger dbgLogFunction__(__FILE__, __func__, __LINE__, "vs");
#else
#define VS_DBG_LOG_FUNCTION ;
#endif

} /* namespace ltfs_logger */
#endif /* LOGGERMANAGER_H_ */
