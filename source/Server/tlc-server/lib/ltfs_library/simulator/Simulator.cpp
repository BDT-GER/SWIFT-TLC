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
 * Simulator.cpp
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "SimChanger.h"
#include "CfgfileParser.h"
#include "CfgfileSerialization.h"
#include "Simulator.h"

namespace ltfs_management
{
	Simulator * Simulator::instance_ = NULL;
	boost::mutex Simulator::instanceMutex_;

	Simulator::Simulator()
	{
		// TODO Auto-generated constructor stub
		Refresh();
	}

	Simulator::~Simulator()
	{
		// TODO Auto-generated destructor stub
	}

	void
	Simulator::Refresh()
	{
		vector<SimChanger> changers;
		CfgfileParser parser(CFG_FILE.c_str());
		if(parser.ParserChangers(changers))
		{
			changers_.clear();
			for(vector<SimChanger>::iterator iter=changers.begin();
					iter!=changers.end(); ++iter)
			{
				changers_.insert(ChangerMap::value_type(iter->GetSerial(),*iter));
			}
		}
		SimDebug("Simulator::Refresh");
	}

	bool
	Simulator::GetCfgDetail(struct CfgDetail& detail)
	{
		boost::unique_lock<boost::mutex> lock(instanceMutex_);

		CfgfileParser parser(CFG_FILE.c_str());
		return parser.ParserElement(detail);
	}

	bool
	Simulator::GetChangerList(vector<struct ChangerDetail>& changers, int& errId)
	{
		errId = 0;

		changers.clear();

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct ChangerDetail detail;
			iter->second.GetDetail(detail);
			changers.push_back(detail);
		}

		if(changers.size()==0)
		{
			SimDebug("Simulator::GetChangerList return false");
			return false;
		}

		SimDebug("Simulator::GetChangerList return true");
		return true;
	}

	bool
	Simulator::GetDriveList(string& changerSerial, vector<struct DriveDetail> & drives, int& errId)
	{
		errId = 0;

		ChangerMap::iterator iter=changers_.find(changerSerial);
		if(iter == changers_.end())
		{
			SimError("not find changer, serial : DE64000F22");
			return false;
		}

		return iter->second.GetDriveList(drives);
	}

	bool
	Simulator::GetSlotList(string& changerSerial, vector<struct SlotDetail> & slots, int& errId)
	{
		errId = 0;

		ChangerMap::iterator iter=changers_.find(changerSerial);
		if(iter == changers_.end())
			return false;

		return iter->second.GetSlotList(slots);
	}

	bool
	Simulator::GetMailSlotList(string& changerSerial, vector<struct MailSlotDetail> & mailSlots, int& errId)
	{
		errId = 0;

		ChangerMap::iterator iter=changers_.find(changerSerial);
		if(iter == changers_.end())
			return false;

		return iter->second.GetMailSlotList(mailSlots);
	}

	bool
	Simulator::GetTapeList(string& changerSerial, vector<struct TapeDetail> & tapes, int& errId)
	{
		errId = 0;

		ChangerMap::iterator iter=changers_.find(changerSerial);
		if(iter == changers_.end())
			return false;

		return iter->second.GetTapeList(tapes);
	}

	bool
	Simulator::MoveTape(string& changerSerial, int srcSlotId, int dstSlotId, int& errId)
	{
		SimDebug("Simulator::MoveTape start, srcSlotId " << srcSlotId << " dstSlotId "<<dstSlotId);

		errId = 0;

		ChangerMap::iterator iter=changers_.find(changerSerial);
		if(iter == changers_.end())
			return false;

		//delay
		struct CfgDetail detail;
		int delay = 0;

		try
		{
			GetCfgDetail(detail);
			delay = detail.m_MoveDelay;
			if(detail.m_MoveFailure)
			{
				errId = detail.m_MoveFailure;
				SimDebug("Simulator::simulate Move Failure");
				return false;
			}
		}
		catch(...)
		{

		}

		sleep(delay);

		if(iter->second.MoveTape(srcSlotId, dstSlotId))
		{
			SerializationCfg();
			SimDebug("Simulator::MoveTape return true");
			return true;
		}

		SimDebug("Simulator::MoveTape return false");
		return false;
	}

	bool
	Simulator::Format(string& label, string& driveSerial, int& errId)
	{
		errId = 0;

		SimDebug("Simulator::Start to Format");
		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{

				struct CfgDetail detail;
				int delay = 0;

				try
				{
					GetCfgDetail(detail);
					delay = detail.m_FormatDelay;
					if(detail.m_FormatError)
					{
						errId = detail.m_FormatError;
						SimDebug("Simulator::simulate Format Failure");
						return false;
					}
				}
				catch(...)
				{

				}

				sleep(delay);

				if(iter->second.Format(label, driveDetail.m_SlotId))
				{
					SerializationCfg();
					SimDebug("Simulator::Format return true");
					return true;
				}
			}
		}
		SimDebug("Simulator::Format return false");
		return false;
	}

	bool
	Simulator::IsMounted(string& driveSerial, bool& mounted, int& errId)
	{
		errId = 0;
		SimDebug("Simulator::Start to IsMounted driveSerial "<<driveSerial);
		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				SimDebug("Simulator::IsMounted got drive");

				if(iter->second.IsMounted(driveDetail.m_SlotId, mounted))
				{
					SimDebug("Simulator::IsMounted return true");
					return true;
				}
			}
		}
		errId = 1;
		SimDebug("Simulator::IsMounted return false");

		return false;
	}

	bool
	Simulator::Mount(string& driveSerial, int& errId)
	{
		errId = 0;
		SimDebug("Simulator::Start to Mount driveSerial "<<driveSerial);
		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				bool mounted = false;
				struct CfgDetail detail;
				int delay = 0;
				SimDebug("Simulator::Mount got drive");

				if(iter->second.IsMounted(driveDetail.m_SlotId, mounted) && mounted)
				{
					SimDebug("Simulator::Mount return true");
					return true;
				}

				try
				{
					GetCfgDetail(detail);
					delay = detail.m_MountDelay;
					if(detail.m_MountError)
					{
						errId = detail.m_MountError;
						SimDebug("Simulator::simulate Mount Failure");
						return false;
					}
				}
				catch(...)
				{

				}

				sleep(delay);

				if(iter->second.Mount(driveDetail.m_SlotId))
				{
					SimDebug("Simulator::Mount return true");
					return true;
				}
			}
		}
		errId = 1;
		SimDebug("Simulator::Mount return false");
		return false;
	}

	bool
	Simulator::Umount(string& driveSerial, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: Umount driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				struct CfgDetail detail;
				int delay = 0;
				SimDebug("Simulator::Umount got drive");
				try
				{
					GetCfgDetail(detail);
					delay = detail.m_UmontDelay;
					if(detail.m_UmontError)
					{
						errId = detail.m_MountError;
						SimDebug("Simulator::simulate Umount Failure");
						return false;
					}
				}
				catch(...)
				{

				}

				sleep(delay);

				if(iter->second.Umount(driveDetail.m_SlotId))
				{
					SimDebug("Simulator::Umount return true");
					return true;
				}
			}
		}
		SimDebug("Simulator::Umount return false");
		return false;
	}

	bool
	Simulator::GetTapeGroupUUID(string& driveSerial, string& uuid, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: GetTapeGroupUUID driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{

				return iter->second.GetTapeGroupUUID(driveDetail.m_SlotId, uuid);
			}

		}
		return false;
	}

	bool
	Simulator::GetTapeDualCopy(string& driveSerial, string& dualCopy, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: GetTapeDualCopy driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{

				return iter->second.GetTapeDualCopy(driveDetail.m_SlotId, dualCopy);
			}

		}
		return false;
	}

	bool
	Simulator::SetTapeGroupUUID(string& driveSerial, const string& uuid, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: SetTapeGroupUUID driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				if( iter->second.SetTapeGroupUUID(driveDetail.m_SlotId, uuid))
				{
					SerializationCfg();
					return true;
				}
			}

		}
		return false;
	}

	bool
	Simulator::SetTapeDualCopy(string& driveSerial, const string& dualCopy, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: SetTapeDualCopy driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				if( iter->second.SetTapeDualCopy(driveDetail.m_SlotId, dualCopy))
				{
					SerializationCfg();
					return true;
				}
			}

		}
		return false;
	}

	bool
	Simulator::GetTapeFaulty(string& driveSerial, bool& faulty, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: GetTapeFaulty driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				return iter->second.GetTapeFaulty(driveDetail.m_SlotId, faulty);
			}

		}
		return false;
	}

	bool
	Simulator::SetTapeFaulty(string& driveSerial, bool faulty, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: SetTapeFaulty driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				if( iter->second.SetTapeFaulty(driveDetail.m_SlotId, faulty))
				{
					SerializationCfg();
					return true;
				}
			}

		}
		return false;
	}

	bool
	Simulator::GetTapeStatus(string& driveSerial, int& status, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: GetTapeStatus driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				return iter->second.GetTapeStatus(driveDetail.m_SlotId, status);
			}

		}
		return false;
	}

	bool
	Simulator::SetTapeStatus(string& driveSerial, int status, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: SetTapeStatus driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				if( iter->second.SetTapeStatus(driveDetail.m_SlotId, status))
				{
					SerializationCfg();
					return true;
				}
			}

		}
		return false;
	}

	bool
	Simulator::GetTapeMediaType(string& driveSerial, int& mediaType, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: GetTapeMediaType driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				return iter->second.GetTapeMediaType(driveDetail.m_SlotId, mediaType);
			}

		}
		return false;
	}

	bool
	Simulator::GetTapeFormatType(string& driveSerial, int& type, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: getTapeFormatFlag driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				return iter->second.GetTapeFormatType(driveDetail.m_SlotId, type);
			}

		}
		return false;
	}

	bool
	Simulator::GetTapeGenerationIndex(string& driveSerial, long long& index, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: getTapeFormatFlag driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				return iter->second.GetTapeGenerationIndex(driveDetail.m_SlotId, index);
			}

		}
		return false;
	}

	bool
	Simulator::SetTapeGenerationIndex(string& driveSerial, long long index, int& errId)
	{
		errId = 0;
		SimDebug("Simulator:: getTapeFormatFlag driveSerial "<<driveSerial);

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				if( iter->second.SetTapeGenerationIndex(driveDetail.m_SlotId, index))
				{
					SerializationCfg();
					return true;
				}
			}

		}
		return false;
	}

	bool
	Simulator::GetChangerInfo(string& changerSerial, struct ChangerDetail& changerDetail)
	{
		ChangerMap::iterator iter=changers_.find(changerSerial);
		if(iter == changers_.end())
			return false;

		iter->second.GetDetail(changerDetail);

		return true;
	}

	bool
	Simulator::GetDriveInfo(string& driveSerial, struct DriveDetail& driveDetail)
	{
		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
				return true;
		}

		return false;
	}

	bool
	Simulator::GetTapeInfo(string& driveSerial, struct TapeDetail& tapeDetail, int& errId)
	{
		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail))
			{
				if(driveDetail.m_TapeKey.empty())
				{
					errId = 231;
					return false;
				}

				if(!driveDetail.m_Barcode.empty())
				{
					if(iter->second.GetTapeDetail(driveDetail.m_Barcode, tapeDetail))
					{
						errId = 0;
						return true;
					}
				}
			}
		}

		errId = 1;

		return false;
	}

	bool
	Simulator::GetLoadedTapeCapacity(string& driveSerial, Int64_t &totalCapacity, Int64_t& freeCapacity)
	{
		int errId = 0;

		for(ChangerMap::iterator iter=changers_.begin();
				iter!=changers_.end(); ++iter)
		{
			struct DriveDetail driveDetail;
			if(iter->second.GetDriveDetail(driveSerial, driveDetail)
					&& !driveDetail.m_Barcode.empty())
			{
				if(iter->second.GetTapeCapacity(driveDetail.m_TapeKey, totalCapacity, freeCapacity))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool
	Simulator::Read(string& barcode, size_t size)
	{
		for(ChangerMap::iterator iter=changers_.begin();
						iter!=changers_.end(); ++iter)
		{
			struct TapeDetail tapeDetail;
			if(iter->second.GetTapeDetail(barcode, tapeDetail))
			{
				struct CfgDetail detail;
				try
				{
					GetCfgDetail(detail);
					if(detail.m_ReadDelay)
						sleep(detail.m_ReadDelay);

					if(detail.m_SizeReadErr>0 && size >= detail.m_SizeReadErr)
					{
						if((detail.m_NumReadErr==0
								|| detail.m_NumReadErr>0) && detail.m_NumReadErr>tapeDetail.m_ErrnumRead)
						{
							iter->second.SetReadErrNum(barcode, 1);
							return false;
						}
					}
				}
				catch(...)
				{

				}

				return true;
			}
		}

		return false;
	}

	bool
	Simulator::Write(string& barcode, size_t size)
	{
		for(ChangerMap::iterator iter=changers_.begin();
						iter!=changers_.end(); ++iter)
		{
			struct TapeDetail tapeDetail;
			if(iter->second.GetTapeDetail(barcode, tapeDetail))
			{
				struct CfgDetail detail;
				try
				{
					GetCfgDetail(detail);
					if(detail.m_WriteDelay)
						sleep(detail.m_WriteDelay);

					SimDebug("Simulator:Write cfgDetail "<< detail.m_SizeWriteErr <<" "<<detail.m_NumWriteErr);
					SimDebug("Simulator:Write filesize "<<size <<" tapeDetail ErrnumWrite "<<tapeDetail.m_ErrnumWrite);

					if(detail.m_SizeWriteErr>0 && size >= detail.m_SizeWriteErr)
					{
						if(detail.m_NumWriteErr==0
								|| detail.m_NumWriteErr>0 && detail.m_NumWriteErr>tapeDetail.m_ErrnumWrite)
						{
							iter->second.SetWriteErrNum(barcode, 1);
							SimDebug("Simulator:Write return false");
							return false;
						}
					}
				}
				catch(...)
				{

				}

				return true;
			}
		}

		SimDebug("Simulator:Write not find ");
		return false;
	}

	bool
	Simulator::SerializationCfg()
	{
		boost::unique_lock<boost::mutex> lock(instanceMutex_);

		struct CfgDetail detail;
		CfgfileParser parser(CFG_FILE.c_str());
		CfgfileSerialization serialization(CFG_FILE.c_str());
		parser.ParserElement(detail);

		serialization.SetCfgDetail(detail);
		serialization.SetChangers(changers_);
		serialization.SaveToFile();

		return true;
	}


} /* namespace ltfs_management */
