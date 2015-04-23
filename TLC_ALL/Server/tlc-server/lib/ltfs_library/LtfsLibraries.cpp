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
 * LtfsLibraries.cpp
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "LtfsLibraries.h"

namespace ltfs_management
{
	boost::mutex 	LtfsLibraries::instanceMutex_;
	LtfsLibraries * LtfsLibraries::instance_ = NULL;

	LtfsLibraries::LtfsLibraries()
	{
		// TODO Auto-generated constructor stub
		bInited_ = false;
		LtfsError err;
		Refresh(err);
		bInited_ = true;
		driveCleaningMap.clear();
		drives_.clear();
	}

	LtfsLibraries::~LtfsLibraries()
	{
		// TODO Auto-generated destructor stub
		changers_.clear();
		drives_.clear();
	}

#define FIND_CHANGER_DRIVE(funcName_, changerSerial, driveSerial_)\
	LtfsChangerMap::iterator iter;\
	if(!FindChanger(changerSerial, iter)){\
		ltfsErr.SetErrCode(ERR_PARAM_CHANGER_SERIAL);\
		LtfsLgError("" << funcName_ << " no changer been found, changerSerial:" << changerSerial);\
		return false;\
	}

#define GET_SET_LOADED_TAPE_ATTRIBUTE(func_, funcName_, changerSerial_, driveSerial_, attribute_, lfsErr_)\
	string strFunc = string(funcName_);\
	LtfsLgDebug("LtfsLibraries::" << strFunc << " start");\
	FIND_CHANGER_DRIVE(funcName_, changerSerial_, driveSerial_);\
	return iter->second.func_(driveSerial_, attribute_, lfsErr_);

	bool
	LtfsLibraries::FindChanger(const string& changerSerial, LtfsChangerMap::iterator& iter)
	{
		boost::shared_lock<boost::shared_mutex> lock(mapMutex_);

		iter = changers_.find(changerSerial);
		if(iter == changers_.end())
			return false;

		return true;
	}

	bool
	LtfsLibraries::FindDrive(const string& driveSerial, VsbDriveMap::iterator& iter)
	{
		boost::shared_lock<boost::shared_mutex> lock(mapMutex_);

		iter = drives_.find(driveSerial);
		if(iter == drives_.end()){
			return false;
		}

		return true;
	}

	bool
	LtfsLibraries::FindChangerByScsi(const string& scsiAddr, LtfsChangerMap::iterator& iter)
	{
		for(iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			if(scsiAddr == iter->second.GetScsiAddr())
				return true;
		}
		LtfsLgInfo("FindChangerByScsi return false, "<<scsiAddr);
		return false;
	}

	bool
	LtfsLibraries::Refresh(LtfsError& ltfsErr)
	{
		boost::unique_lock<boost::shared_mutex> lockRefresh(refreshMutex_);
		LtfsLgDebug("LtfsLibraries::Refresh start");

		LtfsError tmpErr;
		map<string, REFRESH_DRIVE_INFO> oldDrives = GetRefreshDriveInfo();
		map<string, LtfsChangerInfo> oldChangers = GetRefreshChangerInfo();
		LtfsLgDebug("oldDrives.size() = " << oldDrives.size() << ", oldChangers = " << oldChangers.size() << ", bInited_ = " << bInited_);
#ifdef SIMULATOR
		int errID = 0;
		ScsiInfo info;
		LtfsChangerMap::iterator iterChanger;
		vector<ChangerDetail> changers;
		if(!Simulator::Instance()->GetChangerList(changers, errID))
		{
			ltfsErr.SetErrCode(errID);
			LtfsLgError("Refresh, GetChangerList failed: "<<errID);
			return false;
		}

		for(iterChanger=changers_.begin();
				iterChanger!=changers_.end(); ++iterChanger)
		{
			bool find = false;
			for(vector<ChangerDetail>::iterator iterDetail = changers.begin();
					iterDetail!=changers.end(); ++iterDetail)
			{
				if(iterDetail->m_ScsiAddr == iterChanger->second.GetScsiAddr())
				{
					find = true;
					break;
				}
			}

			if(!find)
			{
				LtfsLgWarn("Refresh changer missing, ScsiAddr:"<<iterChanger->second.GetScsiAddr());
				iterChanger->second.SetMissing(true);
			}
		}

		for(vector<ChangerDetail>::iterator iterDetail = changers.begin();
				iterDetail!=changers.end(); ++iterDetail)
		{
			if(!FindChangerByScsi(iterDetail->m_ScsiAddr, iterChanger))
			{

				info.mScsiAddr = iterDetail->m_ScsiAddr;
				info.mVendor   = iterDetail->m_Vendor;
				info.mProduct  = iterDetail->m_Product;
				info.mVersion  = iterDetail->m_Version;
				info.mStDev    = iterDetail->m_StDev;
				info.mSgDev    = iterDetail->m_SgDev;

				LtfsChanger changer(info);

				if(changer.Refresh(true, ltfsErr))
				{
					boost::unique_lock<boost::shared_mutex> lock(mapMutex_);
					LtfsLgDebug("Add changer to changers_, "<<changer.GetSerial());
					changers_.insert(LtfsChangerMap::value_type(changer.GetSerial(), changer));
				}
			}
			else
			{
				LtfsLgDebug("Refresh changer, "<<iterChanger->second.GetSerial());
				iterChanger->second.SetMissing(false);
				iterChanger->second.Refresh(true, ltfsErr);
			}

		}

		//deal with missing here

		LtfsLgDebug("changers_.size " << changers_.size());
#else
		ScsiInfo info;
		LtfsChangerMap::iterator iterChanger;
		vector<LSSCSI_INFO> changers;
		if(!ltfs_management::GetChangerList(changers, ltfsErr))
		{
			LtfsLgError("Refresh, GetChangerList failed: "<<ltfsErr.GetErrMsg());
			return false;
		}

		for(iterChanger=changers_.begin();
				iterChanger!=changers_.end(); ++iterChanger)
		{
			bool find = false;
			for(vector<LSSCSI_INFO>::iterator iterDetail = changers.begin();
					iterDetail!=changers.end(); ++iterDetail)
			{
				if(iterDetail->scsiAddr == iterChanger->second.GetScsiAddr())
				{
					find = true;
					break;
				}
			}

			if(!find)
			{
				LtfsLgWarn("Refresh changer missing, ScsiAddr:"<<iterChanger->second.GetScsiAddr());
				iterChanger->second.SetMissing(true);
			}
		}

		for(vector<LSSCSI_INFO>::iterator iterDetail = changers.begin(); iterDetail!=changers.end(); ++iterDetail){
			info.mScsiAddr = iterDetail->scsiAddr;
			info.mVendor   = iterDetail->vendor;
			info.mProduct  = iterDetail->product;
			info.mVersion  = iterDetail->version;
			info.mStDev    = iterDetail->stDev;
			info.mSgDev    = iterDetail->sgDev;
			LtfsChanger changer(info);
			if(changer.Refresh(true, ltfsErr)){
				LtfsChangerMap::iterator  itt;
				if(FindChanger(changer.GetSerial(), itt)){
					LtfsLgDebug("Refresh changer, "<< itt->second.GetSerial());
					// update SCSI/dev info of the changer
					itt->second.SetLsSCSIInfo(info);
					itt->second.SetMissing(false);
					itt->second.Refresh(true, ltfsErr);
				}else{
					boost::unique_lock<boost::shared_mutex> lock(mapMutex_);
					LtfsLgDebug("Add changer to changers_, "<<changer.GetSerial());
					changers_.insert(LtfsChangerMap::value_type(changer.GetSerial(), changer));
				}
			} // if(Refresh
			else{
				LtfsLgError("Failed to refresh new changer, SCSI: " << iterDetail->scsiAddr);
			}
		}// for(changers

		//deal with missing here

		LtfsLgDebug("changers_.size " << changers_.size());
#endif
		//
		// check changer/drive conn/disconn events
		//
		map<string, LtfsChangerInfo> newChangers = GetRefreshChangerInfo();
		LtfsLgDebug("newChangers = " << newChangers.size());
		for(map<string, LtfsChangerInfo>::iterator itNew = newChangers.begin(); itNew != newChangers.end(); itNew++){
			LtfsLgDebug("bInited_ = " << bInited_ << ", itNew->second.mStatus = " << itNew->second.mStatus);
			if(oldChangers.find(itNew->first) == oldChangers.end()){
				if(bInited_ && itNew->second.mStatus != CHANGER_STATUS_DISCONNECTED){
					LtfsLgEvent(EVENT_LEVEL_INFO, "Lib_Conn", "Library " << itNew->first << " connected.");
				}
				if(itNew->second.mStatus == CHANGER_STATUS_DISCONNECTED){
					LtfsLgEvent(EVENT_LEVEL_CRITICAL, "Lib_Disconn", "Library " << itNew->first << " disconnected.");
				}
			}
		}
		for(map<string, LtfsChangerInfo>::iterator itOld = oldChangers.begin(); itOld != oldChangers.end(); itOld++){
			map<string, LtfsChangerInfo>::iterator itNew = newChangers.find(itOld->first);
			if(itNew == newChangers.end()){
				if(itOld->second.mStatus != CHANGER_STATUS_DISCONNECTED){
					LtfsLgEvent(EVENT_LEVEL_CRITICAL, "Lib_Disconn", "Library " << itOld->first << " disconnected.");
				}
			}else{
				LtfsLgDebug("itOld->second.mStatus = " << itOld->second.mStatus << ", itNew->second.mStatus = " << itNew->second.mStatus);
				if(itOld->second.mStatus != CHANGER_STATUS_DISCONNECTED && itNew->second.mStatus == CHANGER_STATUS_DISCONNECTED){
					LtfsLgEvent(EVENT_LEVEL_CRITICAL, "Lib_Disconn", "Library " << itOld->first << " disconnected.");
				}
				if(itOld->second.mStatus == CHANGER_STATUS_DISCONNECTED && itNew->second.mStatus != CHANGER_STATUS_DISCONNECTED){
					LtfsLgEvent(EVENT_LEVEL_INFO, "Lib_Conn", "Library " << itOld->first << " connected.");
				}
			}
		}

		map<string, REFRESH_DRIVE_INFO> newDrives = GetRefreshDriveInfo();
		LtfsLgDebug("newDrives = " << newDrives.size());
		for(map<string, REFRESH_DRIVE_INFO>::iterator itNew = newDrives.begin(); itNew != newDrives.end(); itNew++){
			LtfsLgDebug("bInited_ = " << bInited_ << ", itNew->second.info.mStatus = " << itNew->second.info.mStatus);
			if(oldDrives.find(itNew->first) == oldDrives.end()){
				if(bInited_ && itNew->second.info.mStatus != DRIVE_STATUS_DISCONNECTED){
					//new drive connected event
					LtfsEvent(EVENT_LEVEL_INFO, "Drv_Conn", "Drive " << itNew->second.info.mLogicSlotID
							<< " (" << itNew->second.info.mSerial << ") on changer " << itNew->second.changerSerial << " connected.");
				}
				if(itNew->second.info.mStatus == DRIVE_STATUS_DISCONNECTED){
					//new drive connected event
					LtfsEvent(EVENT_LEVEL_ERR, "Inv_Drv_Disconn", "Drive " << itNew->second.info.mLogicSlotID
							<< " (" << itNew->second.info.mSerial << ") on changer " << itNew->second.changerSerial << " disconnected.");
				}
			}
		}
		for(map<string, REFRESH_DRIVE_INFO>::iterator itOld = oldDrives.begin(); itOld != oldDrives.end(); itOld++){
			map<string, REFRESH_DRIVE_INFO>::iterator itNew = newDrives.find(itOld->first);
			bool bDisconn = false;
			if(itNew == newDrives.end()){
				bDisconn = true;
			}else{
				LtfsLgDebug("itOld->second.info.mStatus = " << itOld->second.info.mStatus << ", itNew->second.info.mStatus = " << itNew->second.info.mStatus);
				if(itOld->second.info.mStatus != DRIVE_STATUS_DISCONNECTED && itNew->second.info.mStatus == DRIVE_STATUS_DISCONNECTED){
					bDisconn = true;
				}
				if(itOld->second.info.mStatus == DRIVE_STATUS_DISCONNECTED && itNew->second.info.mStatus != DRIVE_STATUS_DISCONNECTED){
					// drive connected event
					LtfsEvent(EVENT_LEVEL_INFO, "Drv_Conn", "Drive " << itNew->second.info.mLogicSlotID
							<< " (" << itNew->second.info.mSerial << ") on changer " << itNew->second.changerSerial << " connected.");
				}
			}
			// drive disconnected event
			if(bDisconn){
				LtfsEvent(EVENT_LEVEL_ERR, "Inv_Drv_Disconn", "Drive " << itOld->second.info.mLogicSlotID
						<< " (" << itOld->second.info.mSerial << ") on changer " << itOld->second.changerSerial << " disconnected.");
			}
		}

		bInited_ = true;
		return true;
	}

	map<string, LtfsChangerInfo> LtfsLibraries::GetRefreshChangerInfo()
	{
		map<string, LtfsChangerInfo> changerMap;
		changerMap.clear();
		vector<LtfsChangerInfo> changers;
		LtfsError lfsErr;
		if(!GetChangerList(changers, lfsErr)){
			return changerMap;
		}
		for(unsigned int i = 0; i < changers.size(); i++){
			changerMap[changers[i].mSerial] = changers[i];
		}

		return changerMap;
	}

	map<string, REFRESH_DRIVE_INFO> LtfsLibraries::GetRefreshDriveInfo()
	{
		map<string, REFRESH_DRIVE_INFO> driveMap;
		driveMap.clear();
		vector<LtfsChangerInfo> changers;
		LtfsError lfsErr;
		if(!GetChangerList(changers, lfsErr)){
			return driveMap;
		}
		for(unsigned int i = 0; i < changers.size(); i++){
			vector<LtfsDriveInfo> drives;
			if(!GetDriveList(changers[i].mSerial, drives, lfsErr)){
				continue;
			}
			for(unsigned int j = 0; j < drives.size(); j++){
				REFRESH_DRIVE_INFO drive;
				drive.info = drives[j];
				drive.changerSerial = changers[i].mSerial;
				driveMap[drives[j].mSerial] = drive;
			}
		}

		return driveMap;
	}

	bool LtfsLibraries::PreventChangerMediaRemoval(const string& changerSerial, bool bPrevent)
	{
		LtfsChangerMap::iterator iter;

		if(!FindChanger(changerSerial, iter))
		{
			LtfsLgError("PreventChangerMediaRemoval: no changer been found, changerSerial:"<<changerSerial);
			return false;
		}

		return iter->second.PreventAllowMediaRemoval(bPrevent);
	}

	bool
	LtfsLibraries::GetChangerList(vector<LtfsChangerInfo>& changers, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::GetChangerList start");

		boost::shared_lock<boost::shared_mutex> lock(mapMutex_);
		for(LtfsChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			//if(!iter->second.GetMissing())
			{
				LtfsChangerInfo changer;
				iter->second.GetChangerInfo(changer);
				if(changer.mSerial != ""){
					changers.push_back(changer);
				}
			}
		}

		//if(changers.size()>0)
		//	return true;

		LtfsLgDebug("GetChangerList return true");
		return true;
	}

	bool
	LtfsLibraries::GetDriveList(const string& changerSerial, vector<LtfsDriveInfo>& drives, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::GetDriveList start");

		LtfsChangerMap::iterator iter;

		if(!FindChanger(changerSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_CHANGER_SERIAL);
			LtfsLgError("GetDriveList no changer been found, changerSerial:"<<changerSerial);
			return false;
		}

		return iter->second.GetDriveList(drives, ltfsErr);
	}

	bool
	LtfsLibraries::GetTapeList(const string& changerSerial, vector<LtfsTapeInfo>& tapes, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::GetTapeList start");

		LtfsChangerMap::iterator iter;

		if(!FindChanger(changerSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_CHANGER_SERIAL);
			LtfsLgError("GetTapeList no changer been found, changerSerial:"<<changerSerial);
			return false;
		}

		return iter->second.GetTapeList(tapes, ltfsErr);
	}

	bool
	LtfsLibraries::GetSlotList(const string& changerSerial, vector<LtfsSlotInfo>& slots, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::GetSlotList start");

		LtfsChangerMap::iterator iter;

		if(!FindChanger(changerSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_CHANGER_SERIAL);
			LtfsLgError("GetSlotList no changer been found, changerSerial:"<<changerSerial);
			return false;
		}

		return iter->second.GetSlotList(slots, ltfsErr);
	}

	bool LtfsLibraries::IsMailSlotBusy()
	{
		boost::shared_lock<boost::shared_mutex> lock(mapMutex_);

		for(LtfsChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			if(!iter->second.GetMissing())
			{
				vector<LtfsMailSlotInfo> slots;
				LtfsError ltfsErr;
				iter->second.GetMailSlotList(slots, ltfsErr);
				for(vector<LtfsMailSlotInfo>::iterator it = slots.begin(); it != slots.end(); it++){
					if(it->mIsEmpty == false){
						return true;
					}
				}
			}
		}
		return false;
	}

	bool
	LtfsLibraries::GetMailSlotList(const string& changerSerial, vector<LtfsMailSlotInfo>& mailslots, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::GetMailSlotList start");

		LtfsChangerMap::iterator iter;

		if(!FindChanger(changerSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_CHANGER_SERIAL);
			LtfsLgError("GetMailSlotList no changer been found, changerSerial:"<<changerSerial);
			return false;
		}

		return iter->second.GetMailSlotList(mailslots, ltfsErr);
	}

	bool
	LtfsLibraries::MoveCartridge(const string& changerSerial, int srcSlot, int dstSlot, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::MoveCartridge start");

		LtfsChangerMap::iterator iter;

		if(!FindChanger(changerSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_CHANGER_SERIAL);
			LtfsLgError("MoveCartridge no changer been found, changerSerial:"<<changerSerial);
			return false;
		}

		bool bRet = iter->second.MoveCartridge(srcSlot, dstSlot, ltfsErr);
		LtfsLgDebug("LtfsLibraries::MoveCartridge end");
		return bRet;
	}

	bool
	LtfsLibraries::Mount(const string& changerSerial, const string& driveSerial, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::Mount start");
		FIND_CHANGER_DRIVE("Mount", changerSerial, driveSerial);

		return iter->second.Mount(driveSerial, ltfsErr);
	}

	bool
	LtfsLibraries::CheckTape(const string& changerSerial, const string& driveSerial, CHECK_TAPE_FLAG flag, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::CheckTape start");
		FIND_CHANGER_DRIVE("CheckTape", changerSerial, driveSerial);

		return iter->second.CheckTape(driveSerial, flag, ltfsErr);
	}

	bool
	LtfsLibraries::UnMount(const string& changerSerial, const string& driveSerial, UInt64_t& genIndex, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::UnMount start");
		FIND_CHANGER_DRIVE("UnMount", changerSerial, driveSerial);

		bool bRet = iter->second.UnMount(driveSerial, genIndex, ltfsErr);
		return bRet;
	}

	bool
	LtfsLibraries::UnFormat(const string& changerSerial, const string& driveSerial, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::UnFormat start");
		FIND_CHANGER_DRIVE("UnFormat", changerSerial, driveSerial);

		return iter->second.UnFormat(driveSerial, ltfsErr);
	}

	bool
	LtfsLibraries::Format(const string& changerSerial, const string& driveSerial, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::Format start");
		FIND_CHANGER_DRIVE("Format", changerSerial, driveSerial);

		return iter->second.Format(driveSerial, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeBarcode(const string& changerSerial, const string& driveSerial, string& barcode, LtfsError& ltfsErr, bool bRefresh)
	{
		LtfsLgDebug("LtfsLibraries::GetLoadedTapeBarcode start");
		FIND_CHANGER_DRIVE("GetLoadedTapeBarcode", changerSerial, driveSerial);

		return iter->second.GetLoadedTapeBarcode(driveSerial, barcode, ltfsErr, bRefresh);
	}

	bool
	LtfsLibraries::SetLoadedTapeBarcode(const string& changerSerial, const string& driveSerial, const string& barcode, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(SetLoadedTapeBarcode, "SetLoadedTapeBarcode", changerSerial, driveSerial, barcode, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeUUID(const string& changerSerial, const string& driveSerial, string& uuid, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeUUID, "GetLoadedTapeUUID", changerSerial, driveSerial, uuid, ltfsErr);
		return iter->second.GetLoadedTapeUUID(driveSerial, uuid, ltfsErr);
	}
	bool
	LtfsLibraries::SetLoadedTapeUUID(const string& changerSerial, const string& driveSerial, const string& uuid, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(SetLoadedTapeUUID, "SetLoadedTapeUUID", changerSerial, driveSerial, uuid, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeLtfsFormat(const string& changerSerial, const string& driveSerial, int& ltfsFormat, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeLtfsFormat, "GetLoadedTapeLtfsFormat", changerSerial, driveSerial, ltfsFormat, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeLoadCounter(const string& changerSerial, const string& driveSerial, int& count, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeLoadCounter, "GetLoadedTapeLoadCounter", changerSerial, driveSerial, count, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeWPFlag(const string& changerSerial, const string& driveSerial, bool& bIsWP, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeWPFlag, "GetLoadedTapeWPFlag", changerSerial, driveSerial, bIsWP, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeMediaType(const string& changerSerial, const string& driveSerial, int& mediaType, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeMediaType, "GetLoadedTapeMediaType", changerSerial, driveSerial, mediaType, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeGenerationIndex(const string& changerSerial, const string& driveSerial, long long& index, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeGenerationIndex, "GetLoadedTapeGenerationIndex", changerSerial, driveSerial, index, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeCapacity(const string& changerSerial, const string& driveSerial, long long& freeCapacity, LtfsError& ltfsErr)
	{
		long long totalCapacity = 0;
		return GetLoadedTapeCapacity(changerSerial, driveSerial, freeCapacity, totalCapacity, ltfsErr);
	}

	bool
	LtfsLibraries::PhysicalSlotToLogicSlot(const string& changerSerial, int phySlot, int& logSlot, ENUM_SLOT_TYPE& slotType, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::PhyzicalSlotToLogicSlot start");

		LtfsChangerMap::iterator iter;

		if(!FindChanger(changerSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_CHANGER_SERIAL);
			LtfsLgError("PhyzicalSlotToLogicSlot no changer is found, changerSerial:"<<changerSerial);
			return false;
		}

		return iter->second.PhysicalSlotToLogicSlot(phySlot, logSlot, slotType, ltfsErr);
	}

	bool
	LtfsLibraries::LogicSlotToPhysicalSlot(const string& changerSerial, int logSlot, int& phySlot, ENUM_SLOT_TYPE slotType, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LogicSlotToPhysicalSlot::PhyzicalSlotToLogicSlot start");

		LtfsChangerMap::iterator iter;

		if(!FindChanger(changerSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_CHANGER_SERIAL);
			LtfsLgError("LogicSlotToPhysicalSlot no changer is found, changerSerial:"<<changerSerial);
			return false;
		}

		return iter->second.LogicSlotToPhysicalSlot(logSlot, phySlot, slotType, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeCapacity(const string& changerSerial, const string& driveSerial, long long& freeCapacity, long long& totalCapacity, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::GetLoadedTapeCapacity start");
		FIND_CHANGER_DRIVE("GetLoadedTapeCapacity", changerSerial, driveSerial);

		return iter->second.GetLoadedTapeCapacity(driveSerial, freeCapacity, totalCapacity, ltfsErr);
	}

	void
	LtfsLibraries::CheckDriveCleaningEvent(const string& changerSerial, const string& driveSerial, int cleaningStatus)
	{
		LtfsLgDebug("LtfsLibraries::CheckDriveCleaningEvent start");

		LtfsError lfsErr;
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter)){
			LtfsLgError("CheckDriveCleaningEvent no changer been found, changerSerial:"<<changerSerial);
			return;
		}

		bool bFound = false;
		int driveLogicId = 0;
		vector<LtfsDriveInfo> drives;
		if (GetDriveList(iter->second.GetSerial(), drives, lfsErr)) {
			for(unsigned j = 0; j < drives.size(); j++){
				if(drives[j].mSerial == driveSerial){
					bFound = true;
					driveLogicId = drives[j].mLogicSlotID;
					break;
				}
			}
		}
		if(false == bFound){
			LtfsLgError("CheckDriveCleaningEvent no drive been found, driveSerial:" << driveSerial);
			return;
		}

		string mapKey = changerSerial + ":" + driveSerial;
		if(cleaningStatus != CLEANING_REQUIRED){
			driveCleaningMap[mapKey] = 0;
			return;
		}

		if(driveCleaningMap.find(mapKey) == driveCleaningMap.end()
			|| time(NULL) - driveCleaningMap[mapKey] > DRIVE_CLEANING_EVENT_THROTTLE
		){
			// fire the event
			driveCleaningMap[mapKey] = time(NULL);
			LtfsLgEvent(EVENT_LEVEL_WARNING,"Drv_Dirty","Drive " << driveLogicId << " requires cleaning.");//Event/Notification
		}

		return;
	}

	bool
	LtfsLibraries::GetDriveCleaningStatus(const string& changerSerial, const string& driveSerial, int& cleaningStatus, LtfsError& ltfsErr, bool bForceRefresh)
	{
		LtfsLgDebug("LtfsLibraries::GetDriveCleaningStatus start");
		FIND_CHANGER_DRIVE("GetDriveCleaningStatus", changerSerial, driveSerial);

		bool bRet = iter->second.GetDriveCleaningStatus(driveSerial, cleaningStatus, ltfsErr, bForceRefresh);
		LtfsLgDebug("LtfsLibraries::GetDriveCleaningStatus: cleaningStatus = " << cleaningStatus << ", driveSerial = " << driveSerial << ", bRet = " << bRet << ".");
		return bRet;
	}

	bool
	LtfsLibraries::GetLoadedTapeStatus(const string& changerSerial, const string& driveSerial, int& status, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeStatus, "GetLoadedTapeStatus", changerSerial, driveSerial, status, ltfsErr);
	}

	bool
	LtfsLibraries::SetLoadedTapeStatus(const string& changerSerial, const string& driveSerial, int status, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(SetLoadedTapeStatus, "SetLoadedTapeStatus", changerSerial, driveSerial, status, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeFaulty(const string& changerSerial, const string& driveSerial, bool& faulty, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeFaulty, "GetLoadedTapeFaulty", changerSerial, driveSerial, faulty, ltfsErr);
	}

	bool
	LtfsLibraries::SetLoadedTapeFaulty(const string& changerSerial, const string& driveSerial, bool faulty, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(SetLoadedTapeFaulty, "SetLoadedTapeFaulty", changerSerial, driveSerial, faulty, ltfsErr);
	}

	bool
	LtfsLibraries::SetLoadedTapeGroup(const string& changerSerial, const string& driveSerial, const string& uuid, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(SetLoadedTapeGroup, "SetLoadedTapeGroup", changerSerial, driveSerial, uuid, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeGroup(const string& changerSerial, const string& driveSerial, string& uuid, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeGroup, "GetLoadedTapeGroup", changerSerial, driveSerial, uuid, ltfsErr);
	}

	bool
	LtfsLibraries::SetLoadedTapeDualCopy(const string& changerSerial, const string& driveSerial, const string& dualCopy, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(SetLoadedTapeDualCopy, "SetLoadedTapeDualCopy", changerSerial, driveSerial, dualCopy, ltfsErr);
	}

	bool
	LtfsLibraries::GetLoadedTapeDualCopy(const string& changerSerial, const string& driveSerial, string& dualCopy, LtfsError& ltfsErr)
	{
		GET_SET_LOADED_TAPE_ATTRIBUTE(GetLoadedTapeDualCopy, "GetLoadedTapeDualCopy", changerSerial, driveSerial, dualCopy, ltfsErr);
	}

	bool
	LtfsLibraries::GetWorkingStatus(const string& changerSerial, const string& driveSerial, bool& working, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::GetWorkingStatus start");

		LtfsChangerMap::iterator iter;

		if(!FindChanger(changerSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_CHANGER_SERIAL);
			LtfsLgError("GetWorkingStatus no changer been found, changerSerial:"<<changerSerial);
			return false;
		}

		return iter->second.GetWorkingStatus(driveSerial, working, ltfsErr);
	}

	SystemHwMode LtfsLibraries::GetSystemHwMode(bool bRefresh)
	{
		if(bRefresh){
			LtfsError lfsErr;
			Refresh(lfsErr);
		}
		SystemHwMode retMode = SYSTEM_HW_READY;
		boost::shared_lock<boost::shared_mutex> lock(mapMutex_);
		if(changers_.size() <= 0){
			retMode = SYSTEM_HW_NOT_READY;
		}
		for(LtfsChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			if(iter->second.GetHwMode(bRefresh) != CHANGER_HW_READY)
			{
				retMode = SYSTEM_HW_NOT_READY;
			}
		}

		LtfsLgDebug("GetSystemHwMode: retMode = " << retMode << ", bRefresh = " << bRefresh);
		return retMode;
	}

	bool
	LtfsLibraries::IsTapeMounted(const string& changerSerial, const string& driveSerial, bool& mounted, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::IsTapeMounted start");
		FIND_CHANGER_DRIVE("IsTapeMounted", changerSerial, driveSerial);
		return iter->second.IsTapeMounted(driveSerial, mounted, ltfsErr);
	}

	bool
	LtfsLibraries::OpenMailSlot(const string& changerSerial, int slotid, LtfsError& ltfsErr)
	{
		LtfsLgDebug("LtfsLibraries::OpenMailSlot start");

		LtfsChangerMap::iterator iter;

		if(!FindChanger(changerSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_CHANGER_SERIAL);
			LtfsLgError("OpenMailSlot no changer been found, changerSerial:"<<changerSerial);
			return false;
		}

		bool bRet = iter->second.OpenMailSlot(slotid, ltfsErr);
		if(bRet){
			// should refresh after mail slot is open
			iter->second.Refresh(false, ltfsErr);
		}
		return bRet;
	}

	bool LtfsLibraries::IsMailSlotAvailable(const string& changerSerial)
	{
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter))
		{
			LtfsLgError("IsMailSlotAvailable: no changer been found, changerSerial:" << changerSerial);
			return false;
		}

		return iter->second.IsMailSlotAvailable();
	}

	bool
	LtfsLibraries::GetChangerMailSlots(const string& changerSerial, vector<LtfsMailSlotInfo>& mailSlots, LtfsError& lfsErr)
	{
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter))
		{
			LtfsLgError("GetChangerMailSlots: no changer been found, changerSerial:" << changerSerial);
			return false;
		}

		return iter->second.GetChangerMailSlots(mailSlots, lfsErr);
	}

    bool LtfsLibraries::RequestMoveLock(const string& changerSerial, int millTimeOut)
    {
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter))
		{
			LtfsLgError("RequestMoveLock: no changer been found, changerSerial:" << changerSerial);
			return false;
		}

		return iter->second.RequestMoveLock(millTimeOut);
    }
    void LtfsLibraries::ReleaseMoveLock(const string& changerSerial)
    {
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter))
		{
			LtfsLgError("ReleaseMoveLock: no changer been found, changerSerial:" << changerSerial);
		}

		iter->second.ReleaseMoveLock();
    }

	ENUM_SLOT_TYPE LtfsLibraries::GetSlotType(const string& changerSerial, int slotId)
	{
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter))
		{
			LtfsLgError("GetSlotType: no changer been found, changerSerial:" << changerSerial);
			return SLOT_UNKNOWN;
		}

		return iter->second.GetSlotType(slotId);

	}

	string LtfsLibraries::GetDriveSerialBySlotId(const string& changerSerial, int slotId)
	{
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter))
		{
			LtfsLgError("GetDriveSerialBySlotId: no changer been found, changerSerial:" << changerSerial);
			return "";
		}

		return iter->second.GetDriveSerialBySlotId(slotId);

	}

	bool LtfsLibraries::IsTapeOffline(const string& changerSerial, int slotId)
	{
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter))
		{
			LtfsLgError("IsTapeOffline: no changer been found, changerSerial:" << changerSerial);
			return "";
		}

		return iter->second.IsTapeOffline(slotId);
	}

	bool LtfsLibraries::GetDriveInfoForTape(const string& barcode, LtfsDriveInfo& driveInfo)
	{
		vector<LtfsChangerInfo> changers;
		LtfsError ltfsErr;
		if(false == GetChangerList(changers, ltfsErr)){
			return false;
		}
		for(unsigned int i = 0; i < changers.size(); i++){
			LtfsChangerMap::iterator iter;
			if(!FindChanger(changers[i].mSerial, iter)){
				continue;
			}
			if(true == iter->second.GetDriveInfoForTape(barcode, driveInfo)){
				return true;
			}
		}

		return false;
	}

	bool LtfsLibraries::IsTapeInDrive(const string& barcode)
	{
		vector<LtfsChangerInfo> changers;
		LtfsError ltfsErr;
		if(false == GetChangerList(changers, ltfsErr)){
			return false;
		}
		for(unsigned int i = 0; i < changers.size(); i++){
			LtfsChangerMap::iterator iter;
			if(!FindChanger(changers[i].mSerial, iter)){
				continue;
			}
			if(true == iter->second.IsTapeInDrive(barcode)){
				return true;
			}
		}

		return false;
	}

	int LtfsLibraries::GetTapeNum(const string& changerSerial, bool bIncludeDrive, bool bIncludeMailSlot)
	{
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter)){
			return 0;
		}
		return iter->second.GetTapeNum(bIncludeDrive, bIncludeMailSlot);
	}
	int LtfsLibraries::GetSlotNum(const string& changerSerial)
	{
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter)){
			return 0;
		}
		return iter->second.GetSlotNum();
	}
	bool LtfsLibraries::IsTapeInMailSlot(const string& changerSerial, const string& barcode, LtfsMailSlotInfo& mailSlotInfo)
	{
		LtfsChangerMap::iterator iter;
		if(!FindChanger(changerSerial, iter)){
			return 0;
		}
		return iter->second.IsTapeInMailSlot(barcode, mailSlotInfo);
	}


} /* namespace ltfs_management */
