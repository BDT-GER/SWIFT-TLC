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
 * SimTape.h
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#pragma once

namespace ltfs_management
{

	struct TapeDetail
	{
		string	m_TapeKey;
		string	m_Barcode;
		int		m_SlotID;
		int		m_LogicSlotId;
		int		m_Status;
		int		m_MediumType;
		int		m_MediaType;
		int		m_LtfsFormat;
		Int64_t	m_TotalCapacity;
		Int64_t	m_FreeCapacity;

		string  m_GroupID;
		string	m_DualCopy;
		bool	m_Faulty;
		bool	m_Formated;
		Int64_t	m_GenerationIndex;

		int		m_ErrnumRead;
		int		m_ErrnumWrite;
		size_t	m_SizeRead;
		size_t	m_SizeWrite;
	};

	class SimTape
	{
	public:
		SimTape(struct TapeDetail& detail);

		virtual
		~SimTape();

		string
		GetBarcode() const;

		string
		GetKey() const;

		void
		GetDetail(struct TapeDetail& detail);

		bool
		SetSlotID(int slotid, int logicSlotId);


		void
		SetFormated();

		string
		GetGroupID() const;

		void
		SetGroupID(const string& groupid);

		string GetDualCopy() const;
		void SetDualCopy(const string& dualCopy);

		bool
		GetFaulty() const;

		void
		SetFaulty(bool fault);

		int
		GetStatus() const;

		void
		SetStatus(int status);

		int
		GetFormatType() const;

		int
		GetMediaType() const;

		long long
		GetGenerationIndex() const;

		void
		SetGenerationIndex(long long index);

		bool
		GetCapacity(Int64_t& total, Int64_t& free);

		void
		Read(size_t & size);

		void
		Write(size_t & size);

		void
		SetReadErrNum(int n);

		void
		SetWriteErrNum(int n);

	private:

		struct TapeDetail detail_;

		boost::mutex*     readMutex_;
		boost::mutex* 	  writeMutex_;

	};

} /* namespace ltfs_management */

