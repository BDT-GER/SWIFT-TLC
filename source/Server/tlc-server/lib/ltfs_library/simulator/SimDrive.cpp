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
 * SimDrive.cpp
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "SimSlot.h"
#include "SimDrive.h"

namespace ltfs_management
{

	SimDrive::SimDrive(struct DriveDetail& detail):
			SimSlot(TP_DRIVE)
	{
		// TODO Auto-generated constructor stub
		detail_.m_TapeKey		= detail.m_TapeKey;
		detail_.m_Status		= detail.m_Status;
		detail_.m_CleaningStatus= detail.m_CleaningStatus;
		detail_.m_InterfaceType	= detail.m_InterfaceType;
		detail_.m_SlotId		= detail.m_SlotId;
		detail_.m_Serial		= detail.m_Serial;
		detail_.m_Barcode		= detail.m_Barcode;
		detail_.m_ScsiAddr		= detail.m_ScsiAddr;
		detail_.m_Vendor		= detail.m_Vendor;
		detail_.m_Product		= detail.m_Product;
		detail_.m_Version		= detail.m_Version;
		detail_.m_StDev			= detail.m_StDev;
		detail_.m_SgDev			= detail.m_SgDev;
		detail_.m_Generation	= detail.m_Generation;
	}

	SimDrive::~SimDrive()
	{
		// TODO Auto-generated destructor stub
	}

	void
	SimDrive::GetDetail(struct DriveDetail& detail)
	{
		detail.m_TapeKey		= detail_.m_TapeKey;
		detail.m_Status			= detail_.m_Status;
		detail.m_CleaningStatus	= detail_.m_CleaningStatus;
		detail.m_InterfaceType	= detail_.m_InterfaceType;
		detail.m_SlotId			= detail_.m_SlotId;
		detail.m_Serial			= detail_.m_Serial;
		detail.m_Barcode		= detail_.m_Barcode;
		detail.m_ScsiAddr		= detail_.m_ScsiAddr;
		detail.m_Vendor			= detail_.m_Vendor;
		detail.m_Product		= detail_.m_Product;
		detail.m_Version		= detail_.m_Version;
		detail.m_StDev			= detail_.m_StDev;
		detail.m_SgDev			= detail_.m_SgDev;
		detail.m_Generation		= detail_.m_Generation;
	}

	string
	SimDrive::GetScsiAddr() const
	{
		return detail_.m_ScsiAddr;
	}

	int
	SimDrive::GetSlotID() const
	{
		return detail_.m_SlotId;
	}

	int
	SimDrive::GetLogicalSlotID() const
	{
		return detail_.m_LogicSlotId;
	}

	string
	SimDrive::GetTapeKey() const
	{
		return detail_.m_TapeKey;
	}

	string
	SimDrive::GetBarcode() const
	{
		return detail_.m_Barcode;
	}

	bool
	SimDrive::InsertTape(const string& barcode, const string& key)
	{
		if(!key.empty() && !detail_.m_TapeKey.empty() )
					return false;

		detail_.m_TapeKey = key;
		detail_.m_Barcode = barcode;

		return true;
	}

	bool
	SimDrive::Format(string& label)
	{
		string barcode = detail_.m_Barcode;
		if ( barcode.empty() )
		{
			return false;
		}

		fs::path pathTape = GetTapeFolder() / barcode;
		fs::path pathLTFS = GetLTFSFolder() / barcode;
		fs::path pathLabel = pathTape / label;
		if ( fs::exists(pathLTFS) )
		{
			return false;
		}
		if ( fs::exists(pathTape) )
		{
			if ( ! fs::remove_all(pathTape) )
			{
				return false;
			}
		}

		string mkdirCmd = "mkdir " + GetTapeFolder().string() + " 1>/dev/null 2>/dev/null";
		std::system(mkdirCmd.c_str());

		try{
			if(label.empty()){
				return fs::create_directory(pathTape);
			}

			fs::create_directory(pathTape);
			return fs::create_directory(pathLabel);
		}catch(...){
			SimError("Failed to create directory.");
			return false;
		}
	}

	bool
	SimDrive::IsMounted(bool& mounted)
	{
		string barcode = detail_.m_Barcode;
		if ( barcode.empty() )
		{
			SimDebug("Simulator::IsMounted barcode is empty");
			return false;
		}

		fs::path pathTape = GetTapeFolder() / barcode;
		fs::path pathLTFS = GetLTFSFolder() / barcode;
		if ( ! fs::exists(pathTape) )
		{
			SimDebug("Simulator::IsMounted tape not exist");
			return false;
		}
		try
		{
			SimDebug("Simulator::IsMounted check link");
			if ( fs::is_symlink(pathLTFS) )
			{
				mounted = true;
			}
			else
			{
				mounted = false;
			}
		}
		catch(std::exception& e)
		{
			SimDebug("Simulator::IsMounted exception " << e.what());
			return false;
		}

		SimDebug("Simulator::IsMounted return true mounted " << mounted);
		return true;
	}

	bool
	SimDrive::Mount()
	{
		string barcode = detail_.m_Barcode;
		if ( barcode.empty() )
		{
			SimDebug("Simulator::Mount barcode is empty");
			return false;
		}

		fs::path pathTape = GetTapeFolder() / barcode;
		fs::path pathLTFS = GetLTFSFolder() / barcode;
		if ( ! fs::exists(pathTape) )
		{
			SimDebug("Simulator::Mount tape not exist");
			return false;
		}
		try
		{
			SimDebug("Simulator::Mount check link");
			if ( ! fs::is_symlink(pathLTFS) )
			{
				SimDebug("Simulator::Mount try "<<pathTape<<" "<<pathLTFS);
				fs::create_symlink(pathTape,pathLTFS);
			}
		}
		catch(std::exception& e)
		{
			SimDebug("Simulator::Mount exception " << e.what());
			return false;
		}

		return true;
	}

	bool
	SimDrive::Umount()
	{
		string barcode = detail_.m_Barcode;
		if ( barcode.empty() )
		{
			return true;
		}

		fs::path pathLTFS = GetLTFSFolder() / barcode;
		if ( ! fs::exists(pathLTFS) )
		{
			return true;
		}
		SimDebug("Simulator::Umount remove "<<pathLTFS);
		return fs::remove(pathLTFS);
	}



} /* namespace ltfs_management */
