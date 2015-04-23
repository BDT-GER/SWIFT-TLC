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
 * Configure.h
 *
 *  Created on: Aug 31, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class Configure
    {
    public:
        Configure();

        ~Configure();

        bool
        Refresh(const fs::path & config);

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
        static const string IgnoreWriteByReadCheckTime;
        static const string IgnoreWriteByReadPercent;

        string
        GetValue(const string & name);

        void
        SetValue(const string & name, const string & value);

        long long
        GetValueSize(const string & name);

        bool
        GetValueBool(const string & name);

    private:
        typedef map<string,string> MapType;
        MapType setting_;
    };

}

