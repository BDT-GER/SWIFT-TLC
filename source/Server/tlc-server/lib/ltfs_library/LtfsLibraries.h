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
 * LtfsLibraries.h
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#pragma once

#include "LtfsChanger.h"
#include "../../ltfs_management/TapeLibraryMgr.h"
#include "../../ltfs_format/ltfsFormatThread.h"

const int DRIVE_CLEANING_EVENT_THROTTLE = 300;

namespace ltfs_management
{
	typedef map<string,LtfsChanger> LtfsChangerMap;
	typedef map<string, LtfsDrive> VsbDriveMap;

	struct REFRESH_DRIVE_INFO
	{
		LtfsDriveInfo	info;
		string			changerSerial;
	};

	class LtfsLibraries
	{
#ifdef MAKE_LFS_TOOL
	public:
#else
	private:
#endif
		static LtfsLibraries*
		Instance()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if(NULL == instance_)
			{
				instance_ = new LtfsLibraries();
			}

			return instance_;
		}

		static void
		Destory()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if( NULL != instance_)
			{
				instance_->changers_.clear();
				delete instance_;
				instance_ = NULL;
			}
		}

		virtual
		~LtfsLibraries();

	public:
		bool
		Refresh(LtfsError& ltfsErr);

		bool
		GetChangerList(vector<LtfsChangerInfo>& changers, LtfsError& ltfsErr);

		bool
		GetDriveList(const string& changerSerial, vector<LtfsDriveInfo>& drives, LtfsError& ltfsErr);

		bool
		GetTapeList(const string& changerSerial, vector<LtfsTapeInfo>& tapes, LtfsError& ltfsErr);

		bool
		GetSlotList(const string& changerSerial, vector<LtfsSlotInfo>& slots, LtfsError& ltfsErr);

		bool
		GetMailSlotList(const string& changerSerial, vector<LtfsMailSlotInfo>& mailslots, LtfsError& ltfsErr);

		bool
		MoveCartridge(const string& changerSerial, int srcSlot, int dstSlot, LtfsError& ltfsErr);

		bool
		Mount(const string& changerSerial, const string& driveSerial, LtfsError& ltfsErr);

		bool
		CheckTape(const string& changerSerial, const string& driveSerial, CHECK_TAPE_FLAG flag, LtfsError& ltfsErr);

		bool
		UnMount(const string& changerSerial, const string& driveSerial, UInt64_t& genIndex, LtfsError& ltfsErr);

		bool
		Format(const string& changerSerial, const string& driveSerial, LtfsError& ltfsErr);

		bool
		UnFormat(const string& changerSerial, const string& driveSerial, LtfsError& ltfsErr);

		bool
		GetLoadedTapeBarcode(const string& changerSerial, const string& driveSerial, string& barcode, LtfsError& ltfsErr, bool bRefresh = true);
		bool
		SetLoadedTapeBarcode(const string& changerSerial, const string& driveSerial, const string& barcode, LtfsError& ltfsErr);

		bool
		GetLoadedTapeLtfsFormat(const string& changerSerial, const string& driveSerial, int& ltfsFormat, LtfsError& ltfsErr);

		bool
		GetLoadedTapeLoadCounter(const string& changerSerial, const string& driveSerial, int& count, LtfsError& ltfsErr);

		bool
		GetLoadedTapeMediaType(const string& changerSerial, const string& driveSerial, int& mediaType, LtfsError& ltfsErr);

		bool
		GetLoadedTapeWPFlag(const string& changerSerial, const string& driveSerial, bool& bIsWP, LtfsError& ltfsErr);

		bool
		GetLoadedTapeGenerationIndex(const string& changerSerial, const string& driveSerial, long long& index, LtfsError& ltfsErr);

		bool
		GetLoadedTapeCapacity(const string& changerSerial, const string& driveSerial, long long& freeCapacity, LtfsError& ltfsErr);

		bool
		GetLoadedTapeCapacity(const string& changerSerial, const string& driveSerial, long long& freeCapacity, long long& totalCapacity, LtfsError& ltfsErr);

		bool
		GetLoadedTapeStatus(const string& changerSerial, const string& driveSerial, int& status, LtfsError& ltfsErr);
		bool
		SetLoadedTapeStatus(const string& changerSerial, const string& driveSerial, int status, LtfsError& ltfsErr);

		bool
		GetLoadedTapeFaulty(const string& changerSerial, const string& driveSerial, bool& faulty, LtfsError& ltfsErr);
		bool
		SetLoadedTapeFaulty(const string& changerSerial, const string& driveSerial, bool faulty, LtfsError& ltfsErr);

		bool
		SetLoadedTapeGroup(const string& changerSerial, const string& driveSerial, const string& uuid, LtfsError& ltfsErr);
		bool
		GetLoadedTapeGroup(const string& changerSerial, const string& driveSerial, string& uuid, LtfsError& ltfsErr);

		bool
		SetLoadedTapeDualCopy(const string& changerSerial, const string& driveSerial, const string& dualCopy, LtfsError& ltfsErr);
		bool
		GetLoadedTapeDualCopy(const string& changerSerial, const string& driveSerial, string& dualCopy, LtfsError& ltfsErr);

		bool
		SetLoadedTapeUUID(const string& changerSerial, const string& driveSerial, const string& dualCopy, LtfsError& ltfsErr);
		bool
		GetLoadedTapeUUID(const string& changerSerial, const string& driveSerial, string& dualCopy, LtfsError& ltfsErr);

		bool
		GetWorkingStatus(const string& changerSerial, const string& driveSerial, bool& working, LtfsError& ltfsErr);

		bool
		IsTapeMounted(const string& changerSerial, const string& driveSerial, bool& mounted, LtfsError& ltfsErr);

		bool
		OpenMailSlot(const string& changerSerial, int slotid, LtfsError& ltfsErr);

		bool
		PhysicalSlotToLogicSlot(const string& changerSerial, int phySlot, int& logSlot, ENUM_SLOT_TYPE& slotType, LtfsError& ltfsErr);

		bool
		LogicSlotToPhysicalSlot(const string& changerSerial, int logSlot, int& phySlot, ENUM_SLOT_TYPE slotType, LtfsError& ltfsErr);

		bool
		GetDriveCleaningStatus(const string& changerSerial, const string& driveSerial, int& cleaningStatus, LtfsError& ltfsErr, bool bForceRefresh = false);

		void
		CheckDriveCleaningEvent(const string& changerSerial, const string& driveSerial, int cleaningStatus);

		bool
		PreventChangerMediaRemoval(const string& changerSerial, bool bPrevent);

		bool
		IsMailSlotAvailable(const string& changerSerial);

		bool IsMailSlotBusy();

		SystemHwMode GetSystemHwMode(bool bRefresh);

		bool
		GetChangerMailSlots(const string& changerSerial, vector<LtfsMailSlotInfo>& mailSlots, LtfsError& lfsErr);

		ENUM_SLOT_TYPE GetSlotType(const string& changerSerial, int slotId);

		string GetDriveSerialBySlotId(const string& changerSerial, int slotId);

		bool IsTapeOffline(const string& changerSerial, int slotId);

		bool IsTapeInDrive(const string& barcode);
		bool GetDriveInfoForTape(const string& barcode, LtfsDriveInfo& driveInfo);

		int GetTapeNum(const string& changerSerial, bool bIncludeDrive, bool bIncludeMailSlot);
		int GetSlotNum(const string& changerSerial);
		bool IsTapeInMailSlot(const string& changerSerial, const string& barcode, LtfsMailSlotInfo& mailSlotInfo);
        bool RequestMoveLock(const string& changerSerial, int millTimeOut);
        void ReleaseMoveLock(const string& changerSerial);

	private:
		LtfsLibraries();

		bool
		FindChanger(const string& changerSerial, LtfsChangerMap::iterator& iter);
		bool FindDrive(const string& driveSerial, VsbDriveMap::iterator& iter);

		bool
		FindChangerByScsi(const string& scsiAddr, LtfsChangerMap::iterator& iter);

		map<string, LtfsChangerInfo> GetRefreshChangerInfo();
		map<string, REFRESH_DRIVE_INFO> GetRefreshDriveInfo();

	private:
		boost::shared_mutex	 			refreshMutex_;
		LtfsChangerMap					changers_;
		boost::shared_mutex 			mapMutex_;
		bool							bInited_;
		map<string, time_t>				driveCleaningMap;

		static boost::mutex 			instanceMutex_;
		static LtfsLibraries * 			instance_;

		VsbDriveMap						drives_;

		friend TapeLibraryMgr* TapeLibraryMgr::Instance();
		friend void TapeLibraryMgr::Destroy();
		friend class FormatThread;
	};

} /* namespace ltfs_management */

