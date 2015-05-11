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
 * LtfsSlot.cpp
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "LtfsDetails.h"
#include "LtfsTape.h"
#include "LtfsSlot.h"

namespace ltfs_management
{

	LtfsSlot::LtfsSlot(int slotid, int logicSlotId, LtfsTape* tape):
		slotID_(slotid), logicSlotID_(logicSlotId), tape_(tape)
	{
		// TODO Auto-generated constructor stub

	}

	LtfsSlot::~LtfsSlot()
	{
		// TODO Auto-generated destructor stub
	}

	LtfsTape*
	LtfsSlot::GetTape() const
	{
		return tape_;
	}

	void
	LtfsSlot::SetTape(LtfsTape* tape)
	{
		tape_ = tape;
	}

	int
	LtfsSlot::GetSlotID() const
	{
		return slotID_;
	}

	int
	LtfsSlot::GetLogicSlotID() const
	{
		return logicSlotID_;
	}

	bool
	LtfsSlot::GetAccessible() const
	{
		return accessible_;
	}

	bool
	LtfsSlot::GetAbonormal() const
	{
		return abnormal_;
	}

	string
	LtfsSlot::GetBarcode() const
	{
		if(tape_==NULL)
		{
			return "";
		}
		else
		{
			return tape_->GetBarcode();
		}
	}

	bool
	LtfsSlot::IsEmpty() const
	{
		return tape_==NULL?true:false;
	}


	bool
	LtfsSlot::IsAvailable() const
	{
		return IsEmpty();
	}

	void
	LtfsSlot::GetSlotInfo(LtfsSlotInfo& slot)
	{
		slot.mSlotID = slotID_;
		slot.mLogicSlotID = logicSlotID_;
		slot.mIsEmpty = (tape_==NULL);
		slot.mBarcode = GetBarcode();
		slot.mAccessible = accessible_;
		slot.mAbnormal = abnormal_;
	}

	void
	LtfsSlot::GetTapeInfo(LtfsTapeInfo& tape)
	{
		if(tape_!=NULL)
		{
			tape.mBarcode = tape_->GetBarcode();
			tape.mSlotID = slotID_;
			tape.mLogicSlotID = logicSlotID_;
			tape.mMediumType = tape_->GetMediumType();
		}
	}

} /* namespace ltfs_management */
