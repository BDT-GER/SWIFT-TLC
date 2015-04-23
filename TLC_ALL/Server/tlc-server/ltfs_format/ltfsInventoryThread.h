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
 * ltfsInventoryThread.h
 *
 *  Created on: Jan 5, 2013
 *      Author: chento
 */

#pragma once

namespace ltfs_management
{

	class InventoryThread
	{
	public:
		InventoryThread(void* manager, const string& barcode);

		virtual
		~InventoryThread();

		boost::posix_time::ptime
		GetStartTime();

		boost::posix_time::ptime
		GetEndTime();

		bool
		Start();

		void
		Execute();

		string
		GetBarcode() const;

		InventoryStatus
		GetStatus();


	private:
		void* manager_;
		string barcode_;

		InventoryStatus status_;

		boost::posix_time::ptime 		 timeStart_;
		boost::posix_time::ptime 		 timeEnd_;
		boost::scoped_ptr<boost::thread> threadPtr_;
	};

} /* namespace ltfs_management */
