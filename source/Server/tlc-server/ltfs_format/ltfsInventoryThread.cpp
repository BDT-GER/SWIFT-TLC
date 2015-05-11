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
 * ltfsInventoryThread.cpp
 *
 *  Created on: Jan 5, 2013
 *      Author: chento
 */

#include "stdafx.h"
#include "../bdt/Factory.h"
#include "../ltfs_management/TapeLibraryMgr.h"
//#include "../socket/ltfsTaskManagement.h"
#include "ltfsInventoryThread.h"

namespace ltfs_management
{

	InventoryThread::InventoryThread(void* manager, const string& barcode):
		manager_(manager), barcode_(barcode)
	{
		// TODO Auto-generated constructor stub
		status_ 		= InventoryStatus_Waiting;
		timeStart_ 		= boost::posix_time::second_clock::local_time();
		timeEnd_ 		= timeStart_;
	}

	InventoryThread::~InventoryThread()
	{
		// TODO Auto-generated destructor stub
	}

	boost::posix_time::ptime
	InventoryThread::GetStartTime()
	{
		return timeStart_;
	}

	boost::posix_time::ptime
	InventoryThread::GetEndTime()
	{
		return timeEnd_;
	}

	bool
	InventoryThread::Start()
	{
		threadPtr_.reset(new boost::thread(boost::bind(&InventoryThread::Execute, this)));

		return true;
	}

	string
	InventoryThread::GetBarcode() const
	{
		return barcode_;
	}

	InventoryStatus
	InventoryThread::GetStatus()
	{
		return status_;
	}

	void
	InventoryThread::Execute()
	{
		//load
		status_			= InventoryStatus_Running;


		//free resource
		timeEnd_ 		= boost::posix_time::second_clock::local_time();
		status_			= InventoryStatus_Finish;
	}

} /* namespace ltfs_management */
