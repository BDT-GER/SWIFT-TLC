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
 * LtfsChanger.h
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#pragma once

#ifdef SIMULATOR
#include "simulator/Simulator.h"
#else
#include "CmnFunc.h"
#endif
#include "LtfsError.h"
#include "LtfsDetails.h"
#include "LtfsTape.h"
#include "LtfsScsiDevice.h"
#include "LtfsSlot.h"
#include "LtfsDrive.h"
#include "LtfsMailSlot.h"

namespace ltfs_management
{
	typedef map<int, LtfsDrive> 	LtfsDriveMap;
	typedef map<int, LtfsSlot> 		LtfsSlotMap;
	typedef map<int, LtfsMailSlot> 	LtfsMailSlotMap;
	typedef map<string, LtfsTape*>	LtfsTapeMap;

	class LtfsChanger : public LtfsScsiDevice
	{
	public:
		friend class LtfsLibraries;

		virtual
		~LtfsChanger();

		virtual void
		SetMissing(bool missing=true);

		virtual bool
		GetMissing();

		void
		GetChangerInfo(LtfsChangerInfo& changer);

		bool
		Refresh(bool bRefreshDrive, LtfsError& ltfsErr);

		bool
		GetDriveList(vector<LtfsDriveInfo>& drives, LtfsError& ltfsErr);

		bool
		GetTapeList(vector<LtfsTapeInfo>& tapes, LtfsError& ltfsErr);

		bool
		GetSlotList(vector<LtfsSlotInfo>& slots, LtfsError& ltfsErr);

		bool
		GetMailSlotList(vector<LtfsMailSlotInfo>& mailslots, LtfsError& ltfsErr);

		bool
		MoveCartridge(int srcSlot, int dstSlot, LtfsError& ltfsErr);

		bool
		Mount(const string& driveSerial, LtfsError& ltfsErr);

		bool
		CheckTape(const string& driveSerial, CHECK_TAPE_FLAG flag, LtfsError& ltfsErr);

		bool
		UnMount(const string& driveSerial, UInt64_t& genIndex, LtfsError& ltfsErr);

		bool
		Format(const string& driveSerial, LtfsError& ltfsErr);

		bool
		UnFormat(const string& driveSerial, LtfsError& ltfsErr);

		bool
		GetLoadedTapeBarcode(const string& driveSerial, string& barcode, LtfsError& ltfsErr, bool bRefresh = true);
		bool
		SetLoadedTapeBarcode(const string& driveSerial, const string& barcode, LtfsError& ltfsErr);

		bool
		GetLoadedTapeUUID(const string& driveSerial, string& tapeUUID, LtfsError& ltfsErr);
		bool
		SetLoadedTapeUUID(const string& driveSerial, const string& tapeUUID, LtfsError& ltfsErr);

		bool
		GetLoadedTapeLtfsFormat(const string& driveSerial, int& ltfsFormat, LtfsError& ltfsErr);

		bool
		GetLoadedTapeLoadCounter(const string& driveSerial, int& count, LtfsError& ltfsErr);

		bool
		GetLoadedTapeMediaType(const string& driveSerial, int& mediaType, LtfsError& ltfsErr);

		bool
		GetLoadedTapeWPFlag(const string& driveSerial, bool& bIsWP, LtfsError& ltfsErr);

		bool
		GetLoadedTapeGenerationIndex(const string& driveSerial, long long& index, LtfsError& ltfsErr);

		bool
		GetLoadedTapeCapacity(const string& driveSerial, long long& freeCapacity, LtfsError& ltfsErr);

		bool
		GetLoadedTapeCapacity(const string& driveSerial, long long& freeCapacity, long long& totalCapacity, LtfsError& ltfsErr);

		bool
		GetLoadedTapeStatus(const string& driveSerial, int& status, LtfsError& ltfsErr);

		bool
		SetLoadedTapeStatus(const string& driveSerial, int status, LtfsError& ltfsErr);

		bool
		GetLoadedTapeFaulty(const string& driveSerial, bool& faulty, LtfsError& ltfsErr);

		bool
		SetLoadedTapeFaulty(const string& driveSerial, bool faulty, LtfsError& ltfsErr);

		bool
		SetLoadedTapeGroup(const string& driveSerial, const string& uuid, LtfsError& ltfsErr);

		bool
		GetLoadedTapeGroup(const string& driveSerial, string& uuid, LtfsError& ltfsErr);

		bool
		SetLoadedTapeDualCopy(const string& driveSerial, const string& dualCopy, LtfsError& ltfsErr);

		bool
		GetLoadedTapeDualCopy(const string& driveSerial, string& dualCopy, LtfsError& ltfsErr);

		bool
		GetWorkingStatus(const string& driveSerial, bool& working, LtfsError& ltfsErr);

		bool
		PhysicalSlotToLogicSlot(int phySlot, int& logSlot, ENUM_SLOT_TYPE& slotType, LtfsError& ltfsErr);

		bool
		LogicSlotToPhysicalSlot(int logSlot, int& phySlot, ENUM_SLOT_TYPE slotType, LtfsError& ltfsErr);

		bool
		IsTapeMounted(const string& driveSerial, bool& mounted, LtfsError& ltfsErr);

		bool
		OpenMailSlot(int slotid, LtfsError& ltfsErr);

		bool
		GetDriveCleaningStatus(const string& driveSerial, int& cleaningStatus, LtfsError& ltfsErr, bool bForceRefresh = false);

		bool PreventAllowMediaRemoval(bool bPrevent);

		bool IsMailSlotAvailable();

		bool GetChangerMailSlots(vector<LtfsMailSlotInfo>& mailSlots, LtfsError& lfsErr);
		ENUM_SLOT_TYPE GetSlotType(int slotId);
		string GetDriveSerialBySlotId(int slotId);
		bool IsTapeOffline(int slotId);
		bool IsTapeInDrive(const string& barcode);
		bool GetDriveInfoForTape(const string& barcode, LtfsDriveInfo& driveInfo);
		ChangerHwMode GetHwMode(bool bRefresh);
		void SetLsSCSIInfo(ScsiInfo scsiInfo);

		int GetTapeNum(bool bIncludeDrive, bool bIncludeMailSlot);
		int GetSlotNum();
		bool IsTapeInMailSlot(const string& barcode, LtfsMailSlotInfo& mailSlotInfo);
        bool RequestMoveLock(int millTimeOut);
        void ReleaseMoveLock();

	private:
		LtfsChanger(const ScsiInfo& info);

#ifdef SIMULATOR
		bool
		RefreshSlot(vector<SlotDetail>& slots, LtfsError& ltfsErr);

		bool
		RefreshMailSlot(vector<MailSlotDetail>& mailSlots, LtfsError& ltfsErr);

		bool
		RefreshDrive(vector<DriveDetail>& drives, LtfsError& ltfsErr);

		bool
		RefreshTape(vector<TapeDetail>& tapes, LtfsError& ltfsErr);

#else
		bool
		RefreshSlot(vector<slotElement>& slots, LtfsError& ltfsErr);

		bool
		RefreshMailSlot(vector<mailSlotElement>& mailSlots, LtfsError& ltfsErr);

		bool
		RefreshDrive(vector<driveElement>& drives, LtfsError& ltfsErr);

		bool
		RefreshTape(vector<tapeElement>& tapes, LtfsError& ltfsErr);
#endif

		bool
		FindDriveBySerial(const string& serial, LtfsDriveMap::iterator& iter);

		void
		MoveTape(int srcSlot, int dstSlot);

		void RefreshDriveCleaningStatus(int slotId);

		boost::mutex* GetDriveLockBySlotId(int slotId);
		void CheckHwMode(bool bEvent = false);
		ChangerHwMode RefreshHwMode();


	private:
		LtfsDriveMap			drives_;
		LtfsSlotMap				slots_;
		LtfsMailSlotMap			mailslots_;
		LtfsTapeMap 			tapes_;

		int 					driveStart_;
		int 					mailSlotStart_;
		int 					slotStart_;
		int 					lastMailSlotNum_;
		bool					autoCleanMode_;
		ChangerHwMode			hwMode_;

		boost::shared_mutex*	mapMutex_;
		boost::timed_mutex*		moveMutex_;
	};

} /* namespace ltfs_management */

