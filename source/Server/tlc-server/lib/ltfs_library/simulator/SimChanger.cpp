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
 * SimChanger.cpp
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "SimChanger.h"

namespace ltfs_management
{

	SimChanger::SimChanger(struct ChangerDetail& detail)
	{
		// TODO Auto-generated constructor stub
		detail_.m_Product 	= detail.m_Product;
		detail_.m_ScsiAddr 	= detail.m_ScsiAddr;
		detail_.m_Serial 	= detail.m_Serial;
		detail_.m_SgDev 	= detail.m_SgDev;
		detail_.m_StDev 	= detail.m_StDev;
		detail_.m_Status 	= detail.m_Status;
		detail_.m_Vendor 	= detail.m_Vendor;
		detail_.m_Version 	= detail.m_Version;
	}

	SimChanger::~SimChanger()
	{
		// TODO Auto-generated destructor stub
		drives_.clear();
		slots_.clear();
		mailslots_.clear();
		tapes_.clear();
	}

	void
	SimChanger::GetDetail(struct ChangerDetail& detail)
	{
		detail.m_Product 	= detail_.m_Product;
		detail.m_ScsiAddr 	= detail_.m_ScsiAddr;
		detail.m_Serial 	= detail_.m_Serial;
		detail.m_SgDev 		= detail_.m_SgDev;
		detail.m_StDev 		= detail_.m_StDev;
		detail.m_Status 	= detail_.m_Status;
		detail.m_Vendor 	= detail_.m_Vendor;
		detail.m_Version 	= detail_.m_Version;
	}

	string
	SimChanger::GetSerial() const
	{
		return detail_.m_Serial;
	}

	bool
	SimChanger::PhysicalSlotToLogicalSlot(int phySlot, int& logicalSlot)
	{
		int mailSlotNum = mailslots_.size();

		// drive
		for(DriveMap::iterator iter = drives_.begin(); iter!=drives_.end(); ++iter){
			if(iter->second.GetSlotID() == phySlot){
				logicalSlot = iter->second.GetSlotID() - driveSlotStart + 1;
				return true;
			}
		}

		// storage slot
		for(SlotMap::iterator iter = slots_.begin(); iter!=slots_.end(); ++iter){
			if(iter->second.GetSlotID() == phySlot){
				logicalSlot = iter->second.GetSlotID() - slotStart + mailSlotNum + 1;
				return true;
			}
		}

		// mail slot
		for(MailSlotMap::iterator iter = mailslots_.begin(); iter != mailslots_.end(); ++iter){
			if(iter->second.GetSlotID() == phySlot){
				logicalSlot = iter->second.GetSlotID() - mailSlotStart + 1;
				return true;
			}
		}
		return false;
	}

	bool
	SimChanger::GetDriveList(vector<struct DriveDetail> & drives)
	{
		drives.clear();

		for(DriveMap::iterator iter=drives_.begin();
				iter!=drives_.end(); ++iter)
		{
			struct DriveDetail detail;
			iter->second.GetDetail(detail);
			detail.m_LogicSlotId = detail.m_SlotId - driveSlotStart + 1;
			drives.push_back(detail);
		}

		if(drives.size()==0)
		{
			//LogError("tony", "not find drive");

			return false;
		}

		return true;
	}

	bool
	SimChanger::GetSlotList(vector<struct SlotDetail> & slots)
	{
		slots.clear();

		for(SlotMap::iterator iter=slots_.begin();
				iter!=slots_.end(); ++iter)
		{
			struct SlotDetail detail;
			iter->second.GetDetail(detail);
			detail.m_LogicSlotId = detail.m_SlotID - slotStart + mailSlotNum + 1;
			slots.push_back(detail);
		}

		if(slots.size()==0)
			return false;

		return true;
	}

	bool
	SimChanger::GetMailSlotList(vector<struct MailSlotDetail> & mailSlots)
	{

		mailSlots.clear();

		for(MailSlotMap::iterator iter=mailslots_.begin();
				iter!=mailslots_.end(); ++iter)
		{
			struct MailSlotDetail detail;
			iter->second.GetDetail(detail);
			detail.m_LogicSlotId = detail.m_SlotID - mailSlotStart + 1;
			mailSlots.push_back(detail);
		}

		if(mailSlots.size()==0)
			return false;

		return true;
	}

	bool
	SimChanger::GetTapeList(vector<struct TapeDetail> & tapes)
	{
		tapes.clear();

		for(TapeMap::iterator iter=tapes_.begin();
				iter!=tapes_.end(); ++iter)
		{
			struct TapeDetail detail;
			iter->second.GetDetail(detail);
			if(false == PhysicalSlotToLogicalSlot(detail.m_SlotID, detail.m_LogicSlotId)){
				//TODO
			}
			tapes.push_back(detail);
		}

		if(tapes.size()==0)
			return false;

		return true;
	}

	void
	SimChanger::RefreshDriveMap(vector<SimDrive>& drives)
	{
		drives_.clear();

		driveSlotStart = 99999;
		for(vector<SimDrive>::iterator iter=drives.begin();
				iter!=drives.end(); ++iter)
		{
			drives_.insert(DriveMap::value_type(iter->GetSlotID(),*iter));
			if(driveSlotStart > iter->GetSlotID()){
				driveSlotStart = iter->GetSlotID();
			}
		}
	}

	void
	SimChanger::RefreshSlotMap(vector<SimSlot>& slots)
	{
		slots_.clear();

		slotStart = 99999;
		for(vector<SimSlot>::iterator iter=slots.begin();
				iter!=slots.end(); ++iter)
		{
			slots_.insert(SlotMap::value_type(iter->GetSlotID(),*iter));
			if(slotStart > iter->GetSlotID()){
				slotStart = iter->GetSlotID();
			}
		}
	}

	void
	SimChanger::RefreshMailSlotMap(vector<SimMailSlot>& mailslots)
	{
		mailslots_.clear();

		mailSlotStart = 99999;
		mailSlotNum = 0;
		for(vector<SimMailSlot>::iterator iter=mailslots.begin();
				iter!=mailslots.end(); ++iter)
		{
			mailSlotNum++;
			mailslots_.insert(MailSlotMap::value_type(iter->GetSlotID(),*iter));
			if(mailSlotStart > iter->GetSlotID()){
				mailSlotStart = iter->GetSlotID();
			}
		}
	}

	void
	SimChanger::RefreshTapeMap(vector<SimTape>& tapes)
	{
		tapes_.clear();

		for(vector<SimTape>::iterator iter=tapes.begin();
				iter!=tapes.end(); ++iter)
		{
			tapes_.insert(TapeMap::value_type(iter->GetKey(),*iter));
		}
	}

	bool
	SimChanger::MoveTape(int srcSlotId, int dstSlotId)
	{
		string barcodeSrc;
		string barcodeDst;
		string keySrc;
		string keyDst;

		//boost::lock_guard<boost::mutex> lock(mutex_);

		if(!GetBarcode(srcSlotId,barcodeSrc, keySrc)
				|| keySrc.empty())
		{

			return false;
		}

		if(!GetBarcode(dstSlotId,barcodeDst, keyDst)
				|| !keyDst.empty())
		{

			return false;
		}

		TapeMap::iterator iterTape = tapes_.find(keySrc);
		if(iterTape == tapes_.end())
		{

			return false;
		}

		if(!SetBarcode(dstSlotId,barcodeSrc, keySrc))
		{

			return false;
		}

		if(!SetBarcode(srcSlotId,barcodeDst, keyDst))
		{

			SetBarcode(dstSlotId,barcodeDst, keyDst);
			return false;
		}

		return SetSlotID(dstSlotId, keySrc);

	}

	SlotType
	SimChanger::GetSlotType(int slotId)
	{
		DriveMap::iterator driveItr = drives_.find(slotId);
		if(driveItr!=drives_.end())
			return TP_DRIVE;
		MailSlotMap::iterator mailSlotItr = mailslots_.find(slotId);
		if(mailSlotItr!=mailslots_.end())
			return TP_MAILSLOT;
		return TP_SLOT;
	}

	bool
	SimChanger::GetBarcode(int slotId, string& barcode, string& key)
	{
		barcode = "";

		SlotMap::iterator slotItr = slots_.find(slotId);
		if(slotItr!=slots_.end())
		{
			barcode = slotItr->second.GetBarcode();
			key	= slotItr->second.GetTapeKey();
			return true;
		}

		DriveMap::iterator driveItr = drives_.find(slotId);
		if(driveItr!=drives_.end())
		{
			barcode = driveItr->second.GetBarcode();
			key	= driveItr->second.GetTapeKey();
			return true;
		}

		MailSlotMap::iterator mailSlotItr = mailslots_.find(slotId);
		if(mailSlotItr!=mailslots_.end())
		{
			barcode = mailSlotItr->second.GetBarcode();
			key	= mailSlotItr->second.GetTapeKey();
			return true;
		}

		return false;
	}

	bool
	SimChanger::SetBarcode(int slotId, const string& barcode, const string& key)
	{
		SlotMap::iterator slotItr = slots_.find(slotId);
		if(slotItr!=slots_.end())
		{
			return slotItr->second.InsertTape(barcode, key);
		}

		DriveMap::iterator driveItr = drives_.find(slotId);
		if(driveItr!=drives_.end())
		{
			return driveItr->second.InsertTape(barcode, key);
		}

		MailSlotMap::iterator mailSlotItr = mailslots_.find(slotId);
		if(mailSlotItr!=mailslots_.end())
		{
			return mailSlotItr->second.InsertTape(barcode, key);
		}

		return false;
	}

	bool
	SimChanger::SetSlotID(int slotId, string& key)
	{
		TapeMap::iterator tapeItr= tapes_.find(key);
		int logicSlotId = slotId;
		PhysicalSlotToLogicalSlot(slotId, logicSlotId);
		if(tapeItr!=tapes_.end())
		{
			return tapeItr->second.SetSlotID(slotId, logicSlotId);
		}
		//LgError("tony", "not find tape "<<barcode.c_str());
		return false;
	}

	bool
	SimChanger::GetDriveDetail(string& driveScsiAddr, struct DriveDetail& detail)
	{
		for(DriveMap::iterator iter=drives_.begin();
				iter!=drives_.end(); ++iter)
		{
			if(iter->second.GetScsiAddr() == driveScsiAddr)
			{
				iter->second.GetDetail(detail);
				return true;
			}
		}

		return false;
	}

	bool
	SimChanger::GetTapeDetail(string& barcode, struct TapeDetail& detail)
	{
		for(TapeMap::iterator iter=tapes_.begin();
				iter!=tapes_.end(); ++iter)
		{
			if(!iter->second.GetBarcode().empty() &&
					barcode == iter->second.GetBarcode())
			{
				iter->second.GetDetail(detail);
				return true;
			}
		}

		return false;
	}

	bool
	SimChanger::Format(string& label, int slotId)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		if(iter->second.Format(label))
		{
			for(TapeMap::iterator itr=tapes_.begin();
					itr!=tapes_.end(); ++itr)
			{
				if(itr->second.GetKey() == iter->second.GetTapeKey())
				{
					itr->second.SetFormated();
					return true;
				}
			}
		}
	}

	bool
	SimChanger::IsMounted(int slotId, bool& mounted)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		return iter->second.IsMounted(mounted);
	}

	bool
	SimChanger::Mount(int slotId)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		return iter->second.Mount();
	}

	bool
	SimChanger::Umount(int slotId)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		return iter->second.Umount();
	}

	bool
	SimChanger::GetTapeGroupUUID(int slotId, string& uuid)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		uuid = iterTape->second.GetGroupID();

		return true;
	}

	bool
	SimChanger::SetTapeDualCopy(int slotId, const string& dualCopy)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		iterTape->second.SetDualCopy(dualCopy);

		return true;
	}
	bool
	SimChanger::GetTapeDualCopy(int slotId, string& dualCopy)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		dualCopy = iterTape->second.GetDualCopy();

		return true;
	}

	bool
	SimChanger::SetTapeGroupUUID(int slotId, const string& uuid)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		iterTape->second.SetGroupID(uuid);

		return true;
	}

	bool
	SimChanger::GetTapeFaulty(int slotId, bool& faulty)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		faulty = iterTape->second.GetFaulty();

		return true;
	}

	bool
	SimChanger::SetTapeFaulty(int slotId, bool faulty)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		iterTape->second.SetFaulty(faulty);

		return true;
	}

	bool
	SimChanger::GetTapeStatus(int slotId, int& status)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		status = iterTape->second.GetStatus();

		return true;
	}

	bool
	SimChanger::SetTapeStatus(int slotId, int status)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		iterTape->second.SetStatus(status);

		return true;
	}

	bool
	SimChanger::GetTapeMediaType(int slotId, int& mediaType)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		mediaType = iterTape->second.GetMediaType();

		return true;
	}

	bool
	SimChanger::GetTapeFormatType(int slotId, int& type)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		type = iterTape->second.GetFormatType();

		return true;
	}

	bool
	SimChanger::GetTapeGenerationIndex(int slotId, long long& index)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		index = iterTape->second.GetGenerationIndex();

		return true;
	}

	bool
	SimChanger::SetTapeGenerationIndex(int slotId, long long index)
	{
		DriveMap::iterator iter = drives_.find(slotId);
		if(iter == drives_.end())
			return false;

		TapeMap::iterator iterTape = tapes_.find(iter->second.GetTapeKey());
		if(iterTape == tapes_.end())
			return false;
		iterTape->second.SetGenerationIndex(index);

		return true;
	}


	bool
	SimChanger::GetTapeCapacity(string& key, Int64_t& totalCapacity, Int64_t& freeCapacity)
	{
		struct TapeDetail detail;
		TapeMap::iterator iter=tapes_.find(key);
		if(iter == tapes_.end())
			return false;

		iter->second.GetDetail(detail);
		totalCapacity = detail.m_TotalCapacity;
		freeCapacity  = detail.m_FreeCapacity;
		return true;

	}

	bool
	SimChanger::Read(string& barcode, size_t size)
	{
		for(TapeMap::iterator iter=tapes_.begin();
				iter!=tapes_.end(); ++iter)
		{
			if(!iter->second.GetBarcode().empty() &&
					barcode == iter->second.GetBarcode())
			{
				iter->second.Read(size);
				return true;
			}
		}

		return false;
	}

	bool
	SimChanger::Write(string& barcode, size_t size)
	{
		for(TapeMap::iterator iter=tapes_.begin();
				iter!=tapes_.end(); ++iter)
		{
			if(!iter->second.GetBarcode().empty() &&
					barcode == iter->second.GetBarcode())
			{
				iter->second.Write(size);
				return true;
			}
		}

		return false;
	}

	void
	SimChanger::SetReadErrNum(string& barcode, int n)
	{
		for(TapeMap::iterator iter=tapes_.begin();
				iter!=tapes_.end(); ++iter)
		{
			if(!iter->second.GetBarcode().empty() &&
					barcode == iter->second.GetBarcode())
			{
				iter->second.SetReadErrNum(n);
				return ;
			}
		}
	}

	void
	SimChanger::SetWriteErrNum(string& barcode, int n)
	{
		for(TapeMap::iterator iter=tapes_.begin();
				iter!=tapes_.end(); ++iter)
		{
			if(!iter->second.GetBarcode().empty() &&
					barcode == iter->second.GetBarcode())
			{
				iter->second.SetWriteErrNum(n);
				return ;
			}
		}

	}

} /* namespace ltfs_management */
