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
 * CfgManager.cpp
 *
 *  Created on: Nov 5, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "CfgParser.h"
#include "CfgSerialization.h"
#include "CfgManager.h"
#include "ConfManager.h"

namespace ltfs_config
{
	CfgManager * CfgManager::instance_ = NULL;
	boost::mutex CfgManager::instanceMutex_;
	static string ConfigPrefix = "VSConf.";

    const string CfgManager::WriteCachePercent("WriteCachePercent");
    const string CfgManager::MetaFreeLeastSize("MetaFreeLeastSize");
    const string CfgManager::CacheFreeLeastSize("CacheFreeLeastSize");
    const string CfgManager::ReadCacheFreeSize("ReadCacheFreeSize");
    const string CfgManager::WriteCacheFreeSize("WriteCacheFreeSize");
    const string CfgManager::CacheFreeMinSize("CacheFreeMinSize");
    const string CfgManager::CacheFreeMaxSize("CacheFreeMaxSize");
    const string CfgManager::CacheFreeMinPercent("CacheFreeMinPercent");
    const string CfgManager::CacheFreeMaxPercent("CacheFreeMaxPercent");
    const string CfgManager::CacheFileSizeRead("CacheFileSizeRead");
    const string CfgManager::CacheFileSizeBlock("CacheFileSizeBlock");
    const string CfgManager::CacheWriteMode("CacheWriteMode");
    const string CfgManager::FileMaxSize("FileMaxSize");
    const string CfgManager::TapeIdleTime("TapeIdleTime");
    const string CfgManager::FileIdleTime("FileIdleTime");
    const string CfgManager::DigestMD5Enable("DigestMD5Enable");
    const string CfgManager::DigestSHA1Enable("DigestSHA1Enable");
    const string CfgManager::ThrottleInterval("ThrottleInterval");
    const string CfgManager::ThrottleValve("ThrottleValve");
    const string CfgManager::BackupWaitSize("WriteToTapeWaitSize");
    const string CfgManager::BackupWaitTime("WriteToTapeWaitTime");
    const string CfgManager::BackupWaitFile("WriteToTapeWaitFile");
    const string CfgManager::ThreadBackupSize("ThreadWriteToTapeSize");
    const string CfgManager::MetaFileSize("MetaFileSize");
    const string CfgManager::FilterFilePattern("FilterFilePattern");
    const string CfgManager::FilterFolderPattern("FilterFolderPattern");
    const string CfgManager::BackupFilePattern("WriteToTapeFilePattern");
    const string CfgManager::BackupFolderPattern("WriteToTapeFolderPattern");
    const string CfgManager::ChangerSyncMode("ChangerSyncMode");
    const string CfgManager::CachePurgeWaitTime("CachePurgeWaitTime");
    const string CfgManager::CachePurgeWaitFile("CachePurgeWaitFile");
    const string CfgManager::TapeAuditorRunAt("TapeAuditorRun");
    const string CfgManager::TapeAuditorMaxnum("TapeAuditorMaxnum");
    const string CfgManager::TapeAuditorInterval("TapeAuditorInterval");
    const string CfgManager::TapeAuditorEnable("TapeAuditorEnable");
    const string CfgManager::ReservedDriveForRead("ReservedDriveForRead");
    const string CfgManager::DeleteTapeFileTriggerNum("DeleteTapeFileTriggerNum");
    const string CfgManager::DeleteTapeFileTimeDiff("DeleteTapeFileTimeDiff");

	CfgManager::CfgManager()
	{
		// TODO Auto-generated constructor stub
        confDefault_[ConfigPrefix + CfgManager::DeleteTapeFileTriggerNum] = boost::lexical_cast<string>(100);
        confDefault_[ConfigPrefix + CfgManager::DeleteTapeFileTimeDiff] = boost::lexical_cast<string>(60*10);
        confDefault_[ConfigPrefix + CfgManager::ThreadBackupSize] = "4G";
        confDefault_[ConfigPrefix + CfgManager::BackupWaitSize] = "1G";
        confDefault_[ConfigPrefix + CfgManager::BackupWaitTime] = boost::lexical_cast<string>(100);
        confDefault_[ConfigPrefix + CfgManager::TapeAuditorRunAt] = boost::lexical_cast<string>(60*10);
        confDefault_[ConfigPrefix + CfgManager::TapeAuditorMaxnum] = boost::lexical_cast<string>(1);
        confDefault_[ConfigPrefix + CfgManager::TapeAuditorInterval] = boost::lexical_cast<string>(3);
        confDefault_[ConfigPrefix + CfgManager::TapeAuditorEnable] = boost::lexical_cast<string>(true);
	}

	CfgManager::~CfgManager()
	{
		// TODO Auto-generated destructor stub
	}

	bool
	CfgManager::Get(const string& confitem, string& value)
	{
		bool bRet = false;
		boost::shared_lock<boost::shared_mutex> lock(rwMutex_);
		ConfManager conf(COMM_SWIFT_OBJECT_SERVER_CONF);
		bRet = conf.GetString(confitem, value);
		if(!bRet){
			CfgParser parser(CFG_FILE.c_str());
			bRet = parser.GetChildValue(confitem, value);
		}
		if(!bRet && confDefault_.find(confitem) != confDefault_.end()){
			bRet = true;
			value = confDefault_[confitem];
		}
		return bRet;
	}


	bool
	CfgManager::ParserSize(string& str, size_t& size)
	{
		try
		{
			size = 0;

			if(str.length() > 1)
			{
				string strUnit = str.substr(str.length()-1);
				string strSize = str.substr(0, str.length()-1);

				if(strUnit == "K" || strUnit == "k")
				{
					size = (size_t)boost::lexical_cast<double>(strSize)*0x400;
				}
				else if(strUnit == "M" || strUnit == "m")
				{
						size = (size_t)boost::lexical_cast<double>(strSize)*0x100000;
				}
				else if(strUnit == "G" || strUnit == "g")
				{
						size = (size_t)boost::lexical_cast<double>(strSize)*0x40000000;
				}
				else if(strUnit == "T" || strUnit == "t")
				{
						size = (size_t)boost::lexical_cast<double>(strSize)*0x10000000000;
				}
				else
					size = (size_t)boost::lexical_cast<double>(str);
			}
			else if(str.length()==1)
			{
				size = (size_t)boost::lexical_cast<double>(str);
			}
			return true;
		}catch(std::exception& e)
		{
			CfgError("ParserSize exception occured!  " << e.what());
		}
		return false;
	}

	bool
	CfgManager::GetUInt64(const string& confitem, UInt64_t& value)
	{
		string str;
		size_t tmpValue = 0;
		if(Get(confitem, str) && ParserSize(str, tmpValue)){
			value = (UInt64_t)tmpValue;
			return true;
		}
		return false;
	}

	bool
	CfgManager::GetUInt32(const string& confitem, UInt32_t& value)
	{
		string str;
		size_t tmpValue = 0;
		if(Get(confitem, str) && ParserSize(str, tmpValue)){
			value = (UInt32_t)tmpValue;
			return true;
		}
		return false;
	}

	bool
	CfgManager::GetUInt16(const string& confitem, UInt16_t& value)
	{
		string str;
		size_t tmpValue = 0;
		if(Get(confitem, str) && ParserSize(str, tmpValue)){
			value = (UInt16_t)tmpValue;
			return true;
		}
		return false;
	}

	bool
	CfgManager::GetFloat(const string& confitem, float& value)
	{
		try
		{
			string str;
			if(Get(confitem, str)){
				value = boost::lexical_cast<float>(str);
				return true;
			}
		}catch(std::exception& e){
			CfgError("GetFloat exception occured!  "<<e.what());
		}
		return false;
	}

	bool
	CfgManager::GetBool(const string& confitem, bool& value)
	{
		string str;
		if(Get(confitem, str)){
			std::transform(str.begin(), str.end(), str.begin(), ::tolower);
			value = str=="true"?true:false;
			return true;
		}

		return false;
	}

	bool
	CfgManager::GetSize(const string& confitem, size_t& value)
	{
		string str;
		if(Get(confitem, str) && ParserSize(str, value)){
			return true;
		}
		return false;
	}

	bool
	CfgManager::SetChild(const string& tag, const string& value)
	{
		boost::unique_lock<boost::shared_mutex> lock(rwMutex_);

		CfgSerialization serialization(CFG_FILE.c_str());
		return serialization.SetChild(tag, value);
	}

	bool
	CfgManager::SetString(const string& confitem, const string& value)
	{
		return SetString(confitem, value);
	}

	bool
	CfgManager::SetUInt64(const string& confitem, UInt64_t value) //
	{
		return SetString(confitem, boost::lexical_cast<string>(value));
	}

	bool
	CfgManager::SetUInt32(const string& confitem, UInt32_t value)
	{
		return SetString(confitem, boost::lexical_cast<string>(value));
	}

	bool
	CfgManager::SetUInt16(const string& confitem, UInt16_t value)
	{
		return SetString(confitem, boost::lexical_cast<string>(value));
	}

	bool
	CfgManager::SetFloat(const string& confitem, float value)
	{
		return SetString(confitem, boost::lexical_cast<string>(value));
	}

	bool
	CfgManager::SetBool(const string& confitem, bool value)
	{
		return SetString(confitem, boost::lexical_cast<string>(value));
	}

	bool
	CfgManager::SetSize(const string& confitem, size_t value)
	{
		return SetString(confitem, boost::lexical_cast<string>(value));
	}

	bool
	CfgManager::DeleteNode(const string& parentNodeName, const string& nodeName)
	{
		boost::unique_lock<boost::shared_mutex> lock(rwMutex_);

		CfgSerialization serialization(CFG_FILE.c_str());

		return serialization.DeleteNode(parentNodeName, nodeName);
	}
} /* namespace ltfs_config */
