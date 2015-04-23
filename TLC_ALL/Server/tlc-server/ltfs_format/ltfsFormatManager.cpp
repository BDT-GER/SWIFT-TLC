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
 * ltfsFormatManager.cpp
 *
 *  Created on: Dec 5, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "ltfsFormatManager.h"
#include "ltfsFormatThread.h"

namespace ltfs_management
{
	FormatManager * FormatManager::instance_ = NULL;
	boost::mutex FormatManager::instanceMutex_;

	FormatManager::FormatManager()
	{
		// TODO Auto-generated constructor stub

	}

	FormatManager::~FormatManager()
	{
		// TODO Auto-generated destructor stub
	}

	FormatThread*
	FormatManager::StartFormat(const string& barcode, FormatType type, int priority, Labels& labels, time_t startTimeInMicroSecs)
	{
		boost::lock_guard<boost::mutex> lock(mutex_);


		FormatThread* formatThread = new FormatThread(this, barcode, type, priority, startTimeInMicroSecs);
		if(formatThread != NULL)
		{
			formatThread->SetLabels(labels);
			if(formatThread->Start())
			{
				formatQueue_.push_back(formatThread);
				SocketDebug("FormatManager::StartFormat ok, barcode "<<barcode);

				return formatThread;
			}

			delete formatThread;
		}
		SocketError("FormatManager::StartFormat failed, barcode "<<barcode);

		return NULL;
	}

	bool
	FormatManager::StartInventory(const string& barcode)
	{
		boost::lock_guard<boost::mutex> lock(inventoryMutex_);

		InventoryThread* inventoryThread = new InventoryThread(this, barcode);
		if(inventoryThread != NULL)
		{
			if(inventoryThread->Start())
			{
				inventoryQueue_.push_back(inventoryThread);
				SocketDebug("FormatManager::StartInventory ok, barcode "<<barcode);

				return true;
			}

			delete inventoryThread;
		}
		SocketError("FormatManager::StartInventory failed, barcode "<<barcode);

		return false;
	}

	bool
	FormatManager::CancelFormat(const string& barcode)
	{
		boost::lock_guard<boost::mutex> lock(mutex_);

		bool find = false;

		for(vector<FormatThread*>::iterator iter=formatQueue_.begin();
				iter!=formatQueue_.end(); ++iter)
		{
			if(barcode == (*iter)->GetBarcode())
			{
				(*iter)->Cancel();

				find = true;
			}
		}

		return find;
	}

	bool
	FormatManager::GetFormatStatus(const string& barcode, FormatThreadStatus& status)
	{
		boost::lock_guard<boost::mutex> lock(mutex_);

		status = FORMAT_THREAD_STATUS_FINISHED;

		for(vector<FormatThread*>::iterator iter=formatQueue_.begin();
				iter!=formatQueue_.end(); ++iter)
		{
			if(barcode == (*iter)->GetBarcode())
			{
				FormatThreadStatus itStatus = (*iter)->GetStatus();
				if(itStatus == FORMAT_THREAD_STATUS_FORMATTING){
					status = itStatus;
					break;
				}
				if(itStatus != FORMAT_THREAD_STATUS_FINISHED){
					status = itStatus;
				}
			}
		}

		return true;
	}

	bool
	FormatManager::GetFormatStatus(FormatThread* formatThread, FormatThreadStatus& status)
	{
		boost::lock_guard<boost::mutex> lock(mutex_);

		status = FORMAT_THREAD_STATUS_FINISHED;

		for(vector<FormatThread*>::iterator iter=formatQueue_.begin();
				iter!=formatQueue_.end(); ++iter)
		{
			if(formatThread == *iter)
			{
				status = (*iter)->GetStatus();
			}
		}

		return true;
	}

	bool
	FormatManager::GetDetailForBackup(vector<FormatDetail>& details)
	{
		boost::lock_guard<boost::mutex> lock(mutex_);

		ClearElement();

		for(vector<FormatThread*>::iterator iter=formatQueue_.begin();
				iter!=formatQueue_.end(); ++iter)
		{
			FormatDetail detail;
			(*iter)->GetDetail(detail);
			if(detail.mStatus != FORMAT_THREAD_STATUS_FINISHED)
				details.push_back(detail);
		}

		return true;
	}

	void
	FormatManager::GetInformation(FormatThread* formatThread, bool& getResource, bool& needCancel)
	{
		boost::lock_guard<boost::mutex> lock(mutex_);

		getResource = true;
		needCancel = false;
		bool bQueueBusy = false;

		ClearElement();

		for(vector<FormatThread*>::iterator iter=formatQueue_.begin();
				iter!=formatQueue_.end(); ++iter)
		{
			if(formatThread == *iter
					|| (*iter)->GetStatus() == FORMAT_THREAD_STATUS_FINISHED)
				continue;

			if(formatThread->GetBarcode() == (*iter)->GetBarcode())
			{
				/*
				if(formatThread->GetStartTime()<=(*iter)->GetStartTime()
						&& formatThread->GetType() == (*iter)->GetType()
						&& formatThread->GetLabelMark() == (*iter)->GetLabelMark())
				{
					needCancel = true;
					getResource = false;
					return;
				}
				*/
				// for the same tape, early task should be handled first
				if(formatThread->GetStartTime() > (*iter)->GetStartTime()){
					getResource = false;
					bQueueBusy = true;
					break;
				}
				// for the same tape with the same start time, high priority task should be handled first
				else if(formatThread->GetStartTime() == (*iter)->GetStartTime()){
					if( (formatThread->GetPriority() < (*iter)->GetPriority())){
						getResource = false;
						bQueueBusy = true;
						break;
					}
				}
				continue;
			}

			// the tape of the thread to check is busy, no need to care about it
			if((*iter)->IsTapeBusy()){
				continue;
			}

			if((*iter)->GetStatus() == FORMAT_THREAD_STATUS_WAITING)
			{
				bool thisInDrive = formatThread->IsTapeInDrive();
				bool itInDrive = (*iter)->IsTapeInDrive();
				if( (formatThread->GetPriority() < (*iter)->GetPriority())  // high priority task should run first
				|| (formatThread->GetPriority()==(*iter)->GetPriority()
						&& false == thisInDrive && true == itInDrive)  // tape in drive should run first
				|| (formatThread->GetPriority()==(*iter)->GetPriority()
						&& thisInDrive == itInDrive
						&& formatThread->GetStartTime() > (*iter)->GetStartTime())
				)
				{
					getResource = false;
				}
			}

		}
		formatThread->SetTapeBusyInQueue(bQueueBusy);
	}

	void
	FormatManager::ClearElement()
	{
		for(vector<FormatThread*>::iterator iter=formatQueue_.begin();
				iter!=formatQueue_.end(); )
		{
			if((*iter)->GetStatus() == FORMAT_THREAD_STATUS_FINISHED
					||(*iter)->GetStartTime() != (*iter)->GetEndTime() )
			{
				struct timeval tvNow;
				gettimeofday(&tvNow, NULL);
				time_t diff = tvNow.tv_sec * 1000000 + tvNow.tv_usec - (*iter)->GetEndTime();
				if(diff > 30 * 1000000) // in seconds
				{
					delete (*iter);
					iter = formatQueue_.erase(iter);
					continue;
				}
			}
			++iter;
		}

	}

	bool
	FormatManager::IsFinishFormat(const string& barcode, FormatThread* formatThread)
	{
		boost::lock_guard<boost::mutex> lock(mutex_);

		bool bFinished = true;

		for(vector<FormatThread*>::iterator iter=formatQueue_.begin();
				iter!=formatQueue_.end(); ++iter)
		{
			if((barcode == (*iter)->GetBarcode() && *iter != formatThread)
					&& (*iter)->GetStatus() != FORMAT_THREAD_STATUS_FINISHED)
			{
				bFinished = false;
				break;
			}
		}

		return bFinished;
	}

	void
	FormatManager::GetInventoryList(vector<string>& barcodes)
	{
		boost::lock_guard<boost::mutex> lock(inventoryMutex_);

		barcodes.clear();

		for(vector<InventoryThread*>::iterator iter=inventoryQueue_.begin();
				iter!=inventoryQueue_.end(); ++iter)
		{
			barcodes.push_back((*iter)->GetBarcode());
		}
	}

	bool
	FormatManager::GetInventorySatus(const string& barcode, InventoryStatus& status)
	{
		boost::lock_guard<boost::mutex> lock(inventoryMutex_);

		for(vector<InventoryThread*>::iterator iter=inventoryQueue_.begin();
				iter!=inventoryQueue_.end(); ++iter)
		{
			if((*iter)->GetBarcode() == barcode)
			{
				status = (*iter)->GetStatus();
				return true;
			}
		}
		return false;
	}

	bool
	FormatManager::ClearInventoryList()
	{
		boost::lock_guard<boost::mutex> lock(inventoryMutex_);

		//check
		for(vector<InventoryThread*>::iterator iter=inventoryQueue_.begin();
				iter!=inventoryQueue_.end(); ++iter)
		{
			if((*iter)->GetStatus() != InventoryStatus_Finish)
				return false;
		}

		for(vector<InventoryThread*>::iterator iter=inventoryQueue_.begin();
				iter!=inventoryQueue_.end(); )
		{
			delete (*iter);
			iter = inventoryQueue_.erase(iter);
		}

		return true;
	}
} /* namespace ltfs_management */
