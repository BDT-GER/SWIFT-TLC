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
 * ltfsFormatThread.cpp
 *
 *  Created on: Dec 5, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "../ltfs_management/TapeLibraryMgr.h"
#include "../ltfs_management/TapeStateMachineMgr.h"
#include "../ltfs_management/TapeDbManager.h"
#include "../socket/ltfsTaskManagement.h"
#include "ltfsFormatManager.h"
#include "../lib/ltfs_library/LtfsLibraries.h"
#include "confParser.h"

const int FORMAT_BUSY_WAIT = 2;     // seconds wait when a tape is busy
static const off_t LTFS5_DEFAULT_SIZE = 0x15000000000;
static const off_t LTFS6_DEFAULT_SIZE = 0x2A000000000;

namespace ltfs_management
{

	FormatThread::FormatThread(void* manager, const string& barcode, FormatType type, int priority, time_t startTimeInMicroSecs, bool bRequested):
		manager_(manager), barcode_(barcode), type_(type), priority_(priority), status_(FORMAT_THREAD_STATUS_WAITING),
		timeStart_(startTimeInMicroSecs), requested_(bRequested)
	{
		// TODO Auto-generated constructor stub
		canceled_ = false;
		autoCanceled_ = false;

		labelMark_ = 0;
		labelStatus_ = 0;
		labelFaulty_ = false;
		tapeBusy_ = false;
		labelGroupID_ = "";
		labelDualCopy_ = "";
		labelTapeId_ = "";
		queueBusy_ = false;

		if(timeStart_ == 0){
			struct timeval tvNow;
			gettimeofday(&tvNow, NULL);
			timeStart_ 		= tvNow.tv_sec * 1000000 + tvNow.tv_usec;
		}
		timeEnd_ 		= timeStart_;
	}

	FormatThread::~FormatThread()
	{
		// TODO Auto-generated destructor stub

	}
    LtfsLibraries* FormatThread::GetHalInstance()
    {
    	return LtfsLibraries::Instance();
    }

	TapeDbManager* FormatThread::GetTapeDbManager()
	{
    	return TapeDbManager::Instance();
	}

	void
	FormatThread::SetLabels(Labels& labels)
	{
		labelMark_ = labels.mMark;
		labelStatus_ = labels.mStatus;
		labelFaulty_ = labels.mFaulty;
		labelGroupID_ = labels.mGroupID;
		labelDualCopy_ = labels.mDualCopy;
		labelTapeId_ = labels.mTapeID;

		SocketDebug("SetLabels: labelMark_ = " << labelMark_ << ", labelStatus_ = " << labelStatus_
				<< ", labelFaulty_ = " << labelFaulty_ << ", labelGroupID_ = " << labelGroupID_
				<< ", labelDualCopy_ = " << labelDualCopy_ << ", labelTapeId_ = " << labelTapeId_
				<< ".");
	}

	string
	FormatThread::GetBarcode()
	{
		return barcode_;
	}


	bool FormatThread::IsTapeInDrive()
	{
		return GetHalInstance()->IsTapeInDrive(GetBarcode());
	}

	bool FormatThread::SetTapeBusyInQueue(bool bQueueBusy)
	{
		queueBusy_ = bQueueBusy;
		return true;
	}

	bool FormatThread::IsTapeBusy()
	{
		return (tapeBusy_ || queueBusy_);
	}

	FormatThreadStatus
	FormatThread::GetStatus()
	{
		return status_;
	}

	FormatType
	FormatThread::GetType()
	{
		return type_;
	}

	int
	FormatThread::GetLabelMark()
	{
		return labelMark_;
	}

	int
	FormatThread::GetPriority()
	{
		return priority_;
	}

	time_t FormatThread::GetStartTime()
	{
		return timeStart_;
	}

	time_t FormatThread::GetEndTime()
	{
		return timeEnd_;
	}

	void
	FormatThread::GetDetail(FormatDetail& detail)
	{
		detail.mBarcode = barcode_;
		detail.mPriority = priority_;
		detail.mStatus = status_;
		detail.mType = type_;
		detail.mStartTime = timeStart_;
		detail.mLabels.mMark = labelMark_;
		detail.mLabels.mStatus = labelStatus_;
		detail.mLabels.mFaulty = labelFaulty_;
		detail.mLabels.mGroupID = labelGroupID_;
		detail.mLabels.mDualCopy = labelDualCopy_;
		detail.mLabels.mTapeID = labelTapeId_;

	}

	bool
	FormatThread::Start()
	{
		threadPtr_.reset(new boost::thread(boost::bind(&FormatThread::Execute, this)));

		return true;
	}

	bool
	FormatThread::Cancel()
	{
		canceled_ = true;
		return true;
	}

	void
	FormatThread::Execute()
	{
		SocketDebug("FormatThread Execute, format tape " << barcode_);
		bool bNeedReleaseTape = false;

		ltfs_management::CartridgeDetail detail;
		bool bGet = GetTapeDbManager()->GetCartridge(barcode_, detail);
		if(bGet && detail.mActivity == ACT_IDLE){
			// set tape activity to waiting
			if(!GetTapeDbManager()->SetActivityForCartridge(barcode_, ACT_WAITING)){
				SocketError("Failed to set tape " << barcode_ << " to waiting activity.");
			}
		}

		if(type_==FormatType_Format	|| type_ == FormatType_Both)
		{//update tape free capacity for format
			if (bGet) {
				long long freeCapacity, usedCapacity;
				GetLTOTapeDefaultFormatSize(detail.mMediaType,freeCapacity, usedCapacity);
				if(!GetTapeDbManager()->SetUsedCapacityForCartridge(barcode_, usedCapacity)//0xF800000
					|| !GetTapeDbManager()->SetFreeCapacityForCartridge(barcode_, freeCapacity)
					|| !GetTapeDbManager()->SetFileCapacityForCartridge(barcode_, 0)
					|| !GetTapeDbManager()->SetFileNumberForCartridge(barcode_, 0))
				{
					SocketError( "Failed to update database : " << barcode_ );
				}
			}else
			{
				SocketError( "Failed to get cartridge detail : " << barcode_ );
			}
		}
		do
		{
			ltfs_soapserver::TaskManagement::GetInstance()->SaveTaskQueue();
			if(canceled_)
			{
				break;
			}

			if(!RequestResource())
			{
				break;
			}

			bNeedReleaseTape = true;

			if(!Process())
			{
				if(type_ != FormatType_Label)
				{
					SocketEvent(EVENT_LEVEL_ERR, "Tape_Format_Failed", "Failed to format tape " << barcode_<<"."); //Event/Notification/Alert
				}
				else
				{
					SocketEvent(EVENT_LEVEL_ERR, "", "Failed to label tape " << barcode_<<"."); //Event/Notification/Alert  TODO:EVENT
				}
			}
			else
			{
				if(type_ != FormatType_Label)
				{
					SocketEvent(EVENT_LEVEL_INFO, "Tape_Format_Finished", "Tape " << barcode_ << " formatted.");//Event/Notification/Alert
				}
				else
				{
					SocketEvent(EVENT_LEVEL_INFO, "", "Tape " << barcode_ << " has been labeled."); //Event/Notification/Alert??  TODO:EVENT
				}
			}

		}while(false);

		if(!autoCanceled_)
		{
		}
		else
		{
			SocketWarn("FormatThread, auto canceled, barcode: " << barcode_);
		}

		status_ = FORMAT_THREAD_STATUS_FINISHED;
		struct timeval tvNow;
		gettimeofday(&tvNow, NULL);
		timeEnd_ 		= tvNow.tv_sec * 1000000 + tvNow.tv_usec;

		ltfs_soapserver::TaskManagement::GetInstance()->SaveTaskQueue();

		SocketDebug("autoCanceled_ = " << autoCanceled_ << ", canceled_ = " << canceled_ << ", bNeedReleaseTape = " << bNeedReleaseTape);
		if(bNeedReleaseTape){
			FreeResource();
		}

		return;
	}

	bool
	FormatThread::RequestResource(bool mount)
	{
		SocketDebug("RequestResource, barcode :" << barcode_ );

		bool getResource = true;
		bool needCancel = false;
		FormatManager* manager = (FormatManager*) manager_;

		if(manager_ == NULL)
			return false;
		do
		{
			tapeBusy_ = false;
			manager->GetInformation(this, getResource, needCancel);
			if(needCancel || canceled_)
			{
				canceled_ = true;
				autoCanceled_ = needCancel;
				SocketDebug("RequestResource , barcode :" << barcode_  << " return false");
				return false;
			}

			if(getResource){
				status_ = FORMAT_THREAD_STATUS_FORMATTING;
				if(requested_){
					SocketDebug("RequestResource, already requested tape: " << barcode_);
					break;
				}
				SocketDebug("RequestResource, getResource ok. calling RequestTape: " << barcode_);
				bool busy = false;
				if(true == ltfs_management::TapeLibraryMgr::Instance()->RequestTape(barcode_, mount, priority_, 1, busy)){
					SocketDebug("DDEBUG: RequestResource, getResource ok. finished calling RequestTape: " << barcode_);
					break;
				}
				if(true == busy){
					tapeBusy_ = true;
					SocketDebug("RequestResource: tape " << barcode_  << " is busy, wait " << FORMAT_BUSY_WAIT << " seconds and retry.");
					sleep(FORMAT_BUSY_WAIT);
				}
			}else{
				sleep(1);
			}
//#endif

		}while(true);

		SocketDebug("RequestResource, barcode :" << barcode_ <<  " return true");

		return true;
	}

	bool
	FormatThread::FreeResource()
	{
		if(false == requested_){
			ltfs_management::TapeLibraryMgr::Instance()->
					ReleaseTape(barcode_);
		}

		SocketDebug("FreeResource, barcode :" << barcode_);

		return true;
	}

	bool
	FormatThread::Process()
	{
		ltfs_management::LtfsError  ltfsError;
		string changerSerial = "";
		string driveSerial = "";

		vector<ltfs_management::LtfsChangerInfo>  changers;
		if(!GetHalInstance()->GetChangerList(changers, ltfsError))
		{
			SocketError("Process, error occured, GetChangerList, barcode :" << barcode_ );
			return false;
		}

		for(vector<ltfs_management::LtfsChangerInfo>::iterator iterChanger= changers.begin();
				iterChanger!= changers.end(); ++iterChanger)
		{
			changerSerial = iterChanger->mSerial;
			vector<ltfs_management::LtfsDriveInfo> drives;

			if(!GetHalInstance()->
					GetDriveList(iterChanger->mSerial, drives, ltfsError))
			{
				SocketError("Process, error occured, GetDriveList, barcode :" << barcode_ );
				return false;
			}

			for(vector<ltfs_management::LtfsDriveInfo>::iterator iterDrive=drives.begin();
					iterDrive!=drives.end(); ++iterDrive)
			{
				driveSerial = iterDrive->mSerial;
				if(barcode_ == iterDrive->mBarcode)
				{
					Labels labels;
					UInt64_t genIndex = 0;

					if(!GetHalInstance()->UnMount(changerSerial, driveSerial, genIndex, ltfsError)){
						SocketError("Process, error occured, UnMount failed, barcode :" << barcode_ );
						return false;
					}

					GetTapeBalel(changerSerial, driveSerial, labels);

					if(type_==FormatType_Format
							|| type_ == FormatType_Both)
					{
						if(!Format(changerSerial, driveSerial))
							return false;
						Int64_t freeCapacity;
						Int64_t totalCapacity;
						int ltfsFormat = (int)LTFS_UNKNOWN;
						if(!TapeLibraryMgr::Instance()->GetLoadedTapeLtfsFormat(barcode_, ltfsFormat, ltfsError)
								|| !GetTapeDbManager()->SetFormatForCartridge(barcode_, ltfsFormat)){
							SocketError("Failed to update ltfs format for tape " << barcode_ << " after formatting it.");
						}

						// mount the tape on to get the real free/used capacity of the tape
						if(!TapeLibraryMgr::Instance()->MountTape(barcode_)){
							SocketWarn("Failed to mount the tape " << barcode_
									<< " to get the real free/used capacity of the tape. Will use SCSI command to get the capacity instead.");
						}
						if(TapeLibraryMgr::Instance()->GetLoadedTapeCapacity(barcode_, freeCapacity, totalCapacity, ltfsError)){
							if( !GetTapeDbManager()->SetUsedCapacityForCartridge(barcode_, totalCapacity - freeCapacity)//0xF800000
								|| !GetTapeDbManager()->SetFreeCapacityForCartridge(barcode_, freeCapacity)
								|| !GetTapeDbManager()->SetFileCapacityForCartridge(barcode_, 0)
								|| !GetTapeDbManager()->SetFileNumberForCartridge(barcode_, 0))
							{
								SocketError( "Failed to update tape capacity : " << barcode_ );
							}
						}
						if(!TapeLibraryMgr::Instance()->UnmountTape(barcode_, false)){
							SocketWarn("Failed to unmount tape " << barcode_ << " after mounting it after formatted.");
						}

					}

					// update/write back MAMs
					if(!WriteLabel(changerSerial, driveSerial, labels))
						return false;

					return true;
				}
			}
		}

		return false;
	}

	bool
	FormatThread::Format(const string& changerSerial, const string& driveSerial)
	{
		ltfs_management::LtfsError  ltfsError;
		UInt64_t genIndex = 0;

		if(TapeLibraryMgr::Instance()->Format(barcode_, ltfsError)){
			// set backup tape barcode in MAM
			if(false == TapeLibraryMgr::Instance()->SetLoadedTapeBarcode(driveSerial, barcode_, ltfsError)){
				SocketError("Failed to set backup barcode to MAM after format the tape " << barcode_);
			}
			if(TapeLibraryMgr::Instance()->GetLoadedTapeGenerationIndex(barcode_, (Int64_t&)genIndex, ltfsError)){
				if(!GetTapeDbManager()->SetGenerationNumberForCartridge(barcode_, genIndex)){
					SocketError( "Failed to set GenerationNumber in DB : " << barcode_ );
					return false;
				}

				SocketDebug("Format success, barcode :" << barcode_ );
				return true;
			}
		}

		TapeLibraryMgr::Instance()->SetLoadedTapeFaulty(barcode_, true, ltfsError, true);
		SocketWarn("Format failed barcode :" << barcode_ );
		return false;
	}

	bool
	FormatThread::WriteLabel(const string& changerSerial, const string& driveSerial, const Labels& labels)
	{
		ltfs_management::LtfsError  ltfsError;

		if(labels.mMark&LABEL_STATUS)
		{
			SocketDebug("Now to SetLoadedTapeStatus for tape " << barcode_ << " to " << labels.mStatus << ".");
			if(!TapeLibraryMgr::Instance()->SetLoadedTapeStatus(barcode_, labels.mStatus , ltfsError, true, false))
			{
				TapeLibraryMgr::Instance()->SetLoadedTapeFaulty(barcode_, true, ltfsError, true, true);
				SocketError( "Failed to set SetLoadedTapeStatus : " << barcode_ );
				return false;
			}
		}

		if(labels.mMark&LABEL_FAULTY)
		{
			SocketDebug("Now to SetLoadedTapeFaulty "<<barcode_);
			if(!TapeLibraryMgr::Instance()->SetLoadedTapeFaulty(barcode_, labels.mFaulty, ltfsError, true, false))
			{
				TapeLibraryMgr::Instance()->SetLoadedTapeFaulty(barcode_, true, ltfsError, true, true);
				SocketError( "Failed to set SetLoadedTapeFaulty : " << barcode_ );
				return false;
			}
		}


		if(labels.mMark&LABEL_GROUP)
		{
			SocketDebug("Now to SetLoadedTapeGroup "<<barcode_ <<"  "<<labels.mGroupID);
			if(!TapeLibraryMgr::Instance()->SetLoadedTapeGroup(barcode_, labels.mGroupID,ltfsError, true, false))
			{
				TapeLibraryMgr::Instance()->SetLoadedTapeFaulty(barcode_, true, ltfsError, true, true);
				SocketError( "Failed to set SetLoadedTapeGroup : " << barcode_ );
				return false;
			}
		}

		if(labels.mMark&LABEL_DUAL_COPY)
		{
			SocketDebug("Now to SetLoadedTapeDualCopy "<<barcode_ <<"  "<<labels.mDualCopy);
			if(!TapeLibraryMgr::Instance()->SetLoadedTapeDualCopy(barcode_, labels.mDualCopy,ltfsError, true, false))
			{
				TapeLibraryMgr::Instance()->SetLoadedTapeFaulty(barcode_, true, ltfsError, true, true);
				SocketError( "Failed to set SetLoadedTapeDualCopy : " << barcode_ );
				return false;
			}
		}

		if(labels.mMark&LABEL_TAPE_ID)
		{
			SocketDebug("Now to SetLoadedTapeUUID "<<barcode_ <<"  "<<labels.mTapeID);
			if(!TapeLibraryMgr::Instance()->SetLoadedTapeUUID(barcode_, labels.mTapeID,ltfsError, true, false))
			{
				TapeLibraryMgr::Instance()->SetLoadedTapeFaulty(barcode_, true, ltfsError, true, true);
				SocketError( "Failed to set SetLoadedTapeDualCopy : " << barcode_ );
				return false;
			}
		}

		return true;
	}

	bool
	FormatThread::GetTapeType(const string& changerSerial, bool& lto5)
	{
		bool ltfs5 = true;
		ltfs_management::LtfsError  ltfsError;

		vector<ltfs_management::TapeInfo> tapes;
		if(!ltfs_management::TapeLibraryMgr::Instance()->
				GetTapeListForChanger(changerSerial, tapes, ltfsError))
		{
			SocketError("Process, error occured, GetTapeList, barcode :" << barcode_ );
			return false;
		}
		for(vector<ltfs_management::TapeInfo>::iterator iterTape=tapes.begin();
				iterTape!=tapes.end(); ++iterTape)
		{
			if(barcode_ == iterTape->mBarcode)
			{
				switch(iterTape->mMediaType)
				{
				case MEDIA_LTO6:
					ltfs5 = false;
					break;
				}
				break;
			}
		}
		return true;
	}

	void
	FormatThread::GetTapeBalel(const string& changerSerial, const string& driveSerial, Labels& labels)
	{
		ltfs_management::LtfsError  ltfsError;

		labels.mMark = 0;

		if(TapeLibraryMgr::Instance()->GetLoadedTapeStatus(barcode_, labels.mStatus, ltfsError))
			labels.mMark |= LABEL_STATUS;

		if(TapeLibraryMgr::Instance()->GetLoadedTapeFaulty(barcode_, labels.mFaulty, ltfsError))
			labels.mMark |= LABEL_FAULTY;

		if(TapeLibraryMgr::Instance()->GetLoadedTapeGroup(barcode_, labels.mGroupID, ltfsError))
			labels.mMark |= LABEL_GROUP;

		if(TapeLibraryMgr::Instance()->GetLoadedTapeDualCopy(barcode_, labels.mDualCopy, ltfsError))
			labels.mMark |= LABEL_DUAL_COPY;

		if(TapeLibraryMgr::Instance()->GetLoadedTapeUUID(barcode_, labels.mTapeID, ltfsError))
			labels.mMark |= LABEL_TAPE_ID;

		if(labelMark_&LABEL_STATUS)
		{
			labels.mStatus = labelStatus_;
			labels.mMark |= LABEL_STATUS;
		}

		if(labelMark_&LABEL_FAULTY)
		{
			labels.mFaulty = labelFaulty_;
			labels.mMark |= LABEL_FAULTY;
		}

		if(labelMark_&LABEL_GROUP)
		{
			labels.mGroupID = labelGroupID_;
			labels.mMark |= LABEL_GROUP;
		}

		if(labelMark_&LABEL_DUAL_COPY)
		{
			labels.mDualCopy = labelDualCopy_;
			labels.mMark |= LABEL_DUAL_COPY;
		}

		if(labelMark_&LABEL_TAPE_ID)
		{
			labels.mTapeID = labelTapeId_;
			labels.mMark |= LABEL_TAPE_ID;
		}


	}

	void
	FormatThread::GetLTOTapeDefaultFormatSize(int tapeType, long long &freeCapacity, long long &usedCapacity)
	{
		string tapeVendor;

		freeCapacity = 0;
		usedCapacity = 0;

		//read vsersion file to get vendor info
		int ret = CONF_PARSER_SIMPLE_INIT(COMM_VERSION_PATH.c_str());
		if (ret != 0)
		{
			SocketError("No vendor information found");
			return ;
		}

		tapeVendor = CONF_PARSER_GET_VAL("", "VENDOR");

		if (tapeVendor != "") {
			switch(tapeType)
			{
				case MEDIA_LTO6:
					if (tapeVendor == "IBM")
						freeCapacity = 2408088338432;
					else if (tapeVendor == "HP")
						freeCapacity = LTFS6_DEFAULT_SIZE;
					break;
				case MEDIA_LTO5:
					if (tapeVendor == "IBM"){
						freeCapacity = 1424990142464;
						usedCapacity = 9437184;
						}
					else if (tapeVendor == "HP")
						freeCapacity = 1453434339328;
					break;
				default:
					break;
			}

			SocketDebug("LTO Tape type:" << tapeVendor << "size:" << freeCapacity);
		}

		return ;
	}
} /* namespace ltfs_management */
