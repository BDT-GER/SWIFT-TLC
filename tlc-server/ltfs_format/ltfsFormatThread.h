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
 * ltfsFormatThread.h
 *
 *  Created on: Dec 5, 2012
 *      Author: chento
 */

#pragma once
#include "ltfsFormatManager.h"

namespace ltfs_management
{
	class TapeDbManager;
	class LtfsLibraries;

	class FormatThread
	{
	private:
		FormatThread(void* manager, const string& barcode, FormatType type, int priority, time_t startTime);

	public:
		virtual
		~FormatThread();

		void
		SetLabels(Labels& labels);

		void
		Execute();

	public:
		string
		GetBarcode();

		bool IsTapeInDrive();

		bool IsTapeBusy();
		bool SetTapeBusyInQueue(bool bQueueBusy);

		FormatThreadStatus
		GetStatus();

		FormatType
		GetType();

		int
		GetPriority();

		time_t GetStartTime();

		time_t GetEndTime();

		int
		GetLabelMark();

		void
		GetDetail(FormatDetail& detail);

		bool
		Start();

		bool
		Cancel();

		bool
		RequestResource(bool mount = false);

		bool
		FreeResource();

		bool
		Process();

		bool
		Format(const string& changerSerial, const string& driveSerial);

		bool
		WriteLabel(const string& changerSerial, const string& driveSerial, const Labels& labels);

		bool
		GetTapeType(const string& changerSerial, bool& lto5);

		void
		GetTapeBalel(const string& changerSerial, const string& driveSerial, Labels& labels);

		void
		GetLTOTapeDefaultFormatSize(int tapeType, long long &freeCapacity, long long &usedCapacity);

	private:
		LtfsLibraries* GetHalInstance();
		TapeDbManager* GetTapeDbManager();

	private:
		//labels
		int labelMark_;
		int	labelStatus_;
		bool labelFaulty_;
		string labelGroupID_;
		string labelDualCopy_;
		string labelTapeId_;

		void* manager_;
		string barcode_;
		bool canceled_;
		bool autoCanceled_;
		bool tapeBusy_;
		FormatType type_;
		int priority_;
		FormatThreadStatus status_;
		bool queueBusy_;

		time_t	timeStart_;		// in microseconds
		time_t  timeEnd_;		// in microseconds
		boost::scoped_ptr<boost::thread> threadPtr_;

		friend FormatThread* FormatManager::StartFormat(const string& barcode, FormatType type, int priority, Labels& labels, time_t startTimeInMicroSecs);
	};

} /* namespace ltfs_management */

