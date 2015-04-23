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
 * LtfsDrive.h
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#pragma once


#include "CmnFunc.h"

namespace ltfs_management
{
	class LtfsDrive : public LtfsScsiDevice, public LtfsSlot
	{
	public:
		friend class LtfsChanger;
		friend class LtfsLibraries;

		virtual
		~LtfsDrive();

		bool
		Refresh();

		virtual void
		SetMissing(bool missing=true);

		virtual bool
		GetMissing();

		void
		GetDriveInfo(LtfsDriveInfo& drive);

		bool
		Mount(LtfsError& ltfsErr);

		bool
		CheckTape(CHECK_TAPE_FLAG flag, LtfsError& ltfsErr);

		bool
		UnMount(UInt64_t& genIndex, LtfsError& ltfsErr);

		bool
		Format(LtfsError& ltfsErr);

		bool
		UnFormat(LtfsError& ltfsErr);

		bool
		GetLoadedTapeBarcode(string& barcode, LtfsError& ltfsErr, bool bRfresh = false);
		bool
		SetLoadedTapeBarcode(const string& barcode, LtfsError& ltfsErr);

		bool
		GetLoadedTapeLtfsFormat(int& ltfsFormat, LtfsError& ltfsErr);

		bool
		GetLoadedTapeLoadCounter(int& count, LtfsError& ltfsErr);

		bool
		GetLoadedTapeMediaType(int& mediaType, LtfsError& ltfsErr);

		bool
		GetLoadedTapeMediumType(int& mediumType, LtfsError& ltfsErr);

		bool
		GetLoadedTapeWPFlag(bool& bIsWP, LtfsError& ltfsErr);

		bool
		GetLoadedTapeGenerationIndex(long long& index, LtfsError& ltfsErr);

		bool
		GetLoadedTapeCapacity(long long& freeCapacity, LtfsError& ltfsErr);

		bool
		GetLoadedTapeCapacity(long long& freeCapacity, long long& totalCapacity, LtfsError& ltfsErr);

		bool
		GetLoadedTapeStatus(int& status, LtfsError& ltfsErr);

		bool
		SetLoadedTapeStatus(int status, LtfsError& ltfsErr);

		bool
		GetLoadedTapeFaulty(bool& faulty, LtfsError& ltfsErr);

		bool
		SetLoadedTapeFaulty(bool faulty, LtfsError& ltfsErr);

		bool
		SetLoadedTapeGroup(const string& uuid, LtfsError& ltfsErr);

		bool
		GetLoadedTapeGroup(string& uuid, LtfsError& ltfsErr);

		bool
		SetLoadedTapeDualCopy(const string& dualCopy, LtfsError& ltfsErr);
		bool
		GetLoadedTapeDualCopy(string& dualCopy, LtfsError& ltfsErr);

		bool
		SetLoadedTapeUUID(const string& uuid, LtfsError& ltfsErr);
		bool
		GetLoadedTapeUUID(string& uuid, LtfsError& ltfsErr);

		bool
		GetWorkingStatus(bool& working, LtfsError& ltfsErr);

		bool
		IsTapeMounted(bool& mounted, LtfsError& ltfsErr);

		bool GetDriveCleaningStatus(int& cleaningStatus, LtfsError& ltfsErr, bool bForceRefresh = false);

		bool IsAvailable() const;

	private:
		bool RefreshDriveCleaningStatus();

	private:
		LtfsDrive(int slotid, int logicSlotId, const string& serial, bool isFullHight, int generation,
				const string& vendor, const string& product, const string& stDev = "", const string& sgDev = "");
		int 	status_;
		int 	cleaningStatus_;
		int 	interfaceType_;
		bool	mounted_;
		bool	isFullHight_;
		int		generation_;

		bool 	inited_;
		bool	hasTape_;
		bool	hasTapeCheked_;

	};

} /* namespace ltfs_management */

