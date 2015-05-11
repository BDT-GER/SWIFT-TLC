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
 * SimSlot.cpp
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "SimSlot.h"

namespace ltfs_management
{

	SimSlot::SimSlot(SlotType type):
		type_(type)
	{

	}

	SimSlot::SimSlot(struct SlotDetail& detail)
	{
		// TODO Auto-generated constructor stub
		detail_.m_Barcode = detail.m_Barcode;
		detail_.m_SlotID  = detail.m_SlotID;
		detail_.m_TapeKey = detail.m_TapeKey;

		type_ = TP_SLOT;
	}

	SimSlot::~SimSlot()
	{
		// TODO Auto-generated destructor stub
	}

	void
	SimSlot::GetDetail(struct SlotDetail& detail)
	{
		detail.m_Barcode = detail_.m_Barcode;
		detail.m_SlotID  = detail_.m_SlotID;
		detail.m_TapeKey = detail_.m_TapeKey;
	}

	int
	SimSlot::GetSlotID() const
	{
		return detail_.m_SlotID;
	}

	int
	SimSlot::GetLogicalSlotID() const
	{
		return detail_.m_LogicSlotId;
	}

	string
	SimSlot::GetTapeKey() const
	{
		return detail_.m_TapeKey;
	}

	string
	SimSlot::GetBarcode() const
	{
		return detail_.m_Barcode;
	}

	bool
	SimSlot::InsertTape(const string& barcode, const string& key)
	{
		if(!key.empty() && !detail_.m_TapeKey.empty() )
					return false;

		detail_.m_TapeKey = key;
		detail_.m_Barcode = barcode;

		return true;
	}

} /* namespace ltfs_management */
