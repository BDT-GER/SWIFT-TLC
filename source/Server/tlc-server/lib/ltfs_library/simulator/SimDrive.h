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
 * SimDrive.h
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#pragma once

namespace ltfs_management
{

	struct DriveDetail
	{
		int 	m_Status;
		int 	m_CleaningStatus;
		int 	m_InterfaceType;
		int		m_SlotId;
		int		m_LogicSlotId;
		string	m_Serial;
		string	m_TapeKey;
		string	m_Barcode;
		string	m_ScsiAddr;
		string	m_Vendor;
		string	m_Product;
		string	m_Version;
		string	m_StDev;
		string	m_SgDev;
		int		m_Generation;
		//bool	m_IsFullHight;
		//bool	m_Accessible;
		//bool	m_Abnormal;
	};

	class SimDrive : public SimSlot
	{
	public:
		SimDrive(struct DriveDetail& detail);

		virtual
		~SimDrive();

		void
		GetDetail(struct DriveDetail& detail);

		string
		GetScsiAddr() const;

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

		bool
		Format(string& label);

		bool
		IsMounted(bool& mounted);

		bool
		Mount();

		bool
		Umount();

	private:

		struct DriveDetail detail_;
	};

} /* namespace ltfs_management */

