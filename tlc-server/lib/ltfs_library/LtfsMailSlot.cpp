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
 * LtfsMailSlot.cpp
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "LtfsDetails.h"
#include "LtfsTape.h"
#include "LtfsSlot.h"
#include "LtfsMailSlot.h"

namespace ltfs_management
{

	LtfsMailSlot::LtfsMailSlot(int slotid, int logicSlotId, bool isOpen):
		LtfsSlot(slotid, logicSlotId), isOpen_(isOpen)
	{
		// TODO Auto-generated constructor stub

	}

	LtfsMailSlot::~LtfsMailSlot()
	{
		// TODO Auto-generated destructor stub
	}

	void
	LtfsMailSlot::SetMailSlotOpened(bool opened)
	{
		isOpen_ = opened;
	}

	void
	LtfsMailSlot::GetMailSlotInfo(LtfsMailSlotInfo& mailSlot)
	{
		mailSlot.mSlotID = slotID_;
		mailSlot.mLogicSlotID = logicSlotID_;
		mailSlot.mIsEmpty = (tape_==NULL);
		mailSlot.mBarcode = GetBarcode();
		mailSlot.mIsOpen = isOpen_;
		mailSlot.mAccessible = accessible_;
		mailSlot.mAbnormal = abnormal_;

	}

} /* namespace ltfs_management */

