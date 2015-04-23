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
 * ltfsFormatManager.h
 *
 *  Created on: Dec 5, 2012
 *      Author: chento
 */

#pragma once

#include "ltfsFormatDetails.h"
#include "ltfsInventoryThread.h"
#include "../ltfs_management/TapeLibraryMgr.h"

namespace ltfs_management
{

	class FormatThread;

	class FormatManager
	{
	private:
		static FormatManager*
		Instance()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if(NULL == instance_)
			{
				instance_ = new FormatManager();
			}

			return instance_;
		}

		static void
		Destory()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if( NULL != instance_)
			{
				delete instance_;
				instance_ = NULL;
			}
		}

	public:
		FormatThread*
		StartFormat(const string& barcode, FormatType type, int priority, Labels& labels, time_t startTimeInMicroSecs = 0);

		bool
		CancelFormat(const string& barcode);

		bool
		GetFormatStatus(const string& barcode, FormatThreadStatus& status);

		bool
		GetFormatStatus(FormatThread* formatThread, FormatThreadStatus& status);

		bool
		GetDetailForBackup(vector<FormatDetail>& details);

		void
		GetInformation(FormatThread* formatThread, bool& getResource, bool& needCancel);

		bool
		StartInventory(const string& barcode);

		void
		GetInventoryList(vector<string>& barcodes);

		bool
		GetInventorySatus(const string& barcode, InventoryStatus& status);

		bool
		ClearInventoryList();

		bool
		IsFinishFormat(const string& barcode, FormatThread*formatThread);

	private:
		FormatManager();

		virtual
		~FormatManager();

		void
		ClearElement();

	private:
		vector<FormatThread*> formatQueue_;
		vector<InventoryThread*> inventoryQueue_;

		boost::mutex mutex_;
		boost::mutex inventoryMutex_;

		static boost::mutex 			instanceMutex_;
		static FormatManager * 			instance_;

		friend TapeLibraryMgr* TapeLibraryMgr::Instance();
		friend void TapeLibraryMgr::Destroy();
	};

} /* namespace ltfs_management */

