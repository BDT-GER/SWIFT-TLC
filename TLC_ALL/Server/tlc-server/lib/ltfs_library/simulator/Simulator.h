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
 * Simulator.h
 *
 *  Created on: Oct 11, 2012
 *      Author: chento
 */

#pragma once

#include "SimChanger.h"

namespace ltfs_management
{

	class Simulator
	{
	public:
		Simulator();

		virtual
		~Simulator();

		static Simulator*
		Instance()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if(NULL == instance_)
			{
				instance_ = new Simulator();
			}

			return instance_;
		}

		static void
		Destory()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if( NULL != instance_)
			{
				instance_->changers_.clear();
			}
		}

		void
		Refresh();

		bool
		GetCfgDetail(struct CfgDetail& detail);

		bool
		GetChangerList(vector<struct ChangerDetail>& changers, int& errId);

		bool
		GetDriveList(string& changerSerial, vector<struct DriveDetail> & drives, int& errId);

		bool
		GetSlotList(string& changerSerial, vector<struct SlotDetail> & slots, int& errId);

		bool
		GetMailSlotList(string& changerSerial, vector<struct MailSlotDetail> & mailSlots, int& errId);

		bool
		GetTapeList(string& changerSerial, vector<struct TapeDetail> & tapes, int& errId);

		bool
		MoveTape(string& changerSerial, int srcSlotId, int dstSlotId, int& errId);

		bool
		Format(string& label, string& driveSerial, int& errId);

		bool
		IsMounted(string& driveSerial, bool& mounted, int& errId);

		bool
		Mount(string& driveSerial, int& errId);

		bool
		Umount(string& driveSerial, int& errId);

		bool
		GetTapeGroupUUID(string& driveSerial, string& uuid, int& errId);

		bool
		SetTapeGroupUUID(string& driveSerial, const string& uuid, int& errId);

		bool SetTapeDualCopy(string& driveSerial, const string& dualCopy, int& errId);
		bool GetTapeDualCopy(string& driveSerial, string& dualCopy, int& errId);

		bool
		GetTapeFaulty(string& driveSerial, bool& faulty, int& errId);

		bool
		SetTapeFaulty(string& driveSerial, bool faulty, int& errId);

		bool
		GetTapeStatus(string& driveSerial, int& status, int& errId);

		bool
		SetTapeStatus(string& driveSerial, int status, int& errId);

		bool
		GetTapeMediaType(string& driveSerial, int& mediaType, int& errId);

		bool
		GetTapeFormatType(string& driveSerial, int& type, int& errId);

		bool
		GetTapeGenerationIndex(string& driveSerial, long long& index, int& errId);

		bool
		SetTapeGenerationIndex(string& driveSerial, long long index, int& errId);

		bool
		GetChangerInfo(string& changerSerial, struct ChangerDetail& changerDetail);

		bool
		GetDriveInfo(string& driveSerial, struct DriveDetail& driveDetail);

		bool
		GetTapeInfo(string& driveSerial, struct TapeDetail& tapeDetail, int& errId);

		bool
		GetLoadedTapeCapacity(string& driveSerial, Int64_t &totalCapacity, Int64_t& freeCapacity);

		bool
		Read(string& barcode, size_t size);

		bool
		Write(string& barcode, size_t size);

	private:
		bool
		SerializationCfg();
	private:

		typedef map<string,SimChanger> 	ChangerMap;

		ChangerMap 						changers_;

		boost::mutex					cfgfileMutex_;

		static boost::mutex 			instanceMutex_;
		static Simulator * 				instance_;

	};

} /* namespace ltfs_management */

