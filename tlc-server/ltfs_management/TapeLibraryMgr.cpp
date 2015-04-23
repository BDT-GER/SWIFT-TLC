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
 * File:         TapeLibraryMgr.h
 * Description:
 * Author:       Sam Chen
 * Created:      Jul 30, 2012
 *
 */


#include "TapeLibraryMgr.h"
#include "../bdt/Factory.h"
#include "../bdt/SchedulePriorityTape.h"
#include "../bdt/ServiceServer.h"
#include "../bdt/ExtendedAttribute.h"
#include "../bdt/ScheduleProxyServer.h"
#include <sys/vfs.h>
#include <xmlrpc-c/client.hpp>
#include <xmlrpc-c/client_transport.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_pstream.hpp>
#include "../lib/config/CfgManager.h"
#include "../ltfs_format/ltfsFormatDetails.h"
#include "../socket/ltfsTaskManagement.h"
#include "../socket/ltfsLibConvertor.h"

#include "../lib/ltfs_library/CmnFunc.h"
#include "../lib/ltfs_library/LtfsLibraries.h"
#include "../ltfs_format/ltfsFormatManager.h"
#include "TapeDbManager.h"
#include "TapeStateMachineMgr.h"
#include "TapeSchedulerMgr.h"

#define INVENTORY_LOAD_TAPE_REQUEST_WAIT	360  // time out for requesting resource to load tape for inventory
using namespace ltfs_management;
using namespace ltfs_soapserver;
using namespace bdt;
#define SOAP_OK				0

namespace ltfs_management
{
    TapeLibraryMgr * TapeLibraryMgr::instance_ = NULL;
    boost::mutex TapeLibraryMgr::mutexInstance_;
    boost::mutex TapeLibraryMgr::mutexRefresh_;

    static const long long SizeTapeFree = 8LL * 1024 * 1024 * 1024;
    static const long long SizeTapeBuffer = 2LL * 1024 * 1024 * 1024;
    static const long long SizeTapeMinBuffer = 512LL * 1024 * 1024;

    const string SYSTEM_DIRTY_SHUTDOWN_FLAG = "/var/tmp/.dirty_shutdown";
    const string TAPE_IN_DRIVE_FLAG_FOLDER	= "/var/tmp/.in_drive_tapes";
    const string DEFAULT_SYSTEM_DIAGNOSE_LOG_PREFIX = COMM_VS_LOG_PATH + "/system_diagnose_";
    const fs::path META_ROOT = COMM_META_CACHE_PATH + "/";
    const string DISK_CACHE_ROOT = COMM_DATA_CACHE_PATH;
    const string FATTR_FIELD_OFFSET = "user.vsvfs.offset";

    const int INV_DRIVE_CLEAN_THROTTLE = 600; // 10 minutes
    static map<string, time_t> DriveCleaningThrottleMap;

    // Throttle between two events for disck cache free size too small
    const time_t CHECK_CACHE_USAGE_THROTTLE = 10 * 60; //10 minutes

	int ExeSystem(const string& cmd)
	{
    	VS_DBG_LOG_FUNCTION;
		LtfsLogDebug("TapeLibraryMgr::ExeSystem: cmd = " << cmd << ".");
		return std::system(cmd.c_str());
	}

	string GetIpHostName()
	{
    	VS_DBG_LOG_FUNCTION;
	    vector<string> outputs = GetCommandOutputLines("hostname 2>/dev/null");
	    for(unsigned int i = 0; i < outputs.size(); i++){
			//ltfs-suse11-sp2
			regex matchHostName("^\\s*(.*)\\s*$");
			cmatch match;
			if(regex_match(outputs[i].c_str(), match, matchHostName)){
				return match[1];
			}
	    }
	    LtfsLogError("Failed to get host name or ip address of the system.");
	    return "";
	}

	long long TapeLibraryMgr::GetSizeMinTapeFree()
	{
		long long sizeMinTapeFree = SizeTapeFree;
        UInt64_t size;
        string configure = "VSConf.TapeReservedMinFreeSize";
        if ( ltfs_config::CfgManager::Instance()->GetUInt64(configure,size) ) {
        	sizeMinTapeFree = size;
        } else {
            LtfsLogError("Failed to get configure for TapeReservedMinFreeSize");
        }
        if ( sizeMinTapeFree <= SizeTapeBuffer ) {
        	sizeMinTapeFree = SizeTapeFree;
        }
        return sizeMinTapeFree;
	}

    TapeLibraryMgr::TapeLibraryMgr(LtfsLibraries* hal, TapeDbManager* db, TapeSchedulerMgr* sch, TapeStateMachineMgr* stm, FormatManager* fmgr, CatalogDbManager* catalogDb)
    {
    	VS_DBG_LOG_FUNCTION;
        bInited = false;
        sizeMinTapeFree_ = TapeLibraryMgr::GetSizeMinTapeFree();

        // make sure the folder to hold tape in drive flag exists
    	string mkdirCmd = "mkdir -p " + TAPE_IN_DRIVE_FLAG_FOLDER + " 2>/dev/null 1>/dev/null";
    	if(0 != std::system(mkdirCmd.c_str())){
    		LtfsLogError("Failed to crate dir " << TAPE_IN_DRIVE_FLAG_FOLDER << ".");
    	}

        database_ = db;
        catalogDb_ = catalogDb;
        hal_ = hal;
        schduler_ = sch;
        stateMachine_ = stm;
        formatMgr_ = fmgr;
        invEjectTapes_.clear();
        bStop_ = false;
        lastCacheUsageEventTime_ = 0;

        refreshHwModeRunning_ = false;
        bBusy_ = false;
        m_ipHostName = GetIpHostName();
        LtfsError error;

        Refresh(false, error);

        bNeedDiagnose_ = false;
        diagStatus_ = DIAG_NOT_NEED;
        sysDiagLogFile_ = DEFAULT_SYSTEM_DIAGNOSE_LOG_PREFIX;
        // check if the system is dirty shutdown last time
        try{
			fs::path flagFile(SYSTEM_DIRTY_SHUTDOWN_FLAG);
			if(fs::exists(flagFile)){
				bNeedDiagnose_ = true;
				diagStatus_ = DIAG_NOT_RUN;
				fs::remove(flagFile);
				DiagnoseSystem(true);
			}
		}catch(...){
			LtfsLogError("Failed to check if system is dirty shutdown last time.");
		}
    }


    TapeLibraryMgr::~TapeLibraryMgr()
    {
    	VS_DBG_LOG_FUNCTION;
    	bStop_ = true;
    	int retry = 30;
    	while(retry-- > 0 && bBusy_){
    		LtfsLogDebug("TapeLibraryMgr busy, waiting...bBusy = " << bBusy_);
    		sleep(1);
    	}

        //unmount all mounted tapes
        LtfsError lfsErr;
        vector<LtfsChangerInfo> changers;
        string changerSerial = "";
        if(true == GetChangerList(changers, lfsErr)){
        	for(vector<LtfsChangerInfo>::iterator it = changers.begin(); it != changers.end(); it++){
        		changerSerial = it->mSerial;
        		vector<LtfsDriveInfo> drives;
        		if(true == GetDriveListForChanger(it->mSerial, drives, lfsErr)){
        			for(vector<LtfsDriveInfo>::iterator itD = drives.begin(); itD != drives.end(); itD++){
        				UnMountDrive(it->mSerial, itD->mSerial, lfsErr, "", false);
        				UnMountDrive(it->mSerial, itD->mSerial, lfsErr, "", true);
        			}
        		}
        	}
        }

        // clear the in drive tape flags
        string delCmd = "rm -f " +  TAPE_IN_DRIVE_FLAG_FOLDER + "/* 2>/dev/null 1>/dev/null";
        if(0 != std::system(delCmd.c_str())){
        	LtfsLogError("Failed to clear in drive tape flags under " << TAPE_IN_DRIVE_FLAG_FOLDER);
        }
    }


#define GET_LOADED_TAPE_ATTRIBUTE(func_, barcode_, attribute_, ltfsErr_)\
	string changer = "", drive = "";\
	int slotId = -1;\
	if(!FindTape(barcode_, changer, drive, slotId) || database_ == NULL || hal_ == NULL){\
		LtfsLogDebug("GET_LOADED_TAPE_ATTRIBUTE: Tape " << barcode_ << " not found.");\
		return false;\
	}\
	SetTapeActivity(barcode_, ACT_READING_LABEL);\
	bool bRet = hal_->func_(changer, drive, attribute_, ltfsErr_);\
	LtfsLogDebug("GET_LOADED_TAPE_ATTRIBUTE: bRet = " << bRet << ", Err msg: " << ltfsErr_.GetErrMsg());\
	SetTapeActivity(barcode_, ACT_IDLE);\
	return bRet;

	bool DeleteFolderFile(const string& pathName)
	{
    	VS_DBG_LOG_FUNCTION;
		if(pathName == "" || pathName == "/"){
			LtfsLogError("DeleteFolderFile error: pathName = " << pathName << ".");
			return false;
		}

		LtfsLogInfo("DeleteFolderFile: pathName = " << pathName);
		string cmd = string("rm -rf ") + QuotaString(pathName) + " 2>/dev/null 1>/dev/null";
		LtfsLogDebug("DeleteFolderFile:  cmd = " << cmd << ".");
		if(0 != std::system(cmd.c_str())){
			LtfsLogError("Failed to delete folder/file " << pathName << ".");
			return false;
		}
		return true;
	}

	bool CopyFolderFile(const string& pathSrc, const string& pathDst)
	{
    	VS_DBG_LOG_FUNCTION;
		if(pathSrc == "" || pathDst == ""){
			LtfsLogError("CopyFolderFile error: pathSrc = " << pathSrc << ", pathDst = " << pathDst << ".");
			return false;
		}

		size_t nPos = pathDst.rfind('/');
		if(nPos != string::npos){
			fs::path dstFolder = pathDst.substr(0, nPos);
			if(!fs::exists(dstFolder)){
				LtfsLogDebug("creating dir " << dstFolder.string() << " for dst file " << pathDst);
				string cmd = "mkdir -p " + QuotaString(dstFolder.string()) + " 2>/dev/null 1>/dev/null";
				if(0 != std::system(cmd.c_str())){
					LtfsLogError("Failed to create dir " << dstFolder.string() << " to copy file to " << pathDst);
				}
			}
		}

		LtfsLogInfo("CopyFolderFile: pathSrc = " << pathSrc << ", pathDst = " << pathDst << ".");
		string cmd = string("cp -a ") + QuotaString(pathSrc) + " " + QuotaString(pathDst) + " 2>/dev/null 1>/dev/null";
		LtfsLogDebug("CopyFolderFile:  cmd = " << cmd << ".");
		if(0 != std::system(cmd.c_str())){
			LtfsLogError("Failed to copy folder/file " << pathSrc << " to " << pathDst << ".");
			return false;
		}
		return true;
	}

    void TapeLibraryMgr::MarkTapeInDriveFlag(const string barcode, bool bCreate)
    {
    	VS_DBG_LOG_FUNCTION;
    	if(barcode == ""){
    		LtfsLogError("tape barcode is empty. Will not update the flag.");
    		return;
    	}

    	string cmd = "touch ";
    	if(!bCreate){
    		cmd = "rm -f ";
    	}
    	cmd += TAPE_IN_DRIVE_FLAG_FOLDER + "/" + barcode + " 1>/dev/null 2>/dev/null";
    	if(0 != std::system(cmd.c_str())){
    		LtfsLogError("Failed to update tape in drive flag for tape " << barcode << ". bCreate = " << bCreate);
    	}
    }

    void TapeLibraryMgr::MarkTapeInDriveFlag(const string barcode, const string& srcDrive, const string& dstDrive)
    {
    	VS_DBG_LOG_FUNCTION;
    	if(srcDrive != "" && dstDrive == ""){
    		MarkTapeInDriveFlag(barcode, false);
    	}else if(srcDrive == "" && dstDrive != ""){
    		MarkTapeInDriveFlag(barcode, true);
    	}
    }

    long long TapeLibraryMgr::GetMinTapeFreeSize()
    {
    	VS_DBG_LOG_FUNCTION;
    	return sizeMinTapeFree_;
    }

    void TapeLibraryMgr::PreventChangerMediaRemoval(bool bPrevent)
    {
    	VS_DBG_LOG_FUNCTION;
        vector<LtfsChangerInfo> changers;
        LtfsError ltfsErr;
        if( ! hal_->GetChangerList(changers, ltfsErr) ) {
            LtfsLogError("PreventChangerMediaRemoval: Failed to get changes from HAL");
            return;
        }
        for(vector<LtfsChangerInfo>::iterator i = changers.begin(); i != changers.end(); i++ ) {
            if(true == hal_->PreventChangerMediaRemoval(i->mSerial, bPrevent)){
                if(bPrevent){
                    LtfsLogInfo("Succeeded to lock library " << i->mSerial << ", media removal on this library will be disabled.");
                }else{
                    LtfsLogInfo("Succeeded to unlock library " << i->mSerial << ", media removal on this library is enabled.");
                }
            }else{
                if(bPrevent){
                    LtfsLogInfo("Failed to lock library " << i->mSerial << ", media removal on this library may not be disabled.");
                }else{
                    LtfsLogInfo("Failed to unlock library " << i->mSerial << ", media removal on this library may be disabled.");
                }
            }
        }
    }


#define REFRESH_RELEASE_TAPE_AND_RETURN \
    if(false == ReleaseTape(barcode)){\
        LtfsLogError("Failed to release resource for missing tape " << barcode);\
    }\
    return;

#define CHECK_MGR_STOP() \
    if(bStop_){\
    	LtfsLogInfo("TapeLibraryMgr stop request got, refresh canceled.");\
		for(map<string, bool>::iterator it = lockedTapes.begin(); it != lockedTapes.end(); it++){\
			if(it->second == true){\
				stateMachine_->StmReleaseTape(it->first);\
			}\
		}\
		bBusy_ = false;\
    	return true;\
    }

    bool
    TapeLibraryMgr::Refresh(bool bInventory, LtfsError & error)
    {
    	VS_DBG_LOG_FUNCTION;
        LtfsLogInfo("TapeLibraryMgr::Refresh start. Getting lock. bInventory = " << bInventory << ".");
        boost::mutex::scoped_lock lock(mutexRefresh_);
        LtfsLogInfo("TapeLibraryMgr::Refresh start. Got lock.");

        map<string, bool>  lockedTapes;
        CHECK_MGR_STOP();

        if(true == bInventory){
        	LtfsEvent(EVENT_LEVEL_INFO, "Inv_Start", "Start inventory.");
        }

        if(false == hal_->Refresh(error)){
            LtfsLogError("TapeLibraryMgr::Refresh: Failed to refresh library device info from HAL.");
            return false;
        }
        LtfsLogDebug("TapeLibraryMgr::Refresh:  HAL refresh finished.");

        if(false == bInventory){
            LtfsLogInfo("TapeLibraryMgr::Refresh finished. bInventory = " << bInventory << ".");
            return true;
        }

        // get changer list
        vector<LtfsChangerInfo> changers;
        LtfsError lfsErr;
        if(false == hal_->GetChangerList(changers, lfsErr)){
            LtfsLogError("TapeLibraryMgr::Refresh: Failed to get changer list from HAL. Error: " << lfsErr.GetErrMsg());
            return false;
        }

        vector<string>  newTapeBarcodes;
        vector<string>  offlineTapeBarcodes;
        vector<string>	onlineTapeBarcodes;
        map<string, bool> mailSlotTapes;
        map<string, int>  newDrives;
        map<string, int>  disconnectedDrives;
        map<string, CartridgeDetail> orgCartridgeList;
        map<string, INV_TAPE_INFO>	curTapeList;
        map<string, INV_DRIVE_INFO>	curDriveList;
        map<string, bool> openMailSlots;

        CHECK_MGR_STOP();
        LtfsLogDebug("TapeLibraryMgr::Refresh, get changer list finished.");

        // check if it is in TOO MANY TAPES HW mode
        for(unsigned int k = 0; k < changers.size(); k++){
            string changerSerial = changers[k].mSerial;
            if(changers[k].mHwMode == CHANGER_HW_TOO_MANY_TAPES){
            	LtfsLogWarn("TapeLibraryMgr::Refresh: Changer " << changers[k].mSerial << " has too many tapes. Will not do inventory.");
            	return true;
            }
        }

        disconnectedDrives.clear();
        if(bInited == false){
            drives_.clear();
        }
        invEjectTapes_.clear();
        bBusy_ = true;

        // locked all new tapes for inventory first
        map<string, bool>  handledTapes;
        for(unsigned int k = 0; k < changers.size(); k++){
            string changerSerial = changers[k].mSerial;

            //lock the changer to move tapes
            if(!RequestMoveLock(changerSerial)){
            	LtfsLogError("Failed to get move lock for changer " << changerSerial);
            }
            vector<LtfsDriveInfo> drives;
            if(hal_->GetDriveList(changerSerial, drives, lfsErr)){
            	for(unsigned int i = 0; i < drives.size(); i++){
            		string barcode = drives[i].mBarcode;
            		if(barcode != ""){
            			MarkTapeInDriveFlag(barcode, true);
            		}
					//a disconnected drive has a tape, should move it to an empty slot
					if(!drives[i].mIsEmpty && (/*barcode == "" ||*/ drives[i].mStatus == DRIVE_STATUS_DISCONNECTED) ){
						LtfsError lfsErr;
						// get slot list
						vector<LtfsSlotInfo> slots;
						if(false == hal_->GetSlotList(changerSerial, slots, lfsErr)){
							LtfsLogError("Failed to get slot list from changer " << changerSerial
									<< " to move tape " << barcode << " in drive to slot. Error: " << lfsErr.GetErrMsg() << ".");
						}else{
							bool bMove = false;
							for(unsigned int iSlot = 0; iSlot < slots.size(); iSlot++){
								if(slots[iSlot].mIsEmpty){
							        CHECK_MGR_STOP();
									// move tape to this empty slot
									if(false == MoveCartridge(changerSerial, drives[i].mSlotID, slots[iSlot].mSlotID, lfsErr)){
										LtfsLogError("Failed to move tape " << barcode << " in drive " << drives[i].mLogicSlotID\
												<< " to storage slot " << slots[iSlot].mLogicSlotID << ". Error: " << lfsErr.GetErrMsg());
									}else{
										bMove = true;
										break;
									}
								}
							}
							if(false == bMove){
								LtfsLogError("Failed to find empty slot to move tape " << barcode << " in drive "
										<<  drives[i].mLogicSlotID << " to storage slot.");
							}
						}
					}//if(false
            	}//for
            }//if(hal_
            ReleaseMoveLock(changerSerial);

            vector<CartridgeDetail> cartridgeList;
            // get tape list from database
            if(NULL == database_ || false == database_->GetCartridgeList(cartridgeList, changerSerial)){
                LtfsLogError("Failed to get tape list from database for changer " << changerSerial << ".");
                continue;
            }
            map<string, bool> refreshFound;
            for(unsigned int i = 0; i < cartridgeList.size(); i++){
                refreshFound[cartridgeList[i].mBarcode] = false;
                orgCartridgeList[cartridgeList[i].mBarcode] = cartridgeList[i];
            }

            // get tape list for the changer
            vector<LtfsTapeInfo> tapes;
            if(false == hal_->GetTapeList(changerSerial, tapes, lfsErr)){
                LtfsLogError("Failed to get tape list from changer " << changerSerial << ". Error: " << lfsErr.GetErrMsg());
                continue;
            }
            for(unsigned int i = 0; i < tapes.size(); i++){
                string barcode = tapes[i].mBarcode;
                // tape without barocode, not handle it
                if(barcode == ""){
                    continue;
                }
                // new tape, load it to drive to update info
                if(refreshFound.find(barcode) == refreshFound.end()){
                    stateMachine_->StmRequestTape(barcode);
                	lockedTapes[barcode] = true;
                }
            }
        }

        for(unsigned int k = 0; k < changers.size(); k++){
            string changerSerial = changers[k].mSerial;
            bool bRetry = true;
            bool bOpenMailSlot = false;

            // inventory the tapes in the changer, do necessary operations
            while(bRetry){
                CHECK_MGR_STOP();
            	if(!RefreshChanger(changerSerial, lockedTapes, handledTapes, bRetry, bOpenMailSlot, lfsErr)){
            		bRetry = false;
            	}
            }
            CHECK_MGR_STOP();

            bool bAvailableCleanTape = false;
            vector<TapeInfo> tapes;
            if(GetTapeListForChanger(changerSerial, tapes, lfsErr)){
				for(unsigned int i = 0; i < tapes.size(); i++){
					INV_TAPE_INFO tapeInfo;
					tapeInfo.changerSerial = changerSerial;
					tapeInfo.info = tapes[i];
					curTapeList[tapes[i].mBarcode] = tapeInfo;
					if(tapes[i].mMediumType == MEDIUM_CLEANING && tapes[i].mStatus != TAPE_CLEAN_EXPIRED){
						bAvailableCleanTape = true;
					}
				}
            }else{
                LtfsLogError("Failed to get tape list from changer " << changerSerial << ". Error: " << lfsErr.GetErrMsg());
            }
            vector<LtfsDriveInfo> drives;
            if(hal_->GetDriveList(changerSerial, drives, lfsErr)){
				for(unsigned int i = 0; i < drives.size(); i++){
					INV_DRIVE_INFO driveInfo;
					driveInfo.changerSerial = changerSerial;
					driveInfo.info = drives[i];
					curDriveList[drives[i].mSerial] = driveInfo;
                    if(drives[i].mBarcode == "" && false == drives[i].mIsEmpty){
                        LtfsEvent(EVENT_LEVEL_WARNING, "Inv_Tape_No_BC", "Tape in slot " << drives[i].mLogicSlotID
                        		<< " has no barcode. Please eject the tape cartridge from the library.");
                    }
                    string driveSerial = drives[i].mSerial;
    	            if(drives[i].mStatus == CLEANING_REQUIRED && bAvailableCleanTape) {
    	            	map<string, time_t>::iterator it = DriveCleaningThrottleMap.find(driveSerial);
    	            	time_t tNow = time(NULL);
    	            	if(it == DriveCleaningThrottleMap.end() || tNow - DriveCleaningThrottleMap[driveSerial] > INV_DRIVE_CLEAN_THROTTLE){
    	            		hal_->CheckDriveCleaningEvent(changerSerial, driveSerial, drives[i].mStatus);
							StartDriveCleaningTask(changerSerial, driveSerial);
							DriveCleaningThrottleMap[driveSerial] = tNow;
    	            	}
    	            }else{
						DriveCleaningThrottleMap[driveSerial] = 0;
    	            }
				}
            }else{
                LtfsLogError("Failed to get drive list from changer " << changerSerial << ". Error: " << lfsErr.GetErrMsg());
            }
            vector<LtfsMailSlotInfo> mailSlots;
            if(hal_->GetMailSlotList(changerSerial, mailSlots, lfsErr)){
				for(unsigned int i = 0; i < mailSlots.size(); i++){
					if(mailSlots[i].mIsOpen){
						openMailSlots[changerSerial] = bOpenMailSlot;
					}
				}
            }else{
                LtfsLogError("Failed to get mail slot list from changer " << changerSerial << ". Error: " << lfsErr.GetErrMsg());
            }

            // refresh storage slot list from HAL, to check if any tapes without barcode
            vector<LtfsSlotInfo> slots;
            if(true == hal_->GetSlotList(changerSerial, slots, lfsErr)){
                for(unsigned int i = 0; i < slots.size(); i++){
                    if(slots[i].mBarcode == "" && false == slots[i].mIsEmpty){
                        LtfsEvent(EVENT_LEVEL_WARNING, "Inv_Tape_No_BC", "Tape in slot " << slots[i].mLogicSlotID << " has no barcode. Please eject the tape cartridge from the library.");
                    }
                }
            }else{
                LtfsLogError("Failed to get storage slot list from changer " << changerSerial << " to check no barcode tapes. Error: " << lfsErr.GetErrMsg());
            }
        }

        CHECK_MGR_STOP();
        //
        // Inventory finished here, check and send some event/alert/notification if need/any
        //

        // check if any tapes in mailslot ejected by inventory
        for(map<string, string>::iterator it = invEjectTapes_.begin(); it != invEjectTapes_.end(); it++){
        	LtfsEvent(EVENT_LEVEL_WARNING, "Auto_Eject_MailSlot_Tape", "All slots are full. Ejecting mailslot(s) to remove tape(s) " << it->second << ".");
        }

		// remove offline cleaning tape in database
        vector<CartridgeDetail> allCartridgeList;
        if(database_->GetCartridgeList(allCartridgeList)){
        	for(unsigned int i = 0; i < allCartridgeList.size(); i++){
        		string barcode = allCartridgeList[i].mBarcode;
				if(allCartridgeList[i].mOffline && barcode.length() >= 3 && barcode.substr(0, 3) == "CLN"){
					LtfsLogInfo("Removing offline clean tape " << barcode << " from database.");
					// offline, remove it from database
					if(NULL == database_ || false == database_->DeleteCartridge(barcode)){
						LtfsLogError("Failed to delete offline clean tape " << barcode << " from database.");
					}
				}
        	}
        }

        // release the locked tapes
		for(map<string, bool>::iterator it = lockedTapes.begin(); it != lockedTapes.end(); it++){
			if(it->second == true){
				stateMachine_->StmReleaseTape(it->first);
			}
		}

        // check for new/remove drives
        for(map<string, INV_DRIVE_INFO>::iterator it = curDriveList.begin(); it != curDriveList.end(); it++){
        	newDrives[it->second.info.mSerial] = it->second.info.mLogicSlotID;
            if(drives_.find(it->second.info.mSerial) == drives_.end()){
            	if(bInited){
            		//LtfsEvent(EVENT_LEVEL_INFO, "Inv_Drv_Add", "Inventory finished. Discovered new drive " <<
            		//		it->second.info.mLogicSlotID << ". Please reboot the server in order to use the new drive.");
            	}
            	if(it->second.info.mStatus == DRIVE_STATUS_DISCONNECTED){
            		drives_[it->second.info.mSerial] = it->second.info.mLogicSlotID;
            	}
            }else{
            	if(it->second.info.mStatus != DRIVE_STATUS_DISCONNECTED){
            		drives_[it->second.info.mSerial] = -1;
            	}
            }
        }
        // check for disconnected drives
        for(map<string, int>::iterator it = drives_.begin(); it != drives_.end(); it++){
        	if(it->second != -1){
            	//LtfsEvent(EVENT_LEVEL_ERR, "Inv_Drv_Disconn", "Inventory finished. Drive " << it->second << " is disconnected.");
        	}
        }
        bInited = true;
        drives_.clear();
        drives_ = newDrives;

        // check for new tapes
        for(map<string, INV_TAPE_INFO>::iterator it = curTapeList.begin(); it != curTapeList.end(); it++){
        	if(orgCartridgeList.find(it->second.info.mBarcode) == orgCartridgeList.end()){
        		newTapeBarcodes.push_back(it->second.info.mBarcode);
        	}
        }
        for(map<string, CartridgeDetail>::iterator it = orgCartridgeList.begin(); it != orgCartridgeList.end(); it++){
        	if(it->second.mOffline){
        		// tape from offline to online
        		if(curTapeList.find(it->first) != curTapeList.end() && (!curTapeList[it->first].info.mOffline)){
                	LtfsEvent(EVENT_LEVEL_INFO, "Tape_Status_Online", "Inventory finished. Found tape " << it->first << " online.");
        		}
        	}else{
        		// tape from online to offline
        		if(curTapeList.find(it->first) == curTapeList.end() || (curTapeList[it->first].info.mOffline)){
    				if(SetTapeOffline(it->first, true)){
    					LtfsLogDebug("Succeed to set tape " << it->first << " offline in database.");
    				}else{
    					LtfsLogError("Failed to set tape " << it->first << " offline in database.");
    				}
                	LtfsEvent(EVENT_LEVEL_WARNING, "Inv_Tape_Offline", "Inventory finished. Found tape " << it->first << " offline.");
        		}
        	}
        }
        if(newTapeBarcodes.size() > 0){
            string strMsg = "Inventory finished. Discovered new tapes ";
            for(unsigned int i = 0; i < newTapeBarcodes.size(); i++){
                if(i != 0){
                    strMsg += ", ";
                }
                strMsg += newTapeBarcodes[i];
            }
            strMsg += ".";
            LtfsEvent(EVENT_LEVEL_INFO, "Inv_Finished", strMsg);
        }else{
        	LtfsLogInfo("Inventory successfully, no new tape discovered.");
        }

        // check if any mail slots open
        if(openMailSlots.size() > 0){
            LtfsEvent(EVENT_LEVEL_WARNING, "Inv_MS_Open", "Inventory finished. Mail slot is opened. Please close it and do manual inventory.");
        }

        LtfsLogInfo("TapeLibraryMgr::Refresh finished.");
    	// add or update tape group for openstack swift node
    	if(!AddUpdateTapeGroupForSwift()){
    		LogError("Failed to and/update tape group for swift node.");
    	}
        bBusy_ = false;

        return true;
    }

    bool TapeLibraryMgr::InventoryTapeList(const vector<LtfsTapeInfo>& tapes, const string& changerSerial,
    		map<string, bool>& lockedTapes, map<string, bool>& handledTapes, bool& bOpenMailSlot, bool bNew, bool& bRetry)
    {
    	VS_DBG_LOG_FUNCTION;
		for(unsigned int i = 0; i < tapes.size(); i++){
			string barcode = tapes[i].mBarcode;
			CartridgeDetail detail;

			// handle the new tapes (not in database)
			if(bNew && database_->GetCartridge(barcode, detail)){
				continue;
			}
			// handle the existing tapes first( in database)
			if(!bNew && false == database_->GetCartridge(barcode, detail)){
				continue;
			}

			InventoryTape(changerSerial, tapes[i], handledTapes, bOpenMailSlot, bNew, bRetry);
			if(bRetry){
				return true;
			}
			handledTapes[tapes[i].mBarcode] = true;
	        // release the locked tapes
			if(lockedTapes.find(tapes[i].mBarcode) != lockedTapes.end()){
				if(lockedTapes[tapes[i].mBarcode]){
					stateMachine_->StmReleaseTape(tapes[i].mBarcode);
				}
			}
		}// for tape
		return true;
    }

    bool TapeLibraryMgr::RefreshChanger(const string& changerSerial, map<string, bool>& lockedTapes, map<string, bool>& handledTapes,
    		bool& bRetry, bool& bOpenMailSlot, LtfsError & lfsErr)
	{
    	VS_DBG_LOG_FUNCTION;
    	LtfsLogDebug("Inventorying changer " << changerSerial);
    	bRetry = false;
		// get tape list for the changer
		vector<LtfsTapeInfo> tapes;
		if(false == hal_->GetTapeList(changerSerial, tapes, lfsErr)){
			LtfsLogError("Failed to get tape list from changer " << changerSerial << ". Error: " << lfsErr.GetErrMsg());
			return false;
		}

		// handle the existing tapes first( in database)
		InventoryTapeList(tapes, changerSerial, lockedTapes, handledTapes, bOpenMailSlot, false, bRetry);
		if(bRetry){
			return true;
		}
		// handle the new tapes (not in database)
		InventoryTapeList(tapes, changerSerial, lockedTapes, handledTapes, bOpenMailSlot, true, bRetry);
		if(bRetry){
			return true;
		}

		// refresh mail slot list from HAL, to move any tapes in mailslot to storage slot, to check if any mail slot is open
		vector<LtfsMailSlotInfo> mailSlots;
		if(true == hal_->GetMailSlotList(changerSerial, mailSlots, lfsErr)){
			for(unsigned int j = 0; j < mailSlots.size(); j++){
				// check if the mail slot is open/empty
				if(mailSlots[j].mIsOpen || mailSlots[j].mIsEmpty){
					continue;
				}
	            CHECK_MGR_STOP();
				InventoryEjectCheck(changerSerial, bRetry, bOpenMailSlot, mailSlots[j]);
				if(bRetry){
					return true;
				}

	            CHECK_MGR_STOP();
				// move the tape to an empty slots
				MoveTapeToStorageSlot(changerSerial, mailSlots[j]);
			} // for mail slots
		}else{
			LtfsLogError("Failed to get mail slot list from changer " << changerSerial << ". Error: " << lfsErr.GetErrMsg());
		}// if GetMailSlotList

		bRetry = false;

		return true;
	}

    bool TapeLibraryMgr::InventoryEjectCheck(const string& changerSerial, bool& bRetry,
    		bool& bOpenMailSlot, const LtfsMailSlotInfo& mailSlotInfo )
    {
    	VS_DBG_LOG_FUNCTION;
    	LtfsError lfsErr;
		// check if the mail slot is open/empty
		if(mailSlotInfo.mIsOpen || mailSlotInfo.mIsEmpty){
			LtfsLogDebug("Mail slot " << mailSlotInfo.mLogicSlotID << " is empty/open.");
			return true;
		}
		// tape number exceeds the max number, need to open mail slot to eject the tape in mail slot
		if(hal_->GetTapeNum(changerSerial, true, false) >= hal_->GetSlotNum(changerSerial)){
			// get tape list in mail slot
			map<string, bool>keptTapes;
			vector<LtfsMailSlotInfo> mailSlots;
			if(true == hal_->GetMailSlotList(changerSerial, mailSlots, lfsErr)){
				for(unsigned int j = 0; j < mailSlots.size(); j++){
					if(mailSlots[j].mIsOpen || mailSlots[j].mBarcode == ""){
						continue;
					}
					keptTapes[mailSlots[j].mBarcode] = false;
				}
			}

			 if(hal_->OpenMailSlot(changerSerial, mailSlotInfo.mSlotID, lfsErr)){
				 bOpenMailSlot = true;
				 LtfsLogInfo("Tape number exceeds the max number. Open mailslot " << mailSlotInfo.mLogicSlotID << " to eject tapes.");
			 }else{
				 LtfsLogError("Failed to open mailslot " << mailSlotInfo.mLogicSlotID << " to eject extra tapes.");
			 }
			 bRetry = true;

			// get tape list in mail slot
			mailSlots.clear();
			if(true == hal_->GetMailSlotList(changerSerial, mailSlots, lfsErr)){
				for(unsigned int j = 0; j < mailSlots.size(); j++){
					if(mailSlots[j].mIsOpen || mailSlots[j].mBarcode == ""){
						continue;
					}
					keptTapes[mailSlots[j].mBarcode] = true;
				}
			}

			string barcodes = invEjectTapes_[changerSerial];
			for(map<string, bool>::iterator it = keptTapes.begin(); it != keptTapes.end(); it++){
				if(!it->second){
					if(barcodes != ""){
						barcodes += ", ";
					}
					barcodes += it->first;
				}
			}
			invEjectTapes_[changerSerial] = barcodes;

			 return true;
		}
		return true;
    }

    bool TapeLibraryMgr::MoveTapeToStorageSlot(const string& changerSerial, const LtfsMailSlotInfo& mailSlot)
    {
    	VS_DBG_LOG_FUNCTION;
    	LtfsError lfsErr;
        //lock the changer to move tapes
        if(!RequestMoveLock(changerSerial)){
        	LtfsLogError("Failed to get move lock for changer " << changerSerial);
        }
		// get slot list
		vector<LtfsSlotInfo> slots;
		if(false == hal_->GetSlotList(changerSerial, slots, lfsErr)){
			LtfsLogError("Failed to get slot list from changer " << changerSerial << ". Error: " \
					<< lfsErr.GetErrMsg() << ". The tape in mail slot " <<  mailSlot.mLogicSlotID << " will not be moved to storage slot.");
			ReleaseMoveLock(changerSerial);
			return false;
		}
		bool bMoved = false;
		for(unsigned int iSlot = 0; iSlot < slots.size(); iSlot++){
			if(slots[iSlot].mIsEmpty){
				// move tape to this empty slot
				if(false == MoveCartridge(changerSerial, mailSlot.mSlotID, slots[iSlot].mSlotID, lfsErr, mailSlot.mBarcode)){
					LtfsLogError("Failed to move tape in mail slot " << mailSlot.mLogicSlotID\
							<< " to storage slot " << mailSlot.mLogicSlotID << ". Error: " << lfsErr.GetErrMsg());
				}else{
					bMoved = true;
					break;
				}
			}
		}
		ReleaseMoveLock(changerSerial);

		return bMoved;
    }

    ChangerStatus TapeLibraryMgr::GetChangerStatus(const string& changerSerial)
    {
    	VS_DBG_LOG_FUNCTION;
    	vector<LtfsChangerInfo> changers;
    	LtfsError error;
    	if(GetChangerList(changers, error)){
    		for(unsigned int i = 0; i < changers.size(); i++){
    			if(changers[i].mSerial == changerSerial){
    				return (ChangerStatus)changers[i].mStatus;
    			}
    		}
    	}
    	return CHANGER_STATUS_UNKNOWN;
    }

#define INV_TAPE_CHECK_STOP(tape_) \
	if(bStop_){\
		LtfsLogInfo("TapeLibraryMgr stop request got, stop inventory for tape " << tape_ << ".");\
    	return;\
	}

    void TapeLibraryMgr::InventoryTape(const string& changerSerial, const LtfsTapeInfo& tapeInfo, map<string, bool>& handledTapes, bool& bOpenMailSlot, bool bNew, bool& bRetry)
    {
    	VS_DBG_LOG_FUNCTION;
    	LtfsError lfsErr;
		string barcode = tapeInfo.mBarcode;
		// tape without barocode, not handle it
		if(barcode == ""){
			return;
		}

        INV_TAPE_CHECK_STOP(barcode);

		// changer is disconnected, not to inventory the tape
		ChangerStatus chStatus = GetChangerStatus(changerSerial);
		if(chStatus == CHANGER_STATUS_DISCONNECTED || chStatus == CHANGER_STATUS_UNKNOWN){
			LtfsLogWarn("Changer " << changerSerial << " does not exist or is disconnected, will not inventory the tape " << barcode);
			return;
		}

		if(handledTapes.find(barcode) != handledTapes.end() && handledTapes[barcode]){
			LtfsLogDebug("Tape " << barcode << " has been handled before. Do not handle it this time.");
			return;
		}

		LtfsLogDebug("TapeLibraryMgr::Refresh:  checking tape " << barcode << ". bNew = " << bNew << ".");

		bool bOffline = hal_->IsTapeOffline(changerSerial, tapeInfo.mSlotID);
		if(bOffline){
			LtfsLogWarn("Tape " << barcode << " is offline.");
		}

		// tape number exceeds the max number, need to open mail slot to eject the tape in mail slot
		LtfsMailSlotInfo mailSlotInfo;
		if(hal_->IsTapeInMailSlot(changerSerial, barcode, mailSlotInfo)){
			InventoryEjectCheck(changerSerial, bRetry, bOpenMailSlot, mailSlotInfo);
			if(bRetry){
				 return;
			}
		}

		bool bLowLTOTape = false;
		// 001840L3
		regex matchLTO("^\\s*\\w+L(\\d+)");
		cmatch match;
		int mediaType = MEDIA_UNKNOWN;
		if(regex_match(barcode.c_str(), match, matchLTO)){
			int tapeLTO = boost::lexical_cast<int>(match[1]);
			LtfsLogDebug("tape LTO: " << tapeLTO << ".");
			if(tapeLTO <= 4){
				LtfsLogDebug("Do not load LTO4 and lower generation tapes into drive for inventory. Barcoce: " << barcode);
				bLowLTOTape = true;
				switch(tapeLTO){
				case 1:
					mediaType = MEDIA_LTO1;
					break;
				case 2:
					mediaType = MEDIA_LTO2;
					break;
				case 3:
					mediaType = MEDIA_LTO3;
					break;
				case 4:
					mediaType = MEDIA_LTO4;
					break;
				case 5:
					mediaType = MEDIA_LTO5;
					break;
				case 6:
					mediaType = MEDIA_LTO6;
					break;
				default:
					mediaType = MEDIA_OTHERS;
					break;
				}
			}
		}

		// new tape, load it to drive to update info
		if(bNew){
			// cleaning tapes, just add it to database
			if(tapeInfo.mMediumType == MEDIUM_CLEANING){
				CartridgeDetail clnTape;
				clnTape.mBarcode = barcode;
				clnTape.mTapeGroupUUID = "";
				clnTape.mMediaType = MEDIA_UNKNOWN;
				clnTape.mMediumType = MEDIUM_UNKNOWN;
				clnTape.mStatus = TAPE_UNKNOWN;
				clnTape.mLoadCount = 0;
				clnTape.mGenerationNumber = 0;
				clnTape.mUsedCapacity = 0;
				clnTape.mFreeCapacity = 0;
				clnTape.mFileNumber = 0;
				clnTape.mFileCapacity = 0;
				clnTape.mFormat = LTFS_UNKNOWN;
				clnTape.mFaulty = 0;
				clnTape.mState = 0;
				clnTape.mActivity = 0;
				clnTape.mWriteProtect = 0;
				clnTape.mOffline = bOffline;
                clnTape.mDualCopy = "";
                clnTape.mTapeUUID = "";
                clnTape.mLastMountTime = 0;
                clnTape.mLastAuditTime = 0;
				if(!bOffline){
					if(NULL == database_ || false == database_->ModifyCartridge(barcode, clnTape)){
						LtfsLogError("Failed to add clean tape " << barcode << " to database.");
					}
				}
				return;
			}

			// offline tape, just added to database
			if(bOffline){
				CartridgeDetail offTape;
				offTape.mBarcode = barcode;
				offTape.mTapeGroupUUID = "";
				offTape.mMediaType = MEDIA_UNKNOWN;
				offTape.mMediumType = MEDIUM_UNKNOWN;
				offTape.mStatus = TAPE_UNKNOWN;
				offTape.mLoadCount = 0;
				offTape.mGenerationNumber = 0;
				offTape.mUsedCapacity = 0;
				offTape.mFreeCapacity = 0;
				offTape.mFileNumber = 0;
				offTape.mFileCapacity = 0;
				offTape.mFormat = LTFS_UNKNOWN;
				offTape.mFaulty = 0;
				offTape.mState = 0;
				offTape.mActivity = 0;
				offTape.mWriteProtect = 0;
				offTape.mOffline = true;
                offTape.mDualCopy = "";
                offTape.mTapeUUID = "";
                offTape.mLastMountTime = 0;
                offTape.mLastAuditTime = 0;
				if(NULL == database_ || false == database_->ModifyCartridge(barcode, offTape)){
					LtfsLogError("Failed to add offline tape " << barcode << " to database.");
				}
				return;
			}

			//LTO4 or lower tape, just add to database
			if(bLowLTOTape){
				CartridgeDetail tapeInf;
				tapeInf.mBarcode = barcode;
				tapeInf.mTapeGroupUUID = "";
				tapeInf.mMediaType = mediaType;
				tapeInf.mMediumType = MEDIUM_UNKNOWN;
				tapeInf.mStatus = TAPE_UNKNOWN;
				tapeInf.mLoadCount = 0;
				tapeInf.mGenerationNumber = 0;
				tapeInf.mUsedCapacity = 0;
				tapeInf.mFreeCapacity = 0;
				tapeInf.mFileNumber = 0;
				tapeInf.mFileCapacity = 0;
				tapeInf.mFormat = LTFS_UNKNOWN;
				tapeInf.mFaulty = 0;
				tapeInf.mState = 0;
				tapeInf.mActivity = 0;
				tapeInf.mWriteProtect = 0;
				tapeInf.mOffline = false;
                tapeInf.mDualCopy = "";
                tapeInf.mTapeUUID = "";
                tapeInf.mLastMountTime = 0;
                tapeInf.mLastAuditTime = 0;
				if(NULL == database_ || false == database_->ModifyCartridge(barcode, tapeInf)){
					LtfsLogError("Failed to add LTO4 or lower tape " << barcode << " to database.");
				}
				return;
			}

	        INV_TAPE_CHECK_STOP(barcode);
			LtfsLogDebug("TapeLibraryMgr::Refresh:  requesting resuource for new tape " << barcode << ".");

			int reqTimeOut = INVENTORY_LOAD_TAPE_REQUEST_WAIT;
			int nSleep = 3;
			while(reqTimeOut > 0 && false == RequestTape(barcode, false , bdt::ScheduleInterface::PRIORITY_MANAGE_CARTRIDGE, nSleep)){
				LtfsLogDebug("Requesting tape " << barcode << ", reqTimeOut = " << reqTimeOut);
		        INV_TAPE_CHECK_STOP(barcode);
				reqTimeOut -= nSleep;
			}
			if(reqTimeOut <= 0){
				LtfsLogError("Failed to load tape " << barcode << " to drive to get info.");
				return;
			}


			string changer, drive;
			int slotID;
			if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
				LtfsLogError("Failed to load tape " << barcode << " to drive to get info.");
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}

			// unmount the tape first
			if(!UnMountDrive(changer,drive,lfsErr, barcode, false)){
				LtfsLogError("Failed to unmount tape " << barcode << " to add it to database.");
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}

	        INV_TAPE_CHECK_STOP(barcode);
			UpdateTapeUnlock(changer, drive, lfsErr, true);
			ReleaseTape(barcode);
			//
			CartridgeDetail detail;
			if(NULL == database_ || false == database_->GetCartridge(barcode, detail)){
				LtfsLogError("Failed to add tape " << barcode << " to database.");
				return;
			}

			// a new tape is in local tape group, should clear the info
			if(detail.mStatus != TAPE_EXPORTED && IsNativeTapeGroup(detail.mTapeGroupUUID)){
				if(false == AssignToTapeGroup(barcode, "", "", TAPE_UNKNOWN)){
					LtfsLogError("Failed to clear tape group info for new tape " << barcode << ".");
				}
			}

			// a LTFS tape, mount it on to update info
			if(detail.mFormat == LTFS_VALID){
				// if the tape is LTO6 and loaded into a LTO5 drive, we should wait the tape_idle_time to request resource again.
				if(detail.mMediaType == MEDIA_LTO6){
					LtfsDriveInfo driveInfo;
					if(!GetDrive(drive, driveInfo) || driveInfo.mGeneration < 6){
						// get tape idle time
						int tapeIdleTime = bdt::Factory::GetConfigure()->GetValueSize(bdt::Configure::TapeIdleTime);
						LtfsLogDebug("DDEBUG: tape idle time = " << tapeIdleTime);
						sleep(tapeIdleTime + 2);
						LtfsLogDebug("DDEBUG: sleep end, tape idle time = " << tapeIdleTime);
					}
				}
				LtfsLogDebug("TapeLibraryMgr::Refresh:  requesting resuource for new tape " << barcode << " to mount it on.");
				if(false == RequestTape(barcode, true , bdt::ScheduleInterface::PRIORITY_MANAGE_CARTRIDGE, INVENTORY_LOAD_TAPE_REQUEST_WAIT)){
					LtfsLogError("Failed to mount tape " << barcode << " to drive to get info.");
					return;
				}
				string changer, drive;
				int slotID;
				if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
					LtfsLogError("Failed to load tape " << barcode << " to drive to get info.");
					REFRESH_RELEASE_TAPE_AND_RETURN;
				}
				if(false == UpdateTapeUnlock(changer, drive, lfsErr)){
					LtfsLogError("Failed to update new tape " << barcode << " in database.");
				}
				ReleaseTape(barcode);
			}
			//}
		}//if
		else{ // existing tape
			CartridgeDetail detail;
			if(NULL == database_ || false == database_->GetCartridge(barcode, detail)){
				LtfsLogError("Failed to get info of tape " << barcode << " from database.");
				return;
			}

			// cleaning tapes
			if(tapeInfo.mMediumType == MEDIUM_CLEANING){
				if(bOffline){
					LtfsLogInfo("Removing offline clean tape " << barcode << " from database.");
					// offline, remove it from database
					if(NULL == database_ || false == database_->DeleteCartridge(barcode)){
						LtfsLogError("Failed to delete offline clean tape " << barcode << " from database.");
					}
				}
				return;
			}

			if(detail.mOffline != bOffline){
				// make sure to update offline status of the tape
				if(true == SetTapeOffline(barcode, bOffline)){
					LtfsLogDebug("Succeed to update offline status for tape " << barcode << " in database.");
				}else{
					LtfsLogError("Failed to update offline status for tape " << barcode << " in database.");
				}
			}

			// previous tape is online, no need to inventory again.
			if(!detail.mOffline){
				return;
			}

			// tape is still offline, no need to handle
			if(bOffline){
				return;
			}

			// LTO4 or lower tape, no need to inventory it.
			if(bLowLTOTape){
				return;
			}

			// tape is online, some handling
			// request resource and get generation number from HAL
			LtfsLogDebug("TapeLibraryMgr::Refresh:  requesting resuource for existing tape " << barcode << ".");
			if(false == RequestTape(barcode, false, bdt::ScheduleInterface::PRIORITY_MANAGE_CARTRIDGE, INVENTORY_LOAD_TAPE_REQUEST_WAIT)){
				LtfsLogError("Failed to request resource to get generation number for offline tape " << barcode);
				return;
			}

			// unmount the tape first
			if(false == UnmountTape(barcode, false)){
				LtfsLogError("Failed to unmount offline tape " << barcode << " to get generation number.");
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}

			string changer, drive;
			int slotID;
			if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
				LtfsLogError("Failed to find tape " << barcode << " to get generation number.");
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}

			// tape is in disconnected drive before, need to force update info to database
			if(detail.mMediaType == MEDIA_UNKNOWN
				&& detail.mStatus == TAPE_UNKNOWN
				&& detail.mFormat == LTFS_UNKNOWN
				&& detail.mTapeGroupUUID == ""
			){
				UpdateTapeUnlock(changer, drive, lfsErr, true, true);
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}

			// check write protect status of the tape
			bool bWP = 0;
			if(false == GetLoadedTapeWPFlag(barcode, bWP, lfsErr)){
				LtfsLogError("Failed to get write protect flag from MAM for offline tape " << barcode << ".");
				//REFRESH_RELEASE_TAPE_AND_RETURN;
			}else{
				// update wp flag to database
				if(NULL == database_ || false == database_->SetWriteProtectForCartridge(barcode, bWP)){
					LtfsLogError("Failed to set write protect flag to database for offline tape " << barcode << ".");
				}
			}

			// only need to continue to do offline handling for local share tapes
			if(false == IsNativeTapeGroup(detail.mTapeGroupUUID) || detail.mStatus == TAPE_EXPORTED){
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}

			long long genIndex = 0;
			if(false == GetLoadedTapeGenerationIndex(barcode, genIndex, lfsErr)){
				LtfsLogError("Failed to get generation number for offline tape " << barcode << ".");
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}
			LtfsLogDebug("genIndex = " << genIndex << ", detail.mGenerationNumber = " << detail.mGenerationNumber << ", barcode = " << barcode << ".");

			//  generation number not match, set tape status to conflict
			if(genIndex != detail.mGenerationNumber){
				if(true == SetLoadedTapeStatus(barcode, TAPE_CONFLICT, lfsErr, true)) {
				//if(true == SetTapeStatus(barcode, TAPE_CONFLICT, -1)){ //TODO: set mam?have resource?
					LtfsLogInfo("Succeed to set offline tape " << barcode << " to conflict status.");
				}else{
					LtfsLogInfo("Failed to set offline tape " << barcode << " to conflict status.");
				}
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}

			// generation number matches, check tape status from MAM
			int tpStatus = 0;
			if(false == GetLoadedTapeStatus(barcode, tpStatus, lfsErr)){
				LtfsLogError("Failed to get status from MAM for offline tape " << barcode << ".");
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}

			LtfsLogDebug("tpStatus = " << tpStatus << ", barcode = " << barcode << ".");

			// tape status not active, keep current status
			if(tpStatus != TAPE_ACTIVE){
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}

			// tape status not active, check if any other active tapes in this tape group
			vector<TapeInfo> tpInfos;
			if(false == GetTapeGroupMembers(tpInfos, detail.mTapeGroupUUID, lfsErr)){
				LtfsLogError("Failed to get members of tape group " << detail.mTapeGroupUUID << ".");
				REFRESH_RELEASE_TAPE_AND_RETURN;
			}
			for(unsigned int iT = 0; iT < tpInfos.size(); iT++){
				if(tpInfos[iT].mStatus == TAPE_ACTIVE && tpInfos[iT].mBarcode != barcode){
					// set tape status to closed
					if(true == SetLoadedTapeStatus(barcode, TAPE_OPEN, lfsErr, true)) {
					//if(true == SetTapeStatus(barcode, TAPE_OPEN, -1)){  //TODO: set mam?have resource?
						LtfsLogInfo("Succeed to set offline tape " << barcode << " to open status.");
					}else{
						LtfsLogInfo("Failed to set offline tape " << barcode << " to open status.");
					}
					break;
				}
			}
			// release tape
			if(false == ReleaseTape(barcode)){
				LtfsLogError("Failed to release resource to get generation number for offline tape " << barcode);
			}
		}// if existing tape

		// if the tape is in mail slot, move it to storage slot
		if(hal_->IsTapeInMailSlot(changerSerial,barcode,  mailSlotInfo)){
			MoveTapeToStorageSlot(changerSerial, mailSlotInfo);
		}


		return;
    }

    bool
    TapeLibraryMgr::GetTapeGroupCapacity(
            const string & tapeGroup,
            size_t & fileNumber,
            off_t & usedSize,
            off_t & freeSize)
    {
    	VS_DBG_LOG_FUNCTION;
        fileNumber = 0;
        usedSize = 0;
        freeSize = 0;

        if(!CatalogDbManager::Instance()->GetTotalSize(tapeGroup, usedSize)){
        	LtfsLogError("Failed to get backed up size for share " << tapeGroup);
        	return false;
        }
        LtfsLogDebug("Total size used for tape group " << tapeGroup << " is " << usedSize);
        TapeDbManager* pTapeDb = TapeDbManager::Instance();

        vector<string> barcodes;
        vector<string> skipbarcodes;
        if ( ! pTapeDb->GetTapeGroupCartridgeList(tapeGroup,barcodes) ) {
            LtfsLogError( "Failed to get cartridge from database"
                    << " for tape group " << tapeGroup );
            return false;
        }

        for ( vector<string>::iterator i = barcodes.begin();
                i != barcodes.end();
                ++ i ) {
        	vector<string>::iterator result = find(skipbarcodes.begin(), skipbarcodes.end(), *i);
        	if ( result != skipbarcodes.end())
        	{
        		LtfsLogDebug("skip barcode:" << *i);
        		continue;
        	}
            CartridgeDetail detail;
            if ( ! pTapeDb->GetCartridge(*i,detail) ) {
                LtfsLogError( "Failed to get cartridge from database"
                        << " for tape " << *i );
                continue;
            }
            if (pTapeDb->GetTapeGroupDualCopy(tapeGroup))
            {
            	if (detail.mDualCopy == "")
            	{
            		fileNumber += detail.mFileNumber;
            		//usedSize += detail.mFileCapacity;
            		continue;
            	}
            	skipbarcodes.push_back(detail.mDualCopy);
            	CartridgeDetail detailCopy;
            	if ( ! pTapeDb->GetCartridge(detail.mDualCopy,detailCopy) ) {
					LtfsLogError( "Failed to get cartridge from database"
							<< " for tape " << detail.mDualCopy );
					continue;
				}
            	if ( detail.mFaulty  || detailCopy.mFaulty) {
					fileNumber += detail.mFileNumber;
					//usedSize += detail.mFileCapacity;
					continue;
				}
            	if (detailCopy.mStatus == TAPE_CONFLICT || detail.mStatus == TAPE_CONFLICT)
            	{
            		continue;
            	}
            	if (detailCopy.mFreeCapacity < detail.mFreeCapacity)
            	{
            		detail = detailCopy;
            	}
            }
            //TODO: should not check activity
            /*
            switch ( detail.mActivity ) {
				case ACT_WAITING:
				case ACT_FORMATTING:
				case ACT_WRITTING_LABEL:
                case ACT_IDLE:
                case ACT_LISTING_FILES:
                case ACT_DELETING_FILES:
                    break;
                default:
                    continue;
            }
            */
            if ( detail.mFaulty ) {
                fileNumber += detail.mFileNumber;
                //usedSize += detail.mFileCapacity;
                continue;
            }

            /* TODO
            if ( detail.mState == TapeStateReadOnly ) {
                fileNumber += detail.mFileNumber;
                usedSize += detail.mFileCapacity;
                continue;
            }
            if ( detail.mState == TapeStateUnavailable ) {
                continue;
            }
            */

            long long sizeMinTapeFree = TapeLibraryMgr::GetSizeMinTapeFree();
            switch ( detail.mStatus ) {
                case TAPE_OPEN:
                    fileNumber += detail.mFileNumber;
                    if ( (detail.mFreeCapacity - sizeMinTapeFree) >= 0 ) {
                        freeSize += detail.mFreeCapacity - sizeMinTapeFree;
                    }
                    break;
                case TAPE_ACTIVE:
                    fileNumber += detail.mFileNumber;
                    if ( (detail.mFreeCapacity - sizeMinTapeFree) >= 0 ) {
                        freeSize += (detail.mFreeCapacity - sizeMinTapeFree);
                    }
                    break;
                case TAPE_CLOSED:
                    fileNumber += detail.mFileNumber;
                    break;
                default:
                    break;
            }
        }
        LtfsLogDebug("Total free size for tape group " << tapeGroup << " is " << freeSize);

        return true;
    }

    bool TapeLibraryMgr::GetMetaFreeCapacity(off_t & usedCapacity, off_t & freeCapacity)
    {
    	VS_DBG_LOG_FUNCTION;
    	struct statfs stat;
    	fs::path folder = Factory::GetMetaFolder();
		if ( 0 != statfs(folder.string().c_str(),&stat) ) {
			LtfsLogError("get meta free capacity error:" << folder);
			return false;
		}

		freeCapacity = (off_t)stat.f_bsize * stat.f_bfree;
		usedCapacity = (off_t)stat.f_bsize * (stat.f_blocks - stat.f_bfree);

		static off_t leastSize = Factory::GetConfigure()->GetValueSize(
		                    Configure::MetaFreeLeastSize );
		if ( freeCapacity < leastSize ) {
			LtfsLogError("Not enough free meta capacity.");
			return false;
		}
    	return true;
    }

    bool
    TapeLibraryMgr::GetTapeGroupMembers(
            vector<TapeInfo> & tapes,
            const string & tapeGroup,
            LtfsError & error,
            bool bIncludeOffline)
    {
    	VS_DBG_LOG_FUNCTION;
        tapes.clear();

       	LtfsLogDebug("GetTapeGroupMembers: tapeGroup = " << tapeGroup);
        vector<string> barcodesTapeGroup;
        if ( ! database_->GetTapeGroupCartridgeList(
                tapeGroup, barcodesTapeGroup ) ) {
            LtfsLogError( "Failed to get cartridge from database"
                    << " for tape group " << tapeGroup );
            return false;
        }
        map<string, bool> addedTapes;

        vector<LtfsChangerInfo> changers;
        if ( ! hal_->GetChangerList(changers,error) ) {
            LtfsLogError("Failed to get changes from HAL");
            return false;
        }

        for(vector<LtfsChangerInfo>::iterator i = changers.begin();
                i != changers.end();
                ++ i ) {
            vector<LtfsTapeInfo> tapesChanger;
            if (! hal_->GetTapeList(i->mSerial,tapesChanger,error)) {
                LtfsLogError( "Failed to get tapes from HAL: " << i->mSerial );
                continue;
            }
            for(vector<LtfsTapeInfo>::iterator iTape = tapesChanger.begin();
                    iTape != tapesChanger.end();
                    ++ iTape ) {
                if ( find(
                        barcodesTapeGroup.begin(),
                        barcodesTapeGroup.end(),
                        iTape->mBarcode ) != barcodesTapeGroup.end() ) {
                    TapeInfo tape;
                    GetTape(tape,*iTape);
                    tapes.push_back(tape);
                    addedTapes[iTape->mBarcode] = true;
                }
            }
        }

        if(bIncludeOffline){
        	for(unsigned int i = 0; i < barcodesTapeGroup.size(); i++){
        		string barcode = barcodesTapeGroup[i];
        		CartridgeDetail detail;
        		if(addedTapes.find(barcode) == addedTapes.end() && database_->GetCartridge(barcode, detail)){
        			tapes.push_back(ConvertTape(detail));
        		}
        	}
        }

        return true;
    }


    TapeLibraryMgr *
    TapeLibraryMgr::Instance()
    {
    	VS_DBG_LOG_FUNCTION;
        boost::mutex::scoped_lock lock(mutexInstance_);

        if ( NULL == instance_ ) {
            instance_ = new TapeLibraryMgr(LtfsLibraries::Instance(), TapeDbManager::Instance(),
            		TapeSchedulerMgr::Instance(), TapeStateMachineMgr::Instance(), FormatManager::Instance(), CatalogDbManager::Instance());
        }
        return instance_;
    }

    void
    TapeLibraryMgr::Destroy()
    {
    	VS_DBG_LOG_FUNCTION;
        boost::mutex::scoped_lock lock(mutexInstance_);

        if (instance_) {
            delete instance_;
            instance_ = NULL;

            FormatManager::Destory();
            TapeSchedulerMgr::Destroy();
            TapeStateMachineMgr::Destroy();
            TapeDbManager::Destroy();
            LtfsLibraries::Destory();
        }
    }

    bool
    TapeLibraryMgr::CheckTapeGroupBuffer(
            const string & tapeGroup, const string & barcode, long long size )
    {
    	VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexBuffer_);

        LtfsLogDebug("CheckTapeGroupBuffer: "
                << tapeGroup << " "
                << barcode << " " << size);

        TapeGroupBufferMap::iterator i = buffer_.find(tapeGroup);
        if ( buffer_.end() == i ) {
            i = buffer_.insert( TapeGroupBufferMap::value_type(
                    tapeGroup, 0 ) ).first;
        }

        if ( i->second >= size ) {
            i->second -= size;
            LtfsLogDebug("CheckTapeGroupBuffer: " << true);
            return true;
        } else {
            i->second = 0;
            LtfsLogDebug("CheckTapeGroupBuffer: " << false);
            return false;
        }
    }


    bool
    TapeLibraryMgr::CheckTapeGroupTape(
            const string & tapeGroup, const string & barcode, long long size )
    {
    	VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexBuffer_);

        LtfsLogDebug("CheckTapeGroupTape: "
                << tapeGroup << " " << barcode);

        string changer,drive;
        int slotID;
        LtfsError error;
        if ( ! FindTape(barcode,changer,drive,slotID) && (! drive.empty()) ) {
            LtfsLogError("CheckTapeGroupTape: failure to locate tape in drive");
        } else {
            if ( ! UpdateTapeUnlock(changer,drive,error) ) {
                LtfsLogError("CheckTapeGroupTape: failure to update tape");
            }
        }

        CartridgeDetail detail;
        if ( ! database_->GetCartridge(barcode,detail) ) {
            LtfsLogError("CheckTapeGroupTape: failure to get tape from database");
            return false;
        }
        TapeGroupBufferMap::iterator i = buffer_.find(tapeGroup);
        if ( buffer_.end() == i ) {
            i = buffer_.insert( TapeGroupBufferMap::value_type(
                    tapeGroup, SizeTapeBuffer ) ).first;
        }
        if ( detail.mFreeCapacity >= sizeMinTapeFree_ + size ) {
            if ( detail.mFreeCapacity >= SizeTapeBuffer + sizeMinTapeFree_ + size ) {
                i->second = SizeTapeBuffer;
            } else {
                i->second = SizeTapeMinBuffer;
            }
            LtfsLogDebug("CheckTapeGroupTape: " << true);
            return true;
        } else {
            i->second = 0;
        }

        if ( detail.mFreeCapacity < size ) {
            LtfsLogDebug("CheckTapeGroupTape: " << false);
            return false;
        }

        bool hasEmptyTape = false;
        vector<string> barcodes;
        if ( database_->GetTapeGroupCartridgeList(tapeGroup,barcodes) ) {
            for ( vector<string>::iterator iter = barcodes.begin();
                    iter != barcodes.end();
                    ++ iter ) {
                if ( *iter == barcode ) {
                    continue;
                }
                if ( ! database_->GetCartridge(*iter,detail) ) {
                    LtfsLogError( "Failed to get cartridge from database: "
                            << *iter );
                    continue;
                }
                if ( ! IsTapeWritable(detail) ) {
                    continue;
                }
                if ( detail.mStatus == TAPE_OPEN ) {
                    hasEmptyTape = true;
                    break;
                }
            }
        }

        if ( hasEmptyTape ) {
            LtfsLogDebug("CheckTapeGroupTape: " << false);
            return false;
        } else {
            LtfsLogDebug("CheckTapeGroupTape: " << true);
            LtfsLogWarn("Force to use the last tape for backup: " << barcode);
            return true;
        }

    }


    bool
    TapeLibraryMgr::IsTapeWritable(const CartridgeDetail & detail)
    {
    	VS_DBG_LOG_FUNCTION;
        if ( detail.mFaulty ) {
            return false;
        }
        if ( detail.mWriteProtect ) {
            return false;
        }
        if ( detail.mFormat != LTFS_VALID ) {
            return false;
        }

        if (detail.mStatus == TAPE_CLOSED){
            return false;
        }

        //should still use this state to check?
        /*
        if ( detail.mState == TapeStateUnavailable ) {
            return false;
        }
        if ( detail.mState == TapeStateReadOnly ) {
            return false;
        }

        // TODO: should check activity?
        if ( detail.mActivity != ltfs_management::ACT_IDLE ) {
            return false;
        }*/
        if ( detail.mOffline ) {
            return false;
        }
        return true;
    }


    bool
    TapeLibraryMgr::SetTapeCapacity(
            const string & barcode,
            int fileNumber,
            off_t fileSize,
            off_t tapeSize)
    {
    	VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexTape_);

        LtfsError error;
        CartridgeDetail detail;
        if ( database_->GetCartridge(barcode,detail) ) {
            if ( tapeSize >= 0 ) {
                detail.mUsedCapacity += tapeSize;
                detail.mFreeCapacity -= tapeSize;
            } else {
                LtfsLogError( "tape used size less than 0: " << tapeSize );
            }
            detail.mFileNumber += fileNumber;
            detail.mFileCapacity += fileSize;
            if ( detail.mFileNumber < 0 ) {
                detail.mFileNumber = 0;
            }
            if ( detail.mFileCapacity < 0 ) {
                detail.mFileCapacity = 0;
            }
            if ( database_->ModifyCartridge(barcode,detail) ) {
                return true;
            } else {
                LtfsLogError( "Failed to set cartridge to database: "
                        << barcode );
                return false;
            }
        } else {
            LtfsLogError( "Failed to get cartridge from database: "
                    << barcode );
            return false;
        }
    }

    bool
    TapeLibraryMgr::GetTapeFreeSize(const string & barcode, off_t &freesize)
    {
    	VS_DBG_LOG_FUNCTION;
		LtfsError error;
		CartridgeDetail detail;
		if ( database_->GetCartridge(barcode,detail) ) {
			if (detail.mStatus == TAPE_CLOSED || detail.mOffline || detail.mFaulty)
				freesize = 0;
			else {
				if (detail.mFreeCapacity > sizeMinTapeFree_)
					freesize = detail.mFreeCapacity - sizeMinTapeFree_;
				else
					freesize = 0;
			}
			return true;
		}else
		{
			LtfsLogError( "Failed to get cartridge detail from database: " << barcode );
		}

		return false;
    }
bool
    TapeLibraryMgr::GetActiveTape(string & barcode, const string & tapeGroup, off_t size)
    {
		VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexTape_);

        vector<string> barcodes;
        if ( ! database_->GetTapeGroupCartridgeList(tapeGroup,barcodes) ) {
            LtfsLogError( "Failed to get cartridges from database"
                    << " for tape group " << tapeGroup );
            return false;
        }

        barcode.clear();
        CartridgeDetail detail;
        for ( vector<string>::iterator i = barcodes.begin();
                i != barcodes.end();
                ++ i ) {
            if ( database_->GetCartridge(*i,detail) ) {
                if ( ! IsTapeWritable(detail) ) {
                    continue;
                }
                if ( detail.mStatus == TAPE_ACTIVE ) {
                    barcode = *i;
                    break;
                }
            } else {
                LtfsLogError("Failed to get cartridge from database: " << *i);
            }
        }
        if ( ! barcode.empty() ) {
            if ( CheckTapeGroupBuffer(tapeGroup,barcode,size) ) {
                return true;
            }
            if ( CheckTapeGroupTape(tapeGroup,barcode,size) ) {
                return true;
            }
            if ( database_->SetStatusForCartridge(barcode,TAPE_CLOSED) ) {
                LtfsEvent(EVENT_LEVEL_INFO, "Tape_Status_Closed", "Tape " << barcode << " is closed." );
            } else {
                LtfsLogError( "Failed to set cartridge close status to database: "
                        << barcode );
            }
        }

        for ( vector<string>::iterator i = barcodes.begin();
                i != barcodes.end();
                ++ i ) {
            if ( database_->GetCartridge(*i,detail) ) {
                if ( ! IsTapeWritable(detail) ) {
                    continue;
                }
                if ( detail.mStatus == TAPE_OPEN ) {
                    barcode = *i;
                    if ( detail.mFreeCapacity <= size ) {
                        LtfsLogError( "Too big file for tape "
                                << barcode << " size " << size );
                        continue;
                    }
                    detail.mStatus = TAPE_ACTIVE;
                    if ( ! database_->ModifyCartridge(*i,detail) ) {
                        LtfsLogError( "Failed to set cartridge to database: "
                                << barcode );
                    }
                    return true;
                }
            }
        }

        LtfsLogError( "No empty tape for tape group " << tapeGroup );
        return false;
    }

    bool
	TapeLibraryMgr::GetActiveTape(vector<string> & barcodes, const string & tapeGroup, off_t size)
	{
		VS_DBG_LOG_FUNCTION;

		boost::lock_guard<boost::mutex> lock(mutexTape_);

		vector<string> barcodeList;
		if ( ! database_->GetTapeGroupCartridgeList(tapeGroup,barcodeList) ) {
			LtfsLogError( "Failed to get cartridges from database"
					<< " for tape group " << tapeGroup );
			return false;
		}

		barcodes.clear();
		CartridgeDetail detail;
		CartridgeDetail dualTapeDetail;
		vector<CartridgeDetail> activeTapes;
		activeTapes.clear();
		for ( vector<string>::iterator i = barcodeList.begin();
				i != barcodeList.end();
				++ i ) {
			if ( database_->GetCartridge(*i,detail) ) {
				if ( ! IsTapeWritable(detail) ) {
					continue;
				}
				if ( detail.mStatus == TAPE_ACTIVE ) {

					if (database_->GetTapeGroupDualCopy(tapeGroup))
					{
						if (detail.mDualCopy == "")
						{
							LtfsLogError("Coupled Tape is gone for " << detail.mBarcode);
							continue;
						}
						if ( !database_->GetCartridge(detail.mDualCopy, dualTapeDetail)
								|| detail.mTapeGroupUUID != dualTapeDetail.mTapeGroupUUID) {
							LtfsLogError("Failed to get cartridge from database: " << detail.mDualCopy);
							continue;
						}

						if ( ! IsTapeWritable(detail) || dualTapeDetail.mStatus != TAPE_ACTIVE ) {
							LtfsLogError("One of the coupled Tapes cann't be write: " << detail.mBarcode);
							continue;
						}
						activeTapes.push_back(dualTapeDetail);
					}
					activeTapes.push_back(detail);
					break;
				}
			} else {
				LtfsLogError("Failed to get cartridge from database: " << *i);
			}
		}

		bool errorOcurred = false;
		if ( activeTapes.size() > 0 ) {
			for ( vector<CartridgeDetail>::iterator i = activeTapes.begin();
									i != activeTapes.end();
									++ i ) {
				if ( CheckTapeGroupBuffer(tapeGroup,i->mBarcode,size) ) {
					continue;
				}
				if ( !CheckTapeGroupTape(tapeGroup,i->mBarcode,size) ) {
					errorOcurred = true;
					break;
				}
			}

			if (errorOcurred)
			{
				for ( vector<CartridgeDetail>::iterator i = activeTapes.begin();
													i != activeTapes.end();
													++ i )  {
					if ( database_->SetStatusForCartridge(i->mBarcode,TAPE_CLOSED) ) {
						LtfsEvent(EVENT_LEVEL_INFO, "Tape_Status_Closed", "Tape " << i->mBarcode << " is closed." );
					} else {
						LtfsLogError( "Failed to set cartridge close status to database: "
								<< i->mBarcode );
					}
				}
				activeTapes.clear();
			}else
			{
				if (activeTapes.size() == 1){
					barcodes.push_back(activeTapes[0].mBarcode);
				}else {
					if (activeTapes[0].mMediaType == activeTapes[1].mMediaType) {
						barcodes.push_back(activeTapes[0].mBarcode);
						barcodes.push_back(activeTapes[1].mBarcode);
						sort(barcodes.begin(), barcodes.end());
					}else if (activeTapes[0].mMediaType > activeTapes[1].mMediaType)					{
						barcodes.push_back(activeTapes[0].mBarcode);
						barcodes.push_back(activeTapes[1].mBarcode);
					}else {
						barcodes.push_back(activeTapes[1].mBarcode);
						barcodes.push_back(activeTapes[0].mBarcode);
					}
					//sort(barcodes.begin(), barcodes.end());
				}
				return true;
			}
		}

		for ( vector<string>::iterator i = barcodeList.begin();
				i != barcodeList.end();
				++ i ) {
			if ( database_->GetCartridge(*i,detail) ) {
				if ( ! IsTapeWritable(detail) ) {
					continue;
				}
				if ( detail.mStatus == TAPE_OPEN && detail.mFreeCapacity > size) {
					if (database_->GetTapeGroupDualCopy(tapeGroup))
					{
						if (detail.mDualCopy == "")
						{
							LtfsLogError("Coupled Tape is gone for " << detail.mBarcode);
							continue;
						}
						if ( !database_->GetCartridge(detail.mDualCopy,dualTapeDetail) ) {
							continue;
						}
						if ( ! IsTapeWritable(dualTapeDetail) ) {
							continue;
						}
						if ( detail.mStatus != TAPE_OPEN || detail.mFreeCapacity <= size) {
							continue;
						}
						dualTapeDetail.mStatus = TAPE_ACTIVE;
						if ( ! database_->ModifyCartridge(dualTapeDetail.mBarcode,dualTapeDetail) ) {
							LtfsLogError( "Failed to set cartridge to database: "
									<< dualTapeDetail.mBarcode );
						}
						activeTapes.push_back(dualTapeDetail);
					}
					detail.mStatus = TAPE_ACTIVE;
					if ( ! database_->ModifyCartridge(*i,detail) ) {
						LtfsLogError( "Failed to set cartridge to database: "
								<< *i );
					}
					activeTapes.push_back(detail);
					if (activeTapes.size() == 1){
						barcodes.push_back(activeTapes[0].mBarcode);
					}else {
						if (activeTapes[0].mMediaType == activeTapes[1].mMediaType) {
							barcodes.push_back(activeTapes[0].mBarcode);
							barcodes.push_back(activeTapes[1].mBarcode);
							sort(barcodes.begin(), barcodes.end());
						}else if (activeTapes[0].mMediaType > activeTapes[1].mMediaType)					{
							barcodes.push_back(activeTapes[0].mBarcode);
							barcodes.push_back(activeTapes[1].mBarcode);
						}else {
							barcodes.push_back(activeTapes[1].mBarcode);
							barcodes.push_back(activeTapes[0].mBarcode);
						}
					}
					return true;
				}
			}
		}

		LtfsLogError( "No empty tape for tape group " << tapeGroup );
		return false;
	}


    bool TapeLibraryMgr:: IsTapeInDrive(const string& barcode)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer, drive;
    	int slotId;
    	if(!FindTape(barcode, changer, drive, slotId) || drive == ""){
    		return false;
    	}

    	return true;
    }

    bool
    TapeLibraryMgr::FindTape(
            const string & barcode,
            string & serialChanger,
            string & serialDrive,
            int & slotID )
    {
		VS_DBG_LOG_FUNCTION;
        serialChanger.clear();
        serialDrive.clear();
        slotID = -1;

        LtfsError lfsErr;
        vector<LtfsChangerInfo> changers;
        if( ! hal_->GetChangerList(changers,lfsErr) ) {
            LtfsLogError("Failed to get changes from HAL");
            return false;
        }

        for ( vector<LtfsChangerInfo>::iterator i = changers.begin();
                i != changers.end();
                ++ i ) {
            vector<LtfsTapeInfo> tapes;
            if( ! hal_->GetTapeList(
                    i->mSerial, tapes, lfsErr ) ) {
                LtfsLogError("Failed to get tapes from HAL: " << i->mSerial);
                continue;
            }
            for(vector<LtfsTapeInfo>::iterator iTape = tapes.begin();
                    iTape != tapes.end();
                    ++ iTape ) {
                if ( iTape->mBarcode == barcode ) {
                    serialChanger = i->mSerial;
                    slotID = iTape->mSlotID;
                }
            }
            vector<LtfsDriveInfo> drives;
            if( ! hal_->GetDriveList(
                    i->mSerial, drives, lfsErr ) ) {
                LtfsLogError("Failed to get drives from HAL: " << i->mSerial);
                continue;
            }
            for(vector<LtfsDriveInfo>::iterator iDrive = drives.begin();
                    iDrive != drives.end();
                    ++ iDrive ) {
                string barcodeDrive;
                if ( hal_->GetLoadedTapeBarcode(
                        i->mSerial, iDrive->mSerial, barcodeDrive, lfsErr ) ) {
                    if ( barcodeDrive == barcode ) {
                        serialChanger = i->mSerial;
                        serialDrive = iDrive->mSerial;
                    }
                } else {
                    LtfsLogDebug("Failed to get barcode from HAL: "
                            << i->mSerial << " : " << iDrive->mSerial << ". Error:" << lfsErr.GetErrMsg());
                }
            }
        }

        if ( ! serialChanger.empty() && slotID >= 0 ) {
            return true;
        } else {
            LtfsLogDebug("Failed to get barcode from HAL: " << barcode << " : " << slotID << ":" << serialChanger.size());
            return false;
        }
    }

    bool
    TapeLibraryMgr::AssignToTapeGroup(
            const string & barcode, const string & tapeGroup, const string& groupName, int status)
    {
		VS_DBG_LOG_FUNCTION;
        CartridgeDetail detail;
        if ( ! database_->GetCartridge(barcode,detail) ) {
            LtfsLogError("Failed to get cartridge from database: " << barcode);
            return false;
        }

        LtfsError error;
        string changer,drive;
        int slotID;
        if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
            LtfsLogError( "Failed to find drive to for tape " << barcode );
            return false;
        }
        if ( ! UnmountTape(barcode) ) {
            LtfsLogError( "Failed to umount drive to for tape " << barcode );
            return false;
        }
        if ( ! HalSetLoadedTapeGroup(
        		barcode, tapeGroup, error ) ) {
            LtfsLogError( "Failed to set tape group in HAL: " << barcode );
            return false;
        }
        if ( ! HalSetLoadedTapeStatus(
        		barcode, status, error ) ) {
            LtfsLogError( "Failed to set tape status in HAL: " << barcode );
            return false;
        }
        Int64_t totalCapacity;
        if ( ! GetLoadedTapeCapacity(
        		barcode,detail.mFreeCapacity,totalCapacity,error)){
            LtfsLogError( "Failed to get tape capacity in HAL: " << barcode );
            return false;
        }

        boost::lock_guard<boost::mutex> lock(mutexTape_);

        detail.mTapeGroupUUID = tapeGroup;
        detail.mStatus = status;
        detail.mFormat = LTFS_VALID;
        detail.mUsedCapacity = totalCapacity - detail.mFreeCapacity;
        detail.mFileNumber = 0;
        detail.mFileCapacity = 0;
        if ( ! database_->ModifyCartridge(barcode,detail) ) {
            LtfsLogError("Failed to set cartridge to database: " << barcode);
            return false;
        }

        vector<string> groups;
        if ( ! database_->GetTapeGroupList(groups) ) {
            LtfsLogError("Failed to get tape groups from database");
            return true;
        }

        vector<string>::iterator i = find(groups.begin(),groups.end(),tapeGroup);
        if ( groups.end() == i ) {
            if ( ! database_->AddTapeGroup(tapeGroup, groupName) ) {
                LtfsLogError("Failed to add tape group to database: "
                        << tapeGroup);
            }
        }

        return true;
    }

    TapeInfo TapeLibraryMgr::ConvertTape(const CartridgeDetail& detail)
    {
		VS_DBG_LOG_FUNCTION;
    	TapeInfo tape;
        tape.mBarcode = detail.mBarcode;
		tape.mGroupID = detail.mTapeGroupUUID;
		tape.mStatus = detail.mStatus;
		tape.mMediaType = detail.mMediaType;
		tape.mMediumType = detail.mMediumType;
		tape.mLtfsFormat = detail.mFormat;
		tape.mGenerationIndex = detail.mGenerationNumber;
		tape.mLoadCount = detail.mLoadCount;
		tape.mTotalCapacity = detail.mUsedCapacity+detail.mFreeCapacity;
		tape.mFreeCapacity = detail.mFreeCapacity;
		tape.mFileNumber = detail.mFileNumber;
		tape.mFileCapacity = detail.mFileCapacity;
		tape.mFaulty = detail.mFaulty;
		tape.mState = detail.mState;
		tape.mActivity = detail.mActivity;
		tape.mWriteProtect = detail.mWriteProtect;
		tape.mOffline = detail.mOffline;
        tape.mDualCopy = detail.mDualCopy;
        tape.mTapeUUID = detail.mTapeUUID;
        tape.mLastMountTime = detail.mLastMountTime;
        tape.mLastAuditTime = detail.mLastAuditTime;

        return tape;
    }

    bool
    TapeLibraryMgr::GetTape(TapeInfo & tape,const LtfsTapeInfo & info)
    {
		VS_DBG_LOG_FUNCTION;
        CartridgeDetail detail;
        tape.mBarcode = info.mBarcode;
        tape.mSlotID = info.mSlotID;
        tape.mMediumType = info.mMediumType;
        tape.mStatus = TAPE_UNKNOWN;
        tape.mMediaType = MEDIA_UNKNOWN;
        tape.mLtfsFormat = LTFS_UNKNOWN;
        tape.mGenerationIndex = 0;
        tape.mLoadCount = 0;
        tape.mTotalCapacity = -1;
        tape.mFreeCapacity = -1;
        tape.mFileNumber = 0;
        tape.mFileCapacity = 0;
        tape.mFaulty = false;
        tape.mState = TAPE_STATE_IDLE;
        tape.mActivity = 0;
        tape.mWriteProtect = false;
        tape.mOffline = false;
        tape.mDualCopy = "";
        tape.mTapeUUID = "";
        tape.mLastMountTime = 0;
        tape.mLastAuditTime = 0;
        if ( database_->GetCartridge(info.mBarcode,detail) ) {
            tape.mGroupID = detail.mTapeGroupUUID;
            tape.mStatus = detail.mStatus;
            tape.mMediaType = detail.mMediaType;
            if(tape.mMediumType == MEDIUM_UNKNOWN){
            	tape.mMediumType = detail.mMediumType;
            }
            tape.mLtfsFormat = detail.mFormat;
            tape.mGenerationIndex = detail.mGenerationNumber;
            tape.mLoadCount = detail.mLoadCount;
            tape.mTotalCapacity = detail.mUsedCapacity+detail.mFreeCapacity;
            tape.mFreeCapacity = detail.mFreeCapacity;
            tape.mFileNumber = detail.mFileNumber;
            tape.mFileCapacity = detail.mFileCapacity;
            tape.mFaulty = detail.mFaulty;
            tape.mState = detail.mState;
            tape.mActivity = detail.mActivity;
            tape.mWriteProtect = detail.mWriteProtect;
            tape.mOffline = detail.mOffline;
            tape.mDualCopy = detail.mDualCopy;
            tape.mTapeUUID = detail.mTapeUUID;
            tape.mLastMountTime = detail.mLastMountTime;
            tape.mLastAuditTime = detail.mLastAuditTime;
        } else {
            LtfsLogInfo("Failed to get cartridge from database: " << tape.mBarcode << ", maybe the tape is not inventoried yet.");
            database_->GetActivityForCartridge(tape.mBarcode, tape.mActivity);
        }
        return true;
    }

    bool TapeLibraryMgr::GetAllTapeList(vector<TapeInfo> & tapes, LtfsError & error)
    {
		VS_DBG_LOG_FUNCTION;
    	vector<LtfsChangerInfo> changers;
    	if(!GetChangerList(changers, error)){
    		return false;
    	}

    	map<string, bool> foundMap;
    	tapes.clear();
    	for(unsigned int i = 0; i < changers.size(); i++){
    		vector<TapeInfo> tmpTapes;
    		if(GetTapeListForChanger(changers[i].mSerial, tmpTapes, error)){
				for(unsigned int j = 0 ; j < tmpTapes.size(); j++){
					tapes.push_back(tmpTapes[j]);
					foundMap[tapes[j].mBarcode] = true;
				}
    		}
    	}

    	// check if any tapes not visible by HAL but in database
    	vector<CartridgeDetail> cartridgeList;
    	if(!database_->GetCartridgeList(cartridgeList)){
    		return false;
    	}
    	for(unsigned int i = 0; i < cartridgeList.size(); i++){
    		if(foundMap.find(cartridgeList[i].mBarcode) == foundMap.end()){
    			tapes.push_back(ConvertTape(cartridgeList[i]));
    			foundMap[cartridgeList[i].mBarcode] = true;
    		}
    	}

    	return true;
    }

    bool
    TapeLibraryMgr::GetTapeListForChanger(
            const string & changer, vector<TapeInfo> & tapes,
            LtfsError & error)
    {
		VS_DBG_LOG_FUNCTION;
        tapes.clear();

        vector<LtfsTapeInfo> infos;
        if (! hal_->GetTapeList(changer,infos,error)) {
            LtfsLogError("Failed to get changes from HAL");
            return false;
        }

        for ( vector<LtfsTapeInfo>::iterator i = infos.begin();
                i != infos.end();
                ++ i ) {
            TapeInfo tape;
            GetTape(tape,*i);
            tapes.push_back(tape);
        }

        return true;
    }


    bool
    TapeLibraryMgr::GetSlotListForChanger(
            const string & changer,
            vector<LtfsSlotInfo> & slots,
            LtfsError & error )
    {
		VS_DBG_LOG_FUNCTION;
        slots.clear();

        if (!hal_->GetSlotList(changer,slots,error)) {
            LtfsLogError("Failed to get slots from HAL: " << changer);
            return false;
        }

        return true;
    }


    bool
    TapeLibraryMgr::GetMailSlotListForChanger(
            const string & changer,
            vector<LtfsMailSlotInfo> & slots,
            LtfsError & error)
    {
		VS_DBG_LOG_FUNCTION;
        slots.clear();

        if (!hal_->GetMailSlotList(changer,slots,error)){
            LtfsLogError("Failed to get mailslots from HAL: " << changer);
            return false;
        }

        return true;
    }

    LtfsDriveInfo TapeLibraryMgr::ConvertDriveInfo(const DriveInfo& driveInfo)
    {
		VS_DBG_LOG_FUNCTION;
    	LtfsDriveInfo retInfo;
    	retInfo.mDriveName = driveInfo.mDriveName;
    	retInfo.mSerial = driveInfo.mDriveSerial;

    	return retInfo;
    }

    bool TapeLibraryMgr::GetDriveInfoForTape(const string& barcode, LtfsDriveInfo& driveInfo)
    {
    	return hal_->GetDriveInfoForTape(barcode, driveInfo);
    }

    bool
    TapeLibraryMgr::GetDriveListForChanger(
            const string & changer,
            vector<LtfsDriveInfo> & drives,
            LtfsError & error)
    {
		VS_DBG_LOG_FUNCTION;
        drives.clear();

        if (!hal_->GetDriveList(changer,drives,error)){
            LtfsLogError("Failed to get drives from HAL: " << changer );
            return false;
        }
        return true;
    }


    bool
    TapeLibraryMgr::GetChangerList(
            vector<LtfsChangerInfo> & changers, LtfsError & error )
    {
		VS_DBG_LOG_FUNCTION;
        changers.clear();

        if ( ! hal_->GetChangerList(changers,error) ) {
            LtfsLogError("Failed to get changers from HAL");
            return false;
        }

        return true;
    }


    bool
    TapeLibraryMgr::DeleteTapeGroup(const string & tapeGroup)
    {
		VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexTape_);

        vector<string> barcodes;
        if ( database_->GetTapeGroupCartridgeList(tapeGroup,barcodes) ) {
            for ( vector<string>::iterator i = barcodes.begin();
                    i != barcodes.end();
                    ++ i ) {
                CartridgeDetail detail;
                if ( ! database_->GetCartridge(*i,detail) ) {
                    LtfsLogError("Failed to get cartridge from database: "
                            << *i);
                    return false;
                }
                detail.mTapeGroupUUID.clear();
                detail.mStatus = TAPE_UNKNOWN;
                if ( ! database_->ModifyCartridge(*i,detail) ) {
                    LtfsLogError("Failed to set cartridge to database: " << *i);
                    return false;
                }
            }
        } else {
            LtfsLogError("Failed to get tapes from database for tape group "
                    << tapeGroup);
        }

        return database_->DeleteTapeGroup(tapeGroup);
    }

    bool TapeLibraryMgr::UpdateTapeWPFlag(const string & changer, const string & drive, LtfsError & error)
    {
		VS_DBG_LOG_FUNCTION;

        LtfsLogDebug("Update tape for drive: " << changer << " : " << drive );

        string barcode;
        if ( ! hal_->GetLoadedTapeBarcode(
                changer, drive, barcode, error ) ) {
            LtfsLogError("Failed to get barcode for drive from HAL: "
                    << changer << " : " << drive );
            return false;
        }
        if ( barcode.empty() ) {
            LtfsLogDebug("No tape in drive: "
                    << changer << " : " << drive );
            return false;
        }

		bool writeProtect = false;
		if (false == GetLoadedTapeWPFlag(barcode, writeProtect, error) ) {
			LtfsLogError("Failed to get tape write protect for drive from HAL, barcode = "
					<< barcode );
			return false;
		}

        if ( ! database_->SetWriteProtectForCartridge(barcode, writeProtect) ) {
            LtfsLogError("Failed to update write protect flag for tape " << barcode << " to " << writeProtect << ".");
            return false;
        }

        return true;
    }


    bool
    TapeLibraryMgr::UpdateTape(
            const string & changer, const string & drive, LtfsError & error)
    {
		VS_DBG_LOG_FUNCTION;
        return UpdateTapeUnlock(changer,drive,error);
    }

    bool
    TapeLibraryMgr::UpdateTapeUnlock(
            const string & changer, const string & drive, LtfsError & error, bool bCheckWP, bool bForce)
    {
		VS_DBG_LOG_FUNCTION;

        LtfsLogDebug("Update tape for drive: " << changer << " : " << drive );

        string barcode;
        if ( ! hal_->GetLoadedTapeBarcode(
                changer, drive, barcode, error, true ) ) {
            LtfsLogError("Failed to get barcode for drive from HAL: "
                    << changer << " : " << drive );
            return false;
        }
        if ( barcode.empty() ) {
            LtfsLogDebug("No tape in drive: "
                    << changer << " : " << drive );
            return false;
        }
        LtfsLogDebug("UpdateTapeUnlock, tape = " << barcode);

        bool bNeedUpdateWp = bCheckWP;
        ltfs_management::CartridgeDetail detail;
        if ( false == bForce && database_->GetCartridge(barcode,detail) ) {
            LtfsLogDebug("tape already exists. " << barcode);
        } else {
        	bNeedUpdateWp = true;

            detail.mStatus = TAPE_UNKNOWN;
            int status;
            if ( GetLoadedTapeStatus(barcode,status,error) ) {
                detail.mStatus = status;
            } else {
                LtfsLogError("Failed to get tape status for drive from HAL: "
                        << changer << " : " << drive );
            }

            detail.mMediaType = MEDIA_UNKNOWN;
            int mediaType;
            if ( GetLoadedTapeMediaType(barcode,mediaType,error) ) {
                detail.mMediaType = mediaType;
            } else {
                LtfsLogError("Failed to get tape media type for drive from HAL: "
                        << changer << " : " << drive );
            }

            detail.mLoadCount = 0;
            int loadCount;
            if ( GetLoadedTapeLoadCounter(barcode,loadCount,error) ) {
                detail.mLoadCount = loadCount;
            } else {
                LtfsLogError("Failed to get tape load count for drive from HAL: "
                        << changer << " : " << drive );
            }

            detail.mGenerationNumber = 0;
            long long generationNumber;
            if ( GetLoadedTapeGenerationIndex(barcode,generationNumber,error) ) {
                detail.mGenerationNumber = generationNumber;
            } else {
                LtfsLogError("Failed to get tape generation index for drive from HAL: "
                        << changer << " : " << drive );
            }

            detail.mFormat = LTFS_UNKNOWN;
            int format;
            if ( GetLoadedTapeLtfsFormat(barcode,format,error) ) {
                detail.mFormat = format;
            } else {
                LtfsLogError("Failed to get tape ltfs format for drive from HAL: "
                        << changer << " : " << drive << ":" << barcode);
            }

            detail.mFaulty = false;
            bool faulty = false;
            if ( GetLoadedTapeFaulty(barcode,faulty,error) ) {
                detail.mFaulty = faulty;
            } else {
                LtfsLogError("Failed to get tape faulty for drive from HAL: "
                        << changer << " : " << drive );
            }

            detail.mTapeGroupUUID = "";
            string tapeGroupUuid = "";
            if ( GetLoadedTapeGroup(barcode,tapeGroupUuid,error) ) {
                detail.mTapeGroupUUID = tapeGroupUuid;
            } else {
                LtfsLogError("Failed to get tape group uuid for tape in drive from HAL: "
                        << changer << " : " << drive );
            }

			detail.mDualCopy = "";
			string dualCopy = "";
            if ( GetLoadedTapeDualCopy(barcode, dualCopy, error) ) {
                detail.mDualCopy = dualCopy;
                LtfsLogDebug("detail.mDualCopy = " << detail.mDualCopy << ", barcode = " << barcode);
            } else {
                LtfsLogError("Failed to get dual copy for tape in drive from HAL: "
                        << changer << " : " << drive << " : " << barcode);
            }

			detail.mTapeUUID = "";
			string tapeUUID = "";
            if ( GetLoadedTapeUUID(barcode, tapeUUID, error) ) {
                detail.mTapeUUID = tapeUUID;
                LtfsLogDebug("detail.mTapeUUID = " << detail.mTapeUUID << ", barcode = " << barcode);
            } else {
                LtfsLogError("Failed to get tape UUID for tape in drive from HAL: "
                        << changer << " : " << drive << " : " << barcode);
            }

            detail.mUsedCapacity = 0;
            detail.mFreeCapacity = 0;
            detail.mFileNumber = 0;
            detail.mFileCapacity = 0;
            detail.mState = TAPE_STATE_IDLE;
            detail.mActivity = 0;
            detail.mOffline = false;
			detail.mWriteProtect = false;
			detail.mLastMountTime = 0;
			detail.mLastAuditTime = 0;
        }

        long long freecapacity;
        long long totalcapacity;
        if ( GetLoadedTapeCapacity(barcode,freecapacity,totalcapacity,error) ) {
            detail.mFreeCapacity = freecapacity;
            detail.mUsedCapacity = totalcapacity - freecapacity;
            LtfsLogDebug("freecapacity = " << freecapacity << ", totalcapacity = " << totalcapacity);
        } else {
            LtfsLogError("Failed to get tape capacity for drive from HAL: "
                    << changer << " : " << drive );
        }

        if(bNeedUpdateWp){
			bool writeProtect = false;
			if ( GetLoadedTapeWPFlag(barcode,writeProtect,error) ) {
				detail.mWriteProtect = writeProtect;
			} else {
				LtfsLogError("Failed to get tape write protect for drive from HAL: "
						<< changer << " : " << drive );
			}
        }

        detail.mBarcode = barcode;
        if ( ! database_->ModifyCartridge(barcode,detail) ) {
            LtfsLogError("Failed to set cartridge to database: " << barcode );
            return false;
        }

        return true;
    }


    bool TapeLibraryMgr::RequestMoveLock(const string& changerSerial, int millTimeOut)
    {
		VS_DBG_LOG_FUNCTION;
    	return hal_->RequestMoveLock(changerSerial, millTimeOut);
    }
    void TapeLibraryMgr::ReleaseMoveLock(const string& changerSerial)
    {
		VS_DBG_LOG_FUNCTION;
    	hal_->ReleaseMoveLock(changerSerial);
    }

    bool
    TapeLibraryMgr::RequestTape(
            const string & barcode, bool mount, int priority, int timeout)
    {
		VS_DBG_LOG_FUNCTION;
    	LtfsLogDebug("DDEBUG: TapeLibraryMgr::RequestTape  " << barcode);
    	bool busy;
    	return RequestTape(barcode, mount, priority, timeout, busy);
    }

    bool
    TapeLibraryMgr::RequestTape(
            const string & barcode, bool mount, int priority, int timeout, bool & busy )
    {
		VS_DBG_LOG_FUNCTION;
    	TapeInfo tapeInfo;
    	if(!GetTape(barcode, tapeInfo) || tapeInfo.mOffline){
    		LtfsLogWarn("RequestTape: tape " << barcode << " not found or offline.");
    		return false;
    	}
    	LtfsLogDebug("DDEBUG: TapeLibraryMgr::RequestTape  " << barcode);
        bdt::SchedulePriorityTape * schedule = static_cast<bdt::SchedulePriorityTape*>(bdt::Factory::GetSchedule());
		bool ret = schedule->RequestTape(barcode, mount, false, timeout*1000, priority);
        busy = false;
        if ( ! ret ) {
            busy = schedule->IsTapeBusy(barcode);
        }
        LtfsLogDebug("TapeLibraryMgr::RequestTape  ret = " << ret << ", barcode = " << barcode);
        return ret;
    }


    bool
    TapeLibraryMgr::ReleaseTape(const string & barcode)
    {
		VS_DBG_LOG_FUNCTION;
    	LtfsLogDebug("DDEBUG: TapeLibraryMgr::ReleaseTape.." << barcode);
        bdt::SchedulePriorityTape * schedule = static_cast<bdt::SchedulePriorityTape*>(bdt::Factory::GetSchedule());
        schedule->ReleaseTape(barcode, false );
        return true;
//#endif
    }

    bool TapeLibraryMgr::RequestTapes(const vector<string>& barcodes, bool mount, int priority, int timeout)
    {
		VS_DBG_LOG_FUNCTION;
    	LtfsLogDebug("DDEBUG: TapeLibraryMgr::RequestTapes  " << barcodes.size());
        bdt::SchedulePriorityTape * schedule = static_cast<bdt::SchedulePriorityTape*>(bdt::Factory::GetSchedule());
		bool ret = schedule->RequestTapes(barcodes, mount, false, timeout*1000, priority);
        LtfsLogDebug("TapeLibraryMgr::RequestTapes  ret = " << ret << ", barcodes.size() = " << barcodes.size());
        return ret;
    }
    bool TapeLibraryMgr::ReleaseTapes(const vector<string>& barcodes)
    {
		VS_DBG_LOG_FUNCTION;
    	LtfsLogDebug("DDEBUG: TapeLibraryMgr::ReleaseTapes.." << barcodes.size());
        bdt::SchedulePriorityTape * schedule = static_cast<bdt::SchedulePriorityTape*>(bdt::Factory::GetSchedule());
        schedule->ReleaseTapes(barcodes, false );
        return true;
    }

    bool
    TapeLibraryMgr::OpenMailSlot(
            const string & changer, int slotID, LtfsError & error )
    {
		VS_DBG_LOG_FUNCTION;
        return hal_->OpenMailSlot(changer,slotID,error);
    }

    bool
    TapeLibraryMgr::OpenMailSlot()
    {
		VS_DBG_LOG_FUNCTION;
		bool bRet = false;
		vector<LtfsChangerInfo> changers;
		LtfsError lfsErr;
		if(!GetChangerList(changers, lfsErr)){
			LtfsLogError("Failed to get changer list to open mail slot.");
			return bRet;
		}
		for(unsigned int i = 0; i < changers.size(); i++){
			string changerSerial = changers[i].mSerial;
			vector<LtfsMailSlotInfo>mailSlots;
			if(!hal_->GetChangerMailSlots(changerSerial, mailSlots, lfsErr)){
				LtfsLogError("Failed to get mail slot info for changer " << changerSerial);
				continue;
			}
			for(unsigned int j = 0; j < mailSlots.size(); j++){
				if(mailSlots[i].mIsOpen || hal_->OpenMailSlot(changerSerial, mailSlots[i].mSlotID, lfsErr)){
					bRet = true;
				}
			}
		}
        return bRet;
    }

    bool
    TapeLibraryMgr::MoveCartridge(const string& barcode, int dstSlot, LtfsError& error)
    {
		VS_DBG_LOG_FUNCTION;
    	string driveSerial = "";
    	string changerSerial = "";
    	int srcSlot = -1;
    	if ( !FindTape(barcode, changerSerial, driveSerial, srcSlot)){
    		LtfsLogError("Failed to move tape " << barcode << ". No tape found.");
    		return false;
    	}

    	return MoveCartridge(changerSerial, srcSlot, dstSlot, error, barcode);
    }


	bool TapeLibraryMgr::MoveCartridge(const string& changerSerial, int srcSlot, int dstSlot, LtfsError& error, const string& barcode)
	{
		VS_DBG_LOG_FUNCTION;
		bool bRet = false;
    	string srcDriveSerial = hal_->GetDriveSerialBySlotId(changerSerial, srcSlot);
    	string dstDriveSerial = hal_->GetDriveSerialBySlotId(changerSerial, dstSlot);
		LtfsLogDebug("MoveCartridge start: " << barcode << ":" << srcSlot << ":" << dstSlot << ":" << changerSerial
				<< ":" << srcDriveSerial << ":" << dstDriveSerial);
    	if(srcDriveSerial == "" && dstDriveSerial != ""){
    		SetTapeActivity(barcode, ACT_LOADING);
    	}else if(srcDriveSerial != "" && dstDriveSerial == ""){
    		SetTapeActivity(barcode, ACT_UNLOADING);
    	}else if(srcSlot != dstSlot){
    		SetTapeActivity(barcode, ACT_MOVING);
    	}else if(srcSlot == dstSlot){
    		LtfsLogInfo("MoveCartridge: srcSlot = " << srcSlot << ", dstSlot = " << dstSlot << ". Source slot equals dest slot, no need to move.");
    		return true;
    	}

    	if(hal_->MoveCartridge(changerSerial, srcSlot, dstSlot, error)){
    		SetTapeActivity(barcode, ACT_IDLE);
    		MarkTapeInDriveFlag(barcode, srcDriveSerial, dstDriveSerial);
        	// drive cleaning check
			for(unsigned int i = 0; i < 2; i++){
				if( (i == 0 && srcDriveSerial == "")
					|| (i == 1 && dstDriveSerial == "")
				){
					continue;
				}
				string driveSerial = (i == 0)? srcDriveSerial : dstDriveSerial;
	            int cleaning;
	            if ( hal_->GetDriveCleaningStatus(changerSerial, driveSerial, cleaning, error, true ) ) {
	            	LtfsLogDebug("driveSerial = " << driveSerial << ", cleaning = " << cleaning << ", CLEANING_REQUIRED = " << CLEANING_REQUIRED << ".");
                	// check if we need to fire the event
	            	hal_->CheckDriveCleaningEvent(changerSerial, driveSerial, cleaning);
	                if ( CLEANING_REQUIRED == cleaning ) {
	                    StartDriveCleaningTask(changerSerial, driveSerial);
	                }
	            }
			}
			// load a tape, check if need to update info
			if(dstDriveSerial != "" && barcode != ""){
				int format;
				if (GetLoadedTapeLtfsFormat(barcode, format, error)
						&& (format == LTFS_VALID) ) {
				} else {
					UpdateTape(changerSerial, dstDriveSerial, error);
				}
	            UpdateTapeWPFlag(changerSerial, dstDriveSerial, error);
			}
			bRet = true;
    	}else{
    		LtfsLogDebug("MoveCartridge  failed, ERR: " << error.GetErrMsg() << ".  " << barcode << ":" << srcSlot << ":" << dstSlot << ":" << changerSerial);
    		SetTapeActivity(barcode, ACT_IDLE);
            switch ( error.GetErrCode() ) {
                case ERR_MOVE_DST_FULL:
                	if(barcode != ""){
                		int srcLogicSlot = srcSlot;
                		int dstLogicSlot = dstSlot;
						LtfsEvent(EVENT_LEVEL_ERR, "Tape_Move_Failed", "Failed to move tape " << barcode << " from "
								<< srcLogicSlot << " to " << dstLogicSlot
								<< ". Destination slot is not empty.");//Event/Notification
                	}
                    hal_->Refresh(error);
                    break;
                case ERR_MOVE_SRC_EMPTY:
                case ERR_MAIL_SLOT_OPEN:
                case ERR_CHANGER_TRANSFER_FULL:
                    hal_->Refresh(error);
                    break;
                default:
                    break;
            }
            bRet = false;
    	}

		LtfsLogDebug("MoveCartridge end: " << barcode << ":" << srcSlot << ":" << dstSlot << ":" << changerSerial
				<< ":" << srcDriveSerial << ":" << dstDriveSerial);
    	return bRet;
    }

    bool
    TapeLibraryMgr::RefreshChangerMailSlots(
            const string& changerSerial, vector<LtfsMailSlotInfo>& mailSlots, LtfsError& lfsErr)
    {
		VS_DBG_LOG_FUNCTION;
        return hal_->GetChangerMailSlots(changerSerial, mailSlots, lfsErr);
    }

	bool TapeLibraryMgr::IsDriveMounted(const string& changerSerial, const string& driveSerial, bool& mounted, LtfsError& ltfsErr)
	{
		VS_DBG_LOG_FUNCTION;
		return hal_->IsTapeMounted(changerSerial, driveSerial, mounted, ltfsErr);
	}

	bool TapeLibraryMgr::UnFormatTape(const string& barcode)
	{
		VS_DBG_LOG_FUNCTION;
		LtfsError lfsErr;
		int slotID = 0;
		string changerSerial = "";
		string driveSerial = "";
		if(!FindTape(barcode, changerSerial, driveSerial, slotID)){
			LtfsLogError("Tape not found, will not unformat it.");
			return false;
		}

		// unmount the tape first
		UInt64_t index = 0;
		if(false == HalUnmountTape(barcode, index, lfsErr)){
			LtfsLogWarn("Failed to unmount tape " << barcode << " to unformat it: " << lfsErr.GetErrMsg());
		}

		return hal_->UnFormat(changerSerial, driveSerial, lfsErr);
	}

    bool
    TapeLibraryMgr::CleanTape(const string & tapeGroup, const string & tape, bool bExport)
    {
		VS_DBG_LOG_FUNCTION;
        int handle = GetClientHandle(tapeGroup);
        if ( handle < 0 ) {
            LtfsLogError( "GetHandle: " << tapeGroup << " : " << tape);
            return false;
        }

        database_->SetFileCapacityForCartridge(tape,0);
        database_->SetFileNumberForCartridge(tape,0);

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(tape));
        params.add(xmlrpc_c::value_boolean(bExport));
        xmlrpc_c::rpc rpc(bdt::ServiceServer::ReleaseTape,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LtfsLogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                ret = xmlrpc_c::value_boolean(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LtfsLogError( e.what() );
        }

        close(handle);

        return ret;
    }


    int
    TapeLibraryMgr::GetClientHandle(const string & tapeGroup,int retry)
    {
		VS_DBG_LOG_FUNCTION;
        do {
            int handle = bdt::Factory::SocketClientHandle(
                    bdt::ServiceServer::Service + tapeGroup );
            if ( handle >= 0 ) {
                return handle;
            }
            LtfsLogDebug( "GetHandle: " << tapeGroup);
            boost::this_thread::sleep(boost::posix_time::seconds(1));
        } while ( retry -- );
        return -1;
    }


    bool
    TapeLibraryMgr::SetClientName(const string & tapeGroup, const string & name)
    {
		VS_DBG_LOG_FUNCTION;
        int handle = GetClientHandle(tapeGroup);
        if ( handle < 0 ) {
            LtfsLogError( "GetHandle: " << tapeGroup << " : " << name);
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(name));
        xmlrpc_c::rpc rpc(bdt::ServiceServer::SetName,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LtfsLogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                ret = xmlrpc_c::value_boolean(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LtfsLogError( e.what() );
        }

        close(handle);

        return ret;
    }

    bool
    TapeLibraryMgr::CanWriteCache()
    {
		VS_DBG_LOG_FUNCTION;
        static off_t freeSize = bdt::Factory::GetConfigure()->GetValueSize(
                bdt::Configure::WriteCacheFreeSize );

        fs::path cache = Factory::GetCacheFolder();

        struct statfs stat;
        if ( 0 != statfs(cache.string().c_str(),&stat) ) {
            LtfsLogWarn(cache);
            return false;
        }

        off_t freeCapacity = (off_t)stat.f_bsize * stat.f_bfree;

        if ( freeCapacity < freeSize ) {
            LogWarn(freeCapacity);
            return false;
        }

        return true;
    }


    bool
    TapeLibraryMgr::GetWriteCacheAction(int & action)
    {
		VS_DBG_LOG_FUNCTION;
        static int writeCachePercent = bdt::Factory::GetConfigure()->GetValueSize(
                bdt::Configure::WriteCachePercent );

        vector<string> groups;
        if ( ! database_->GetTapeGroupList(groups) ) {
            LtfsLogError("Failure to get tape group list");
            return false;
        }

        off_t sizeTotal = 0;
        BOOST_FOREACH( const string & group, groups ) {
            int handle = GetClientHandle(group,0);
            if ( handle < 0 ) {
                LtfsLogError("Cannot connect " << group);
                continue;
            }

            xmlrpc_c::clientXmlTransport_pstream transport(
                    xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                    .fd(handle));
            xmlrpc_c::client_xml client(&transport);
            xmlrpc_c::paramList params;
            params.add(xmlrpc_c::value_boolean(true));
            xmlrpc_c::rpc rpc(bdt::ServiceServer::GetCacheCapacity,params);
            xmlrpc_c::carriageParm_pstream carriage;

            off_t size = 0;
            try {
                rpc.call(&client,&carriage);
                if ( ! rpc.isSuccessful() ) {
                    xmlrpc_c::fault fault = rpc.getFault();
                    LtfsLogError(fault.getCode() << ":" << fault.getDescription());
                } else {
                    size = xmlrpc_c::value_i8(rpc.getResult());
                }
            } catch ( std::exception const & e ) {
                LtfsLogError( e.what() );
            }

            close(handle);

            sizeTotal += size;
        }

        struct statfs stat;
        fs::path cacheFolder = bdt::Factory::GetCacheFolder();
        if ( 0 != statfs( cacheFolder.string().c_str(), &stat ) ) {
            LtfsLogError( cacheFolder );
            return false;
        }
        off_t sizeCacheOnePercent = (off_t)stat.f_bsize * stat.f_blocks / 100;

        off_t sizeWriteCache = sizeCacheOnePercent * writeCachePercent;

        if ( sizeWriteCache > sizeTotal ) {
            action = 0;
        } else {
            action = 1;
        }

        return true;
    }


    bool
    TapeLibraryMgr::SetWriteCacheAction(int action)
    {
		VS_DBG_LOG_FUNCTION;
        vector<string> groups;
        if ( ! database_->GetTapeGroupList(groups) ) {
            LtfsLogError("Failure to get tape group list");
            return false;
        }

        BOOST_FOREACH( const string & group, groups ) {
            int handle = GetClientHandle(group,0);
            if ( handle < 0 ) {
                LtfsLogError("Cannot connect " << group);
                continue;
            }

            xmlrpc_c::clientXmlTransport_pstream transport(
                    xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                    .fd(handle));
            xmlrpc_c::client_xml client(&transport);
            xmlrpc_c::paramList params;
            params.add(xmlrpc_c::value_int(action));
            xmlrpc_c::rpc rpc(bdt::ServiceServer::SetCacheState,params);
            xmlrpc_c::carriageParm_pstream carriage;

            bool retValue = false;
            try {
                rpc.call(&client,&carriage);
                if ( ! rpc.isSuccessful() ) {
                    xmlrpc_c::fault fault = rpc.getFault();
                    LtfsLogError(fault.getCode() << ":" << fault.getDescription());
                } else {
                    retValue = xmlrpc_c::value_boolean(rpc.getResult());
                }
            } catch ( std::exception const & e ) {
                LtfsLogError( e.what() );
            }

            close(handle);
        }

        return true;
    }

    bool
    TapeLibraryMgr::GetTapeGroupCacheSize(const string & tapeGroup,off_t & size)
    {
		VS_DBG_LOG_FUNCTION;
        int handle = GetClientHandle(tapeGroup);
        if ( handle < 0 ) {
            LtfsLogError( "GetHandle: " << tapeGroup);
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_boolean(true));
        xmlrpc_c::rpc rpc(bdt::ServiceServer::GetCacheCapacity,params);
        xmlrpc_c::carriageParm_pstream carriage;

        size = 0;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LtfsLogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                size = xmlrpc_c::value_i8(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LtfsLogError( e.what() );
        }

        close(handle);

        return true;
    }

    bool
    TapeLibraryMgr::DeleteFile(const string & tapeGroup, const fs::path & path)
    {
		VS_DBG_LOG_FUNCTION;
        int handle = GetClientHandle(tapeGroup);
        if ( handle < 0 ) {
            LtfsLogError( "GetHandle: " << tapeGroup << " : " << path);
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(path.string()));
        xmlrpc_c::rpc rpc(bdt::ServiceServer::ReleaseInode,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LtfsLogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                ret = xmlrpc_c::value_boolean(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LtfsLogError( e.what() );
        }

        close(handle);

        return ret;
    }

    bool TapeLibraryMgr::GetShareAvailableTapes(const string& uuid, vector<map<string, off_t> >& tapesList)
    {
		VS_DBG_LOG_FUNCTION;
		boost::lock_guard<boost::mutex> lock(mutexTape_);

		vector<string> barcodeList;
		if ( ! database_->GetTapeGroupCartridgeList(uuid,barcodeList) ) {
			LtfsLogError( "Failed to get cartridges from database"
					<< " for tape group " << uuid );
			return false;
		}

		CartridgeDetail detail;
		CartridgeDetail dualTapeDetail;
		for ( vector<string>::iterator i = barcodeList.begin(); i != barcodeList.end(); ++ i ) {
			map<string, off_t> mapItem;
			if ( database_->GetCartridge(*i,detail) ) {
				if ( ! IsTapeWritable(detail) ) {
					continue;
				}

				if (database_->GetTapeGroupDualCopy(uuid))
				{
					if (detail.mDualCopy == "")
					{
						LtfsLogError("Coupled Tape is gone for " << detail.mBarcode);
						continue;
					}
					if ( !database_->GetCartridge(detail.mDualCopy, dualTapeDetail)
							|| detail.mTapeGroupUUID != dualTapeDetail.mTapeGroupUUID) {
						LtfsLogError("Failed to get cartridge from database: " << detail.mDualCopy);
						continue;
					}

					if ( !IsTapeWritable(dualTapeDetail)) {
						LtfsLogError("One of the coupled Tapes cann't be write: " << detail.mBarcode);
						continue;
					}
					mapItem[dualTapeDetail.mBarcode] = dualTapeDetail.mFreeCapacity;
				}
				mapItem[detail.mBarcode] = detail.mFreeCapacity;
				tapesList.push_back(mapItem);
			} else {
				LtfsLogError("Failed to get cartridge from database: " << *i);
			}
		}

		return true;
    }


	bool
	TapeLibraryMgr::StopTapeReadWrite(const string& barcode, const string& uid)
	{
		bool bRet = false;
		LtfsLogDebug("Start to stop tape " << barcode << " UUID is " << uid);
		if (!StopReadWriteTape(barcode, uid))
		{
			LtfsLogError("Cann't stop tape " << barcode << " read/write operation, share UUID:" << uid);
			return bRet;
		}

		//wait stop successful
		int retry = 60*5;  //60*1 sec
		while (GetTapeStmState(barcode) == STM_ST_READWRITE && retry-- > 0)
		{
			LtfsLogDebug("wait for tape stop read/write;" << GetTapeStmState(barcode));
			boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		}

		if (retry > 0 && StmTapeCanBeChangedTo(barcode, STM_ST_UN_ASSIGNED, STM_OP_UNASSIGN))
		{
			LtfsLogDebug("Stop Tape " << barcode << " successful. share UUID:" << uid);
			bRet = true;
		}
		return bRet;
	}

	bool
	TapeLibraryMgr::StopTapeGroupReadWrite(const string& uid)
	{
		bool bRet = true;
		vector<string> tapeList;
		if (uid.empty() || !GetTapeGroupMerbers(uid, tapeList))
		{
			LtfsLogError("Can't get tape group:" << uid);
			bRet = false;
			return bRet;
		}

		if (tapeList.size()	> 0)
		{
			for (vector<string>::iterator iter = tapeList.begin(); iter != tapeList.end(); iter++)
			{
				if (!StopTapeReadWrite(*iter, uid))
				{
					bRet = false;
				}
			}
		}

		return bRet;
	}

    bool TapeLibraryMgr::StopReadWriteTape(const string& barcode, const string& uuid)
    {
		VS_DBG_LOG_FUNCTION;
    	int handle = GetClientHandle(uuid);
		if ( handle < 0 ) {
			LtfsLogError( "GetHandle: " << uuid << " : barcode" << barcode);
			return false;
		}

		xmlrpc_c::clientXmlTransport_pstream transport(xmlrpc_c::clientXmlTransport_pstream::constrOpt().fd(handle));
		xmlrpc_c::client_xml client(&transport);
		xmlrpc_c::paramList params;
		params.add(xmlrpc_c::value_string(barcode));

		xmlrpc_c::rpc rpc(bdt::ServiceServer::StopTape, params); //
		xmlrpc_c::carriageParm_pstream carriage;

		bool ret = false;
		try {
			rpc.call(&client,&carriage);
			if ( ! rpc.isSuccessful() ) {
				xmlrpc_c::fault fault = rpc.getFault();
				LtfsLogError(fault.getCode() << ":" << fault.getDescription());
			} else {
				ret = xmlrpc_c::value_boolean(rpc.getResult());
			}
		} catch ( std::exception const & e ) {
			LtfsLogError( e.what() );
		}
		close(handle);
		return ret;
    }

    bool
    TapeLibraryMgr::SetTapeState(const string & barcode, enum TapeState state)
    {
		VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexTape_);

        if ( database_->SetStateForCartridge(barcode,state) ) {
            return true;
        } else {
            LtfsLogError( "Failed to set cartridge state to database: "
                    << barcode << " " << state);
            return false;
        }
        return false;
    }

    bool
    TapeLibraryMgr::SetTapeOffline(const string & barcode,bool offline)
    {
		VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexTape_);

        LtfsError error;
        if ( false == database_->SetOfflineForCartridge(barcode,offline) ) {
            LtfsLogError( "Failed to set cartridge offline flag to database: "
                    << barcode << " " << offline << ".");
            return false;
        }
        return true;
    }


    bool TapeLibraryMgr::SetTapeGroup(const string& barcode, const string& tapeGroup, int priority, bool bSetMam)
    {
		VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexTape_);

        LtfsError error;
        if ( database_->SetGroupUUIDForCartridge(barcode, tapeGroup) ) {
    		Labels labels;
			labels.mGroupID = tapeGroup;
			labels.mMark = LABEL_GROUP;
			if(bSetMam){
				formatMgr_->StartFormat(barcode, FormatType_Label, priority, labels);
			}
    		return true;
        } else {
            LtfsLogError( "Failed to set tape group to database: "
                    << barcode << " " << tapeGroup);
            return false;
        }

        return false;
    }

    bool TapeLibraryMgr::SetTapeDualCopy(const string& barcode, const string& dualCopy, int priority, bool bSetMam)
    {
		VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexTape_);

        LtfsError error;
        if ( database_->SetDualCopyForCartridge(barcode, dualCopy) ) {
    		Labels labels;
			labels.mDualCopy = dualCopy;
			labels.mMark = LABEL_DUAL_COPY;
			if(bSetMam){
				formatMgr_->StartFormat(barcode, FormatType_Label, priority, labels);
			}
    		return true;
        } else {
            LtfsLogError( "Failed to set dual copy to database: "
                    << barcode << " " << dualCopy);
            return false;
        }

        return false;
    }

    bool
    TapeLibraryMgr::SetTapeFaulty(const string & barcode,bool faulty, int priority, bool bSetMam)
    {
		VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexTape_);

        LtfsError error;
        if ( database_->SetFaultyForCartridge(barcode,faulty) ) {
    		Labels labels;
			labels.mFaulty = faulty;
			labels.mMark = LABEL_FAULTY;
            if ( faulty ) {
            	LtfsEvent(EVENT_LEVEL_CRITICAL, "Tape_Faulty", "Tape " << barcode << " became faulty." ); //Event/Notification
                CartridgeDetail detail;
                if ( database_->GetCartridge(barcode,detail) ) {
                    switch ( detail.mStatus ) {
                        //case TAPE_OPEN:
                        case TAPE_ACTIVE:
                        	labels.mStatus = TAPE_OPEN;
                        	labels.mMark |= LABEL_STATUS;
                            database_->SetStatusForCartridge(barcode, TAPE_OPEN);
                            if (detail.mDualCopy != "") {
                            	Labels labelsDualCopy;
                            	labelsDualCopy.mStatus = TAPE_OPEN;
                            	labelsDualCopy.mMark = LABEL_STATUS;
                            	database_->SetStatusForCartridge(detail.mDualCopy, TAPE_OPEN);
                            	if(bSetMam){
                            		formatMgr_->StartFormat(detail.mDualCopy, FormatType_Label, priority, labelsDualCopy);
                            	}
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        	if(bSetMam){
				formatMgr_->StartFormat(barcode, FormatType_Label, priority, labels);
        	}

        	// start tape check
        	if(faulty){
        		vector<string> barcodes;
        		barcodes.push_back(barcode);
        		if(!TaskManagement::GetInstance()->IsTaskRunning(barcode, Type_CheckTape)){
					TaskCheckTape *pTask = new TaskCheckTape(barcodes, true);
					if(pTask == NULL || false == TaskManagement::GetInstance()->AddTask(pTask, false)){
						LtfsLogError("Failed to start check tape task for tape " << barcode);
					}
        		}
        	}

    		return true;
        	/*string changer,drive;
            int slotID;
            if ( FindTape(barcode,changer,drive,slotID)
                    && (!drive.empty()) ) {
                hal_->SetLoadedTapeFaulty(
                        changer, drive, faulty, error );
            } else {
                LtfsLogError( "Failed to set cartridge faulty to HAL: "
                        << barcode << " " << faulty);
            }
            return true;
            */
        } else {
            LtfsLogError( "Failed to set cartridge faulty to database: "
                    << barcode << " " << faulty);
            return false;
        }

        return false;
    }

    bool TapeLibraryMgr::SetTapeMamInfo(const string& barcode, TapeMamInfo mamInfo)
    {
		VS_DBG_LOG_FUNCTION;
    	LtfsLogDebug("SetTapeMamInfo: barcode = " << barcode << ", mamInfo.mTapeGroup = " << mamInfo.mTapeGroup
    			<< ", mamInfo.mStatus = " << mamInfo.mStatus << ", mamInfo.mFaulty = " << mamInfo.mFaulty
    			<< ", mamInfo.mMask = " << mamInfo.mMask << ", mamInfo.mPriority = " << mamInfo.mPriority
    			<< ", mamInfo.mDualCopy = " << mamInfo.mDualCopy << ".");
    	FormatType formatType;
		Labels labels;
		bool bLabel = false;
		labels.mMark = 0;
		if(mamInfo.mMask & MAM_INFO_MASK_UUID){
			labels.mGroupID = mamInfo.mTapeGroup;
			labels.mMark |= LABEL_GROUP;
			bLabel = true;
			if(!database_->SetGroupUUIDForCartridge(barcode, mamInfo.mTapeGroup)){
			}
		}
		if(mamInfo.mMask & MAM_INFO_MASK_DUAL_COPY){
			labels.mDualCopy = mamInfo.mDualCopy;
			labels.mMark |= LABEL_DUAL_COPY;
			bLabel = true;
			if(!database_->SetDualCopyForCartridge(barcode, mamInfo.mDualCopy)){
			}
		}
		if(mamInfo.mMask & MAM_INFO_MASK_STATUS){
			labels.mStatus = mamInfo.mStatus;
			labels.mMark |= LABEL_STATUS;
			bLabel = true;
			if(!database_->SetStatusForCartridge(barcode, mamInfo.mStatus)){
			}
		}
		if(mamInfo.mMask & MAM_INFO_MASK_FAULTY){
			labels.mFaulty = mamInfo.mFaulty;
			labels.mMark |= LABEL_FAULTY;
			bLabel = true;
			if(!database_->SetFaultyForCartridge(barcode, mamInfo.mFaulty)){
			}
		}

		bool bFormat = (mamInfo.mMask & MAM_INFO_MASK_FORMAT);
		if(bLabel && bFormat){
			formatType = FormatType_Both;
		}else if(bLabel){
			formatType = FormatType_Label;
		}else if(bFormat){
			formatType = FormatType_Format;
		}

		// if it need to format the tape and status is not provided, need to set tape status to open
		if(bFormat && !(mamInfo.mMask & MAM_INFO_MASK_STATUS)){
			labels.mStatus = TAPE_OPEN;
			labels.mMark |= LABEL_STATUS;
		}

		if((bLabel || bFormat) && mamInfo.mPriority != -1){
			formatMgr_->StartFormat(barcode, formatType, mamInfo.mPriority, labels);
		}

    	return true;
    }

    bool
    TapeLibraryMgr::SetTapeStatus(const string & barcode, TapeStatus status, int priority, bool bSetMam)
    {
		VS_DBG_LOG_FUNCTION;
        boost::lock_guard<boost::mutex> lock(mutexTape_);

        LtfsError error;
        if ( database_->SetStatusForCartridge(barcode, (int)status) ) {
    		Labels labels;
			labels.mStatus = status;
			labels.mMark = LABEL_STATUS;
			if(bSetMam){
				formatMgr_->StartFormat(barcode, FormatType_Label, priority, labels);
			}

            switch (status) {
                case TAPE_CLOSED:
                    LtfsEvent(EVENT_LEVEL_INFO, "Tape_Status_Closed", "Tape " << barcode << " is closed." );
                    break;
                case TAPE_EXPORTED:
                    break;
                default:
                    LtfsLogInfo( "Tape " << barcode
                            << " is changed to status " << status );
                    break;
            }
            return true;
        } else {
            LtfsLogError( "Failed to set cartridge status to database: "
                    << barcode << " " << status);
            return false;
        }
    }

    bool TapeLibraryMgr::SetTapeActivity(const string& barcode, TapeActivity activity)
    {
		VS_DBG_LOG_FUNCTION;
    	if(NULL != database_){
    		return database_->SetActivityForCartridge(barcode, (int)activity);
    	}

    	return false;
    }

    bool TapeLibraryMgr::GetDriveCleaningStatus(const string& changer, const string& drive, int& status, LtfsError error, bool bForceRefresh)
    {
		VS_DBG_LOG_FUNCTION;
    	return hal_->GetDriveCleaningStatus(changer, drive, status, error, bForceRefresh);
    }

    bool TapeLibraryMgr::GetDrive(const string& driveSerial, LtfsDriveInfo& drive)
    {
		VS_DBG_LOG_FUNCTION;
        LtfsError error;
        vector<LtfsChangerInfo> changers;
        if( ! hal_->GetChangerList(changers,error) ) {
            LtfsLogError("Failed to get changes from HAL");
            return false;
        }

        for(unsigned int i = 0; i < changers.size(); i++){
			vector<LtfsDriveInfo> drives;
			if (! hal_->GetDriveList(changers[i].mSerial, drives, error)) {
				LtfsLogError("Failed to get drive list for changer " << changers[i].mSerial << ".");
				continue;
			}

			for(unsigned j = 0; j < drives.size(); j++){
				if(drives[j].mSerial == driveSerial){
					drive = drives[j];
					return true;
				}
			}
        }

        return false;
    }

    bool
    TapeLibraryMgr::GetTape(const string & barcode,TapeInfo & tape)
    {
		VS_DBG_LOG_FUNCTION;
        string changer,drive;
        LtfsError error;
        // check if the tape is visible by HAL
        int slotID;
        if (FindTape(barcode,changer,drive,slotID)) {
			vector<LtfsTapeInfo> infos;
			if (! hal_->GetTapeList(changer,infos,error)) {
				LtfsLogError("Failed to get changes from HAL");
				return false;
			}

			for(vector<LtfsTapeInfo>::iterator i = infos.begin();
					i != infos.end();
					++ i ) {
				if ( i->mBarcode == barcode ) {
					return GetTape(tape,*i);
				}
			}
        }else{
        	CartridgeDetail detail;
        	if(!database_->GetCartridge(barcode, detail)){
        		return false;
        	}
        	tape = ConvertTape(detail);
        }

        return true;
    }

    long long TapeLibraryMgr::GetPendingDeleteFileNum(const string& barcode)
    {
		TapeInfo tapeInfo;
		string tapeGroup = "";
		if(GetTape(barcode, tapeInfo)){
			tapeGroup = tapeInfo.mGroupID;
		}
		if(tapeGroup == ""){
			LtfsLogDebug("GetPendingDeleteFileNum: tape " << barcode << " is not in share yet.");
			return 0;
		}

		vector<string> toDeletUuids;
		if(false == CatalogDbManager::Instance()->GetFilesToDelete(tapeGroup, barcode, toDeletUuids)){
			LtfsLogError("GetPendingDeleteFileNum: Failed to get files to delete on tape " << barcode);
		}
		return toDeletUuids.size();
    }

    bool TapeLibraryMgr::DeleteFilesOnTape(const string& barcode)
    {
		TapeInfo tapeInfo;
		string tapeGroup = "";
		if(GetTape(barcode, tapeInfo)){
			tapeGroup = tapeInfo.mGroupID;
		}
		if(tapeGroup == ""){
			LtfsLogDebug("DeleteFilesOnTape: tape " << barcode << " is not in share yet.");
			return false;
		}

		vector<string> toDeletUuids;
		if(false == CatalogDbManager::Instance()->GetFilesToDelete(tapeGroup, barcode, toDeletUuids)){
			LtfsLogError("DeleteFilesOnTape: Failed to get files to delete on tape " << barcode);
			return false;
		}
		if(toDeletUuids.size() <= 0){
			LtfsLogDebug("DeleteFilesOnTape: no files to delete on tape " << barcode);
			return true;
		}

		vector<string> deletedUuids;
		string tapeMountPoint = GetTapeMountPoint(barcode);
		for(unsigned int i = 0; i < toDeletUuids.size(); i++){
			string filePath = tapeMountPoint + "/" + GetPathFromUuid(toDeletUuids[i]);
			try{
				boost::filesystem::remove_all(filePath);
				deletedUuids.push_back(toDeletUuids[i]);
				LtfsLogDebug("DeleteFilesOnTape: delete file " << filePath << " on tape " << barcode << " finished.");
			}catch(...){
				LtfsLogError("DeleteFilesOnTape: Failed to delete file " << filePath << " on tape " << barcode);
			}
		}

		if(false == CatalogDbManager::Instance()->DeleteTapeFiles(tapeGroup, barcode, deletedUuids)){
			LtfsLogError("DeleteFilesOnTape: Failed to delete files in database for tape " << barcode);
			return false;
		}
		return true;
    }

    bool TapeLibraryMgr::StartDeleteTapeFileTask(const string& barcode)
    {
    	CartridgeDetail detail;
    	string shareUuid = "";
    	if(GetCartridge(barcode, detail)){
    		shareUuid = detail.mTapeGroupUUID;
    	}
    	if(shareUuid == ""){
    		return true;
    	}
    	if(false == CatalogDbManager::Instance()->NeedDeleteFileOnTape(shareUuid, barcode)){
    		return true;
    	}
		vector<string> barcodes;
		barcodes.push_back(barcode);
		if(!TaskManagement::GetInstance()->IsTaskRunning(barcode, Type_DeleteTapeFile)){
			TaskDeleteTapeFile* pTask = new TaskDeleteTapeFile(barcodes);
			if(pTask == NULL || false == TaskManagement::GetInstance()->AddTask(pTask, false)){
				LogError("Failed to add task to delete files on tape tape " << barcode << ".");
				return false;
			}
			mLastDeleteFileTaskMap_[barcode] = time(NULL);
		}
		return true;
    }

    bool
    TapeLibraryMgr::MountTape(const string & barcode)
    {
		VS_DBG_LOG_FUNCTION;
        string changer,drive;
        int slotID;
        LtfsError ltfsErr;
        if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
        	LtfsLogError("MountTape " << barcode << " failed. Tape not loaded to drive yet.");
            return false;
        }

        bool bOrgMounted = false;
        if( !hal_->IsTapeMounted(changer, drive, bOrgMounted, ltfsErr ) ) {
        	LtfsLogError("Failed to check if the tape is already mounted or not. Err: " << ltfsErr.GetErrMsg());
        }

        if(!HalMountTape(barcode, ltfsErr)){
            switch ( ltfsErr.GetErrCode() ) {
                case ERR_MOUNT_FORMAT_CHECK:
                case ERR_MOUNT_EOD_MISSING:
                case ERR_MOUNT_READ_PARTITION_FAIL:
                case ERR_MOUNT_FAIL:
                	SetLoadedTapeFaulty(barcode, true, ltfsErr, true);
                    break;
                default:
                    break;
            }
            LtfsEvent(EVENT_LEVEL_ERR, "Tape_Mount_Failed", "Failed to mount tape " << barcode << ".");
            return false;
        }

        if ( !bOrgMounted ) {
            UpdateTape(changer, drive, ltfsErr);
        }


        time_t tNow = time(NULL);
        if(!database_->SetLastMountTimeForCartridge(barcode, tNow)){
        	LtfsLogError("Failed to set last mount time for barcode " << barcode << " to " << tNow);
        }

        return true;
    }


    bool TapeLibraryMgr::HalMountTape(const string & barcode, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
        string changer,drive;
        int slotID;
        if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
        	LtfsLogError("MountTape " << barcode << " failed. Tape not loaded to drive yet.");
            return false;
        }

        bool bMounted = false;
        if(!IsDriveMounted(changer, drive, bMounted, ltfsErr)){
        	bMounted = false;
        }

        if(!bMounted){
			SetTapeActivity(barcode, ACT_MOUNTING);
        }
        bool bRet =  hal_->Mount(changer, drive, ltfsErr);
        if(!bMounted){
			SetTapeActivity(barcode, ACT_IDLE);
        }

		return bRet;
    }

    bool TapeLibraryMgr::DirectAccessTape(const string& barcode)
    {
		VS_DBG_LOG_FUNCTION;
        string changer,drive;
        int slotID;
        LtfsError error;
        if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
            return false;
        }

        bool bRet = HalMountTape(barcode, error);
        if(bRet){
    		SetTapeActivity(barcode, ACT_ACCESSING);
        }

        return bRet;
    }

    bool TapeLibraryMgr::StopDirectAccessTape(const string& barcode)
    {
		VS_DBG_LOG_FUNCTION;
        string changer,drive;
        int slotID;
        LtfsError error;
        if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
            return false;
        }

        UInt64_t genIndex = 0;
        bool bRet = HalUnmountTape(barcode, genIndex, error);
        // update generation index to database
        if(bRet && genIndex > 0){
            UpdateTapeGeneraionIndex(barcode, genIndex);
        }

        // Stop direct access failed, set it back to accessing activity
        if(!bRet){
            SetTapeActivity(barcode, ACT_ACCESSING);
        }

        return bRet;
    }

    bool
    TapeLibraryMgr::CheckTape(const string & barcode, CHECK_TAPE_FLAG flag, LtfsError& lfsErr)
    {
		VS_DBG_LOG_FUNCTION;
        string changer,drive;
        int slotID;
        if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
            return false;
        }

        SetTapeActivity(barcode, ACT_DIAGNOSING);
        bool bRet = hal_->CheckTape(changer,drive, flag, lfsErr);
        SetTapeActivity(barcode, ACT_IDLE);
        return bRet;
    }

    bool TapeLibraryMgr::UpdateTapeGeneraionIndex(const string& barcode, UInt64_t genIndex)
    {
		VS_DBG_LOG_FUNCTION;
        if(NULL == database_ || false == database_->SetGenerationNumberForCartridge(barcode, (int)genIndex)){
            LtfsLogError("Failed to update generation index to database for tape " << barcode << ".");
            return false;
        }
        return true;
    }

    bool TapeLibraryMgr::UnMountDrive(const string& changerSerial, const string& driveSerial,
    		LtfsError& ltfsErr, const string& barcode, bool bUpdateGeneration)
    {
		VS_DBG_LOG_FUNCTION;
    	string tapeBarcode = barcode;
    	if(tapeBarcode == ""){
    		if(false == hal_->GetLoadedTapeBarcode(changerSerial, driveSerial, tapeBarcode, ltfsErr) || tapeBarcode == ""){
    			LtfsLogError("Failed to get tape barcode in drive " << driveSerial << ".");
    		}
    	}

    	if(tapeBarcode == ""){
    		return false;
    	}

    	UpdateTapeUnlock(changerSerial, driveSerial, ltfsErr);

        UInt64_t genIndex = 0;
        bool bRet = HalUnmountTape(tapeBarcode, genIndex, ltfsErr);

        if(tapeBarcode != ""){
        	TapeInfo tapeInfo;
        	if(false == GetTape(tapeBarcode, tapeInfo)){
    			LtfsLogError("Failed to get tape info for tape to update status to MAM for tape " << tapeBarcode << ".");
        	}else{
        		if(bUpdateGeneration && (UInt64_t)tapeInfo.mGenerationIndex != genIndex && genIndex > 0){
        			UpdateTapeGeneraionIndex(tapeBarcode, genIndex);
        		}
        		// set tape status to MAM
        		if(false == HalSetLoadedTapeStatus(tapeBarcode, tapeInfo.mStatus, ltfsErr)){
        			LtfsLogError("Failed to update tape status " << tapeInfo.mStatus << " for tape " << tapeBarcode << " to MAM.");
        		}else{
        			LtfsLogInfo("Succeed to update tape status " << tapeInfo.mStatus << " for tape " << tapeBarcode << " to MAM.");
        		}
        	}
        }

        int cleaningStatus;
        if ( hal_->GetDriveCleaningStatus(changerSerial, driveSerial, cleaningStatus, ltfsErr, true ) ) {
        	// check if we need to fire the event
        	hal_->CheckDriveCleaningEvent(changerSerial, driveSerial, cleaningStatus);
            if ( CLEANING_REQUIRED == cleaningStatus ) {
                StartDriveCleaningTask(changerSerial, driveSerial);
            }
        }

        return bRet;
    }

    bool
    TapeLibraryMgr::UnmountTape(const string & barcode, bool bUpdateGeneration)
    {
		VS_DBG_LOG_FUNCTION;
    	LtfsLogDebug("UnmountTape: start Setting activity for tape  " << barcode);
        string changer,drive;
        int slotID;
        LtfsError lfsErr;
        if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
            return false;
        }

        if(false == DeleteFilesOnTape(barcode)){
        	LtfsLogWarn("Failed to delete files on tape " << barcode << " before unmounting it.");
        }

        //UpdateTapeUnlock(changer, drive, lfsErr);

        bool bRet = UnMountDrive(changer,drive,lfsErr, barcode, bUpdateGeneration);
    	LtfsLogDebug("UnmountTape: end Setting activity for tape  " << barcode);

		if(!bRet){
        	LtfsEvent(EVENT_LEVEL_ERR, "Tape_Unmount_Failed", "Failed to unmount tape " << barcode << ".");
		}

    	return bRet;
    }

    bool TapeLibraryMgr::HalUnmountTape(const string & barcode, UInt64_t& genIndex, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
        string changer,drive;
        int slotID;
        if ( (! FindTape(barcode,changer,drive,slotID)) || drive.empty() ) {
        	LtfsLogError("HalUnmountTape " << barcode << " failed. Tape not loaded to drive yet.");
            return false;
        }

        bool bMounted = true;
        if(!IsDriveMounted(changer, drive, bMounted, ltfsErr)){
        	bMounted = true;
        }

        if(bMounted){
			SetTapeActivity(barcode, ACT_UNMOUNTING);
        }
        bool bRet =  hal_->UnMount(changer, drive, genIndex, ltfsErr);
        if(bMounted){
			SetTapeActivity(barcode, ACT_IDLE);
        }

		return bRet;
    }

    bool TapeLibraryMgr::IsMailSlotAvailable(const string& barcode)
    {
		VS_DBG_LOG_FUNCTION;
        LtfsError error;
        string changer,drive;
        int slotID;
        if ( (! FindTape(barcode,changer,drive,slotID)) ) {
            LtfsLogError( "IsMailSlotAvailable(): Failed to find tape " << barcode );
            return false;
        }

        return hal_->IsMailSlotAvailable(changer);
    }

    bool TapeLibraryMgr::StartDriveCleaningTask(const string& changerSerial, const string& driveSerial)
    {
		VS_DBG_LOG_FUNCTION;
        vector<string> driveSerials;

        driveSerials.push_back(driveSerial);
        TaskAutoClean *pTask = new TaskAutoClean(changerSerial, driveSerials);
        if(pTask == NULL || false == TaskManagement::GetInstance()->AddTask(pTask, false)){
            LtfsLogError("Failed to add task to clean drive " << driveSerial << " on changer " << changerSerial << ".");
            return false;
        }
        return true;
    }

    void TapeLibraryMgr::LogDebugTapeInfo(const TapeInfo& tapeInfo)
    {
		VS_DBG_LOG_FUNCTION;
        LtfsLogDebug("TapeLibraryMgr::GetTapeStmState()  for tape: " << tapeInfo.mBarcode << ".");
        LtfsLogDebug("		mGroupID: 			" << tapeInfo.mGroupID << ".");
        LtfsLogDebug("		mSlotID: 			" << tapeInfo.mSlotID << ".");
        LtfsLogDebug("		mStatus: 			" << tapeInfo.mStatus << ".");
        LtfsLogDebug("		mMediumType: 		" << tapeInfo.mMediumType << ".");
        LtfsLogDebug("		mMediaType: 		" << tapeInfo.mMediaType << ".");
        LtfsLogDebug("		mMediumType: 		" << tapeInfo.mMediumType << ".");
        LtfsLogDebug("		mLtfsFormat: 		" << tapeInfo.mLtfsFormat << ".");
        LtfsLogDebug("		mGenerationIndex: 	" << tapeInfo.mGenerationIndex << ".");
        LtfsLogDebug("		mLoadCount: 		" << tapeInfo.mLoadCount << ".");
        LtfsLogDebug("		mTotalCapacity: 	" << tapeInfo.mTotalCapacity << ".");
        LtfsLogDebug("		mFreeCapacity: 		" << tapeInfo.mFreeCapacity << ".");
        LtfsLogDebug("		mFileNumber: 		" << tapeInfo.mFileNumber << ".");
        LtfsLogDebug("		mFileCapacity: 		" << tapeInfo.mFileCapacity << ".");
        LtfsLogDebug("		mFaulty: 			" << tapeInfo.mFaulty << ".");
        LtfsLogDebug("		mState: 			" << tapeInfo.mState << ".");
        LtfsLogDebug("		mActivity: 			" << tapeInfo.mActivity << ".");
        LtfsLogDebug("		mWriteProtect:		" << tapeInfo.mWriteProtect << ".");
        LtfsLogDebug("		mOffline:			" << tapeInfo.mOffline << ".");
        LtfsLogDebug("		mDualCopy:			" << tapeInfo.mDualCopy << ".");
        LtfsLogDebug("		mTapeUUID:			" << tapeInfo.mTapeUUID << ".");
        LtfsLogDebug("		mLastMountTime:		" << tapeInfo.mLastMountTime << ".");
        LtfsLogDebug("		mLastAuditTime:		" << tapeInfo.mLastAuditTime << ".");
    }


    bool TapeLibraryMgr::IsNativeTapeGroup(const string& tapeGroup)
    {
		VS_DBG_LOG_FUNCTION;
        if("" == tapeGroup){
            return false;
        }

        vector<string> list;
        if(false == database_ || false == database_->GetTapeGroupList(list)){
            return false;
        }

        for(unsigned int i = 0; i < list.size(); i++){
            if(list[i] == tapeGroup){
                return true;
            }
        }

        return false;
    }

    string TapeLibraryMgr::GetShareNameByBarcode(const string& barcode)
    {
		VS_DBG_LOG_FUNCTION;
    	TapeInfo tape;
        if(!GetTape(barcode, tape)){
        	LtfsLogError("Failed to get tape info for " << barcode);
        	return "";
        }
        string shareName = "";
        if(!database_->GetGroupName(tape.mGroupID, shareName)){
        	LtfsLogError("Failed to get group name for " << tape.mGroupID << " for tape " << barcode);
        }
        return shareName;
    }

    bool TapeLibraryMgr::SetGroupName(const string& group, const string& name)
    {
		VS_DBG_LOG_FUNCTION;
    	return database_->SetGroupName(group, name);
    }

    bool TapeLibraryMgr::GetGroupName(const string& group, string& name)
    {
		VS_DBG_LOG_FUNCTION;
    	return database_->GetGroupName(group, name);
    }

	bool TapeLibraryMgr::StartReadTape(const string& barcode)
	{
		VS_DBG_LOG_FUNCTION;
		return SetTapeActivity(barcode, ACT_READING_DATA);
	}
	bool TapeLibraryMgr::StartWriteTape(const string& barcode)
	{
		VS_DBG_LOG_FUNCTION;
		return SetTapeActivity(barcode, ACT_WRITTING_DATA);
	}
	bool TapeLibraryMgr::StopReadWriteTape(const string& barcode)
	{
		VS_DBG_LOG_FUNCTION;
		return SetTapeActivity(barcode, ACT_IDLE);
	}

	bool TapeLibraryMgr::SetDeletingActivity(const string& barcode, bool bSet)
	{
		VS_DBG_LOG_FUNCTION;
		if(bSet){
			return SetTapeActivity(barcode, ACT_DELETING_FILES);
		}else{
			return SetTapeActivity(barcode, ACT_IDLE);
		}
	}

	//// functions for state machine manager ///////////////////////////////////
    bool TapeLibraryMgr::StmRequestTape(const string& barcode, int millTimeOut)
    {
		VS_DBG_LOG_FUNCTION;
    	return stateMachine_->StmRequestTape(barcode, millTimeOut);
    }
    bool TapeLibraryMgr::StmRequestTape(const string& barcode, STM_OPERATION stmOperation, const string& shareUuid, int millTimeOut)
    {
		VS_DBG_LOG_FUNCTION;
    	return stateMachine_->StmRequestTape(barcode, stmOperation, shareUuid, millTimeOut);
    }
    void TapeLibraryMgr::StmReleaseTape(const string& barcode)
    {
		VS_DBG_LOG_FUNCTION;
    	return stateMachine_->StmReleaseTape(barcode);
    }

    bool TapeLibraryMgr::StmTapeCanBeChangedTo(const string& barcode, STM_STATE stmState, STM_OPERATION stmOperation)
    {
		VS_DBG_LOG_FUNCTION;
    	return stateMachine_->StmTapeCanBeChangedTo(barcode, stmState, stmOperation);
    }

    bool TapeLibraryMgr::StmTapeCanBeChangedTo(const string& barcode, STM_STATE stmState, const vector<STM_OPERATION> stmOperations)
    {
		VS_DBG_LOG_FUNCTION;
    	return stateMachine_->StmTapeCanBeChangedTo(barcode, stmState, stmOperations);
    }

    string TapeLibraryMgr::GetStmStateStr(STM_STATE stmState)
    {
		VS_DBG_LOG_FUNCTION;
    	return stateMachine_->GetStmStateStr(stmState);
    }
    string TapeLibraryMgr::GetStmOperationStr(STM_OPERATION stmOp)
    {
		VS_DBG_LOG_FUNCTION;
    	return stateMachine_->GetStmOperationStr(stmOp);
    }
    STM_STATE TapeLibraryMgr::GetTapeStmState(const string& barcode)
    {
		VS_DBG_LOG_FUNCTION;
    	return stateMachine_->GetTapeStmState(barcode);
    }


    //// functions for drive scheduler manager ////////////////////
    SchResult TapeLibraryMgr::SchRequestTape(const string& barcode, vector<string>& tapesInUse, int priority)
	{
		VS_DBG_LOG_FUNCTION;
		return schduler_->SchRequestTape(barcode, tapesInUse, priority);
	}
	bool TapeLibraryMgr::SchReleaseTape(const string & barcode)
	{
		VS_DBG_LOG_FUNCTION;
		return schduler_->SchReleaseTape(barcode);
	}
    SchResult TapeLibraryMgr::SchRequestTapes(const vector<string>& barcodes, map<string, vector<string> >& tapesInUse, int priority)
	{
		VS_DBG_LOG_FUNCTION;
		return schduler_->SchRequestTapes(barcodes, tapesInUse, priority);
	}
	bool TapeLibraryMgr::SchReleaseTapes(const vector<string>& barcodes)
	{
		VS_DBG_LOG_FUNCTION;
		return schduler_->SchReleaseTapes(barcodes);
	}
    bool TapeLibraryMgr::BindTape(const string& barcode, const string& changerSerial, const string& driveSerial)
	{
		VS_DBG_LOG_FUNCTION;
		return schduler_->BindTape(barcode, changerSerial, driveSerial);
	}
    bool TapeLibraryMgr::UnbindTape(const string& barcode)
	{
		VS_DBG_LOG_FUNCTION;
		return schduler_->UnbindTape(barcode);
	}

    //// functions for TapeDbManager ///////////////////
	bool TapeLibraryMgr::GetTapeGroupMerbers(const string& group, vector<string>& list)
    {
		VS_DBG_LOG_FUNCTION;
    	return database_->GetTapeGroupCartridgeList(group, list);
    }
	bool TapeLibraryMgr::GetTapeGroupList(vector<string>& list)
    {
		VS_DBG_LOG_FUNCTION;
    	return database_->GetTapeGroupList(list);
    }
	bool TapeLibraryMgr::DeleteTape(const string& barcode)
    {
		VS_DBG_LOG_FUNCTION;
    	bool bRet = database_->DeleteCartridge(barcode);
    	return bRet;
    }
	bool TapeLibraryMgr::AddTapeGroup(const string& group, const string& name)
    {
		VS_DBG_LOG_FUNCTION;
    	return database_->AddTapeGroup(group, name);
    }
	bool TapeLibraryMgr::SetFileNumberForTape(const string& barcode, long fileNumber)
    {
		VS_DBG_LOG_FUNCTION;
    	return database_->SetFileNumberForCartridge(barcode, fileNumber);
    }
	bool TapeLibraryMgr::SetFileCapacityForTape(const string& barcode, long long fileCapacity)
    {
		VS_DBG_LOG_FUNCTION;
    	return database_->SetFileCapacityForCartridge(barcode, fileCapacity);
    }
	bool TapeLibraryMgr::SetTapeLtfsFormat(const string& barcode, int format)
    {
		VS_DBG_LOG_FUNCTION;
    	return database_->SetFormatForCartridge(barcode, format);
    }
	bool TapeLibraryMgr::GetShareUUID(const string& name, string& uuid)
    {
		VS_DBG_LOG_FUNCTION;
    	return database_->GetShareUUID(name, uuid);
    }

    bool TapeLibraryMgr::GetCartridge(const string& barcode, CartridgeDetail& detail)
    {
		VS_DBG_LOG_FUNCTION;
    	return database_->GetCartridge(barcode, detail);
    }

    bool TapeLibraryMgr::Format(const string& barcode, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || drive == "" || database_ == NULL || hal_ == NULL){
    		return false;
    	}

    	SetTapeActivity(barcode, ACT_FORMATTING);
    	bool bRet = hal_->Format(changer, drive, ltfsErr);
    	SetTapeActivity(barcode, ACT_IDLE);

        int code = ltfsErr.GetErrCode();
        switch( code ) {
            case ERR_FORMAT_FAIL:
            	SetLoadedTapeFaulty(barcode, true, ltfsErr, true);
                    break;
            default:
            	break;
        }

    	return bRet;
    }
	bool TapeLibraryMgr::GetLoadedTapeLtfsFormat(const string& barcode, int& ltfsFormat, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
		GET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeLtfsFormat, barcode, ltfsFormat, ltfsErr);
    }
	bool TapeLibraryMgr::GetLoadedTapeLoadCounter(const string& barcode, int& count, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
		GET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeLoadCounter, barcode, count, ltfsErr);
    }
	bool TapeLibraryMgr::GetLoadedTapeMediaType(const string& barcode, int& mediaType, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
		GET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeMediaType, barcode, mediaType, ltfsErr);
    }
	bool TapeLibraryMgr::GetLoadedTapeWPFlag(const string& barcode, bool& bIsWP, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
		GET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeWPFlag, barcode, bIsWP, ltfsErr);
    }
	bool TapeLibraryMgr::GetLoadedTapeGenerationIndex(const string& barcode, long long& index, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
		GET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeGenerationIndex, barcode, index, ltfsErr);
    }
	bool TapeLibraryMgr::GetLoadedTapeCapacity(const string& barcode, long long& freeCapacity, long long& totalCapacity, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || database_ == NULL || hal_ == NULL){
    		return false;
    	}

    	SetTapeActivity(barcode, ACT_READING_LABEL);
    	bool bRet = hal_->GetLoadedTapeCapacity(changer, drive, freeCapacity, totalCapacity, ltfsErr);
    	SetTapeActivity(barcode, ACT_IDLE);

    	return bRet;
    }
	bool TapeLibraryMgr::GetLoadedTapeStatus(const string& barcode, int& status, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
		GET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeStatus, barcode, status, ltfsErr);
    }

	bool TapeLibraryMgr::SetLoadedTapeStatus(const string& barcode, int status, LtfsError& ltfsErr, bool bSetMam, bool bSetDb)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || database_ == NULL || hal_ == NULL){
    		return false;
    	}

    	if(bSetDb){
			if(!database_->SetStatusForCartridge(barcode, status)){
				return false;
			}
    	}

    	bool bRet = true;
    	if(bSetMam){
			bRet = HalSetLoadedTapeStatus(barcode, status, ltfsErr);
    	}

    	return bRet;
    }
	bool TapeLibraryMgr::GetLoadedTapeFaulty(const string& barcode, bool& faulty, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
		GET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeFaulty, barcode, faulty, ltfsErr);
    }

	bool TapeLibraryMgr::SetLoadedTapeFaulty(const string& barcode, bool faulty, LtfsError& ltfsErr, bool bSetMam, bool bSetDb)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || database_ == NULL || hal_ == NULL){
    		return false;
    	}

    	if(bSetDb){
    		bool bOrgFaulty = false;
            CartridgeDetail detail;
            if ( database_->GetCartridge(barcode, detail) ) {
            	bOrgFaulty = detail.mFaulty;
            }
			if(!database_->SetFaultyForCartridge(barcode, faulty)){
				return false;
			}
            if ( !bOrgFaulty && faulty ) {
            	LtfsEvent(EVENT_LEVEL_CRITICAL, "Tape_Faulty", "Tape " << barcode << " became faulty." ); //Event/Notification
				switch ( detail.mStatus ) {
					//case TAPE_OPEN:
					case TAPE_ACTIVE:
						database_->SetStatusForCartridge(barcode, TAPE_OPEN);
						break;
					default:
						break;
				}
            }
    	}

    	bool bRet = true;
    	if(bSetMam){
			bRet = HalSetLoadedTapeFaulty(barcode, faulty, ltfsErr);
    	}

    	return bRet;
    }
	bool TapeLibraryMgr::SetLoadedTapeGroup(const string& barcode, const string& uuid, LtfsError& ltfsErr, bool bSetMam, bool bSetDb)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || database_ == NULL || hal_ == NULL){
    		return false;
    	}

    	if(bSetDb){
			if(!database_->SetGroupUUIDForCartridge(barcode, uuid)){
				return false;
			}
    	}

    	bool bRet = true;
    	if(bSetMam){
			bRet = HalSetLoadedTapeGroup(barcode, uuid, ltfsErr);
    	}

    	return true;
    }
	bool TapeLibraryMgr::SetLoadedTapeUUID(const string& barcode, const string& uuid, LtfsError& ltfsErr, bool bSetMam, bool bSetDb)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || database_ == NULL || hal_ == NULL){
    		return false;
    	}

    	if(bSetDb){
			if(!database_->SetTapeUUIDForCartridge(barcode, uuid)){
				return false;
			}
    	}

    	bool bRet = true;
    	if(bSetMam){
			bRet = HalSetLoadedTapeUUID(barcode, uuid, ltfsErr);
    	}

    	return true;
    }
	bool TapeLibraryMgr::GetLoadedTapeGroup(const string& barcode, string& uuid, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
		GET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeGroup, barcode, uuid, ltfsErr);
    }

	bool TapeLibraryMgr::SetLoadedTapeDualCopy(const string& barcode, const string& dualCopy, LtfsError& ltfsErr, bool bSetMam, bool bSetDb)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || database_ == NULL || hal_ == NULL){
    		return false;
    	}

    	if(bSetDb){
			if(!database_->SetDualCopyForCartridge(barcode, dualCopy)){
				return false;
			}
    	}

    	bool bRet = true;
    	if(bSetMam){
			bRet = HalSetLoadedTapeDualCopy(barcode, dualCopy, ltfsErr);
    	}

    	return true;
    }
	bool TapeLibraryMgr::GetLoadedTapeDualCopy(const string& barcode, string& dualCopy, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
		GET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeDualCopy, barcode, dualCopy, ltfsErr);
    }

	bool TapeLibraryMgr::GetLoadedTapeUUID(const string& barcode, string& uuid, LtfsError& ltfsErr)
	{
		VS_DBG_LOG_FUNCTION;
		GET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeUUID, barcode, uuid, ltfsErr);
	}

	bool TapeLibraryMgr::HalSetLoadedTapeGroup(const string& barcode, const string& uuid, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || hal_ == NULL){
    		return false;
    	}

    	bool bRet = true;
		SetTapeActivity(barcode, ACT_WRITTING_LABEL);
		bRet = hal_->SetLoadedTapeGroup(changer, drive, uuid, ltfsErr);
		SetTapeActivity(barcode, ACT_IDLE);

    	return bRet;
    }

	bool TapeLibraryMgr::HalSetLoadedTapeUUID(const string& barcode, const string& uuid, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || hal_ == NULL){
    		return false;
    	}

    	bool bRet = true;
		SetTapeActivity(barcode, ACT_WRITTING_LABEL);
		bRet = hal_->SetLoadedTapeUUID(changer, drive, uuid, ltfsErr);
		SetTapeActivity(barcode, ACT_IDLE);

    	return bRet;
    }

	bool TapeLibraryMgr::HalSetLoadedTapeDualCopy(const string& barcode, const string& dualCopy, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || hal_ == NULL){
    		return false;
    	}

    	bool bRet = true;
		SetTapeActivity(barcode, ACT_WRITTING_LABEL);
		bRet = hal_->SetLoadedTapeDualCopy(changer, drive, dualCopy, ltfsErr);
		SetTapeActivity(barcode, ACT_IDLE);

    	return bRet;
    }
	bool TapeLibraryMgr::HalSetLoadedTapeStatus(const string& barcode, int status, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || hal_ == NULL){
    		return false;
    	}

    	bool bRet = true;
		SetTapeActivity(barcode, ACT_WRITTING_LABEL);
		bRet = hal_->SetLoadedTapeStatus(changer, drive, status, ltfsErr);
		SetTapeActivity(barcode, ACT_IDLE);

    	return bRet;
    }
	bool TapeLibraryMgr::HalSetLoadedTapeFaulty(const string& barcode, bool faulty, LtfsError& ltfsErr)
    {
		VS_DBG_LOG_FUNCTION;
    	string changer = "", drive = "";
    	int slotId = -1;
    	if(!FindTape(barcode, changer, drive, slotId) || hal_ == NULL){
    		return false;
    	}

    	bool bRet = true;
		SetTapeActivity(barcode, ACT_WRITTING_LABEL);
		bRet = hal_->SetLoadedTapeFaulty(changer, drive, faulty, ltfsErr);
		SetTapeActivity(barcode, ACT_IDLE);

    	return bRet;
    }

    ////functions for format queue
	FormatThread* TapeLibraryMgr::StartFormat(const string& barcode, FormatType type, int priority, Labels& labels, time_t startTimeInMicroSecs)
	{
		VS_DBG_LOG_FUNCTION;
		return formatMgr_->StartFormat(barcode, type, priority, labels, startTimeInMicroSecs);
	}
	bool TapeLibraryMgr::CancelFormat(const string& barcode)
	{
		VS_DBG_LOG_FUNCTION;
		return formatMgr_->CancelFormat(barcode);
	}
	bool TapeLibraryMgr::GetFormatStatus(const string& barcode, FormatThreadStatus& status)
	{
		VS_DBG_LOG_FUNCTION;
		return formatMgr_->GetFormatStatus(barcode, status);
	}
	bool TapeLibraryMgr::GetFormatStatus(FormatThread* formatThread, FormatThreadStatus& status)
	{
		VS_DBG_LOG_FUNCTION;
		return formatMgr_->GetFormatStatus(formatThread, status);
	}
	bool TapeLibraryMgr::GetDetailForBackup(vector<FormatDetail>& details)
	{
		VS_DBG_LOG_FUNCTION;
		return formatMgr_->GetDetailForBackup(details);
	}

	bool TapeLibraryMgr::CheckTapeGroupExistByName(const string name)
	{
		VS_DBG_LOG_FUNCTION;
		return database_->CheckTapeGroupExistByName(name);
	}

	int
	TapeLibraryMgr::MountAllShare()
	{
		int status = 0;
		//get share list from share database.
		vector<string> tapeGroupList;
		database_->GetTapeGroupList(tapeGroupList);
		if (tapeGroupList.size() == 0) {
			LtfsLogDebug("No share need mount!");
			return status;
		}

		for (vector<string>::iterator tapeGroupIter = tapeGroupList.begin(); tapeGroupIter != tapeGroupList.end(); tapeGroupIter++)
		{
			string name = "";
			database_->GetGroupName(*tapeGroupIter, name);
			int ret = MountShare(*tapeGroupIter, name);
			if (ret != 0){
				status = ret;
			}
		}
		return status;
	}

	int
	TapeLibraryMgr::MountShare(const string& uuid, const string& name)
	{
		string mountPoint = COMM_STORAGE_VFS_PATH + "/" + name;
		int status = 0;
		string cmd = "mkdir -p " + mountPoint + " && ";
#ifdef SIMULATOR
		cmd += " " + COMM_BINARY_CLIENT_SIMULATOR + " ";
#else
		cmd += " " + COMM_BINARY_CLIENT + " ";
#endif
		cmd += " " + COMM_MOUNT_PATH + " " + COMM_META_CACHE_PATH + " " + COMM_DATA_CACHE_PATH + " " + uuid
			+  " " + QuotaString(name) + "  " + mountPoint + " 2>/dev/null 1>/dev/null";
		LtfsLogDebug(cmd);
		int ret = ::system(cmd.c_str());
		if (ret != 0){
			status = -1;
			LtfsLogError("Fail to mount share " << name);
			LtfsEvent(EVENT_LEVEL_CRITICAL,"Share_Mount_Failed", "Failed to mount share " << name << " to mount point " << COMM_STORAGE_VFS_PATH << "/"
									<< uuid << ". Please reboot the server to recover from this issue.");
		}
		return status;
	}

	int
	TapeLibraryMgr::UnmountAllShare()
	{
		int status = 0;
		//get share list from share database.
		vector<string> tapeGroupList;
		database_->GetTapeGroupList(tapeGroupList);
		if (tapeGroupList.size() == 0) {
			LtfsLogDebug("UnmountAllShare: No share need unmount!");
			return status;
		}

		for (vector<string>::iterator tapeGroupIter = tapeGroupList.begin(); tapeGroupIter != tapeGroupList.end(); tapeGroupIter++)
		{
			int ret = UnmountShare(*tapeGroupIter);
			LtfsLogDebug("UnmountAllShare: ret = " << ret);
			if (ret != 0){
				status = ret;
			}
		}
		return status;
	}


	int
	TapeLibraryMgr::UnmountShare(const string& uuid)
	{
		string shareName = "";
		if(!GetGroupName(uuid, shareName)){
			LtfsLogError("Failed to get name from uuid " << uuid);
			return -1;
		}
		string mountPoint = COMM_STORAGE_VFS_PATH + "/" + shareName;
		int status = 0;
		string cmd = "fusermount -u " + mountPoint + " 2>/dev/null 1>/dev/null";
		int ret = ::system(cmd.c_str());
		LtfsLogDebug("UnmountShare: cmd = " << cmd << ", ret = " << ret);
		if (ret != 0){
			status = ret;
			string name = "";
			database_->GetGroupName(uuid, name);
			LtfsLogError("Fail to unmount share " << name);
			LtfsEvent(EVENT_LEVEL_CRITICAL,"Share_Umount_Failed", "Failed to un-mount share " << name << " from mount point " << COMM_STORAGE_VFS_PATH << "/"
					<< uuid << ". Please reboot the server to recover from this issue.");
		}
		return status;
	}

	string TapeLibraryMgr::GenerUUIDByName(const char* name)
	{
		string strRet = "";

		boost::uuids::uuid uuid = boost::uuids::random_generator()();
		strRet = boost::lexical_cast<std::string>(uuid);
		return strRet;
	}


    bool TapeLibraryMgr::IsMailSlotBusy(const string& changerSerial)
    {
		VS_DBG_LOG_FUNCTION;
    	return hal_->IsMailSlotBusy();
    }

    void TapeLibraryMgr::RefreshSysHwModeThread()
    {
		VS_DBG_LOG_FUNCTION;
    	LtfsLogDebug("Start refreshing system HW mode.");
		hal_->GetSystemHwMode(true);

    	{
			boost::mutex::scoped_lock lock(mutexRefreshHwMode_);
			refreshHwModeRunning_ = false;
    	}
    	LtfsLogDebug("Finished refreshing system HW mode.");
    }

    SystemHwMode TapeLibraryMgr::GetSystemHwMode(bool bRefresh)
    {
		VS_DBG_LOG_FUNCTION;

    	if(diagStatus_ == DIAG_RUNNING){
    		return SYSTEM_HW_DIAGNOSING;
    	}

    	if(bNeedDiagnose_){
    		return SYSTEM_HW_NEED_DIAGNOSE;
    	}

    	SystemHwMode sysMode = hal_->GetSystemHwMode(false);

    	if(bRefresh){
			boost::mutex::scoped_lock lock(mutexRefreshHwMode_);
			if(!refreshHwModeRunning_){
				refreshHwModeRunning_ = true;
				boost::thread(boost::bind(&TapeLibraryMgr::RefreshSysHwModeThread, this));
			}
    	}

    	return sysMode;
    }

    bool TapeLibraryMgr::CheckSystemHwMode(bool bRefresh)
    {
		VS_DBG_LOG_FUNCTION;
    	SystemHwMode newMode = GetSystemHwMode(bRefresh);
    	if(lastHwMode_ == newMode || newMode == SYSTEM_HW_NEED_DIAGNOSE || newMode == SYSTEM_HW_DIAGNOSING){
    		return true;
    	}
    	lastHwMode_ = newMode;

    	if(SYSTEM_HW_READY == newMode){
    		LtfsLogDebug("Hardware ready, need mount all shares.");
    		if(0 != MountAllShare()){
    			LtfsLogError("Failed to mount all shares after system changed to HW_READY mode.");
    			return false;
    		}
    	}else{
    		LtfsLogDebug("Hardware not ready, need unmount all shares.");
    		if(0 != UnmountAllShare()){
    			LtfsLogError("Failed to unmount all shares after system changed to HW_NOT_READY mode.");
    			return false;
    		}
    	}
    	return true;
    }

    bool TapeLibraryMgr::GetDiagnoseSystemStatus(SYS_DIAGNOSE_STATUS& diagStatus, string& logFile)
    {
		VS_DBG_LOG_FUNCTION;
		boost::mutex::scoped_lock lock(mutexDiagnose_);
    	diagStatus = diagStatus_;
    	logFile = sysDiagLogFile_;

    	return true;
    }

	bool TapeLibraryMgr::DeleteFile(const string& pathName)
	{
		VS_DBG_LOG_FUNCTION;
		if(pathName == "" || pathName == "/"){
			return false;
		}

		LtfsLogInfo("DeleteFile: " << pathName);
		try{
			fs::remove(pathName);
		}catch(...){
			LtfsLogError("Failed to delete file " << pathName);
			return false;
		}
		return true;
	}

	string Time2Str(time_t tTime)
	{
		VS_DBG_LOG_FUNCTION;
		string strTime = "";

		struct tm * timeinfo;
		timeinfo = localtime ( &tTime );
        ostringstream os;
        os << setw(4) << setfill('0') << timeinfo->tm_year + 1900 << "-";
        os << setw(2) << setfill('0') << timeinfo->tm_mon + 1 << "-";
        os << setw(2) << setfill('0') << timeinfo->tm_mday << " ";
        os << setw(2) << setfill('0') << timeinfo->tm_hour << ":";
        os << setw(2) << setfill('0') << timeinfo->tm_min << ":";
        os << setw(2) << setfill('0') << timeinfo->tm_sec;
        strTime = os.str();

		return strTime;
	}

	bool TapeLibraryMgr::SaveDiagnoseLog(const string& logMsg)
	{
		VS_DBG_LOG_FUNCTION;
		string strMsg = logMsg + string("\n");
		if(sysDiagLogFile_ == ""){
			LtfsLogError("System diagnose log file pathname not defined, logs will not be saved.");
			return false;
		}
		try{
			time_t tNow = time(NULL);
	        strMsg = string("[") + Time2Str(tNow) + string("]: ") + strMsg;
			FILE* pFile = fopen(sysDiagLogFile_.c_str(), "a");
			if(pFile != NULL){
				fwrite(strMsg.c_str(), strMsg.length(), sizeof(char), pFile);
				fclose(pFile);
				return true;
			}
		}catch(...){
			LtfsLogError("Failed to save diagnose log. logMsg:" << logMsg);
		}

		LtfsLogError("Failed to save message '" << logMsg << "' to log file: " << sysDiagLogFile_ << ".");
		return false;
	}

#define SAVE_DIAGNOSE_LOG(bDirtyShutdown_, _LogFunc, _msg) \
	_LogFunc(_msg);\
	if(bDirtyShutdown_){\
		ostringstream os;\
		os << _msg;\
		SaveDiagnoseLog(os.str());\
	}

	string GenSysDiagnosLogPath()
	{
		VS_DBG_LOG_FUNCTION;
    	string filePath = DEFAULT_SYSTEM_DIAGNOSE_LOG_PREFIX;

		struct tm * timeinfo;
		time_t tNow = time(NULL);
		timeinfo = localtime ( &tNow );
        ostringstream os;
        os << setw(4) << setfill('0') << timeinfo->tm_year + 1900;
        os << setw(2) << setfill('0') << timeinfo->tm_mon + 1;
        os << setw(2) << setfill('0') << timeinfo->tm_mday;
        os << setw(2) << setfill('0') << timeinfo->tm_hour;
        os << setw(2) << setfill('0') << timeinfo->tm_min;
        os << setw(2) << setfill('0') << timeinfo->tm_sec;
        filePath += os.str() + string(".log");

        return filePath;
	}

	void RevmoveDirtyShutdownFlag()
	{
		VS_DBG_LOG_FUNCTION;
		string cmd = "rm -f " + SYSTEM_DIRTY_SHUTDOWN_FLAG + " 1>/dev/null 2>/dev/null";
		if(0 != std::system(cmd.c_str())){
			LtfsLogError("Failed to remove dirty shutdown flag file " << SYSTEM_DIRTY_SHUTDOWN_FLAG << ".");
		}
	}

	void TapeLibraryMgr::StartTaskShare()
	{
		VS_DBG_LOG_FUNCTION;
		// mount all shares and recover/start tasks
		int mountRet = MountAllShare();
		if (mountRet != 0){
			LtfsLogError("Failed to mount all samba shares after system diagnose finished. mountRet:" << mountRet);
		}
		ltfs_soapserver::TaskManagement::GetInstance()->LoadTaskQueue();
	}

	bool TapeLibraryMgr::DiagnoseSystem(bool bDoDiagnose)
	{
		VS_DBG_LOG_FUNCTION;
		boost::mutex::scoped_lock lock(mutexDiagnose_);

		if(!bNeedDiagnose_){
			LtfsLogInfo("System check is not needed.");
			return true;
		}

		if(diagStatus_ == DIAG_RUNNING){
			LtfsLogInfo("System check is already running.");
			if(!bDoDiagnose){
				SAVE_DIAGNOSE_LOG(true, LtfsLogWarn, "System check canceled.");
		    	LtfsEvent(EVENT_LEVEL_WARNING, "Sys_Cancel_Diagnose", "System check canceled.");
				diagStatus_ = DIAG_CANCELED;
				bNeedDiagnose_ = false;
			}
			return true;
		}

		if(!bDoDiagnose){
			bNeedDiagnose_ = false;
			diagStatus_ = DIAG_CANCELED;
	    	LtfsEvent(EVENT_LEVEL_WARNING, "Sys_Cancel_Diagnose", "System check canceled.");
		}else{
			diagStatus_ = DIAG_RUNNING;
		}

		LtfsLogDebug("bDoDiagnose = " << bDoDiagnose);

		// start a thread to diagnose system
		boost::thread(boost::bind(&TapeLibraryMgr::DiagnoseSystemThread, this, bDoDiagnose));

		return true;
	}

#define CHECK_CANCEL_DIAGNOSE(barcode, dualCopy) \
	{\
		bool bStop_ = false;\
		{\
			boost::mutex::scoped_lock lock(mutexDiagnose_);\
			bStop_ = !bNeedDiagnose_;\
		}\
		if(bStop_){\
			if(barcode != ""){\
				ReleaseTape(barcode);\
			}\
			if(dualCopy != ""){\
				ReleaseTape(dualCopy);\
			}\
			RevmoveDirtyShutdownFlag();\
			StartTaskShare();\
			return;\
		}\
	}

	bool TapeLibraryMgr::ServiceDiagnoseTape(bool bDirtyShutdown, const string& barcode, const string& tapeGroupUUID, vector<fs::path> &failedFileList)
	{
		VS_DBG_LOG_FUNCTION;
		if(tapeGroupUUID == ""){
			LtfsLogError("ServiceDiagnoseTape failed. tapeGroupUUID = " << tapeGroupUUID);
			return false;
		}

		string shareName = "";
		GetGroupName(tapeGroupUUID, shareName);
		SAVE_DIAGNOSE_LOG(bDirtyShutdown, LtfsLogInfo, "Checking data consistency on the tape " << barcode << ".");
		// call VFS to check the tape
		bool bRet = CatalogDbManager::Instance()->DiagnoseCheckTape(tapeGroupUUID, barcode);

		return bRet;
	}

    void TapeLibraryMgr::DiagnoseSystemThread(bool bDoDiagnose)
    {
		VS_DBG_LOG_FUNCTION;
		if(!bDoDiagnose){
			LtfsLogInfo("System check canceled.");
			// remove the dirty shutdown flag
			RevmoveDirtyShutdownFlag();
			StartTaskShare();
			return;
		}

		CHECK_CANCEL_DIAGNOSE(string(""), string(""))
    	// generate diagnose log file pathname
		sysDiagLogFile_ = GenSysDiagnosLogPath();

    	LtfsEvent(EVENT_LEVEL_INFO, "Start_Sys_Diagnose", "System check started.");
		SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, "Starting system check.");
    	LtfsError lfsErr;
    	bool bSucceed = true;
    	fs::directory_iterator end_iter;
    	vector<fs::path> failedFileList;
    	// check if any tape is in drive last time
		fs::path tapeFlagFolder(TAPE_IN_DRIVE_FLAG_FOLDER);
		SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, "Finding out tapes need to be diagnosed.");
		if(fs::exists(tapeFlagFolder) && fs::is_directory(tapeFlagFolder)){
			try{
				for(fs::directory_iterator it(tapeFlagFolder); it != end_iter; it++){
				if(it->path().filename().string() == "." || it->path().filename().string() == ".."){
					continue;
				}
				if(!fs::exists(*it)){
					LtfsLogInfo("DiagnoseSystemThread: file does not exist: " << it->path().filename().string());
					continue;
				}

				CHECK_CANCEL_DIAGNOSE(string(""), string(""))
				if(fs::exists(*it) && !fs::is_regular_file(*it)){
					SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, "Skipping file/folder " << it->path().filename().string() << ".");
					continue;
				}

				// check if the tape is in native share
				string barcode = it->path().filename().string();
				CartridgeDetail tapeInfo;
				if(!database_->GetCartridge(it->path().filename().string(), tapeInfo) || !IsNativeTapeGroup(tapeInfo.mTapeGroupUUID)){
					SAVE_DIAGNOSE_LOG(true, LtfsLogError, "Failed to get information of tape " << barcode
							<< ". Skipping the tape.");
					DeleteFile(TAPE_IN_DRIVE_FLAG_FOLDER + "/" + it->path().filename().string());
					continue;
				}

				string tapeGroupUUID = tapeInfo.mTapeGroupUUID;
				CHECK_CANCEL_DIAGNOSE(string(""), string(""))
				SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, "Requesting resource to load tape " << barcode << ".");
				// load the tape to drive
				if(!RequestTape(barcode, false , bdt::ScheduleInterface::PRIORITY_MANAGE_CARTRIDGE, INVENTORY_LOAD_TAPE_REQUEST_WAIT)){
					SAVE_DIAGNOSE_LOG(true, LtfsLogError, "Failed to request resource to diagnose the tape " << barcode << ".");
					bSucceed = false;
					continue;
				}

				CHECK_CANCEL_DIAGNOSE(barcode, string(""))
				SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, "Mounting tape " << barcode << " to check if it needs to be diagnosed.");
				// try to mount the tape, if failed, diagnose the tape
				if(!MountTape(barcode)){
					CHECK_CANCEL_DIAGNOSE(barcode, string(""))
					SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, "Failed to mount the tape " << barcode << ". Starting to diagnose it.");
					if(!CheckTape(barcode, FLAG_FULL_RECOVERY, lfsErr) && CheckTape(barcode, FLAG_DEEP_RECOVERY, lfsErr)){
						SAVE_DIAGNOSE_LOG(true, LtfsLogError, "Failed to diagnose the tape " << barcode << " .");
					}
					CHECK_CANCEL_DIAGNOSE(barcode, string(""))
					if(!MountTape(barcode)){
						SAVE_DIAGNOSE_LOG(true, LtfsLogError, "Failed to mount the tape " << barcode
								<< " after diagnosing it. Cannot check data consistency of the tape.");
						ReleaseTape(barcode);
						bSucceed = false;
						continue;
					}

					CartridgeDetail tapeDetail;
					if(database_->GetCartridge(barcode, tapeDetail) && tapeDetail.mFaulty){
						// succeed to mount the tape, mark the tape to not faulty
						if(!SetLoadedTapeFaulty(barcode, false, lfsErr, true, true)){
							LtfsLogError("Failed to clear faulty flag for tape " << barcode << ".");
						}
					}
				}

				string tmpDualCopy = tapeInfo.mDualCopy;
				string dualCopy = "";
				CartridgeDetail dualInfo;
				while(tmpDualCopy != "" && database_->GetCartridge(tmpDualCopy, dualInfo) && dualInfo.mTapeGroupUUID == tapeInfo.mTapeGroupUUID){
					dualCopy = tmpDualCopy;
					tmpDualCopy = "";
					SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, "Requesting resource to load tape " << dualCopy << ".");
					// load the tape to drive
					if(!RequestTape(dualCopy, false , bdt::ScheduleInterface::PRIORITY_MANAGE_CARTRIDGE, INVENTORY_LOAD_TAPE_REQUEST_WAIT)){
						SAVE_DIAGNOSE_LOG(true, LtfsLogError, "Failed to request resource to diagnose the tape " << dualCopy << ".");
						bSucceed = false;
						break;
					}

					CHECK_CANCEL_DIAGNOSE(barcode, dualCopy)
					SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, "Mounting tape " << dualCopy << " to check if it needs to be diagnosed.");
					// try to mount the tape, if failed, diagnose the tape
					if(!MountTape(dualCopy)){
						CHECK_CANCEL_DIAGNOSE(barcode, dualCopy)
						SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, "Failed to mount the tape " << dualCopy << ". Starting to diagnose it.");
						if(!CheckTape(dualCopy, FLAG_FULL_RECOVERY, lfsErr) && CheckTape(dualCopy, FLAG_DEEP_RECOVERY, lfsErr)){
							SAVE_DIAGNOSE_LOG(true, LtfsLogError, "Failed to diagnose the tape " << dualCopy << " .");
						}
						CHECK_CANCEL_DIAGNOSE(barcode, dualCopy)
						if(!MountTape(dualCopy)){
							SAVE_DIAGNOSE_LOG(true, LtfsLogError, "Failed to mount the tape " << dualCopy
									<< " after diagnosing it. Cannot check data consistency of the tape.");
							//ReleaseTape(dualCopy);
							bSucceed = false;
							break;
						}

						CartridgeDetail tapeDetail;
						if(database_->GetCartridge(dualCopy, tapeDetail) && tapeDetail.mFaulty){
							// succeed to mount the tape, mark the tape to not faulty
							if(!SetLoadedTapeFaulty(dualCopy, false, lfsErr, true, true)){
								LtfsLogError("Failed to clear faulty flag for tape " << dualCopy << ".");
							}
						}
					}
					// check if file on two tapes are consistent
					DiagnoseCheckDualCopyTape(barcode, dualCopy, tapeGroupUUID);
				}

				CHECK_CANCEL_DIAGNOSE(barcode, dualCopy)
				ServiceDiagnoseTape(true, barcode, tapeGroupUUID, failedFileList);
				if(dualCopy != ""){
					CHECK_CANCEL_DIAGNOSE(barcode, dualCopy)
					ServiceDiagnoseTape(true, dualCopy, tapeGroupUUID, failedFileList);
				}

				CHECK_CANCEL_DIAGNOSE(barcode, dualCopy)

				ReleaseTape(barcode);
				if(dualCopy != ""){
					ReleaseTape(dualCopy);
					DeleteFile(TAPE_IN_DRIVE_FLAG_FOLDER + "/" + dualCopy);
				}
				DeleteFile(TAPE_IN_DRIVE_FLAG_FOLDER + "/" + it->path().filename().string());
			}// for
	        }catch(const boost::filesystem::filesystem_error& e){
	        	LtfsLogError(e.what());
	        }catch(...){
	        	LtfsLogError("Exception.");
	        }
		}else{
			SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, TAPE_IN_DRIVE_FLAG_FOLDER << "does not exist or is not a folder.");
			boost::mutex::scoped_lock lock(mutexDiagnose_);
			diagStatus_ = DIAG_FAILED;
		}

		if(bSucceed){
			SAVE_DIAGNOSE_LOG(true, LtfsLogInfo, "System check finished.");
			if(failedFileList.size() > 0){
	    		LtfsEvent(EVENT_LEVEL_WARNING, "Sys_Diag_Check", "System check finished and some errors have been detected. Please get more information from "
	    				<< sysDiagLogFile_ << ".");
			}else{
	    		LtfsEvent(EVENT_LEVEL_INFO, "Sys_Diag_Finish", "System check finished without any errors found. Please get more information from "
	    				<< sysDiagLogFile_ << ".");
			}
			boost::mutex::scoped_lock lock(mutexDiagnose_);
			diagStatus_ = DIAG_FINISHED;
		}else{
			boost::mutex::scoped_lock lock(mutexDiagnose_);
			diagStatus_ = DIAG_FAILED;
			SAVE_DIAGNOSE_LOG(true, LtfsLogError, "System check failed.");
	    	LtfsEvent(EVENT_LEVEL_ERR, "Sys_Diag_Fail", "System check failed. Please get more information from " << sysDiagLogFile_ << ".");
		}

		{
			boost::mutex::scoped_lock lock(mutexDiagnose_);
			bNeedDiagnose_ = false;
			RevmoveDirtyShutdownFlag();
		}

		// mount all shares and recover/start tasks
		StartTaskShare();
    }

    bool AddFileToCopyMap(const fs::path& pathMeta, const string& subPath, map<string, off_t>& copyFileMap)
    {
		VS_DBG_LOG_FUNCTION;
    	off_t fileOffset = 9999999999;
		try{
			ExtendedAttribute attr(pathMeta);
			int nSize = 0;
			if(false == attr.GetValue(FATTR_FIELD_OFFSET, &fileOffset, sizeof(off_t), nSize)){
				LtfsLogWarn("Failed to get extend attribute offset for " << pathMeta.string() << ".");
			}
		}catch(...){
			LtfsLogWarn("Failed to get extend attribute offset for " << pathMeta.string() << ".");
		}

		copyFileMap[subPath] = fileOffset;

		return true;
    }

    bool TapeLibraryMgr::DiagnoseCheckFolderFile(const string& barcode, const string& dualCopy, const string subPath, const string& uuid, map<string, off_t>& copyFileMap)
    {
		VS_DBG_LOG_FUNCTION;
    	fs::path pathTape = GetLtfsFolder() / barcode / subPath;
    	fs::path pathCopy = GetLtfsFolder() / dualCopy / subPath;
    	fs::path pathMeta = META_ROOT / uuid / subPath;

    	if(!fs::exists(pathTape)){
    		return true;
    	}

    	// one of them not exists
    	if(!fs::is_directory(pathTape) && !fs::exists(pathCopy)){
			return AddFileToCopyMap(pathMeta, subPath, copyFileMap);
    	}

    	// type not match
    	if(fs::exists(pathCopy) && fs::is_directory(pathTape) != fs::is_directory(pathCopy)){
    		LtfsLogInfo("DiagnoseCheckFolderFile: deleting file/folder " << pathCopy.string() << " on tape " << dualCopy);
    		bool bRet = DeleteFolderFile(pathCopy.string());
    		LtfsLogInfo("DiagnoseCheckFolderFile: deleting file/folder " << pathTape.string() << " on tape " << barcode);
    		bRet &= DeleteFolderFile(pathTape.string());
    		return bRet;
    	}

    	// both file and exist, check file size
    	if(fs::exists(pathCopy) && !fs::is_directory(pathTape) && !fs::is_directory(pathCopy)){
    		if(fs::file_size(pathTape) > fs::file_size(pathCopy)){
    			LtfsLogInfo("DiagnoseCheckFolderFile: overwriting file/folder " << pathCopy.string() << " on tape " << dualCopy
    					<< " with " << pathTape.string() << " on tape " << barcode);
    			DeleteFolderFile(pathCopy.string());
    			return AddFileToCopyMap(pathMeta, subPath, copyFileMap);
    		}
    		return true;
    	}

    	try{
			// both folders, continue to check
			fs::directory_iterator end_iter;
			for(fs::directory_iterator it(pathTape); it != end_iter; it++){
				string leafName = it->path().filename().string();
				if(leafName == "." || leafName == ".."){
					continue;
				}
				DiagnoseCheckFolderFile(barcode, dualCopy, subPath + "/" + leafName, uuid, copyFileMap);
			}
			return true;
        }catch(const boost::filesystem::filesystem_error& e){
        	LtfsLogError(e.what());
        }catch(...){
        	LtfsLogError("Exception.");
        }
		return false;
    }

	typedef pair<string, off_t> OFFSET_PAIR;
    int offset_cmp(const OFFSET_PAIR& x, const OFFSET_PAIR& y)
    {
		VS_DBG_LOG_FUNCTION;
        return x.second < y.second;
    }

    bool CopyFileListMap(const map<string, off_t>& copyFileMap, const string& barcode, const string& dualCopy)
    {
		VS_DBG_LOG_FUNCTION;
		vector<OFFSET_PAIR> fileVec;
		for (map<string, off_t>::const_iterator it = copyFileMap.begin(); it != copyFileMap.end(); it++){
			fileVec.push_back(make_pair(it->first, it->second));
		}

		bool bRet = true;
		sort(fileVec.begin(), fileVec.end(), offset_cmp);
		for(unsigned int i = 0; i < fileVec.size(); i++){
			string subPath = fileVec[i].first;
			fs::path pathTape = GetLtfsFolder() / barcode / subPath;
			fs::path pathCopy = GetLtfsFolder() / dualCopy / subPath;
			LtfsLogInfo("DiagnoseCheckFolderFile: copying folder/file " << pathTape.string() << " from tape " <<
					barcode << " to " << dualCopy << ". Offset = " << fileVec[i].second);
    		if(!CopyFolderFile(pathTape.string(), pathCopy.string())){
    			LtfsLogError("DiagnoseCheckFolderFile: failed on copying folder/file " << pathTape.string() << " from tape " <<
    					barcode << " to " << dualCopy << ". Offset = " << fileVec[i].second);
    			bRet = false;
    		}
		}
		return bRet;
    }

    bool TapeLibraryMgr::DiagnoseCheckDualCopyTape(const string& barcode, const string& dualCopy, const string& uuid)
    {
		VS_DBG_LOG_FUNCTION;
    	if(barcode == dualCopy || barcode == "" || dualCopy == ""){
    		LtfsLogError("DiagnoseCheckDualCopyTape error: barcode = " << barcode << ", dualCopy = " << dualCopy);
    		return false;
    	}

    	map<string, off_t> copyFileMap;

    	bool bRet = DiagnoseCheckFolderFile(barcode, dualCopy, "", uuid, copyFileMap);
    	bRet &= CopyFileListMap(copyFileMap, barcode, dualCopy);
    	copyFileMap.clear();
    	bRet &= DiagnoseCheckFolderFile(dualCopy, barcode, "", uuid, copyFileMap);
    	bRet &= CopyFileListMap(copyFileMap, dualCopy, barcode);

    	LtfsLogDebug("DiagnoseCheckDualCopyTape: bRet = " << bRet);
    	return bRet;
    }

	int TapeLibraryMgr::GetChangerTapeNum(const string& changerSerial, bool bIncludeDrive, bool bIncludeMailSlot)
	{
		VS_DBG_LOG_FUNCTION;
		return hal_->GetTapeNum(changerSerial, bIncludeDrive, bIncludeMailSlot);
	}

	int TapeLibraryMgr::GetChangerSlotNum(const string& changerSerial)
	{
		VS_DBG_LOG_FUNCTION;
		return hal_->GetSlotNum(changerSerial);
	}

	bool TapeLibraryMgr::SetLoadedTapeBarcode(const string& driveSerial, const string& barcode, LtfsError& ltfsErr)
	{
		VS_DBG_LOG_FUNCTION;
		string oldBarcode = "";
		if(!GetLoadedTapeBarcode(driveSerial, oldBarcode, ltfsErr, true)){
			LtfsLogDebug("Failed to get original barcode for tape: " << barcode << ":" << driveSerial);
		}
		if(oldBarcode == barcode){
			return true;
		}

		LtfsLogDebug("oldBarcode = " << oldBarcode);

		bool bRet = hal_->SetLoadedTapeBarcode("", driveSerial, barcode, ltfsErr);
		if(bRet){
			if(oldBarcode != ""){
				database_->ChangeTapeBarcode(oldBarcode, barcode);
			}
		}
		return bRet;
	}

	bool TapeLibraryMgr::GetLoadedTapeBarcode(const string& driveSerial, string& barcode, LtfsError& ltfsErr, bool bRefresh)
	{
		VS_DBG_LOG_FUNCTION;
		return hal_->GetLoadedTapeBarcode("", driveSerial, barcode, ltfsErr, bRefresh);
	}

	bool TapeLibraryMgr::GetTapeGroupDualCopy(const string& uuid)
	{
		return database_->GetTapeGroupDualCopy(uuid);
	}

	bool TapeLibraryMgr::CheckCacheUsage()
	{
        //static off_t leastSize = bdt::Factory::GetConfigure()->GetValueSize(Configure::CacheFreeLeastSize);
        static off_t freeSize = bdt::Factory::GetConfigure()->GetValueSize( Configure::WriteCacheFreeSize);

        struct statfs stat;
        if ( 0 != statfs(COMM_DATA_CACHE_PATH.c_str(),&stat) ) {
            LtfsLogWarn("Failed to check statfs for " << COMM_DATA_CACHE_PATH);
            return false;
        }
        off_t freeCapacity = (off_t)stat.f_bsize * stat.f_bfree;
        if(freeCapacity < freeSize){
        	time_t tNow = time(NULL);
        	LtfsLogDebug("CheckCacheUsage: last event time: " << lastCacheUsageEventTime_ << ", tNow: " << tNow << ", diff: " << tNow - lastCacheUsageEventTime_ << ", throttle: " << CHECK_CACHE_USAGE_THROTTLE);
        	if(tNow - lastCacheUsageEventTime_ >= CHECK_CACHE_USAGE_THROTTLE){
        		LtfsEvent(EVENT_LEVEL_WARNING, "Cache_Full_Slow", "Write cache free size is less than configured size. Write to shares will be dropped to very slow speed.");
        		lastCacheUsageEventTime_ = tNow;
        	}
        }else{
        	lastCacheUsageEventTime_ = 0;
        }

        return true;
	}


    string TapeLibraryMgr::GetNextTapeToAudit(const vector<string>& auditingTapes, bool& bFaulty)
    {
    	vector<CartridgeDetail> tapes;
    	if(!database_->GetCartridgeList(tapes) || tapes.size() <= 0){
    		LtfsLogError("GetNextTapeToAudit:Failed to get tape list.");
    		return "";
    	}
    	CartridgeDetail curDetail = tapes[0];
    	bool bNoTape = true;
    	for(unsigned int i = 0; i < tapes.size(); i++){
    		bool bIgnore = false;
    		for(unsigned int j = 0; j < auditingTapes.size(); j++){
    			if(tapes[i].mBarcode == auditingTapes[j]){
    				bIgnore = true;
    			}
    		}
    		if(!bIgnore && (tapes[i].mMediaType == MEDIA_LTO6 || tapes[i].mMediaType == MEDIA_LTO5) && tapes[i].mTapeGroupUUID != ""){
    			curDetail = tapes[i];
    			bNoTape = false;
    		}
    	}
    	if(bNoTape){
    		LtfsLogDebug("GetNextTapeToAudit:No data tapes found.");
    		return "";
    	}

    	for(unsigned int i = 0; i < tapes.size(); i++){
    		CartridgeDetail newTape = tapes[i];
    		bool bFound = false;
    		for(unsigned int j = 0; j < auditingTapes.size(); j++){
    			if(newTape.mBarcode == auditingTapes[j]){
    				bFound = true;
    				break;
    			}
    		}
    		if(bFound){
    			continue;
    		}
    		if(tapes[i].mMediaType != MEDIA_LTO6 && tapes[i].mMediaType != MEDIA_LTO5){
    			continue;
    		}
    		if(newTape.mTapeGroupUUID == ""){
    			continue;
    		}
    		if(!curDetail.mFaulty && newTape.mFaulty){
    			curDetail = newTape;
    			continue;
    		}
    		if(curDetail.mLastAuditTime > newTape.mLastAuditTime){
    			curDetail = newTape;
    			continue;
    		}
    	}
		for(unsigned int j = 0; j < auditingTapes.size(); j++){
			if(curDetail.mBarcode == auditingTapes[j]){
				return "";
			}
		}

		bFaulty = curDetail.mFaulty;
		return curDetail.mBarcode;
    }

    bool TapeLibraryMgr::SetLastAuditTime(const string& barcode, time_t auditTime)
    {
    	return database_->SetLastAuditTimeForCartridge(barcode, auditTime);
    }

    bool TapeLibraryMgr::GetTapeActivity(const string& barcode, int& act, int& percentage)
    {
    	percentage = ScheduleProxyServer::RecentReadActPercentage(barcode);
    	if(!database_->GetActivityForCartridge(barcode, act)){
    		LtfsLogError("Failed to get tape activity for tape " << barcode);
    		return false;
    	}
    	LtfsLogDebug("GetTapeActivity: percent = " << percentage << ", act = " << act);
    	return true;
    }

    int TapeLibraryMgr::GetDriveNum()
    {
    	int driveNum = 0;
    	vector<LtfsChangerInfo> changers;
    	LtfsError lfsErr;
        if(!GetChangerList(changers, lfsErr)){
        	LtfsLogError("GetDriveNum failed: failed to get changer list: " << lfsErr.GetErrMsg());
        	return 0;
        }
        for(unsigned int i = 0; i < changers.size(); i++){
        	vector<LtfsDriveInfo> drives;
        	if(!GetDriveListForChanger(changers[i].mSerial, drives, lfsErr)){
        		LtfsLogError("GetDriveNum failed to get drive list for changer " << changers[i].mSerial << ": " << lfsErr.GetErrMsg());
        		continue;
        	}
        	for(unsigned int j = 0; j < drives.size(); j++){
        		if(drives[j].mStatus != DRIVE_STATUS_OK){
        			LtfsLogWarn("GetDriveNum ignoring drive " << drives[j].mSerial << " whose status is " << drives[j].mStatus);
        			continue;
        		}else if(drives[j].mGeneration < 5){
        			LtfsLogWarn("GetDriveNum ignoring drive " << drives[j].mSerial << " whose generation is " << drives[j].mGeneration);
        			continue;
        		}
    			driveNum++;
        	}
        }
        return driveNum;
    }

	#define ERR_SUCCESS 0
	bool TapeLibraryMgr::AddUpdateTapeGroupForSwift()
	{
		string type = "ASSIGN";
		string uuid = "";
		vector<string> tapes;
		BarcodesList barcodeList;

		LtfsError ltError;
		vector<TapeInfo> ltfsTapeTotalList;
		if(!TapeLibraryMgr::Instance()->GetAllTapeList(ltfsTapeTotalList, ltError)){
			LogError("Failed to get tape list to create share for swift node.");
			return false;
		}
		for(vector<TapeInfo>::iterator iter=ltfsTapeTotalList.begin(); iter!=ltfsTapeTotalList.end(); ++iter)
		{
			if(TapeLibraryMgr::Instance()->StmTapeCanBeChangedTo(iter->mBarcode, STM_ST_ASSIGNED_OPEN, STM_OP_ASSIGN))
			{
				LtfsLogDebug("AddUpdateTapeGroupForSwift add tape:"<<iter->mBarcode);
				tapes.push_back(iter->mBarcode);
			}
			else
			{
				LtfsLogDebug("AddUpdateTapeGroupForSwift not add tape:"<<iter->mBarcode);
			}
		}
		barcodeList.barcodes = tapes;

		// check if the share exists
		if(!database_->CheckTapeGroupExistByName(COMM_SWIFT_SHARE_NAME)){
			struct ShareLit shareInfo;
			shareInfo.name = COMM_SWIFT_SHARE_NAME;
			shareInfo.shareUUID = "";
			shareInfo.dualCopy = "No";
			struct CreateShareRequest req;
			req.barcodeList = tapes;
			req.shareToAdd = shareInfo;

			struct ErrorReturn errRslt;
			int ret = ltfs_soapserver::LibConvertor::CreateShare(req, errRslt);

			if(ret != SOAP_OK || errRslt.errorCode != ERR_SUCCESS){
				LogError("Failed to create share for swift node.");
				return false;
			}
			string uuid = "";
			if(!TapeLibraryMgr::Instance()->GetShareUUID(COMM_SWIFT_SHARE_NAME, uuid)){
				LogError("Failed to get uuid for share for swift node.");
				return false;
			}
			/*string linkCmd = "rm -f " + COMM_SWIFT_VS_CACHE;
			linkCmd += " && ln -s " + COMM_STORAGE_VFS_PATH + "/" + shareInfo.name + " " + COMM_SWIFT_VS_CACHE;
			linkCmd += " 1>/dev/null 2>/dev/null";
			if(0 != ExeSystem(linkCmd)){
				LogError("Failed to link share vfs folder to swift cache folder: " << linkCmd);
			}*/
		}
		else{
			if(barcodeList.barcodes.size() > 0){
				string uuid = "";
				if(!TapeLibraryMgr::Instance()->GetShareUUID(COMM_SWIFT_SHARE_NAME, uuid)){
					LogError("Failed to get uuid for share for swift node.");
					return false;
				}
				struct ErrorReturn errRslt;
				if(SOAP_OK != ltfs_soapserver::LibConvertor::AddTape(uuid, barcodeList, false, errRslt) || errRslt.errorCode != ERR_SUCCESS){
					LogError("Failed to assign tapes to share for swift node.");
					return false;
				}
			}
		}

		return true;
	}

}
