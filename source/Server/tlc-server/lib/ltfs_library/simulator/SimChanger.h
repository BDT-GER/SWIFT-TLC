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
 * SimChanger.h
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#pragma once

#include "SimSlot.h"
#include "SimDrive.h"
#include "SimMailSlot.h"
#include "SimTape.h"

namespace ltfs_management
{

	struct ChangerDetail
	{
		int m_Status;

		string	m_Serial;
		string	m_ScsiAddr;
		string	m_Vendor;
		string	m_Product;
		string	m_Version;
		string	m_StDev;
		string	m_SgDev;
		//bool	m_AutoClean;

	};

	class SimChanger
	{
	public:
		SimChanger(struct ChangerDetail& detail);

		virtual
		~SimChanger();

		void
		GetDetail(struct ChangerDetail& detail);

		string
		GetSerial() const;

		bool
		GetDriveList(vector<struct DriveDetail> & drives);

		bool
		GetSlotList(vector<struct SlotDetail> & slots);

		bool
		GetMailSlotList(vector<struct MailSlotDetail> & mailSlots);

		bool
		GetTapeList(vector<struct TapeDetail> & tapes);

		void
		RefreshDriveMap(vector<SimDrive>& drives);

		void
		RefreshSlotMap(vector<SimSlot>& slots);

		void
		RefreshMailSlotMap(vector<SimMailSlot>& mailslots);

		void
		RefreshTapeMap(vector<SimTape>& tapes);

		bool
		MoveTape(int srcSlotId, int dstSlotId);

		bool
		GetDriveDetail(string& driveScsiAddr, struct DriveDetail& detail);

		bool
		GetTapeDetail(string& barcode, struct TapeDetail& detail);

		bool
		Format(string& label, int slotId);

		bool
		IsMounted(int slotId, bool& mounted);

		bool
		Mount(int slotId);

		bool
		Umount(int slotId);

		bool
		GetTapeGroupUUID(int slotId, string& uuid);

		bool
		SetTapeGroupUUID(int slotId, const string& uuid);

		bool GetTapeDualCopy(int slotId, string& dualCopy);
		bool SetTapeDualCopy(int slotId, const string& dualCopy);

		bool
		GetTapeFaulty(int slotId, bool& faulty);

		bool
		SetTapeFaulty(int slotId, bool faulty);

		bool
		GetTapeStatus(int slotId, int& status);

		bool
		SetTapeStatus(int slotId, int status);

		bool
		GetTapeMediaType(int slotId, int& mediaType);

		bool
		GetTapeFormatType(int slotId, int& type);

		bool
		GetTapeGenerationIndex(int slotId, long long& index);

		bool
		SetTapeGenerationIndex(int slotId, long long index);

		bool
		GetTapeCapacity(string& key, Int64_t& totalCapacity, Int64_t& freeCapacity);

		bool
		Read(string& barcode, size_t size);

		bool
		Write(string& barcode, size_t size);

		void
		SetReadErrNum(string& barcode, int n);

		void
		SetWriteErrNum(string& barcode, int n);

	private:
		SlotType
		GetSlotType(int slotId);

		bool
		GetBarcode(int slotId, string& barcode, string& key);

		bool
		SetBarcode(int slotId, const string& barcode, const string& key);

		bool
		SetSlotID(int slotId, string& key);

		bool
		PhysicalSlotToLogicalSlot(int phySlot, int& logicalSlot);

	private:
		typedef map<int, SimDrive> 		DriveMap;
		typedef map<int, SimSlot> 		SlotMap;
		typedef map<int, SimMailSlot> 	MailSlotMap;
		typedef map<string, SimTape> 	TapeMap;

		DriveMap						drives_;
		SlotMap							slots_;
		MailSlotMap						mailslots_;
		TapeMap							tapes_;
		struct ChangerDetail			detail_;

		int 							mailSlotNum;
		int 							mailSlotStart;
		int 							slotStart;
		int 							driveSlotStart;


		static boost::mutex 			mutex_;
	};

} /* namespace ltfs_management */

