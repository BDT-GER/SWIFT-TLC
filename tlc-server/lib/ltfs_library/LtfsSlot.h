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
 * LtfsSlot.h
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#pragma once

namespace ltfs_management
{
	class LtfsSlot
	{
	public:
		friend class LtfsChanger;

		virtual
		~LtfsSlot();

		LtfsTape*
		GetTape() const;

		void
		SetTape(LtfsTape* tape);

		int
		GetSlotID() const;

		int
		GetLogicSlotID() const;

		bool
		GetAccessible() const;

		bool
		GetAbonormal() const;

		string
		GetBarcode() const;

		bool
		IsEmpty() const;

		bool IsAvailable() const;

		void
		GetSlotInfo(LtfsSlotInfo& slot);

		void
		GetTapeInfo(LtfsTapeInfo& tape);

	protected:
		LtfsSlot(int slotid, int logicSlotId, LtfsTape* tape=NULL);

	protected:
		int			slotID_;
		int			logicSlotID_;
		bool 		accessible_;
		bool		abnormal_;
		LtfsTape* 	tape_;
	};

} /* namespace ltfs_management */

