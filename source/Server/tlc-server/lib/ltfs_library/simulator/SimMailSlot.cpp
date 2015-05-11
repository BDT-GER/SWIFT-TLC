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
 * SimMailSlot.cpp
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "SimSlot.h"
#include "SimMailSlot.h"

namespace ltfs_management
{

	SimMailSlot::SimMailSlot(struct MailSlotDetail& detail):
			SimSlot(TP_MAILSLOT)
	{
		// TODO Auto-generated constructor stub
		detail_.m_Barcode = detail.m_Barcode;
		detail_.m_SlotID  = detail.m_SlotID;
		detail_.m_TapeKey	  = detail.m_TapeKey;
	}

	SimMailSlot::~SimMailSlot()
	{
		// TODO Auto-generated destructor stub
	}

	void
	SimMailSlot::GetDetail(struct MailSlotDetail& detail)
	{
		detail.m_Barcode = detail_.m_Barcode;
		detail.m_SlotID  = detail_.m_SlotID;
		detail.m_TapeKey = detail_.m_TapeKey;
	}

	int
	SimMailSlot::GetSlotID() const
	{
		return detail_.m_SlotID;
	}

	int
	SimMailSlot::GetLogicalSlotID() const
	{
		return detail_.m_LogicSlotId;
	}

	string
	SimMailSlot::GetTapeKey() const
	{
		return detail_.m_TapeKey;
	}

	string
	SimMailSlot::GetBarcode() const
	{
		return detail_.m_Barcode;
	}

	bool
	SimMailSlot::InsertTape(const string& barcode, const string& key)
	{
		if(!key.empty() && !detail_.m_TapeKey.empty() )
					return false;

		detail_.m_TapeKey = key;
		detail_.m_Barcode = barcode;

		return true;
	}

} /* namespace ltfs_management */
