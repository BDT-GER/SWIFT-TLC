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
 * Configure.cpp
 *
 *  Created on: Aug 31, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#ifdef MORE_TEST
#else
typedef unsigned long long UInt64_t;
typedef unsigned long UInt32_t;
typedef unsigned short UInt16_t;
#include "../lib/config/CfgManager.h"
#endif


namespace bdt
{

    const string Configure::WriteCachePercent("WriteCachePercent");
    const string Configure::MetaFreeLeastSize("MetaFreeLeastSize");
    const string Configure::CacheFreeLeastSize("CacheFreeLeastSize");
    const string Configure::ReadCacheFreeSize("ReadCacheFreeSize");
    const string Configure::WriteCacheFreeSize("WriteCacheFreeSize");
    const string Configure::CacheFreeMinSize("CacheFreeMinSize");
    const string Configure::CacheFreeMaxSize("CacheFreeMaxSize");
    const string Configure::CacheFreeMinPercent("CacheFreeMinPercent");
    const string Configure::CacheFreeMaxPercent("CacheFreeMaxPercent");
    const string Configure::CacheFileSizeRead("CacheFileSizeRead");
    const string Configure::CacheFileSizeBlock("CacheFileSizeBlock");
    const string Configure::CacheWriteMode("CacheWriteMode");
    const string Configure::FileMaxSize("FileMaxSize");
    const string Configure::TapeIdleTime("TapeIdleTime");
    const string Configure::FileIdleTime("FileIdleTime");
    const string Configure::DigestMD5Enable("DigestMD5Enable");
    const string Configure::DigestSHA1Enable("DigestSHA1Enable");
    const string Configure::ThrottleInterval("ThrottleInterval");
    const string Configure::ThrottleValve("ThrottleValve");
    const string Configure::BackupWaitSize("WriteToTapeWaitSize");
    const string Configure::BackupWaitTime("WriteToTapeWaitTime");
    const string Configure::BackupWaitFile("WriteToTapeWaitFile");
    const string Configure::ThreadBackupSize("ThreadWriteToTapeSize");
    const string Configure::MetaFileSize("MetaFileSize");
    const string Configure::FilterFilePattern("FilterFilePattern");
    const string Configure::FilterFolderPattern("FilterFolderPattern");
    const string Configure::BackupFilePattern("WriteToTapeFilePattern");
    const string Configure::BackupFolderPattern("WriteToTapeFolderPattern");
    const string Configure::ChangerSyncMode("ChangerSyncMode");
    const string Configure::CachePurgeWaitTime("CachePurgeWaitTime");
    const string Configure::CachePurgeWaitFile("CachePurgeWaitFile");
    const string Configure::TapeAuditorRunAt("TapeAuditorRun");
    const string Configure::TapeAuditorMaxnum("TapeAuditorMaxnum");
    const string Configure::TapeAuditorInterval("TapeAuditorInterval");
    const string Configure::TapeAuditorEnable("TapeAuditorEnable");
    const string Configure::ReservedDriveForRead("ReservedDriveForRead");
    const string Configure::DeleteTapeFileTriggerNum("DeleteTapeFileTriggerNum");
    const string Configure::DeleteTapeFileTimeDiff("DeleteTapeFileTimeDiff");
    const string Configure::IgnoreWriteByReadCheckTime("IgnoreWriteByReadCheckTime");
    const string Configure::IgnoreWriteByReadPercent("IgnoreWriteByReadPercent");

    static const unsigned long long defaultMetaFreeLeastSize =
            1LL * 1024 * 1024 * 1024;
    static const unsigned long long defaultCacheFreeLeastSize =
            2LL * 1024 * 1024 * 1024;
    static const unsigned long long defaultReadCacheFreeSize =
            10LL * 1024 * 1024 * 1024;
    static const unsigned long long defaultWriteCacheFreeSize =
            15LL * 1024 * 1024 * 1024;
    static const unsigned long long defaultWriteCachePercent = 90;
    static const unsigned long long defaultCacheFreeMinSize =
            15LL * 1024 * 1024 * 1024;
    static const unsigned long long defaultCacheFreeMaxSize =
            20LL * 1024 * 1024 * 1024;
    static const int defaultCacheFreeMinPercent = 10;
    static const int defaultCacheFreeMaxPercent = 20;
    static const unsigned long long defaultCacheFileSizeRead =
            4LL * 1024 * 1024 * 1024;
    static const int defaultCacheFileSizeBlock = 128 * 1024 * 1024;
    static const int defaultCacheWriteMode = true;
    static const unsigned long long defaultFileMaxSize = 0;
    static const int defaultTapeIdleTime = 10;
    static const int defaultFileIdleTime = 3;
    static const bool defaultDigestMD5Enable = true;
    static const bool defaultDigestSHA1Enable = true;
    static const int defaultThrottleInterval = 1000;
    static const unsigned long long defaultThrottleValve = 256LL * 1024 * 1024;
    static const unsigned long long defaultBackupWaitSize =
            1LL * 1024 * 1024 * 1024;
    static const int defaultBackupWaitTime = 180;
    static const int defaultBackupWaitFile = 60;
    static const unsigned long long defaultThreadBackupSize = 4LL * 1024 * 1024 * 1024;
    static const unsigned long long defaultMetaFileSize = 1LL * 1024 * 1024;
    static const string defaultFilterFilePattern("");
    static const string defaultFilterFolderPattern("");
    static const string defaultBackupFilePattern(".*");
    static const string defaultBackupFolderPattern(".*");
    static const bool defaultChangerSyncMode = false;
    static const int defaultCachePurgeWaitTime = 5;
    static const int defaultCachePurgeWaitFile = 600;
    static const string defaultTapeAuditorRun = "01:01";
    static const int defaultTapeAuditorMaxnum = 2;
    static const int defaultTapeAuditorInterval = 3;
    static const bool defaultTapeAuditorEnable = true;
    static const int defaultReservedDriveForRead = 1;
    static const unsigned long long defaultDeleteTapeFileTriggerNum = 1000;
    static const unsigned long long defaultDeleteTapeFileTimeDiff = 60*60*24;
    static const unsigned long defaultIgnoreWriteByReadCheckTime = 300;
    static const unsigned long defaultIgnoreWriteByReadPercent = 80;


    Configure::Configure()
    {
        setting_.insert( MapType::value_type(
                Configure::WriteCachePercent,
                boost::lexical_cast<string>(defaultWriteCachePercent)));
        setting_.insert( MapType::value_type(
                Configure::MetaFreeLeastSize,
                boost::lexical_cast<string>(defaultMetaFreeLeastSize)));
        setting_.insert( MapType::value_type(
                Configure::CacheFreeLeastSize,
                boost::lexical_cast<string>(defaultCacheFreeLeastSize)));
        setting_.insert( MapType::value_type(
                Configure::ReadCacheFreeSize,
                boost::lexical_cast<string>(defaultReadCacheFreeSize)));
        setting_.insert( MapType::value_type(
                Configure::WriteCacheFreeSize,
                boost::lexical_cast<string>(defaultWriteCacheFreeSize)));
        setting_.insert( MapType::value_type(
                Configure::CacheFreeMinSize,
                boost::lexical_cast<string>(defaultCacheFreeMinSize)));
        setting_.insert( MapType::value_type(
                Configure::CacheFreeMaxSize,
                boost::lexical_cast<string>(defaultCacheFreeMaxSize)));
        setting_.insert( MapType::value_type(
                Configure::CacheFreeMinPercent,
                boost::lexical_cast<string>(defaultCacheFreeMinPercent)));
        setting_.insert( MapType::value_type(
                Configure::CacheFreeMaxPercent,
                boost::lexical_cast<string>(defaultCacheFreeMaxPercent)));
        setting_.insert( MapType::value_type(
                Configure::CacheFileSizeRead,
                boost::lexical_cast<string>(defaultCacheFileSizeRead)));
        setting_.insert( MapType::value_type(
                Configure::CacheFileSizeBlock,
                boost::lexical_cast<string>(defaultCacheFileSizeBlock)));
        setting_.insert( MapType::value_type(
                Configure::CacheWriteMode,
                boost::lexical_cast<string>(defaultCacheWriteMode)));
        setting_.insert( MapType::value_type(
                Configure::FileMaxSize,
                boost::lexical_cast<string>(defaultFileMaxSize)));
        setting_.insert( MapType::value_type(
                Configure::TapeIdleTime,
                boost::lexical_cast<string>(defaultTapeIdleTime)));
        setting_.insert( MapType::value_type(
                Configure::FileIdleTime,
                boost::lexical_cast<string>(defaultFileIdleTime)));
        setting_.insert( MapType::value_type(
                Configure::DigestMD5Enable,
                boost::lexical_cast<string>(defaultDigestMD5Enable)));
        setting_.insert( MapType::value_type(
                Configure::DigestSHA1Enable,
                boost::lexical_cast<string>(defaultDigestSHA1Enable)));
        setting_.insert( MapType::value_type(
                Configure::ThrottleInterval,
                boost::lexical_cast<string>(defaultThrottleInterval)));
        setting_.insert( MapType::value_type(
                Configure::ThrottleValve,
                boost::lexical_cast<string>(defaultThrottleValve)));
        setting_.insert( MapType::value_type(
                Configure::BackupWaitSize,
                boost::lexical_cast<string>(defaultBackupWaitSize)));
        setting_.insert( MapType::value_type(
                Configure::BackupWaitTime,
                boost::lexical_cast<string>(defaultBackupWaitTime)));
        setting_.insert( MapType::value_type(
                Configure::BackupWaitFile,
                boost::lexical_cast<string>(defaultBackupWaitFile)));
        setting_.insert( MapType::value_type(
                Configure::ThreadBackupSize,
                boost::lexical_cast<string>(defaultThreadBackupSize)));
        setting_.insert( MapType::value_type(
                Configure::MetaFileSize,
                boost::lexical_cast<string>(defaultMetaFileSize)));
        setting_.insert( MapType::value_type(
                Configure::FilterFilePattern,
                defaultFilterFilePattern));
        setting_.insert( MapType::value_type(
                Configure::FilterFolderPattern,
                defaultFilterFolderPattern));
        setting_.insert( MapType::value_type(
                Configure::BackupFilePattern,
                defaultBackupFilePattern));
        setting_.insert( MapType::value_type(
                Configure::BackupFolderPattern,
                defaultBackupFolderPattern));
        setting_.insert( MapType::value_type(
                Configure::ChangerSyncMode,
                boost::lexical_cast<string>(defaultChangerSyncMode)));
        setting_.insert( MapType::value_type(
                Configure::CachePurgeWaitTime,
                boost::lexical_cast<string>(defaultCachePurgeWaitTime)));
        setting_.insert( MapType::value_type(
                Configure::CachePurgeWaitFile,
                boost::lexical_cast<string>(defaultCachePurgeWaitFile)));
        setting_.insert( MapType::value_type(
                Configure::TapeAuditorRunAt,
                defaultTapeAuditorRun));
        setting_.insert( MapType::value_type(
                Configure::TapeAuditorMaxnum,
                boost::lexical_cast<string>(defaultTapeAuditorMaxnum)));
        setting_.insert( MapType::value_type(
                Configure::TapeAuditorInterval,
                boost::lexical_cast<string>(defaultTapeAuditorInterval)));
        setting_.insert( MapType::value_type(
                Configure::ReservedDriveForRead,
                boost::lexical_cast<string>(defaultReservedDriveForRead)));
        setting_.insert( MapType::value_type(
                Configure::TapeAuditorEnable,
                boost::lexical_cast<string>(defaultTapeAuditorEnable)));
        setting_.insert( MapType::value_type(
                Configure::DeleteTapeFileTriggerNum,
                boost::lexical_cast<string>(defaultDeleteTapeFileTriggerNum)));
        setting_.insert( MapType::value_type(
                Configure::DeleteTapeFileTimeDiff,
                boost::lexical_cast<string>(defaultDeleteTapeFileTimeDiff)));
        setting_.insert( MapType::value_type(
                Configure::IgnoreWriteByReadCheckTime,
                boost::lexical_cast<string>(defaultIgnoreWriteByReadCheckTime)));
        setting_.insert( MapType::value_type(
                Configure::IgnoreWriteByReadPercent,
                boost::lexical_cast<string>(defaultIgnoreWriteByReadPercent)));
    }


    Configure::~Configure()
    {
    }


    bool
    Configure::Refresh(const fs::path & config)
    {
        bool ret = true;

        if ( fs::is_regular_file(config) ) {
            ifstream input(config.string().c_str());
            string name;
            string separator;
            string value;
            while ( input >> name >> separator >> value ) {
                if ( separator != ":" ) {
                    ret = false;
                    LogError("Invalid configure " << name << " : " << value);
                    continue;
                }
                MapType::iterator i = setting_.find(name);
                if ( i != setting_.end() ) {
                    i->second = value;
                } else {
                    ret = false;
                    LogError("Invalid configure " << name << " : " << value);
                }
            }
        } else {
            ret = false;
        }

        return ret;
    }


    static string ConfigPrefix = "VSConf.";


    string
    Configure::GetValue(const string & name)
    {
        MapType::iterator i = setting_.find(name);
        if ( i == setting_.end() ) {
            LogError(name);
            return "";
        }

#ifdef MORE_TEST
        return i->second;
#else
        string nameConfig = ConfigPrefix + name;
        string value;
        if ( ltfs_config::CfgManager::Instance()->Get(
                nameConfig, value ) ) {
            return value;
        } else {
            LogWarn(name);
            return i->second;
        }
#endif
    }


    void
    Configure::SetValue(const string & name, const string & value)
    {
#ifdef MORE_TEST
        MapType::iterator i = setting_.find(name);
        if ( i == setting_.end() ) {
            LogError(name);
            return;
        }

        i->second = value;
#else
        assert(false);
#endif
    }


    long long
    Configure::GetValueSize(const string & name)
    {
    	LogDebug("GetValueSize: " << name);
        MapType::iterator i = setting_.find(name);
        if ( i == setting_.end() ) {
            LogError(name);
            return 0;
        }

        long long value = 0;
        try {
            value = boost::lexical_cast<long long>(i->second);
        } catch (...) {
            LogError(name << " : " << i->second);
        }

#ifdef MORE_TEST
        return value;
#else
        if ( name == ThrottleInterval || name == ThrottleValve
                || name == BackupWaitFile ) {
            string shareConfPrefix = ConfigPrefix + "ShareRetention."
                    + Factory::GetService() + ".";
            UInt64_t result = 0;
            if ( ltfs_config::CfgManager::Instance()->GetUInt64(
                    shareConfPrefix + name, result ) ) {
                return result;
            } else {
                LogWarn(name);
                return value;
            }
        }

        string nameConfig = ConfigPrefix + name;
        UInt64_t result = 0;
        if ( ltfs_config::CfgManager::Instance()->GetUInt64(
                nameConfig, result ) ) {
            return result;
        } else {
            LogWarn(name);
            return value;
        }
#endif
    }


    bool
    Configure::GetValueBool(const string & name)
    {
        MapType::iterator i = setting_.find(name);
        if ( i == setting_.end() ) {
            LogError(name);
            return false;
        }

        bool value = false;
        try {
            value = boost::lexical_cast<bool>(i->second);
        } catch (...) {
            LogError(name << " : " << i->second);
        }
#ifdef MORE_TEST
        return value;
#else
        string nameConfig = ConfigPrefix + name;
        bool result;
        if ( ltfs_config::CfgManager::Instance()->GetBool(
                nameConfig, result ) ) {
            return result;
        } else {
            LogWarn(name);
            return value;
        }
#endif
    }

}

