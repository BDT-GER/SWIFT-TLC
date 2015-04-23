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
 * CfgfileParser.cpp
 *
 *  Created on: Oct 10, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "SimChanger.h"
#include "CfgfileParser.h"

namespace ltfs_management
{

	CfgfileParser::CfgfileParser(const char* filename)
	{
		// TODO Auto-generated constructor stub

		try
		{
			read_xml(filename, root_);
			//LgDebug("tony", "Config file load ok!");
		}
		catch(std::exception& e)
		{
			//LgError("tony", "Exception occured!");
		}

	}

	CfgfileParser::~CfgfileParser()
	{
		// TODO Auto-generated destructor stub
	}

	bool
	CfgfileParser::ParserElement(struct CfgDetail& detail)
	{
		try
		{
			string str;
			boost::property_tree::ptree conf = root_.get_child("simulator_conf");

			boost::property_tree::ptree tree = conf.get_child("size_for_trigger_read_error");
			str = tree.data();
			if(!ParserSize(str, detail.m_SizeReadErr))
				throw std::exception();

			tree = conf.get_child("size_for_trigger_write_error");
			str = tree.data();
			if(!ParserSize(str, detail.m_SizeWriteErr))
				throw std::exception();

			tree = conf.get_child("number_of_read_errors");
			detail.m_NumReadErr =  boost::lexical_cast<int>(tree.data());

			tree = conf.get_child("number_of_write_errors");
			detail.m_NumWriteErr = boost::lexical_cast<int>(tree.data());

			tree = conf.get_child("read_delay");
			detail.m_ReadDelay  = boost::lexical_cast<int>(tree.data());

			tree = conf.get_child("write_delay");
			detail.m_WriteDelay = boost::lexical_cast<int>(tree.data());

			tree = conf.get_child("move_tape_delay");
			detail.m_MoveDelay = boost::lexical_cast<int>(tree.data());

			tree = conf.get_child("enable_move_tape_failure");
			detail.m_MoveFailure  = boost::lexical_cast<int>(tree.data());



			tree = conf.get_child("format_delay");
			detail.m_FormatDelay = boost::lexical_cast<int>(tree.data());

			tree = conf.get_child("enable_format_failure");
			detail.m_FormatError  = boost::lexical_cast<int>(tree.data());

			tree = conf.get_child("mount_delay");
			detail.m_MountDelay = boost::lexical_cast<int>(tree.data());

			tree = conf.get_child("enable_mount_failure");
			detail.m_MountError  = boost::lexical_cast<int>(tree.data());

			tree = conf.get_child("umount_delay");
			detail.m_UmontDelay = boost::lexical_cast<int>(tree.data());

			tree = conf.get_child("enable_umount_failure");
			detail.m_UmontError  = boost::lexical_cast<int>(tree.data());

			return true;
		}
		catch(...)
		{
			SimError("Exception occured!");
		}

		return false;
	}

	bool
	CfgfileParser::ParserChangers(vector<SimChanger>& changers)
	{
		try
		{
			boost::property_tree::ptree conf = root_.get_child("simulator_conf");
			boost::property_tree::ptree changersTree = conf.get_child("changers");

			for(boost::property_tree::ptree::iterator iter=changersTree.begin();
					iter!=changersTree.end(); ++iter)
			{
				struct ChangerDetail detail;
				boost::property_tree::ptree changerTree = iter->second;
				boost::property_tree::ptree tree;

				tree = changerTree.get_child("status");
				detail.m_Status = boost::lexical_cast<int>(tree.data());

				tree = changerTree.get_child("serial");
				detail.m_Serial = tree.data();

				tree = changerTree.get_child("vendor");
				detail.m_Vendor = tree.data();

				tree = changerTree.get_child("product");
				detail.m_Product = tree.data();

				tree = changerTree.get_child("version");
				detail.m_Version = tree.data();

				tree = changerTree.get_child("scsi_addr");
				detail.m_ScsiAddr = tree.data();

				tree = changerTree.get_child("stdev");
				detail.m_StDev = tree.data();

				tree = changerTree.get_child("sgdev");
				detail.m_SgDev = tree.data();


				SimChanger changer(detail);


				ParserDrives(changerTree, changer);
				ParserTapes(changerTree, changer);
				ParserSlots(changerTree, changer);
				ParserMailSlots(changerTree, changer);

				changers.push_back(changer);

			}
			return true;
		}
		catch(...)
		{
			changers.clear();

			SimError("Exception occured!");
		}

		return false;
	}

	bool
	CfgfileParser::ParserDrives(boost::property_tree::ptree & changerTree, SimChanger& changer)
	{
		boost::property_tree::ptree drivesTree = changerTree.get_child("drives");
		vector<SimDrive> drives;

		for(boost::property_tree::ptree::iterator iter=drivesTree.begin();
				iter!=drivesTree.end(); ++iter)
		{
			struct DriveDetail detail;
			boost::property_tree::ptree driveTree = iter->second;

			boost::property_tree::ptree tree = driveTree.get_child("status");
			detail.m_Status = boost::lexical_cast<int>(tree.data());

			tree = driveTree.get_child("cleaning_status");
			detail.m_CleaningStatus = boost::lexical_cast<int>(tree.data());

			tree = driveTree.get_child("interface_type");
			detail.m_InterfaceType = boost::lexical_cast<int>(tree.data());

			tree = driveTree.get_child("slot_id");
			detail.m_SlotId = boost::lexical_cast<int>(tree.data());

			tree = driveTree.get_child("tape_key");
			detail.m_TapeKey = tree.data();

			tree = driveTree.get_child("serial");
			detail.m_Serial = tree.data();

			tree = driveTree.get_child("vendor");
			detail.m_Vendor = tree.data();

			tree = driveTree.get_child("product");
			detail.m_Product = tree.data();

			tree = driveTree.get_child("version");
			detail.m_Version = tree.data();

			tree = driveTree.get_child("barcode");
			detail.m_Barcode = tree.data();

			tree = driveTree.get_child("scsi_addr");
			detail.m_ScsiAddr = tree.data();

			tree = driveTree.get_child("stdev");
			detail.m_StDev = tree.data();

			tree = driveTree.get_child("sgdev");
			detail.m_SgDev = tree.data();

			tree = driveTree.get_child("generation");
			detail.m_Generation = boost::lexical_cast<int>(tree.data());

			SimDrive drive(detail);
			drives.push_back(drive);
		}

		changer.RefreshDriveMap(drives);

		return true;
	}

	bool
	CfgfileParser::ParserTapes(boost::property_tree::ptree & changerTree, SimChanger& changer)
	{
		boost::property_tree::ptree tapesTree = changerTree.get_child("tapes");
		vector<SimTape> tapes;
		string strTapeDir("");
		string str;

		AutoMakeTapeDir(strTapeDir);

		for(boost::property_tree::ptree::iterator iter=tapesTree.begin();
				iter!=tapesTree.end(); ++iter)
		{
			struct TapeDetail detail;
			boost::property_tree::ptree tapeTree = iter->second;

			boost::property_tree::ptree tree = tapeTree.get_child("slot_id");
			detail.m_SlotID = boost::lexical_cast<int>(tree.data());

			tree = tapeTree.get_child("tape_key");
			detail.m_TapeKey = tree.data();

			tree = tapeTree.get_child("barcode");
			detail.m_Barcode = tree.data();

			tree = tapeTree.get_child("status");
			detail.m_Status = boost::lexical_cast<int>(tree.data());

			tree = tapeTree.get_child("medium_type");
			detail.m_MediumType = boost::lexical_cast<int>(tree.data());

			tree = tapeTree.get_child("media_type");
			detail.m_MediaType = boost::lexical_cast<int>(tree.data());

			tree = tapeTree.get_child("ltfs_format");
			detail.m_LtfsFormat = boost::lexical_cast<int>(tree.data());

			tree = tapeTree.get_child("total_capacity");
			detail.m_TotalCapacity = boost::lexical_cast<Int64_t>(tree.data());

			SimDebug("ParserTapes group ");
			tree = tapeTree.get_child("group");
			detail.m_GroupID = tree.data();

			SimDebug("ParserTapes Dual Copy ");
			tree = tapeTree.get_child("dual_copy");
			detail.m_DualCopy = tree.data();

			SimDebug("ParserTapes faulty ");
			tree = tapeTree.get_child("faulty");
			str = tree.data();
			detail.m_Faulty = str=="true"?true:false;

			SimDebug("ParserTapes generation_index ");
			tree = tapeTree.get_child("generation_index");
			detail.m_GenerationIndex = boost::lexical_cast<Int64_t>(tree.data());

			SimTape tape(detail);
			tapes.push_back(tape);

			AutoMakeTapeDir(detail.m_Barcode);
		}

		changer.RefreshTapeMap(tapes);

		return true;
	}

	bool
	CfgfileParser::ParserSlots(boost::property_tree::ptree & changerTree, SimChanger& changer)
	{
		boost::property_tree::ptree slotsTree = changerTree.get_child("slots");
		vector<SimSlot> slots;

		for(boost::property_tree::ptree::iterator iter=slotsTree.begin();
				iter!=slotsTree.end(); ++iter)
		{
			struct SlotDetail detail;
			boost::property_tree::ptree slotTree = iter->second;

			boost::property_tree::ptree tree = slotTree.get_child("slot_id");
			detail.m_SlotID = boost::lexical_cast<int>(tree.data());

			tree = slotTree.get_child("tape_key");
			detail.m_TapeKey = tree.data();

			tree = slotTree.get_child("barcode");
			detail.m_Barcode = tree.data();

			SimSlot slot(detail);
			slots.push_back(slot);
		}

		changer.RefreshSlotMap(slots);

		return true;
	}

	bool
	CfgfileParser::ParserMailSlots(boost::property_tree::ptree & changerTree, SimChanger& changer)
	{
		boost::property_tree::ptree mslotsTree = changerTree.get_child("mailslots");
		vector<SimMailSlot> mslots;

		for(boost::property_tree::ptree::iterator iter=mslotsTree.begin();
				iter!=mslotsTree.end(); ++iter)
		{
			struct MailSlotDetail detail;
			boost::property_tree::ptree mslotTree = iter->second;

			boost::property_tree::ptree tree = mslotTree.get_child("slot_id");
			detail.m_SlotID = boost::lexical_cast<int>(tree.data());

			tree = mslotTree.get_child("tape_key");
			detail.m_TapeKey = tree.data();

			tree = mslotTree.get_child("barcode");
			detail.m_Barcode = tree.data();

			SimMailSlot mslot(detail);
			mslots.push_back(mslot);
		}

		changer.RefreshMailSlotMap(mslots);

		return true;
	}

	bool
	CfgfileParser::ParserSize(string& str, size_t& size)
	{
			size = 0;

			if(str.length() > 1)
			{
				string strUnit = str.substr(str.length()-1);
				string strSize = str.substr(0, str.length()-1);

				if(strUnit == "M")
				{
						size = (size_t)boost::lexical_cast<double>(strSize)*0x100000;
				}
				else if(strUnit == "G")
				{
						size = (size_t)boost::lexical_cast<double>(strSize)*0x40000000;
				}
				else if(strUnit == "T")
				{
						size = (size_t)boost::lexical_cast<double>(strSize)*0x10000000000;
				}
				else
					size = (size_t)boost::lexical_cast<double>(str);
			}
			else if(str.length()==1)
			{
				size = (size_t)boost::lexical_cast<double>(str);
			}

			return true;

	}

	void
	CfgfileParser::AutoMakeTapeDir(string& barcode)
	{
		if(barcode.empty())
			return;
		try
		{
			fs::path path = GetTapeFolder() / barcode;

			if ( ! fs::exists(path) )
			{
				fs::create_directory(path);
			}
		}
		catch(...)
		{

		}
	}


} /* namespace ltfs_management */
