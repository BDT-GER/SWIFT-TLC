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
 * SimSlot.h
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#pragma once

namespace ltfs_management
{

	enum SlotType
	{
		TP_SLOT,
		TP_DRIVE,
		TP_MAILSLOT
	};

	struct SlotDetail
	{
		int 	m_SlotID;
		int		m_LogicSlotId;
		string	m_TapeKey;
		string 	m_Barcode;
	};

	class SimSlot
	{
	public:

		SimSlot(SlotType type);

		SimSlot(struct SlotDetail& detail);

		virtual
		~SimSlot();

		void
		GetDetail(struct SlotDetail& detail);

		virtual int
		GetSlotID() const;

		virtual int
		GetLogicalSlotID() const;

		virtual string
		GetTapeKey() const;

		virtual string
		GetBarcode() const;

		virtual bool
		InsertTape(const string& barcode, const string& key);

	protected:
		SlotType type_;

	private:

		struct SlotDetail detail_;
	};

} /* namespace ltfs_management */
