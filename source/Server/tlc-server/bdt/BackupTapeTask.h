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
 * BackupTapeTask.h
 *
 *  Created on: Aug 29, 2012
 *      Author: More Zeng
 */


#pragma once
#include "MetaManager.h"
#include "FileMetaParser.h"
#include "../ltfs_management/CatalogDbManager.h"
using namespace ltfs_management;


namespace bdt
{
	struct TapeBackupItem
	{
		BackupItem 	item;
		bool		running;
        bool        isMultiple;
        string      manifest;
        int         totalMultiple;
        int         numberMultiple;
	};

    struct PendingTapes
    {
    	vector<string> bkTapes;
    	off_t		maxSize;
    };

    struct MultipleFile
    {
        boost::posix_time::ptime time;
        int total;
        vector<long long> numbers;
        vector<bool> exists;

        MultipleFile()
        : total(0)
        {
            time = boost::posix_time::second_clock::local_time();
        }

        void
        Resize(int total_)
        {
            if ( total == total_ ) {
                return;
            }

            total = total_;
            numbers.resize(total);
            exists.resize(total);
            for (int i=0; i<total; ++i) {
                numbers[i] = -1;
                exists[i] = false;
            }
        }

        bool
        IsFull()
        {
            for (int i=0; i<total; ++i) {
                if (numbers[i] < 0) {
                    return false;
                }
            }
            return true;
        }
    };

    class BackupTapeTask : public BackendTask
    {
    public:
        BackupTapeTask();

        virtual
        ~BackupTapeTask();

        virtual void
        Start();

    private:
        void StartBackupSub(const vector<string>& bkTapes, off_t maxSize);
        void HandleBackupSub(const vector<string>& bkTapes, off_t maxSize);
        bool Backup(const vector<BackupItem> &items, const vector<string>& tapes);
        void SetBackupTapes(const vector<string>& bkTapes, bool bRunning);
        int GetRunningNum();
        bool IsTapesRunning(const vector<string>& tapes);
        bool GetFileItemToBackup(vector<BackupItem>& fileItems, off_t& maxSize, off_t maxFileSize = 100*1024*1024, unsigned long maxFileNum = 1000);
        void InsertTapeBackupItem(const BackupItem & item);
        void DeleteTapeBackupItem(const BackupItem & item);

        auto_ptr<boost::thread_group> threads_;
        TapeManagerInterface * tape_;
        CacheManager * cache_;
        ScheduleInterface * schedule_;
        MetaManager * meta_;
        map<string, bool>	tapes_;
        boost::mutex 		tapesMutex_;
        map<unsigned long long, TapeBackupItem>	files_;
        map<string, MultipleFile> multipleFiles_;
        boost::mutex 		filesMutex_;
        string				uuid_;
        auto_ptr<CatalogDbManager> catalogDb_;
        FileMetaParser      parser_;
    };

}

