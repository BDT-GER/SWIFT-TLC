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
 * CfgManager.h
 *
 *  Created on: Nov 5, 2012
 *      Author: chento
 */

#pragma once

#include "stdafx.h"

namespace ltfs_config
{

	const string CFG_FILE = COMM_APP_CONF_PATH + "/vs_conf.xml";

	class CfgManager
	{
	public:
		static const string WriteCachePercent;
        static const string MetaFreeLeastSize;
        static const string CacheFreeLeastSize;
        static const string ReadCacheFreeSize;
        static const string WriteCacheFreeSize;
        static const string CacheFreeMinSize;
        static const string CacheFreeMaxSize;
        static const string CacheFreeMinPercent;
        static const string CacheFreeMaxPercent;
        static const string CacheFileSizeRead;
        static const string CacheFileSizeBlock;
        static const string CacheWriteMode;
        static const string FileMaxSize;
        static const string TapeIdleTime;
        static const string FileIdleTime;
        static const string DigestMD5Enable;
        static const string DigestSHA1Enable;
        static const string ThrottleInterval;
        static const string ThrottleValve;
        static const string BackupWaitSize;
        static const string BackupWaitTime;
        static const string ThreadBackupSize;
        static const string BackupWaitFile;
        static const string MetaFileSize;
        static const string FilterFilePattern;
        static const string FilterFolderPattern;
        static const string BackupFilePattern;
        static const string BackupFolderPattern;
        static const string ChangerSyncMode;
        static const string CachePurgeWaitTime;
        static const string CachePurgeWaitFile;
        static const string TapeAuditorRunAt;
        static const string TapeAuditorMaxnum;
        static const string TapeAuditorInterval;
        static const string TapeAuditorEnable;
        static const string ReservedDriveForRead;
        static const string DeleteTapeFileTriggerNum;
        static const string DeleteTapeFileTimeDiff;

		static CfgManager*
		Instance()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if(NULL == instance_)
			{
				instance_ = new CfgManager();
			}

			return instance_;
		}

		static void
		Destory()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if( NULL != instance_)
			{
				delete instance_;
				instance_ = NULL;
			}
		}

		bool
		Get(const string& confitem, string& value);

		bool
		GetUInt64(const string& confitem, UInt64_t& value);

		bool
		GetUInt32(const string& confitem, UInt32_t& value);

		bool
		GetUInt16(const string& confitem, UInt16_t& value);

		bool
		GetFloat(const string& confitem, float& value);

		bool
		GetBool(const string& confitem, bool& value);

		bool
		GetSize(const string& confitem, size_t& value);

		bool
		SetChild(const string& tag, const string& value);

		bool
		SetString(const string& confitem, const string& value);

		bool
		SetUInt64(const string& confitem, UInt64_t value);

		bool
		SetUInt32(const string& confitem, UInt32_t value);

		bool
		SetUInt16(const string& confitem, UInt16_t value);

		bool
		SetFloat(const string& confitem, float value);

		bool
		SetBool(const string& confitem, bool value);

		bool
		SetSize(const string& confitem, size_t value);//confitem:VSConf.xxxx

		bool
		DeleteNode(const string& parentNodeName, const string& nodeName);//node name:xxxx
	private:
		CfgManager();
		virtual
		~CfgManager();
		bool ParserSize(string& str, size_t& size);

	private:
		boost::shared_mutex 			rwMutex_;

		static boost::mutex 			instanceMutex_;
		static CfgManager * 			instance_;
		map<string, string>				confDefault_;
	};

} /* namespace ltfs_config */

