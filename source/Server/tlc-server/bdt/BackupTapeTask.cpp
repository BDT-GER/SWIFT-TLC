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
 * BackupTapeTask.cpp
 *
 *  Created on: Aug 29, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "BackupTapeTask.h"
#include "CacheManager.h"
#include "FileOperationTape.h"
#include "FileMetaParser.h"
#include "../ltfs_management/TapeDbManager.h"
#include "../lib/common/Common.h"

const string DISK_CACHE_FOLDER = COMM_DATA_CACHE_PATH + "/";
const string META_CACHE_FOLDER = COMM_META_CACHE_PATH + "/";

using namespace ltfs_management;
namespace bdt
{
    BackupTapeTask::BackupTapeTask()
    : tape_(Factory::GetTapeManager()),
      cache_(Factory::GetCacheManager()),
      schedule_(Factory::GetSchedule()),
      meta_(Factory::GetMetaManager())
    {
        uuid_ = Factory::GetService();
        catalogDb_.reset(CatalogDbManager::Instance());
        tapes_.clear();
        files_.clear();
        threads_.reset(new boost::thread_group());
    }


    BackupTapeTask::~BackupTapeTask()
    {
        if ( threads_.get() ) {
            threads_->interrupt_all();
            threads_->join_all();
        }
    }

    void BackupTapeTask::SetBackupTapes(const vector<string>& bkTapes, bool bRunning)
    {
        boost::unique_lock<boost::mutex> lock(tapesMutex_);
        for(unsigned int i = 0; i < bkTapes.size(); i++){
            tapes_[bkTapes[i]] = bRunning;
        }
    }

    int BackupTapeTask::GetRunningNum()
    {
        boost::unique_lock<boost::mutex> lock(tapesMutex_);
        int runningNum = 0;
        for(map<string, bool>::iterator it = tapes_.begin(); it != tapes_.end(); it++){
            if(it->second){
                runningNum++;
            }
        }
        return runningNum;
    }

    bool BackupTapeTask::IsTapesRunning(const vector<string>& tapes)
    {
        boost::unique_lock<boost::mutex> lock(tapesMutex_);
        for(unsigned int i = 0; i < tapes.size(); i++){
            map<string, bool>::iterator it = tapes_.find(tapes[i]);
            if ( it != tapes_.end() ) {
                if ( it->second ) {
                    return true;
                }
            }
        }
        return false;
    }

    bool BackupTapeTask::GetFileItemToBackup(vector<BackupItem>& fileItems, off_t& maxSize, off_t maxFileSize, unsigned long maxFileNum)
    {
        Configure * config = Factory::GetConfigure();
        int multipleWaitTime = config->GetValueSize(Configure::BackupMultipleWaitTime);

        fileItems.clear();
        boost::unique_lock<boost::mutex> lock(filesMutex_);
        off_t curSize = 0;
        if(maxFileSize > maxSize){
            maxFileSize = maxSize;
        }
        unsigned long curNum = 0;
        for(map<unsigned long long, TapeBackupItem>::iterator it = files_.begin(); it != files_.end(); it++){
            TapeBackupItem item = it->second;
            LogDebug("GetFileItemToBackup query: number: " << item.item.number << " file: " << item.item.path.string());
            if(item.running == true){
                LogDebug("GetFileItemToBackup running: number: " << item.item.number << " file: " << item.item.path.string());
                continue;
            }
            if ( item.numberMultiple > 0 ) {
                MultipleFile multiple = multipleFiles_[item.manifest];
                LogDebug("GetFileItemToBackup multiple: " << item.manifest << " number: " << item.item.number << " file: " << item.item.path.string());
                if ( ! multiple.IsFull() ) {
                    boost::posix_time::ptime current = boost::posix_time::second_clock::local_time();
                    int elapse = (current - multiple.time).total_seconds();
                    if (elapse < multipleWaitTime) {
                        LogDebug("GetFileItemToBackup ignore multiple: " << item.manifest);
                        continue;
                    } else {
                        LogDebug("GetFileItemToBackup reach time multiple: " << item.manifest);
                    }
                } else {
                    LogDebug("GetFileItemToBackup full multiple: " << item.manifest);
                }
                bool ready = true;
                for ( int i = 0; i < item.numberMultiple; ++i ) {
                    if ( multiple.numbers[i] < 0 ) {
                        LogDebug("GetFileItemToBackup multiple: " << item.manifest << " invalid sequence: " << i);
                        continue;
                    }
                    map<unsigned long long, TapeBackupItem>::iterator iter = files_.find(multiple.numbers[i]);
                    if ( iter == files_.end() ) {
                        LogDebug("GetFileItemToBackup multiple: " << item.manifest << " not find: " << multiple.numbers[i]);
                        continue;
                    }
                    if ( iter->second.running == true ) {
                        LogDebug("GetFileItemToBackup multiple: " << item.manifest << " running: " << multiple.numbers[i]);
                        continue;
                    }
                    if( (curSize + iter->second.item.size <= maxFileSize && curNum + 1 <= maxFileNum) || (curNum == 0 && iter->second.item.size <= maxSize)){
                        LogDebug("GetFileItemToBackup multiple: " << item.manifest << " add number: " << iter->first << " file: " << iter->second.item.path);
                        multiple.time = boost::posix_time::second_clock::local_time();
                        curNum++;
                        curSize += iter->second.item.size;
                        iter->second.running = true;
                        fileItems.push_back(iter->second.item);
                        maxSize -= iter->second.item.size;
                        continue;
                    } else {
                        LogDebug("GetFileItemToBackup multiple: " << item.manifest << " size full");
                        ready = false;
                        break;
                    }
                }
                if ( ! ready ) {
                    if ( curNum > 0 ) {
                        break;
                    } else {
                        continue;
                    }
                }
            }
            LogDebug("GetFileItemToBackup: curSize + item.item.size = " << curSize + item.item.size << ", maxFileSize = " << maxFileSize
                    << ", curNum = " << curNum << ", maxFileNum = " << maxFileNum);
            if( (curSize + item.item.size <= maxFileSize && curNum + 1 <= maxFileNum) || (curNum == 0 && item.item.size <= maxSize)){
                curNum++;
                curSize += item.item.size;
                it->second.running = true;
                fileItems.push_back(item.item);
                maxSize -= item.item.size;
                continue;
            }
            if(curNum > 0){
                break;
            }
        }
        LogDebug("GetFileItemToBackup: fileItems.size() = " << fileItems.size());
        if(fileItems.size() > 0L){
            for(int i=0; i<fileItems.size(); ++i) {
                LogDebug("GetFileItemToBackup " << i << ": "  << fileItems[i].number);
            }
            return true;
        }
        return false;
    }

    void
    BackupTapeTask::InsertTapeBackupItem(const BackupItem & item)
    {
        LogDebug("Insert " << item.number << " " << item.path);

        if(files_.find(item.number) != files_.end()){
            return;
        }

        TapeBackupItem tapeBackupItem;
        tapeBackupItem.item = item;
        tapeBackupItem.running = false;

        if ( ! parser_.ParseSwiftMeta(item.path) ) {
            tapeBackupItem.isMultiple = false;
            tapeBackupItem.manifest = "";
            tapeBackupItem.totalMultiple = 1;
            tapeBackupItem.numberMultiple = -1;
            files_[item.number] = tapeBackupItem;
            return;
        }

        tapeBackupItem.isMultiple = parser_.IsMultiple();
        tapeBackupItem.manifest = parser_.GetManifest();
        tapeBackupItem.totalMultiple = parser_.GetTotal();
        tapeBackupItem.numberMultiple = parser_.GetNumber();
        LogDebug(item.number << " " << item.path << " " << tapeBackupItem.numberMultiple);

        files_[item.number] = tapeBackupItem;
        if ( ! tapeBackupItem.isMultiple ) {
            return;
        }

        string manifest = tapeBackupItem.manifest;
        int number = tapeBackupItem.numberMultiple;
        int total = tapeBackupItem.totalMultiple;
        multipleFiles_[manifest].Resize(total + 1);
        if ( number >= 0 ) {
            multipleFiles_[manifest].numbers[number] = item.number;
            multipleFiles_[manifest].exists[number] = true;
        } else {
            multipleFiles_[manifest].numbers[total] = item.number;
            multipleFiles_[manifest].exists[total] = true;
        }
    }

    void
    BackupTapeTask::DeleteTapeBackupItem(const BackupItem & item)
    {
        LogDebug("Delete " << item.number << " " << item.path);

        if(files_.find(item.number) == files_.end()){
            return;
        }

        TapeBackupItem tapeBackupItem = files_[item.number];
        string manifest = tapeBackupItem.manifest;
        if( manifest == "" ) {
            files_.erase(item.number);
            return;
        }
        int number = tapeBackupItem.numberMultiple;
        int total = tapeBackupItem.totalMultiple;
        if ( number >= 0 ) {
            multipleFiles_[manifest].exists[number] = false;
        } else {
            multipleFiles_[manifest].exists[total] = false;
        }
        bool erase = true;
        for ( int i = 0; i <= total; ++ i ) {
            if ( multipleFiles_[manifest].exists[i] == true ) {
                erase = false;
                break;
            }
        }
        if ( erase ) {
            multipleFiles_.erase(manifest);
        }

        files_.erase(item.number);
    }

    bool
    SortBackupItem(const BackupItem& a, const BackupItem& b)
    {
        return (a.time < b.time);
    }

    void
    BackupTapeTask::Start ()
    {
        if ( NULL == cache_ || NULL == tape_ || NULL == schedule_) {
            return;
        }

        // get configurations
        Configure * config = Factory::GetConfigure();
        off_t waitSize = config->GetValueSize(Configure::BackupWaitSize);
        int waitTime = config->GetValueSize(Configure::BackupWaitTime);
        long long threadSize = config->GetValueSize(Configure::ThreadBackupSize);
        int reservedDrive = config->GetValueSize(Configure::ReservedDriveForRead);
        int chkPercent = int(config->GetValueSize(Configure::IgnoreWriteByReadPercent));
        LogInfo("waitSize = " << waitSize << ", waitTime = " << waitTime << ", threadSize = " << threadSize << ", reservedDrive = " << reservedDrive);
        if(threadSize <= 0){
            threadSize = 1024*1024*1024*4L;
        }

        run_ = true;

        try {
            int backupInterval = 1;
            boost::posix_time::ptime backupTime =
                    boost::posix_time::second_clock::local_time();
            int runningNum = GetRunningNum();
            int driveNum = 0; // available drive number
            if(!tape_->GetDriveNum(driveNum)){
                LogError("Failed to get drive num.");
                driveNum = 1;
            }
            int availableNum = driveNum;// - reservedDrive;
            if(availableNum <= 0){
                availableNum = 1;
            }
            time_t lastCheck = time(NULL);

            while(true) {
                boost::this_thread::sleep( boost::posix_time::seconds(
                        backupInterval ) );
                backupInterval = 1;
                bool backup = false;

                if(time(NULL) - lastCheck > 60*1){
                    lastCheck = time(NULL);
                    if(!tape_->GetDriveNum(driveNum)){
                        LogError("Failed to get drive num.");
                        driveNum = 1;
                    }
                    availableNum = driveNum;// - reservedDrive;
                    if(availableNum <= 0){
                        availableNum = 1;
                    }
                }
                runningNum = GetRunningNum();
                LogDebug("runningNum = " << runningNum << ", driveNum = " << driveNum << ", availableNum = " << availableNum);
                if(runningNum >= availableNum){
                    LogDebug("Previous backup threads running, no start new ones.");
                    boost::this_thread::sleep( boost::posix_time::seconds(10));
                    continue;
                }

                // check backup wait time
                if ( ! backup ) {
                    boost::posix_time::ptime current =
                            boost::posix_time::second_clock::local_time();
                    if ((current - backupTime).total_seconds() >= waitTime) {
                        LogDebug("Backup reaches the wait time");
                        backup = true;
                    }
                }

                off_t allTotalSize = 0;
                {
                    // check backup wait size
                    vector<BackupItem> list;
                    boost::unique_lock<boost::mutex> lock(filesMutex_);
                    meta_->GetBackupList(list);
                    sort(list.begin(), list.end(), SortBackupItem);

                    for(unsigned int i = 0; i < list.size(); i++){
                        allTotalSize += list[i].size;
                    }
                    LogDebug("list.size() = " << list.size() << ", allTotalSize = " << allTotalSize);
                    if ( list.size() > 0 && allTotalSize >= waitSize ) {
                        LogDebug("Backup reaches the wait size");
                        backup = true;
                    }
                    if(list.size() <= 0){
                        LogDebug("No files to backup");
                        backup = false;
                    }
                    if ( ! backup ) {
                        backupInterval = config->GetValueSize(
                                Configure::TapeIdleTime );
                        continue;
                    }
                    for(unsigned i = 0; i < list.size(); i++){
                        InsertTapeBackupItem(list[i]);
                    }
                }

                vector<map<string, off_t> > tapesList;
                TapeDbManager::Instance()->GetShareAvailableTapes(Factory::GetService(), tapesList);
                if (tapesList.size() <= 0) {
                    LogError("No tapes to backup files for share");
                    continue;
                }

                backupTime = boost::posix_time::second_clock::local_time();
                int maxThreadNum = (allTotalSize / threadSize) + 1;
                if(allTotalSize % threadSize == 0 && maxThreadNum > 1){
                    maxThreadNum--;
                }
                LogDebug("runningNum = " << runningNum << ", driveNum = " << driveNum << ", availableNum = " \
                        << availableNum << ", reservedDrive = " << reservedDrive << ", maxThreadNum = " << maxThreadNum << "tapesList.size() = " << tapesList.size());
                vector<PendingTapes>  pendingTapes;
                int nStarted = 0;
                for(vector<map<string, off_t> >::iterator itTape = tapesList.begin(); itTape != tapesList.end() && runningNum < availableNum && runningNum < maxThreadNum; itTape++){
                    off_t maxSize = 0;
                    vector<string> bkTapes;
                    bool bIgnore = false;
                    bool bPending = false;
                    for(map<string, off_t>::iterator itMap = itTape->begin(); itMap != itTape->end(); itMap++){
                        int act = ACT_IDLE;
                        int readPercent = 0;
                        if(tape_->GetTapeActivity(itMap->first, act, readPercent)){
                            // ignore diagnosing and auditing taps
                            if(act == ACT_AUDITING || act == ACT_DIAGNOSING){
                                if(availableNum > 1 || runningNum > 0){
                                    availableNum--;
                                }
                                bIgnore = true;
                                break;
                            }
                            LogDebug("runningNum = " << runningNum << ". RecentReadActPercentage = " << readPercent << ", tape = " << itMap->first);
                            // if the tape is busy reading, does not use this tape
                            if(readPercent > chkPercent){
                                LogDebug("runningNum = " << runningNum << ". RecentReadActPercentage = " << readPercent << ", tape = " << itMap->first);
                                bPending = true;
                                break;
                            }
                        }else{
                            LogError("Failed to get info of the tape " << itMap->first);
                        }
                        if(maxSize == 0 || maxSize > itMap->second){
                            maxSize = itMap->second;
                        }
                        bkTapes.push_back(itMap->first);
                    }
                    LogDebug("maxSize=" << maxSize);
                    if(bIgnore || maxSize <= 0L){
                        continue;
                    }
                    if(IsTapesRunning(bkTapes)){
                        LogDebug("IsTapesRunning: " << boost::join(bkTapes, ","));
                        continue;
                    }
                    if(bPending){
                        struct PendingTapes tapes;
                        tapes.bkTapes = bkTapes;
                        tapes.maxSize = maxSize;
                        pendingTapes.push_back(tapes);
                        continue;
                    }
                    StartBackupSub(bkTapes, maxSize);
                    sleep(1);
                    nStarted++;
                    runningNum = GetRunningNum();
                    LogDebug("#####runningNum = " << runningNum << ", driveNum = " << driveNum << ", availableNum = " << availableNum << ", reservedDrive = " << reservedDrive);
                }//for
                if(nStarted <= 0 && pendingTapes.size() > 0){
                    StartBackupSub(pendingTapes[0].bkTapes, pendingTapes[0].maxSize);
                }
                runningNum = GetRunningNum();
                LogDebug("END: runningNum = " << runningNum << ", driveNum = " << driveNum << ", availableNum = " << availableNum << ", reservedDrive = " << reservedDrive);

                int retry = 60 * 5;
                while(runningNum > 0 && retry-- > 0){
                    sleep(1);
                    runningNum = GetRunningNum();
                }
            }//while
        } catch ( const boost::thread_interrupted & e ) {
        }

        run_ = false;
    }


    void
    BackupTapeTask::StartBackupSub(const vector<string>& bkTapes, off_t maxSize)
    {
        threads_->create_thread(
                boost::bind(
                    &BackupTapeTask::HandleBackupSub,
                    this,
                    bkTapes,
                    maxSize) );
    }


    void
    BackupTapeTask::HandleBackupSub(const vector<string>& bkTapes, off_t maxSize)
    {
        try {
            SetBackupTapes(bkTapes, true);
            while(maxSize > 0L){
                if ( boost::this_thread::interruption_requested() ) {
                    LogWarn("Interrupt backup");
                    SetBackupTapes(bkTapes, false);
                    return;
                }
                bool ret = true;
                vector<BackupItem> fileItems;
                if(false == GetFileItemToBackup(fileItems, maxSize)){
                    LogDebug("Failed to get files to backup. maxSize = " << maxSize);
                    break;
                }
                try {
                    ret = Backup(fileItems, bkTapes);
                } catch (const std::exception & e) {
                    LogError(e.what());
                    ret = false;
                }
                if ( ! ret ) {
                    boost::this_thread::sleep(boost::posix_time::seconds(1));
                }
            }
        } catch ( const boost::thread_interrupted & e ) {
            LogWarn("Interrupt backup");
        }
        SetBackupTapes(bkTapes, false);
    }


    bool
    BackupTapeTask::Backup(const vector<BackupItem> &items, const vector<string>& tapes)
    {
        int retryTimes = 0;

BackupRetry:
        LogDebug("retryTimes: " << retryTimes);
        if(retryTimes++ > 5){
            LogError("Failed to request tape to backup files.");
            return false;
        }

        if ( boost::this_thread::interruption_requested() ) {
            return false;
        }

        string stringTapes =  boost::join(tapes,",");
        TapeStateSwitch stateSwitch(tape_,TapeManagerInterface::STATE_IDLE);
        BOOST_FOREACH( const string & tape, tapes ) {
            if ( ! tape_->CheckTapeState(
                    tape, TapeManagerInterface::STATE_WRITE ) ) {
                LogWarn(tape);
                boost::this_thread::sleep(boost::posix_time::seconds(1));
                goto BackupRetry;
            }
            if ( tape_->SetTapeState(
                    tape, TapeManagerInterface::STATE_WRITE ) ) {
                stateSwitch.AddTape(tape);
            } else {
                LogWarn(tape);
                boost::this_thread::sleep(boost::posix_time::seconds(1));
                goto BackupRetry;
            }
        }

        LogDebug("############# start request tape " << boost::join(tapes, ","));
        ScheduleRelease release(schedule_,false);
        if ( schedule_->RequestTapes( tapes, true,
                false, 300000, ScheduleInterface::PRIORITY_WRITE ) ) {
            release.AddTapes(tapes);
        } else {
            LogWarn(stringTapes);
            boost::this_thread::sleep(boost::posix_time::seconds(10));
            goto BackupRetry;
        }
        LogDebug("############# end request tape " << boost::join(tapes, ","));

        unsigned long fileNum = 0;
        off_t sizeFileTotal = 0;
        off_t sizeOnTape = 0;
        map<string, vector<TapeFileInfo> > fileInfoMap;
        for(unsigned int i = 0; i < items.size(); i++){
            bool bRet = true;
            BackupItem item = items[i];
            fs::path pathRelative = item.path;
            string pathSrc = COMM_META_CACHE_PATH + "/" + uuid_ + pathRelative.string();
            auto_ptr<ExtendedAttribute> eaMerge(new ExtendedAttribute(pathSrc));
            bool bWrittenToTape = false;
            BOOST_FOREACH( const string & tape, tapes ) {
                string dstPath = "";
                if(!catalogDb_->GetPathForBackup(boost::lexical_cast<string>(item.number), dstPath)){
                    LogError("Failed to get backup path for file " << pathRelative.string());
                    bRet = false;
                    break;
                }
                dstPath = COMM_MOUNT_PATH + "/" + tape + "/" + dstPath;
                LogDebug("dstPath: " << dstPath);
                fs::path pathDst = dstPath;
                if ( fs::exists(pathDst) ) {
                    try {
                        if ( fs::is_directory(pathDst) ) {
                            fs::remove_all(pathDst);
                        } else {
                            LogError(pathDst);
                            fs::remove(pathDst);
                        }
                    } catch (const std::exception & e) {
                        LogError(pathDst << " " << e.what());
                    }
                }
                if ( fs::exists(pathDst) ) {
                    LogError(pathDst);
                    bRet = false;
                    break;
                }

                LogDebug("pathRelative: " << pathRelative.string() << ", pathDst: " << pathDst.string() << ", tape = " << tape);
                auto_ptr<FileOperationInterface> target;
                try{
                    mode_t mode = 0644;
                    fs::path pFolder = pathDst.parent_path();
                    string cmd = "mkdir -p " + pFolder.string();
                    std::system(cmd.c_str());
                    target.reset(new FileOperationTape(pathDst, mode, O_RDWR, tape));
                }catch(...){
                    LogError("Exception to create tape file: " << pathDst.string() << ", pathRelative: " << pathRelative.string());
                    bRet = false;
                    break;
                }
                fs::path pathNew = pathRelative;
                bool writeTape = false;
                if(!meta_->Backup(pathRelative, target.get(), tape, pathNew, writeTape)){
                    LogWarn("Failed to backup file " << pathRelative.string() << " to tape " << tape << ". Dst path: " << pathDst.string());
                    if (fs::exists(pathDst)) {
                        fs::remove(pathDst);
                    }
                    bRet = false;
                    if(writeTape == true){
                        bWrittenToTape = true;
                    }
                    break;
                }else{
                    LogDebug("Finished backup file " << pathRelative.string() << " to tape" << tape << ". Dst path: " << pathDst.string());
                    auto_ptr<Inode> inode;
                    inode.reset(meta_->GetInode(pathRelative));
                    off_t offset = 0;
                    auto_ptr<ExtendedAttribute> ea(new ExtendedAttribute(pathDst));
                    char startblock[1024];
                    memset(startblock,0,sizeof(startblock));
                    int valuesize;
                    if ( ea->GetValue( "user.ltfs.startblock", startblock, sizeof(startblock), valuesize ) ) {
                        startblock[valuesize] = '\0';
                        offset = boost::lexical_cast<off_t>(string(startblock));
                    }
                    if(!eaMerge->MergeAttrTo(pathDst)){
                        LogError("Failed to merge Extended Attributes from file " << pathSrc << " to " << pathDst.string());
                    }
                    inode->SetBackup(1);
                    TapeFileInfo fInfo;
                    fInfo.mUuid = boost::lexical_cast<string>(item.number);
                    fInfo.mMetaFilePath = pathNew.string();
                    fInfo.mOffset = offset;
                    fInfo.mSize = item.size;
                    if(fileInfoMap.find(tape) == fileInfoMap.end()){
                        vector<TapeFileInfo> info;
                        fileInfoMap[tape] = info;
                    }
                    fileInfoMap[tape].push_back(fInfo);
                }//if
            }//BOOST_FOREACH
            if(bRet == true){
                fileNum++;
                sizeFileTotal += item.size;
                sizeOnTape += item.size;
                boost::unique_lock<boost::mutex> lock(filesMutex_);
                DeleteTapeBackupItem(item);
            }else{
                if (bWrittenToTape){
                    sizeOnTape += item.size;
                }
                boost::unique_lock<boost::mutex> lock(filesMutex_);
                files_[item.number].running = false;
            }
        }//for
        if ( ! tape_->SetTapesUse(tapes, fileNum, sizeFileTotal, sizeOnTape) ) {
            LogWarn(stringTapes);
        }
        if(!catalogDb_->AddTapeFiles(uuid_, fileInfoMap)){
            LogError("Failed to add files to database.");
        }

        return true;
    }

}
