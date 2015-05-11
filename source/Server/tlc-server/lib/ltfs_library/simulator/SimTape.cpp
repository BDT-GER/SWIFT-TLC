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
 * SimTape.cpp
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "../CmnFunc.h"
#include "SimTape.h"

namespace ltfs_management
{

	SimTape::SimTape(struct TapeDetail& detail)
	{
		// TODO Auto-generated constructor stub
		detail_.m_TapeKey		= detail.m_TapeKey;
		detail_.m_Barcode 		= detail.m_Barcode;
		detail_.m_FreeCapacity 	= detail.m_FreeCapacity;
		detail_.m_LtfsFormat 	= detail.m_LtfsFormat;
		detail_.m_MediaType 	= detail.m_MediaType;
		detail_.m_MediumType 	= detail.m_MediumType;
		detail_.m_SlotID 		= detail.m_SlotID;
		detail_.m_Status 		= detail.m_Status;
		detail_.m_FreeCapacity	= 0;
		if(detail_.m_MediaType == 1)
			GetCapacity(detail_.m_TotalCapacity, detail_.m_FreeCapacity);
		detail_.m_TotalCapacity = detail.m_TotalCapacity;

		detail_.m_GroupID		= detail.m_GroupID;
		detail_.m_DualCopy		= detail.m_DualCopy;
		detail_.m_Faulty		= detail.m_Faulty;
		detail_.m_GenerationIndex= detail.m_GenerationIndex;

		detail_.m_ErrnumRead 	= 0;
		detail_.m_ErrnumWrite 	= 0;
		detail_.m_SizeRead		= 0;
		detail_.m_SizeWrite		= 0;

		readMutex_ 				= new boost::mutex();
		writeMutex_ 			= new boost::mutex();
	}

	SimTape::~SimTape()
	{
		// TODO Auto-generated destructor stub
	}

	void
	SimTape::GetDetail(struct TapeDetail& detail)
	{
		detail.m_TapeKey		= detail_.m_TapeKey;
		detail.m_Barcode 		= detail_.m_Barcode;
		detail.m_FreeCapacity 	= detail_.m_FreeCapacity;
		detail.m_LtfsFormat 	= detail_.m_LtfsFormat;
		detail.m_MediaType 		= detail_.m_MediaType;
		detail.m_MediumType 	= detail_.m_MediumType;
		detail.m_SlotID 		= detail_.m_SlotID;
		detail.m_Status 		= detail_.m_Status;
		if(detail_.m_MediaType == 2 || detail_.m_MediaType == 1)
			GetCapacity(detail.m_TotalCapacity, detail_.m_FreeCapacity);
		detail.m_TotalCapacity 	= detail_.m_TotalCapacity;
		detail.m_FreeCapacity  	= detail_.m_FreeCapacity;

		detail.m_GroupID		= detail_.m_GroupID;
		detail.m_DualCopy		= detail_.m_DualCopy;
		detail.m_Faulty			= detail_.m_Faulty;
		detail.m_GenerationIndex= detail_.m_GenerationIndex;

		detail.m_ErrnumRead 	= detail_.m_ErrnumRead;
		detail.m_ErrnumWrite 	= detail_.m_ErrnumWrite;
		detail.m_SizeRead		= detail_.m_SizeRead;
		detail.m_SizeWrite		= detail_.m_SizeWrite;
	}

	string
	SimTape::GetBarcode() const
	{
		return detail_.m_Barcode;
	}

	string
	SimTape::GetKey() const
	{
		return detail_.m_TapeKey;
	}

	bool
	SimTape::SetSlotID(int slotid, int logicSlotId)
	{
		detail_.m_SlotID = slotid;
		detail_.m_LogicSlotId = logicSlotId;
		return true;
	}

	void
	SimTape::SetFormated()
	{
		detail_.m_LtfsFormat = 1;
		detail_.m_FreeCapacity = detail_.m_TotalCapacity;
	}

	string
	SimTape::GetGroupID() const
	{
		return detail_.m_GroupID;
	}

	void
	SimTape::SetGroupID(const string& groupid)
	{
		detail_.m_GroupID = groupid;
	}

	string
	SimTape::GetDualCopy() const
	{
		return detail_.m_DualCopy;
	}

	void
	SimTape::SetDualCopy(const string& dualCopy)
	{
		detail_.m_DualCopy = dualCopy;
	}

	bool
	SimTape::GetFaulty() const
	{
		return detail_.m_Faulty;
	}

	void
	SimTape::SetFaulty(bool fault)
	{
		detail_.m_Faulty = fault;
	}

	int
	SimTape::GetStatus() const
	{
		return detail_.m_Status;
	}

	void
	SimTape::SetStatus(int status)
	{
		detail_.m_Status = status;
	}

	int
	SimTape::GetFormatType() const
	{
		return detail_.m_LtfsFormat;
	}

	int
	SimTape::GetMediaType() const
	{
		return detail_.m_MediaType;
	}

	long long
	SimTape::GetGenerationIndex() const
	{
		return detail_.m_GenerationIndex;
	}

	void
	SimTape::SetGenerationIndex(long long index)
	{
		detail_.m_GenerationIndex = index;
	}

	bool
	SimTape::GetCapacity(Int64_t& totalCapacity, Int64_t& freeCapacity)
	{
		//SimDebug("Simulator:GetCapacity start "<<detail_.m_Barcode);

		totalCapacity = detail_.m_TotalCapacity;
		freeCapacity = 0;

		fs::path pathLTFS = GetTapeFolder() / detail_.m_Barcode;

		string cmd  = "du -s " + pathLTFS.string()+"/";

		vector<string> rets = GetCommandOutputLines(cmd);

		for(vector<string>::iterator it = rets.begin();
				it != rets.end(); it++)
		{
			//SimDebug("Simulator:cmd  "<<cmd<<"  ret  "<<*it );
			cmatch match;
			// 43534543  /opt/VS/ltfsTapes/LTO009908
			regex matchFuse("^\\s*(\\d+)\\s+.*");
			if(regex_match((*it).c_str(), match, matchFuse))
			{
				Int64_t usedSize = (boost::lexical_cast<Int64_t>(match[1])) * 1024;
				//usedSize *= 5; //TODO: use this to simulate large used files
				freeCapacity = totalCapacity - usedSize;
				if(freeCapacity < 0)
				{
					freeCapacity = 0;
				}

				//SimDebug("Simulator:GetCapacity return true used "<<usedSize);
				return true;
			}
		}
		//SimDebug("Simulator:GetCapacity return false");
		return false;
	}

	void
	SimTape::Read(size_t & size)
	{
		boost::unique_lock<boost::mutex> lock(*readMutex_);
		detail_.m_SizeRead += size;
	}

	void
	SimTape::Write(size_t & size)
	{
		boost::unique_lock<boost::mutex> lock(*writeMutex_);
		detail_.m_SizeWrite += size;
	}

	void
	SimTape::SetReadErrNum(int n)
	{
		boost::unique_lock<boost::mutex> lock(*readMutex_);
		detail_.m_ErrnumRead += n;
	}

	void
	SimTape::SetWriteErrNum(int n)
	{
		boost::unique_lock<boost::mutex> lock(*writeMutex_);
		detail_.m_ErrnumWrite += n;
	}
} /* namespace ltfs_management */
