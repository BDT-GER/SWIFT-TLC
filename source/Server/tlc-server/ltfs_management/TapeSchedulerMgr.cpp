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
 * File:         TapeSchedulerMgr.cpp
 * Description:
 * Author:       Sam Chen
 * Created:      Mar 21, 2013
 *
 */


#include "stdafx.h"
#include "TapeSchedulerMgr.h"
#include "../bdt/ScheduleInterface.h"

using namespace ltfs_management;
using namespace bdt;

namespace ltfs_management
{
	const UInt64_t DRIVE_SCH_SCORE_BASE = 5000000000LL;    // should not be less than 5000000000, seconds from 1970 to 2113 is about 4509648000
	const UInt64_t DRIVE_SCH_SCORE_DRIVE_BOUND			= 2 * DRIVE_SCH_SCORE_BASE; // the drive is bound to a tape
	//const UInt64_t DRIVE_SCH_SCORE_UNKNOWN_LTO6_DRIVE	= 4 * DRIVE_SCH_SCORE_BASE; // Tape media(LTO?) unknown, use LTO6 drive first?
	const UInt64_t DRIVE_SCH_SCORE_LTO5_DRIVE 			= 4 * DRIVE_SCH_SCORE_BASE; // LTO5 tape should use LTO5 drive first
	const UInt64_t DRIVE_SCH_SCORE_DRIVE_EMPTY 			= 8 * DRIVE_SCH_SCORE_BASE; // the drive is empty
	const UInt64_t DRIVE_SCH_SCORE_TAPE_IN_DRIVE		= 16 * DRIVE_SCH_SCORE_BASE; // the tape is currently in drive


	TapeSchedulerMgr *				TapeSchedulerMgr::instance_ = NULL;
    boost::mutex 					TapeSchedulerMgr::mutexInstance_;
    boost::mutex   					TapeSchedulerMgr::mutexChangerBusyMap_;

	TapeSchedulerMgr::TapeSchedulerMgr()
	{
		bindTapeMap_.clear();
		driveTapeMap_.clear();
		lastReleaseMap_.clear();
		schChangerBusyMap_.clear();
		schTapeStatusMap_.clear();
	}
	TapeSchedulerMgr::~TapeSchedulerMgr()
	{
	}
	TapeSchedulerMgr* TapeSchedulerMgr::Instance()
	{
        boost::mutex::scoped_lock lock(mutexInstance_);

        if ( NULL == instance_ ) {
            instance_ = new TapeSchedulerMgr();
        }
        return instance_;
	}
	void TapeSchedulerMgr::Destroy()
	{
        boost::mutex::scoped_lock lock(mutexInstance_);

        if (instance_) {
            delete instance_;
            instance_ = NULL;
        }
	}

	bool TapeSchedulerMgr::GetChangerDriveList(const string& changerSerial, vector<SchDriveInfo>& driveList)
	{
		LtfsError lfsErr;
		vector<LtfsDriveInfo> drives;
		if(!TapeLibraryMgr::Instance()->GetDriveListForChanger(changerSerial, drives, lfsErr)){
			return false;
		}
		for(unsigned int k = 0; k < drives.size(); k++){
			SchDriveInfo drInfo;
			drInfo.info = drives[k];
			driveList.push_back(drInfo);
		}
		return true;
	}

	bool TapeSchedulerMgr::GetDeviceInfoByTape(const string& barcode, LtfsChangerInfo& changerInfo,
			vector<SchDriveInfo>& driveList, vector<TapeInfo>& tapeList, TapeInfo& requestTapeInfo)
	{
		vector<LtfsChangerInfo> changers;
		LtfsError lfsErr;
		if(!TapeLibraryMgr::Instance()->GetChangerList(changers, lfsErr)){
			return false;
		}

		for(unsigned int i = 0 ; i < changers.size(); i++){
			tapeList.clear();
			driveList.clear();
			changerInfo = changers[i];
			if(!TapeLibraryMgr::Instance()->GetTapeListForChanger(changers[i].mSerial, tapeList, lfsErr)){
				continue;
			}
			bool bFound = false;
			for(unsigned int j = 0; j < tapeList.size(); j++){
				if(barcode == tapeList[j].mBarcode){
					bFound = true;
					requestTapeInfo = tapeList[j];
				}
			}
			if(!bFound){
				continue;
			}
			vector<LtfsDriveInfo> drives;
			if(!TapeLibraryMgr::Instance()->GetDriveListForChanger(changers[i].mSerial, drives, lfsErr)){
				return false;
			}
			for(unsigned int k = 0; k < drives.size(); k++){
				SchDriveInfo drInfo;
				drInfo.info = drives[k];
				driveList.push_back(drInfo);
			}

			return true;
		}

		return false;
	}

	void TapeSchedulerMgr::SetTapeSchStatus(const string& barcode, SchResult status, bool bCheck)
	{
		boost::unique_lock<boost::mutex> lock(driveTapeMutex_);
		SetTapeSchStatusNoLock(barcode, status, bCheck);
	}

	void TapeSchedulerMgr::SetTapeSchStatusNoLock(const string& barcode, SchResult status, bool bCheck)
	{
		// if this request has been released, no need to continue, just exit
		if(!bCheck || schTapeStatusMap_.find(barcode) != schTapeStatusMap_.end()){
			schTapeStatusMap_[barcode] = status;
		}
	}

	void TapeSchedulerMgr::ReleaseFailTapesNoLock(const vector<string>& barcodes, map<string, vector<string> >& tapesInUse)
	{
		for(unsigned int i = 0; i < barcodes.size(); i++){
			vector<string> busyTapes;
			GetBusyTapesNolock(barcodes[i], busyTapes);
			map<string, SchResult>::iterator itTape = schTapeStatusMap_.find(barcodes[i]);
			if(itTape != schTapeStatusMap_.end()){
				LtfsLogDebug("Removing sch status " << schTapeStatusMap_[barcodes[i]] << " for tape " << barcodes[i]);
				schTapeStatusMap_.erase(itTape);
			}
			tapesInUse[barcodes[i]] = busyTapes;
		}
	}


	void TapeSchedulerMgr::SchRequestTapeThread(const string& changerSerial)
	{
		LtfsLogDebug("Starting SchRequestTapeThread for changer " << changerSerial << ".");
		if(changerSerial == ""){
			LtfsLogDebug("Handle thread for changer " << changerSerial << "finished.");
			return;
		}
		else{
			boost::unique_lock<boost::mutex> lock(mutexChangerBusyMap_);
			schChangerBusyMap_[changerSerial] = changerSerial;
		}

		int retry = 60;
		while(retry > 0){
			vector<SchDriveInfo> driveList;
			if(!GetChangerDriveList(changerSerial, driveList)){
				LtfsLogDebug("Failed to get device info for the changerSerial " << changerSerial << ".");
				sleep(1);
				continue;
			}
			retry = 60;

			SchTapeInfo curInfo;
			SchDriveInfo *pCurDrive = NULL;
			bool bFound = false;
			{
				boost::unique_lock<boost::mutex> lockDriveMap(driveTapeMutex_);
				for(unsigned int i = 0; i < driveList.size(); i++){
					if(driveTapeMap_.find(driveList[i].info.mSerial) != driveTapeMap_.end()
							&& driveTapeMap_[driveList[i].info.mSerial].barcode != ""){
						if(!bFound || curInfo.reqTime > driveTapeMap_[driveList[i].info.mSerial].reqTime){
							string barcode = driveTapeMap_[driveList[i].info.mSerial].barcode;
							if(schTapeStatusMap_.find(barcode) != schTapeStatusMap_.end() && schTapeStatusMap_[barcode] == SCH_SUCCESS){
								continue;
							}
							curInfo = driveTapeMap_[driveList[i].info.mSerial];
							pCurDrive = &driveList[i];
							bFound = true;
						}
					}// if
				}// for
			}

			if(!bFound){
				break;
			}

			string barcode = curInfo.barcode;
			LtfsChangerInfo changerInfo;
			vector<TapeInfo> tapeList;
			TapeInfo requestTapeInfo;
			// Get changer, drive list, tape list of the changer this tape belongs to
			if(!GetDeviceInfoByTape(barcode, changerInfo, driveList, tapeList, requestTapeInfo)){
				LtfsLogDebug("Failed to request tape " << barcode << ". Failed to get device info for the tape.");
				SetTapeSchStatus(barcode, SCH_NO_RESOURCE, true);
				goto SCH_THREAD_RELEASE_DRIVE;
			}

			// the tape is offline, should return false
			if(requestTapeInfo.mOffline){
				LtfsLogDebug("Failed to request tape " << barcode << ". The tape is offline.");
				SetTapeSchStatus(barcode, SCH_NO_RESOURCE, true);
				goto SCH_THREAD_RELEASE_DRIVE;
			}

			LtfsLogDebug("DDEBUG: requestTapeInfo.barcode = " << barcode);
			// start to load the tape to drive, mark the drive to be occupied
			if(LoadTapeToDrive(changerSerial, requestTapeInfo, *pCurDrive)){
				// if this request has been released, no need to continue, just exit
				boost::unique_lock<boost::mutex> lock(driveTapeMutex_);
				if(schTapeStatusMap_.find(barcode) != schTapeStatusMap_.end()){
					if(driveTapeMap_.find(pCurDrive->info.mSerial) != driveTapeMap_.end()
							&& driveTapeMap_[pCurDrive->info.mSerial].barcode == barcode){
						schTapeStatusMap_[barcode] = SCH_SUCCESS;
					}
					continue;
				}
			}else{
				LtfsLogDebug("Failed to request tape " << barcode << ". Failed to load tape to drive.");
				SetTapeSchStatus(barcode, SCH_NO_RESOURCE, true);
			}

SCH_THREAD_RELEASE_DRIVE:
			// failed to load tape to the drive, or the request has been released
			boost::unique_lock<boost::mutex> lockDriveMap(driveTapeMutex_);
			if(driveTapeMap_.find(pCurDrive->info.mSerial) != driveTapeMap_.end()
					&& driveTapeMap_[pCurDrive->info.mSerial].barcode == barcode){
				LtfsLogDebug("DDEBUG: releasing drive " << pCurDrive->info.mSerial << " for tape " << driveTapeMap_[pCurDrive->info.mSerial].barcode);
				driveTapeMap_.erase(pCurDrive->info.mSerial);
			}
		}

		if(changerSerial != ""){
			boost::unique_lock<boost::mutex> lock(mutexChangerBusyMap_);
			schChangerBusyMap_[changerSerial] = "";
		}
		LtfsLogDebug("Handle thread for changer " << changerSerial << " finished.");
	}

	string TapeSchedulerMgr::GetTapeChangerSerial(const string& barcode)
	{
		LtfsChangerInfo changerInfo;
		vector<SchDriveInfo> driveList;
		vector<TapeInfo> tapeList;
		TapeInfo requestTapeInfo;
		// Get changer, drive list, tape list of the changer this tape belongs to
		if(!GetDeviceInfoByTape(barcode, changerInfo, driveList, tapeList, requestTapeInfo)){
			LtfsLogDebug("Failed to get device info for the tape " << barcode << ".");
			return "";
		}
		return changerInfo.mSerial;
	}

	bool TapeSchedulerMgr::GetBusyTapesNolock(const string& barcode, vector<string>& busyTapes)
	{
		LtfsChangerInfo changerInfo;
		vector<SchDriveInfo> driveList;
		vector<TapeInfo> tapeList;
		TapeInfo requestTapeInfo;
		// Get changer, drive list, tape list of the changer this tape belongs to
		if(!GetDeviceInfoByTape(barcode, changerInfo, driveList, tapeList, requestTapeInfo)){
			LtfsLogError("Failed to get busy tapes for requesting tape " << barcode << ". Failed to get device info for the tape.");
			return false;
		}
		// get busy tapes, check the requested tape is busy in use
		busyTapes.clear();
		for(unsigned int i = 0; i < driveList.size(); i++){
			string busyTape = driveTapeMap_[driveList[i].info.mSerial].barcode;
			if(busyTape != ""){
				LtfsLogDebug("SchRequestTape: Adding busy tape " << busyTape << " for tape " << barcode);
				busyTapes.push_back(busyTape);
			}
		}

		return true;
	}

	time_t TapeSchedulerMgr::GetCurMicroSecondTime()
	{
		struct timeval tvNow;
		gettimeofday(&tvNow, NULL);
		return tvNow.tv_sec * 1000000 + tvNow.tv_usec;
	}

	SchResult TapeSchedulerMgr::SchRequestTapes(const vector<string>& barcodes, map<string, vector<string> >& tapesInUse, int priority)
	{
		LtfsLogDebug("DDEBUG: SchRequestTapes ...., barcode[0] = " << barcodes[0]);
		tapesInUse.clear();

		bool bFail = false;
		unsigned int nSuccess = 0;
		vector<string> toRequest;
		// check if the tape is being handled or has been handled
		{
			boost::unique_lock<boost::mutex> lock(driveTapeMutex_);
			for(unsigned int i = 0; i < barcodes.size(); i++){
				string barcode = barcodes[i];
				map<string, SchResult>::iterator itTape = schTapeStatusMap_.find(barcode);
				if(itTape != schTapeStatusMap_.end()){
					SchResult schRet = schTapeStatusMap_[barcode];
					// no resource, should clear the record and return
					if(schRet == SCH_NO_RESOURCE){
						bFail = true;
						schTapeStatusMap_.erase(itTape);
					}
					if(schRet == SCH_SUCCESS){
						LtfsLogDebug("DDEBUG: schREquest: SSSSSSSSSSS schRet = " << schRet << ", barcode = " << barcode);
						nSuccess++;
					}
				}else{
					toRequest.push_back(barcode);
				}
			}
			if(bFail){
				ReleaseFailTapesNoLock(barcodes, tapesInUse);
				LtfsLogDebug("DDEBUG: SchRequestTape ...., barcode = " << barcodes[0]);
				return SCH_NO_RESOURCE;
			}
			if(nSuccess >= barcodes.size()){
				LtfsLogDebug("DDEBUG: SchRequestTape ......, barcode = " << barcodes[0]);
				return SCH_SUCCESS;
			}
			if(toRequest.size() <= 0){
				LtfsLogDebug("DDEBUG: SchRequestTape ......., barcode = " << barcodes[0]);
				return SCH_WAIT;
			}
		}


		{
			boost::unique_lock<boost::mutex> lock(driveTapeMutex_);
			nSuccess = 0;
			map<string, bool> handledTapes;
			vector<string> sortedTapes;
			map<string, string>  tapeChangerMap;
			while(nSuccess < toRequest.size()){
				UInt64_t selectedScore = 0;
				string selectedDrive = "";
				string selectedTape = "";
				for(unsigned int i = 0; i < toRequest.size(); i++){
					string barcode = toRequest[i];
					if(handledTapes.find(barcode) != handledTapes.end() && handledTapes[barcode]){
						continue;
					}
					LtfsChangerInfo changerInfo;
					vector<SchDriveInfo> driveList;
					vector<TapeInfo> tapeList;
					TapeInfo requestTapeInfo;
					if(!GetDeviceInfoByTape(barcode, changerInfo, driveList, tapeList, requestTapeInfo)){
						LtfsLogError("Failed to request tape " << barcode << ". Failed to get device info for the tape.");
						LtfsLogDebug("schREquest: schRet = " << SCH_NO_RESOURCE << ", barcode = " << barcode);
						schTapeStatusMap_[barcode] = SCH_NO_RESOURCE;
						ReleaseFailTapesNoLock(barcodes, tapesInUse);
						return SCH_NO_RESOURCE;
					}
					tapeChangerMap[barcode] = changerInfo.mSerial;

					// the tape is offline, should return false
					if(requestTapeInfo.mOffline){
						LtfsLogDebug("Failed to request tape " << barcode << ". The tape is offline.");
						schTapeStatusMap_[barcode] = SCH_NO_RESOURCE;
						ReleaseFailTapesNoLock(barcodes, tapesInUse);
						return SCH_NO_RESOURCE;
					}

					string bindDrive = "";
					// the tape is bound to a drive, just try to use it only
					for(map<string, string>::iterator it = bindTapeMap_.begin(); it != bindTapeMap_.end(); it++){
						if(it->first == requestTapeInfo.mBarcode){
							bindDrive = it->second;
							break;
						}
					}

					UInt64_t curScore = 0;
					SchDriveInfo* pCurDrive = NULL;
					// start to find a drive to load the tape
					for(unsigned int i = 0; i < driveList.size(); i++){
						if(bindDrive != "" && bindDrive != driveList[i].info.mSerial){
							continue;
						}
						// count the score of a drive, a drive with highest score will be used.
						UInt64_t schScore = CountDriveScore(requestTapeInfo, driveList[i], priority);
						if(schScore > curScore){
							pCurDrive = &driveList[i];
							curScore = schScore;
						}
					}//for

					// no available drive found
					if(curScore <= 0 || pCurDrive == NULL){
						LtfsLogInfo("Failed to request tape " << barcode << ". No drive resource available.");
						schTapeStatusMap_[barcode] = SCH_NO_RESOURCE;
						ReleaseFailTapesNoLock(barcodes, tapesInUse);
						return SCH_NO_RESOURCE;
					}
					if(selectedScore < curScore){
						selectedDrive = pCurDrive->info.mSerial;
						selectedTape = barcode;
					}
				}// for
				SchTapeInfo tapeInf;
				tapeInf.reqTime = GetCurMicroSecondTime();
				tapeInf.barcode = selectedTape;
				driveTapeMap_[selectedDrive] = tapeInf;
				LtfsLogDebug("DDEBUG: using drive " << selectedDrive << " for tape " << selectedTape);
				sortedTapes.push_back(selectedTape);
				handledTapes[selectedTape] = true;
				nSuccess++;
			}//while

			for(unsigned int i = 0; i < sortedTapes.size(); i++){
				string barcode = sortedTapes[i];
				// Set tape to wait status and start a thread to handle the tape
				SetTapeSchStatusNoLock(barcode, SCH_WAIT, false);
				// check if the thread of the changer exists
				{
					string changerSerial = tapeChangerMap[barcode];
					boost::unique_lock<boost::mutex> lock(mutexChangerBusyMap_);
					if(schChangerBusyMap_.find(changerSerial) == schChangerBusyMap_.end() ||
							"" == schChangerBusyMap_[changerSerial] ){
						boost::thread(boost::bind(&TapeSchedulerMgr::SchRequestTapeThread, this, changerSerial));
					}
				}
				LtfsLogDebug("DDEBUG: schREquest: schRet = " << SCH_WAIT << ", barcode = " << barcode);
			}
		}

		LtfsLogDebug("DDEBUG: SchRequestTape ...., barcode[0] = " << barcodes[0]);
		return SCH_WAIT;
	}

	SchResult TapeSchedulerMgr::SchRequestTape(const string& barcode, vector<string>& tapesInUse, int priority)
	{
		LtfsLogDebug("DDEBUG: SchRequestTape, barcode = " << barcode);
		tapesInUse.clear();
		map<string, vector<string> > busyTapes;
		vector<string> barcodes;
		barcodes.push_back(barcode);
		SchResult schRet = SchRequestTapes(barcodes, busyTapes, priority);
		tapesInUse = busyTapes[barcode];
		return schRet;
	}

	bool TapeSchedulerMgr::LoadTapeToDrive(const string& changerSerial, const TapeInfo& tapeInfo, const SchDriveInfo& driveInfo)
	{
		// tape already in drive, just return true
		if(tapeInfo.mSlotID == driveInfo.info.mSlotID){
			return true;
		}

		{
			boost::unique_lock<boost::mutex> lock(mutexChangerBusyMap_);
			schChangerBusyMap_[changerSerial] = driveInfo.info.mSerial;
		}

        //lock the changer to move tapes
        if(!TapeLibraryMgr::Instance()->RequestMoveLock(changerSerial)){
        	LtfsLogError("Failed to get move lock for changer " << changerSerial);
        }

		int retry = 2;
		while(retry > 0){
			retry--;

			LtfsError lfsErr;
			// check if the drive is mounted or not
			bool bMounted = false;
			if(TapeLibraryMgr::Instance()->IsDriveMounted(changerSerial, driveInfo.info.mSerial, bMounted, lfsErr) && bMounted){
				if(!TapeLibraryMgr::Instance()->UnmountTape(driveInfo.info.mBarcode)){
					continue;
				}
			}

			bool bDriveEmpty = true;
			// check if the drive is empty
			if(!driveInfo.info.mIsEmpty){
				bDriveEmpty = false;
				// find an empty slot to unload the tape to
				vector<LtfsSlotInfo> slots;
				if(!TapeLibraryMgr::Instance()->GetSlotListForChanger(changerSerial, slots, lfsErr)){
					continue;
				}
				for(unsigned int i = 0; i < slots.size(); i++){
					if(slots[i].mIsEmpty){
						// try to unload the tape to this slot
						LtfsLogDebug("DDEBUG: MoveCartridge: " << driveInfo.info.mBarcode << ":" << slots[i].mSlotID);
						if(driveInfo.info.mBarcode != ""){
							if(TapeLibraryMgr::Instance()->MoveCartridge(driveInfo.info.mBarcode, slots[i].mSlotID, lfsErr)){
								bDriveEmpty = true;
								break;
							}
						}else{
							if(TapeLibraryMgr::Instance()->MoveCartridge(changerSerial, driveInfo.info.mSlotID, slots[i].mSlotID, lfsErr, "")){
								bDriveEmpty = true;
								break;
							}
						}
					}
				}
			}

			if(!bDriveEmpty){
				continue;
			}

			LtfsLogDebug("DDEBUG: MoveCartridge: " << tapeInfo.mBarcode << ":" << driveInfo.info.mSlotID);
			// load the tape to drive
			if(!TapeLibraryMgr::Instance()->MoveCartridge(tapeInfo.mBarcode, driveInfo.info.mSlotID, lfsErr)){
				continue;
			}
			TapeLibraryMgr::Instance()->ReleaseMoveLock(changerSerial);
			return true;
		}

		TapeLibraryMgr::Instance()->ReleaseMoveLock(changerSerial);
		return false;
	}

	UInt64_t TapeSchedulerMgr::CountDriveScore(const TapeInfo& tapeInfo, const SchDriveInfo& driveInfo, int priority)
	{
		UInt64_t score = 0;

		// should not use drive whose generation less than 5
		if(driveInfo.info.mGeneration < 5){
			return score;
		}

		// if the tape is LTO6 and the drive is LTO5, should not use this drive
		if(tapeInfo.mMediaType == MEDIA_LTO6 && driveInfo.info.mGeneration < 6){
			return score;
		}

		// bug fix: Inventory would stop if there are tape less than 2 generation of the drive in
		// 001840L3
		regex matchLTO("^\\s*\\w+L(\\d+)");
		cmatch match;
		if(regex_match(tapeInfo.mBarcode.c_str(), match, matchLTO)){
			int tapeLTO = boost::lexical_cast<int>(match[1]);
			LtfsLogDebug("tape LTO: " << tapeLTO << ", drive LTO:" << driveInfo.info.mGeneration);
			if(driveInfo.info.mGeneration - tapeLTO > 2 && tapeInfo.mMediumType != MEDIUM_CLEANING){
				LtfsLogDebug("Should not load LTO" << tapeLTO << " tape to LTO" << driveInfo.info.mGeneration << " drive.");
				return score;
			}
			if(tapeInfo.mMediaType == MEDIA_UNKNOWN && tapeLTO > 5 && driveInfo.info.mGeneration < 6){
				LtfsLogDebug("Should not load LTO" << tapeLTO << " tape to LTO" << driveInfo.info.mGeneration << " drive.");
				return score;
			}
		}

		// the drive should be ignored
		if(driveInfo.bIgnored){
			return score;
		}

		// disconnected drive should not be used
		if(driveInfo.info.mStatus == DRIVE_STATUS_DISCONNECTED){
			return score;
		}

		// the drive is in use
		{
			//boost::unique_lock<boost::mutex> lock(driveTapeMutex_);
			if(driveTapeMap_.find(driveInfo.info.mSerial) != driveTapeMap_.end()
					&& driveTapeMap_[driveInfo.info.mSerial].barcode != ""){
				LtfsLogDebug("DDEBUG: CountDriveScore: barcode: " << tapeInfo.mBarcode << " : " << driveTapeMap_[driveInfo.info.mSerial].barcode);
				return score;
			}
		}

		// the drive is usable here, at least with basic score
		score += DRIVE_SCH_SCORE_BASE;

		// the drive is bound to another tape, do not use it.
		{
			//boost::unique_lock<boost::mutex> lock(driveTapeMutex_);
			for(map<string, string>::iterator it = bindTapeMap_.begin(); it != bindTapeMap_.end(); it++){
				if(it->second == driveInfo.info.mSerial && it->first != tapeInfo.mBarcode){
					score += DRIVE_SCH_SCORE_TAPE_IN_DRIVE;
					break;
				}
			}
		}

		// check if the tape in drive is the requested tape
		if(tapeInfo.mBarcode == driveInfo.info.mBarcode){
			score += DRIVE_SCH_SCORE_TAPE_IN_DRIVE;
		}else if(priority == ScheduleInterface::PRIORITY_CARTRIDGE_DELETE_FILE){
			score = 0;
			return 0;
		}

		// check if the drive is empty
		if(driveInfo.info.mIsEmpty){
			score += DRIVE_SCH_SCORE_DRIVE_EMPTY;
		}

		// check if the tape is LTO5 tape and the drive is LTO5 drive
		if(tapeInfo.mMediaType == MEDIA_LTO5 && driveInfo.info.mGeneration == 5){
			score += DRIVE_SCH_SCORE_LTO5_DRIVE;
		}

		// Tape media type is unknown and the drive is larger than LTO6
		//if(tapeInfo.mMediaType == MEDIA_UNKNOWN && driveInfo.info.mGeneration >=6){
		//	score += DRIVE_SCH_SCORE_UNKNOWN_LTO6_DRIVE;
		//}

		// check last release time of the tape in drive
		if(driveInfo.info.mBarcode != ""){
			if(lastReleaseMap_.find(driveInfo.info.mBarcode) != lastReleaseMap_.end()){
				score -= lastReleaseMap_[driveInfo.info.mBarcode];
				LtfsLogDebug("last release time of tape " << driveInfo.info.mBarcode << " is " << lastReleaseMap_[driveInfo.info.mBarcode]);
			}
		}
		LtfsLogDebug("scheduler score of drive " << driveInfo.info.mSerial << " for tape " << tapeInfo.mBarcode << " is " << score);

		return score;
	}

	bool TapeSchedulerMgr::SchReleaseTapes(const vector<string>& barcodes)
	{
		boost::unique_lock<boost::mutex> lockDriveMap(driveTapeMutex_);
		bool bRet = true;
		for(unsigned int i = 0; i < barcodes.size(); i++){
			bRet &= SchReleaseTapeNoLock(barcodes[i]);
		}
		return bRet;
	}

	bool TapeSchedulerMgr::SchReleaseTape(const string & barcode)
	{
		boost::unique_lock<boost::mutex> lockDriveMap(driveTapeMutex_);
		return SchReleaseTapeNoLock(barcode);
	}

	bool TapeSchedulerMgr::SchReleaseTapeNoLock(const string & barcode)
	{
		string driveSerial = TapeRequestedNoLock(barcode);
		if(driveSerial != ""){
			LtfsLogDebug("DDEBUG: SchReleaseTape , barcode = " << barcode << ", driveSerial = " << driveSerial << ":" << driveTapeMap_[driveSerial].barcode);
			driveTapeMap_.erase(driveSerial);
			lastReleaseMap_[barcode] = time(NULL);
		}

		map<string, SchResult>::iterator itTape = schTapeStatusMap_.find(barcode);
		if(itTape != schTapeStatusMap_.end()){
			LtfsLogDebug("DDEBUG: SchReleaseTape , barcode = " << barcode << ", status = " << schTapeStatusMap_[barcode]);
			schTapeStatusMap_.erase(itTape);
		}

		return true;
	}

	string TapeSchedulerMgr::TapeRequestedNoLock(const string& barcode)
	{
		for(map<string, SchTapeInfo>::iterator it = driveTapeMap_.begin(); it != driveTapeMap_.end(); it++){
			if(it->second.barcode == barcode){
				return it->first;
			}
		}
		return "";
	}

	bool TapeSchedulerMgr::BindTape(const string& barcode, const string& changerSerial, const string& driveSerial)
	{
		if(barcode == ""){
			return false;
		}

		boost::unique_lock<boost::mutex> lock(driveTapeMutex_);
		if(bindTapeMap_.find(barcode) != bindTapeMap_.end()){
			if(bindTapeMap_[barcode] != driveSerial && bindTapeMap_[barcode] != ""){
				return false;
			}
		}

		bindTapeMap_[barcode] = driveSerial;

		return true;
	}
	bool TapeSchedulerMgr::UnbindTape(const string& barcode)
	{
		if(barcode == ""){
			return false;
		}

		boost::unique_lock<boost::mutex> lock(driveTapeMutex_);
		if(bindTapeMap_.find(barcode) != bindTapeMap_.end()){
			bindTapeMap_[barcode] = "";
		}

		return true;
	}
}

