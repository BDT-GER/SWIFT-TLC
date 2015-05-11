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
 * CfgfileSerialization.cpp
 *
 *  Created on: Oct 16, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "SimChanger.h"
#include "CfgfileParser.h"
#include "CfgfileSerialization.h"

namespace ltfs_management
{

	CfgfileSerialization::CfgfileSerialization(const char* filename)
	{
		// TODO Auto-generated constructor stub
		const char* str =	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
		filename_ 	 = filename;
		document_ 	 = new TiXmlDocument();
		document_->Parse(str);
		rootElement_ = new TiXmlElement("simulator_conf");

		document_->LinkEndChild(rootElement_);
	}

	CfgfileSerialization::~CfgfileSerialization()
	{
		// TODO Auto-generated destructor stub

		delete document_;
	}

	bool
	CfgfileSerialization::SetCfgDetail(struct CfgDetail& detail)
	{
		try
		{
			TiXmlElement* element = new TiXmlElement("size_for_trigger_read_error");
			TiXmlText* content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_SizeReadErr).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("size_for_trigger_write_error");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_SizeWriteErr).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("number_of_read_errors");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_NumReadErr).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("number_of_write_errors");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_NumWriteErr).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("read_delay");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_ReadDelay).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("write_delay");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_WriteDelay).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("move_tape_delay");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_MoveDelay).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("enable_move_tape_failure");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_MoveFailure).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("format_delay");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_FormatDelay).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("enable_format_failure");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_FormatError).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("mount_delay");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_MountDelay).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("enable_mount_failure");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_MountError).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("umount_delay");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_UmontDelay).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);

			element = new TiXmlElement("enable_umount_failure");
			content = new TiXmlText(
					boost::lexical_cast<string>(detail.m_UmontError).c_str());
			element->LinkEndChild(content);
			rootElement_->LinkEndChild(element);


		}
		catch(...)
		{
			SimDebug("SetCfgDetail exception");
			return false;
		}

		return true;
	}

	bool
	CfgfileSerialization::SetChangers(map<string,SimChanger>& changers)
	{
		try
		{
			TiXmlElement* element = NULL;
			TiXmlText* content = NULL;
			TiXmlElement* changersElement = new TiXmlElement("changers");
			rootElement_->LinkEndChild(changersElement);

			for(map<string,SimChanger>::iterator iter=changers.begin();
					iter!=changers.end(); ++iter)
			{
				vector<struct DriveDetail> drives;
				vector<struct SlotDetail> slots;
				vector<struct MailSlotDetail> mailslots;
				vector<struct TapeDetail> tapes;

				TiXmlElement* changerElement = new TiXmlElement("changer");
				struct ChangerDetail Changeretail;

				iter->second.GetDetail(Changeretail);
				iter->second.GetDriveList(drives);
				iter->second.GetSlotList(slots);
				iter->second.GetMailSlotList(mailslots);
				iter->second.GetTapeList(tapes);

				element = new TiXmlElement("status");
				content = new TiXmlText(
						boost::lexical_cast<string>(Changeretail.m_Status).c_str());
				element->LinkEndChild(content);
				changerElement->LinkEndChild(element);

				element = new TiXmlElement("serial");
				content = new TiXmlText(Changeretail.m_Serial.c_str());
				element->LinkEndChild(content);
				changerElement->LinkEndChild(element);

				element = new TiXmlElement("vendor");
				content = new TiXmlText(Changeretail.m_Vendor.c_str());
				element->LinkEndChild(content);
				changerElement->LinkEndChild(element);

				element = new TiXmlElement("product");
				content = new TiXmlText(Changeretail.m_Product.c_str());
				element->LinkEndChild(content);
				changerElement->LinkEndChild(element);

				element = new TiXmlElement("version");
				content = new TiXmlText(Changeretail.m_Version.c_str());
				element->LinkEndChild(content);
				changerElement->LinkEndChild(element);

				element = new TiXmlElement("scsi_addr");
				content = new TiXmlText(Changeretail.m_ScsiAddr.c_str());
				element->LinkEndChild(content);
				changerElement->LinkEndChild(element);

				element = new TiXmlElement("stdev");
				content = new TiXmlText(Changeretail.m_StDev.c_str());
				element->LinkEndChild(content);
				changerElement->LinkEndChild(element);

				element = new TiXmlElement("sgdev");
				content = new TiXmlText(Changeretail.m_SgDev.c_str());
				element->LinkEndChild(content);
				changerElement->LinkEndChild(element);

				SetDrives(changerElement, drives);
				SetSlots(changerElement, slots);
				SetMailSlots(changerElement, mailslots);
				SetTapes(changerElement, tapes);

				changersElement->LinkEndChild(changerElement);
			}
		}
		catch(...)
		{
			SimDebug("SetChangers exception");
			return false;
		}

		return true;
	}

	bool
	CfgfileSerialization::SaveToFile()
	{
		try
		{
			document_->SaveFile(filename_.c_str());
		}
		catch(std::exception& e)
		{
			SimDebug("SaveToFile exception");
			return false;
		}

		return true;
	}

	bool
	CfgfileSerialization::SetDrives(TiXmlElement* parentElement, vector<struct DriveDetail>& drives)
	{
		TiXmlElement* element = NULL;
		TiXmlText* content = NULL;
		TiXmlElement* drivesElement = new TiXmlElement("drives");
		parentElement->LinkEndChild(drivesElement);

		for(vector<struct DriveDetail>::iterator iter=drives.begin();
				iter!=drives.end(); ++iter)
		{
			TiXmlElement* driveElement = new TiXmlElement("drive");

			element = new TiXmlElement("status");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_Status).c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("cleaning_status");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_CleaningStatus).c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("interface_type");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_InterfaceType).c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("slot_id");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_SlotId).c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("serial");
			content = new TiXmlText(iter->m_Serial.c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("vendor");
			content = new TiXmlText(iter->m_Vendor.c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("product");
			content = new TiXmlText(iter->m_Product.c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("version");
			content = new TiXmlText(iter->m_Version.c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("tape_key");
			content = new TiXmlText(iter->m_TapeKey.c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("barcode");
			content = new TiXmlText(iter->m_Barcode.c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("scsi_addr");
			content = new TiXmlText(iter->m_ScsiAddr.c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("stdev");
			content = new TiXmlText(iter->m_StDev.c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("sgdev");
			content = new TiXmlText(iter->m_SgDev.c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			element = new TiXmlElement("generation");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_Generation).c_str());
			element->LinkEndChild(content);
			driveElement->LinkEndChild(element);

			drivesElement->LinkEndChild(driveElement);
		}


		return true;
	}

	bool
	CfgfileSerialization::SetSlots(TiXmlElement* parentElement, vector<struct SlotDetail>& slots)
	{
		TiXmlElement* element = NULL;
		TiXmlText* content = NULL;
		TiXmlElement* slotsElement = new TiXmlElement("slots");
		parentElement->LinkEndChild(slotsElement);

		for(vector<struct SlotDetail>::iterator iter=slots.begin();
				iter!=slots.end(); ++iter)
		{
			TiXmlElement* slotElement = new TiXmlElement("slot");

			element = new TiXmlElement("slot_id");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_SlotID).c_str());
			element->LinkEndChild(content);
			slotElement->LinkEndChild(element);

			element = new TiXmlElement("tape_key");
			content = new TiXmlText(iter->m_TapeKey.c_str());
			element->LinkEndChild(content);
			slotElement->LinkEndChild(element);

			element = new TiXmlElement("barcode");
			content = new TiXmlText(iter->m_Barcode.c_str());
			element->LinkEndChild(content);
			slotElement->LinkEndChild(element);

			slotsElement->LinkEndChild(slotElement);
		}

		return true;
	}

	bool
	CfgfileSerialization::SetMailSlots(TiXmlElement* parentElement, vector<struct MailSlotDetail>& mailslots)
	{
		TiXmlElement* element = NULL;
		TiXmlText* content = NULL;
		TiXmlElement* mailslotsElement = new TiXmlElement("mailslots");
		parentElement->LinkEndChild(mailslotsElement);

		for(vector<struct MailSlotDetail>::iterator iter=mailslots.begin();
				iter!=mailslots.end(); ++iter)
		{
			TiXmlElement* mailslotElement = new TiXmlElement("mailslot");

			element = new TiXmlElement("slot_id");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_SlotID).c_str());
			element->LinkEndChild(content);
			mailslotElement->LinkEndChild(element);

			element = new TiXmlElement("tape_key");
			content = new TiXmlText(iter->m_TapeKey.c_str());
			element->LinkEndChild(content);
			mailslotElement->LinkEndChild(element);

			element = new TiXmlElement("barcode");
			content = new TiXmlText(iter->m_Barcode.c_str());
			element->LinkEndChild(content);
			mailslotElement->LinkEndChild(element);

			mailslotsElement->LinkEndChild(mailslotElement);
		}

		return true;
	}

	bool
	CfgfileSerialization::SetTapes(TiXmlElement* parentElement, vector<struct TapeDetail>& tapes)
	{
		TiXmlElement* element = NULL;
		TiXmlText* content = NULL;
		TiXmlElement* tapesElement = new TiXmlElement("tapes");
		parentElement->LinkEndChild(tapesElement);

		for(vector<struct TapeDetail>::iterator iter=tapes.begin();
				iter!=tapes.end(); ++iter)
		{
			TiXmlElement* tapeElement = new TiXmlElement("tape");

			element = new TiXmlElement("slot_id");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_SlotID).c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("tape_key");
			content = new TiXmlText(iter->m_TapeKey.c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("barcode");
			content = new TiXmlText(iter->m_Barcode.c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("status");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_Status).c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("medium_type");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_MediumType).c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("media_type");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_MediaType).c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("ltfs_format");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_LtfsFormat).c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("total_capacity");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_TotalCapacity).c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("group");
			content = new TiXmlText(iter->m_GroupID.c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("dual_copy");
			content = new TiXmlText(iter->m_DualCopy.c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("faulty");
			content = new TiXmlText(iter->m_Faulty?"true":"false");
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			element = new TiXmlElement("generation_index");
			content = new TiXmlText(
					boost::lexical_cast<string>(iter->m_GenerationIndex).c_str());
			element->LinkEndChild(content);
			tapeElement->LinkEndChild(element);

			tapesElement->LinkEndChild(tapeElement);
		}

		return true;
	}



} /* namespace ltfs_management */
