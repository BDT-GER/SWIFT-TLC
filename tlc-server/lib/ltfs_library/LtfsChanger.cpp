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
 * LtfsChanger.cpp
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "LtfsChanger.h"

namespace ltfs_management
{

	LtfsChanger::LtfsChanger(const ScsiInfo& info):
		LtfsScsiDevice(), mapMutex_(new boost::shared_mutex())
	{
		// TODO Auto-generated constructor stub
		scsiAddr_ 	= info.mScsiAddr;
		vendor_ 	= info.mVendor;
		product_ 	= info.mProduct;
		version_ 	= info.mVersion;
		stDev_ 		= info.mStDev;
		sgDev_ 		= info.mSgDev;

		status_		= CHANGER_STATUS_UNKNOWN;

		driveStart_ = 0;
		mailSlotStart_ = 0;
		slotStart_ = 0;
		lastMailSlotNum_ = -1;
		hwMode_ = CHANGER_HW_READY;

		autoCleanMode_ = false;
		moveMutex_ = new boost::timed_mutex();
	}

	LtfsChanger::~LtfsChanger()
	{
		// TODO Auto-generated destructor stub
	}

	void
	LtfsChanger::SetMissing(bool missing)
	{
		missing_ = missing;
		if(missing){
			status_ = (int)CHANGER_STATUS_DISCONNECTED;
			CheckHwMode(true);
		}
	}

	bool
	LtfsChanger::GetMissing()
	{
		return missing_;
	}

	bool
	LtfsChanger::FindDriveBySerial(const string& serial, LtfsDriveMap::iterator& iter)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		for(iter=drives_.begin();
				iter!=drives_.end(); ++iter)
		{
			if(iter->second.GetSerial() == serial)
				return true;
		}

		return false;
	}
    bool LtfsChanger::RequestMoveLock(int millTimeOut)
    {
        boost::system_time timeOut = boost::get_system_time() + boost::posix_time::milliseconds(millTimeOut);
        if(!moveMutex_->timed_lock(timeOut)){
        	LtfsLgError("RequestMoveLock failed, changer: " << serial_ << ", millTimeOut = " << millTimeOut);
        	return false;
        }

        LtfsLgDebug("RequestMoveLock finished, changer: " << serial_ << ", millTimeOut = " << millTimeOut);
        return true;
    }

    void LtfsChanger::ReleaseMoveLock()
    {
    	moveMutex_->unlock();
    	LtfsLgDebug("ReleaseMoveLock finished, changer: " << serial_ << ".");
    }

	void
	LtfsChanger::MoveTape(int srcSlot, int dstSlot)
	{
		boost::unique_lock<boost::shared_mutex> lock(*mapMutex_);
		LtfsTape * tape = NULL;
		do
		{
			LtfsDriveMap::iterator iterDrive = drives_.find(srcSlot);
			if(iterDrive!=drives_.end())
			{
				tape = iterDrive->second.GetTape();
				iterDrive->second.SetTape(NULL);
				break;
			}

			LtfsSlotMap::iterator iterSlot = slots_.find(srcSlot);
			if(iterSlot!=slots_.end())
			{
				tape = iterSlot->second.GetTape();
				iterSlot->second.SetTape(NULL);
				break;
			}

			LtfsMailSlotMap::iterator iterMailSlot = mailslots_.find(srcSlot);
			if(iterMailSlot!=mailslots_.end())
			{
				tape = iterMailSlot->second.GetTape();
				iterMailSlot->second.SetTape(NULL);
				break;
			}
		}while(false);

		do
		{
			LtfsDriveMap::iterator iterDrive = drives_.find(dstSlot);
			if(iterDrive!=drives_.end())
			{
				iterDrive->second.SetTape(tape);
				break;
			}

			LtfsSlotMap::iterator iterSlot = slots_.find(dstSlot);
			if(iterSlot!=slots_.end())
			{
				iterSlot->second.SetTape(tape);
				break;
			}

			LtfsMailSlotMap::iterator iterMailSlot = mailslots_.find(dstSlot);
			if(iterMailSlot!=mailslots_.end())
			{
				iterMailSlot->second.SetTape(tape);
				break;
			}
		}while(false);
	}

	void
	LtfsChanger::GetChangerInfo(LtfsChangerInfo& changer)
	{
		changer.mSerial = serial_;
		changer.mProduct = product_;
		changer.mVendor = vendor_;
		changer.mVersion = version_;
		changer.mStatus = status_;
		changer.mDriveStart = driveStart_;
		changer.mMailSlotStart = mailSlotStart_;
		changer.mSlotStart = slotStart_;
		changer.mAutoCleanMode = autoCleanMode_;
		changer.mHwMode = hwMode_;
		LtfsLgDebug("GetChangerInfo: changer.mHwMode = " << changer.mHwMode);
	}

#ifdef SIMULATOR
	bool
	LtfsChanger::RefreshSlot(vector<SlotDetail>& slots, LtfsError& ltfsErr)
	{
		int start = 65530;
		for(vector<SlotDetail>::iterator iter=slots.begin();
				iter!=slots.end(); ++iter)
		{
			LtfsTape* tape = NULL;
			LtfsSlotMap::iterator itr = slots_.find(iter->m_SlotID);

			if(!iter->m_Barcode.empty())
			{
				LtfsTapeMap::iterator iterTape = tapes_.find(iter->m_Barcode);
				if(iterTape != tapes_.end())
					tape = iterTape->second;
			}
			else
			{
				if(!iter->m_TapeKey.empty())
				{
					tape = &LtfsTape::NoBarcodeTape;
				}
			}

			if(itr == slots_.end())
			{
				LtfsSlot slot(iter->m_SlotID, iter->m_LogicSlotId);

				slot.SetTape(tape);

				slots_.insert(LtfsSlotMap::value_type(iter->m_SlotID, slot));
			}
			else
			{
#if 0
				if(iter->m_TapeKey.empty()
						|| itr->second.GetBarcode() != iter->m_Barcode
						|| tape != NULL)
#else
				if(tape != itr->second.GetTape())
#endif
				{
					itr->second.SetTape(tape);
				}
			}

			if(start>iter->m_SlotID)
				start = iter->m_SlotID;
		}
		if(slots.size()>0)
			slotStart_ = start;

		return true;
	}
#else
	bool
	LtfsChanger::RefreshSlot(vector<slotElement>& slots, LtfsError& ltfsErr)
	{
		for(vector<slotElement>::iterator iter=slots.begin();
				iter!=slots.end(); ++iter)
		{
			LtfsTape* tape = NULL;
			LtfsSlotMap::iterator itr = slots_.find(iter->mSlotID);

			if(!iter->mBarcode.empty())
			{
				LtfsTapeMap::iterator iterTape = tapes_.find(iter->mBarcode);
				if(iterTape != tapes_.end())
					tape = iterTape->second;
			}
			else
			{
				if(!iter->mIsEmpty)
				{
					tape = &LtfsTape::NoBarcodeTape;
				}
			}

			if(itr == slots_.end())
			{
				LtfsSlot slot(iter->mSlotID, iter->mLogicSlotID);

				slot.SetTape(tape);

				slots_.insert(LtfsSlotMap::value_type(iter->mSlotID, slot));
			}
			else
			{
#if 0
				if(iter->mIsEmpty
						|| itr->second.GetBarcode() != iter->mBarcode
						|| tape != NULL)
#else
				if(tape != itr->second.GetTape())
#endif
				{
					itr->second.SetTape(tape);
				}
			}
		}

		return true;
	}
#endif

#ifdef SIMULATOR
	bool
	LtfsChanger::RefreshMailSlot(vector<MailSlotDetail>& mailSlots, LtfsError& ltfsErr)
	{
		int start = 65530;
		for(vector<MailSlotDetail>::iterator iter=mailSlots.begin();
				iter!=mailSlots.end(); ++iter)
		{
			LtfsTape* tape = NULL;
			LtfsMailSlotMap::iterator itr = mailslots_.find(iter->m_SlotID);

			if(!iter->m_Barcode.empty())
			{
				LtfsTapeMap::iterator iterTape = tapes_.find(iter->m_Barcode);
				if(iterTape != tapes_.end())
					tape = iterTape->second;
			}
			else
			{
				if(!iter->m_TapeKey.empty())
				{
					tape = &LtfsTape::NoBarcodeTape;
				}
			}

			if(itr == mailslots_.end())
			{
				LtfsMailSlot mailslot(iter->m_SlotID, iter->m_LogicSlotId, false);

				mailslot.SetTape(tape);

				mailslots_.insert(LtfsMailSlotMap::value_type(iter->m_SlotID, mailslot));
			}
			else
			{
#if 0
				if(iter->m_TapeKey.empty()
						|| itr->second.GetBarcode() != iter->m_Barcode
						|| tape != NULL)
#else
				if(tape != itr->second.GetTape())
#endif
				{
					itr->second.SetTape(tape);
				}
			}

			if(start>iter->m_SlotID)
				start = iter->m_SlotID;
		}
		if(mailSlots.size()>0)
			mailSlotStart_ = start;

		return true;
	}
#else
	bool
	LtfsChanger::RefreshMailSlot(vector<mailSlotElement>& mailSlots, LtfsError& ltfsErr)
	{
		for(vector<mailSlotElement>::iterator iter=mailSlots.begin();
				iter!=mailSlots.end(); ++iter)
		{
			LtfsTape* tape = NULL;
			LtfsMailSlotMap::iterator itr = mailslots_.find(iter->mSlotID);

			if(!iter->mBarcode.empty())
			{
				LtfsTapeMap::iterator iterTape = tapes_.find(iter->mBarcode);
				if(iterTape != tapes_.end())
					tape = iterTape->second;
			}
			else
			{
				if(!iter->mIsEmpty)
				{
					tape = &LtfsTape::NoBarcodeTape;
				}
			}

			if(itr == mailslots_.end())
			{
				LtfsMailSlot mailslot(iter->mSlotID, iter->mLogicSlotID, iter->mIsOpen);

				mailslot.SetTape(tape);

				mailslots_.insert(LtfsMailSlotMap::value_type(iter->mSlotID, mailslot));
			}
			else
			{
#if 0
				if(iter->mIsEmpty
						|| itr->second.GetBarcode() != iter->mBarcode
						|| tape != NULL)
#else
				if(tape != itr->second.GetTape())
#endif
				{
					itr->second.SetTape(tape);
				}
				itr->second.SetMailSlotOpened(iter->mIsOpen);
			}
		}

		return true;
	}
#endif

	bool LtfsChanger::PreventAllowMediaRemoval(bool bPrevent)
	{
#ifdef SIMULATOR
		return true;
#else
		return ChangerPreventMediaRemoval(sgDev_, bPrevent);
#endif
	}

#ifdef SIMULATOR
	bool
	LtfsChanger::RefreshDrive(vector<DriveDetail>& drives, LtfsError& ltfsErr)
	{
		int start = 65530;

		for(LtfsDriveMap::iterator itr=drives_.begin();
				itr!=drives_.end(); ++itr)
		{
			bool find = false;
			for(vector<DriveDetail>::iterator iter=drives.begin();
					iter!=drives.end(); ++iter)
			{
				if(iter->m_SlotId == itr->second.GetSlotID())
				{
					find = true;
					break;
				}
			}

			if(!find)
			{
				//mark missing
				LtfsLgWarn("Refresh changer missing, ScsiAddr:"<<itr->second.GetScsiAddr());
				itr->second.SetMissing(true);
			}
		}

		for(vector<DriveDetail>::iterator iter=drives.begin();
				iter!=drives.end(); ++iter)
		{
			LtfsTape* tape = NULL;
			LtfsDriveMap::iterator itr = drives_.find(iter->m_SlotId);

			if(!iter->m_Barcode.empty())
			{
				LtfsTapeMap::iterator iterTape = tapes_.find(iter->m_Barcode);
				if(iterTape != tapes_.end())
					tape = iterTape->second;
			}
			else
			{
				if(iter->m_TapeKey != "")
				{
					tape = &LtfsTape::NoBarcodeTape;
				}
			}

			if(itr==drives_.end())
			{
				boost::unique_lock<boost::shared_mutex> lock(*mapMutex_);

				LtfsDrive drive(iter->m_SlotId, iter->m_LogicSlotId, iter->m_Serial, false, iter->m_Generation, "", "", "", "");
				drive.SetTape(tape);
				if(drive.Refresh())
					drives_.insert(LtfsDriveMap::value_type(iter->m_SlotId, drive));
			}
			else
			{
				itr->second.SetMissing(false);
				itr->second.Refresh();
#if 0
				if(iter->m_TapeKey.empty()
						|| itr->second.GetBarcode() != iter->m_Barcode
						|| tape != NULL)
#else
				if(tape != itr->second.GetTape())
#endif
				{
					itr->second.SetTape(tape);
				}
			}

			if(start>iter->m_SlotId)
				start = iter->m_SlotId;
		}

		if(drives.size()>0)
			driveStart_ = start;

		return true;
	}
#else
	bool
	LtfsChanger::RefreshDrive(vector<driveElement>& drives, LtfsError& ltfsErr)
	{

		for(LtfsDriveMap::iterator itr=drives_.begin();
				itr!=drives_.end(); ++itr)
		{
			bool find = false;
			for(vector<driveElement>::iterator iter=drives.begin();
					iter!=drives.end(); ++iter)
			{
				if(iter->mSlotID == itr->second.GetSlotID())
				{
					find = true;
					break;
				}
			}

			if(!find)
			{
				//mark missing
				LtfsLgWarn("Refresh changer missing, ScsiAddr:"<<itr->second.GetScsiAddr());
				itr->second.SetMissing(true);
			}
		}

		for(vector<driveElement>::iterator iter=drives.begin();
				iter!=drives.end(); ++iter)
		{
			LtfsTape* tape = NULL;
			LtfsDriveMap::iterator itr = drives_.find(iter->mSlotID);

			if(!iter->mBarcode.empty())
			{
				LtfsTapeMap::iterator iterTape = tapes_.find(iter->mBarcode);
				if(iterTape != tapes_.end())
					tape = iterTape->second;
			}
			else
			{
				if(!iter->mIsEmpty)
				{
					tape = &LtfsTape::NoBarcodeTape;
				}
			}

			if(itr==drives_.end())
			{
				LtfsDrive drive(iter->mSlotID, iter->mLogicSlotID, iter->mSerial, iter->mIsFullHight, iter->mGeneration,
						iter->mScsiInfo.vendor,iter->mScsiInfo.product, iter->mScsiInfo.stDev, iter->mScsiInfo.sgDev);
				drive.SetTape(tape);
				if(drive.Refresh())
				{
					boost::unique_lock<boost::shared_mutex> lock(*mapMutex_);
					drives_.insert(LtfsDriveMap::value_type(iter->mSlotID, drive));
				}
			}
			else
			{
				itr->second.SetMissing(false);
				itr->second.Refresh();
#if 0
				if(iter->mIsEmpty
						|| itr->second.GetBarcode() != iter->mBarcode
						|| tape != NULL)
#else
				if(tape != itr->second.GetTape())
#endif
				{
					itr->second.SetTape(tape);
				}
			}
		}

		return true;
	}
#endif

#ifdef SIMULATOR
	bool
	LtfsChanger::RefreshTape(vector<TapeDetail>& tapes, LtfsError& ltfsErr)
	{
#if 1
		vector<string> missingTape;

		for(vector<TapeDetail>::iterator iter=tapes.begin();
				iter!=tapes.end(); ++iter)
		{
			if(iter->m_Barcode.empty())
				continue;

			LtfsTapeMap::iterator itr = tapes_.find(iter->m_Barcode);
			if(itr == tapes_.end())
			{
				boost::unique_lock<boost::shared_mutex> lock(*mapMutex_);

				LtfsTape* tape = new LtfsTape(iter->m_Barcode, iter->m_MediumType);
				tapes_.insert(LtfsTapeMap::value_type(iter->m_Barcode, tape));
			}
		}

		//deal with missing
		for(LtfsTapeMap::iterator itr=tapes_.begin();
				itr!=tapes_.end(); ++itr)
		{
			bool find = false;
			for(vector<TapeDetail>::iterator iter=tapes.begin();
					iter!=tapes.end(); ++iter)
			{
				if(itr->second->GetBarcode() == iter->m_Barcode)
				{
					find = true;
					break;
				}
			}

			if(!find)
			{
				LtfsLgWarn("RefreshTape tape missing, barcode:"<<itr->second->GetBarcode());
				missingTape.push_back(itr->second->GetBarcode());
			}
		}

		//delete missing tape
		for(vector<string>::iterator iterMiss = missingTape.begin();
				iterMiss!=missingTape.end(); ++iterMiss)
		{
			LtfsTapeMap::iterator iterTape = tapes_.find(*iterMiss);
			if(iterTape != tapes_.end())
			{
				LtfsTape* tape = iterTape->second;
				LtfsLgWarn("RefreshTape tape missing, barcode:"<<iterTape->second->GetBarcode());
				delete tape;
				LtfsLgWarn("release missing tape ");
				tapes_.erase(iterTape);
				LtfsLgWarn("deleted missing tape from map");
			}
		}
#else
		for(vector<TapeDetail>::iterator iter=tapes.begin();
				iter!=tapes.end(); ++iter)
		{
			if(iter->m_Barcode.empty())
				continue;

			LtfsTapeMap::iterator itr = tapes_.find(iter->m_Barcode);
			if(itr == tapes_.end())
			{
				boost::unique_lock<boost::shared_mutex> lock(*mapMutex_);

				LtfsTape* tape = new LtfsTape(iter->m_Barcode, iter->m_MediumType);
				tapes_.insert(LtfsTapeMap::value_type(iter->m_Barcode, tape));
			}
		}

		//deal with missing
		for(LtfsTapeMap::iterator itr=tapes_.begin();
				itr!=tapes_.end(); )
		{
			bool find = false;
			for(vector<TapeDetail>::iterator iter=tapes.begin();
					iter!=tapes.end(); ++iter)
			{
				if(itr->second->GetBarcode() == iter->m_Barcode)
				{
					find = true;
					break;
				}
			}

			if(!find)
			{
				LtfsLgWarn("RefreshTape tape missing, barcode:"<<itr->second->GetBarcode());
				LtfsTape* tape = itr->second;
				tapes_.erase(itr++);
				delete tape;
			}
			else
			{
				++itr;
			}
		}

#endif

		return true;
	}

#else
	bool
	LtfsChanger::RefreshTape(vector<tapeElement>& tapes, LtfsError& ltfsErr)
	{
#if 1
		vector<string> missingTape;

		for(vector<tapeElement>::iterator iter=tapes.begin();
				iter!=tapes.end(); ++iter)
		{
			if(iter->mBarcode.empty())
				continue;

			LtfsTapeMap::iterator itr = tapes_.find(iter->mBarcode);
			if(itr == tapes_.end())
			{
				boost::unique_lock<boost::shared_mutex> lock(*mapMutex_);

				LtfsTape* tape = new LtfsTape(iter->mBarcode, iter->mMediumType);
				LtfsLgDebug("RefreshTape insert tape barcode:"<<iter->mBarcode <<" mMediumType:" <<iter->mMediumType);
				tapes_.insert(LtfsTapeMap::value_type(iter->mBarcode, tape));
			}
		}

		//deal with missing
		for(LtfsTapeMap::iterator itr=tapes_.begin();
				itr!=tapes_.end(); ++itr)
		{
			bool find = false;
			for(vector<tapeElement>::iterator iter=tapes.begin();
					iter!=tapes.end(); ++iter)
			{
				if(itr->second->GetBarcode() == iter->mBarcode)
				{
					find = true;
					break;
				}
			}

			if(!find)
			{
				LtfsLgWarn("RefreshTape tape missing, barcode:"<<itr->second->GetBarcode());
				missingTape.push_back(itr->second->GetBarcode());
			}
		}

		//delete missing tape
		for(vector<string>::iterator iterMiss = missingTape.begin();
				iterMiss!=missingTape.end(); ++iterMiss)
		{
			LtfsTapeMap::iterator iterTape = tapes_.find(*iterMiss);
			if(iterTape != tapes_.end())
			{
				LtfsTape* tape = iterTape->second;
				LtfsLgWarn("RefreshTape tape missing, barcode:"<<iterTape->second->GetBarcode());
				delete tape;
				LtfsLgWarn("release missing tape ");
				tapes_.erase(iterTape);
				LtfsLgWarn("deleted missing tape from map");
			}
		}
#else
		//deal with missing
		for(LtfsTapeMap::iterator itr=tapes_.begin();
				itr!=tapes_.end(); )
		{
			bool find = false;
			for(vector<tapeElement>::iterator iter=tapes.begin();
					iter!=tapes.end(); ++iter)
			{
				if(itr->second->GetBarcode() == iter->mBarcode)
				{
					find = true;
					break;
				}
			}

			if(!find)
			{
				LtfsLgWarn("RefreshTape tape missing, barcode:"<<itr->second->GetBarcode());
				LtfsTape* tape = itr->second;

				delete tape;
				tapes_.erase(itr++);
			}
			else
			{
				++itr;
			}
		}
#endif
		return true;
	}
#endif

	bool
	LtfsChanger::Refresh(bool bRefreshDrive, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
#ifdef SIMULATOR
		int errID = 0;
		ChangerDetail detail;
		vector<SlotDetail> slots;
		vector<MailSlotDetail> mailSlots;
		vector<DriveDetail> drives;
		vector<TapeDetail> tapes;

		if(!Simulator::Instance()->GetChangerInfo(scsiAddr_, detail))
		{
			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("Simulator::Instance()->GetChangerInfo failed");
			return false;
		}

		serial_ = scsiAddr_;
		status_ = detail.m_Status;

		if(!Simulator::Instance()->GetDriveList(serial_, drives, errID)
				|| !Simulator::Instance()->GetSlotList(serial_, slots, errID)
				|| !Simulator::Instance()->GetMailSlotList(serial_, mailSlots, errID)
				|| !Simulator::Instance()->GetTapeList(serial_, tapes, errID))
		{
			ltfsErr.SetErrCode(errID);
			LtfsLgError("GetDriveList .... GetTapeList  failed, serial , "<<serial_);
			return false;
		}

		if(!RefreshTape(tapes, ltfsErr))
		{
			LtfsLgError("RefreshTape  failed, serial "<<serial_);
			return false;
		}

		if(!RefreshSlot(slots, ltfsErr))
		{
			LtfsLgError("RefreshSlot  failed, serial "<<serial_);
			return false;
		}

		if(!RefreshMailSlot(mailSlots, ltfsErr))
		{
			LtfsLgError("RefreshMailSlot  failed, serial "<<serial_);
			return false;
		}

		if(!RefreshDrive(drives, ltfsErr))
		{
			LtfsLgError("RefreshDrive  failed, serial "<<serial_);
			return false;
		}

#else
		ChangerInfo_ cinfo;
		LSSCSI_INFO linfo;
		linfo.scsiAddr = scsiAddr_;
		linfo.vendor = vendor_;
		linfo.product = product_;
		linfo.version = version_;
		linfo.stDev = stDev_;
		linfo.sgDev = sgDev_;

		if(!GetChanger(linfo, cinfo, ltfsErr))
		{
			LtfsLgError("GetChanger failed : " << ltfsErr.GetErrMsg());
			return false;
		}
		//DebugPrintChanger(cinfo);

		serial_ 		= cinfo.mSerial;
		status_ 		= cinfo.mStatus;
		driveStart_		= cinfo.mDriveStart;
		mailSlotStart_	= cinfo.mMailSlotStart;
		slotStart_		= cinfo.mSlotStart;
		autoCleanMode_	= cinfo.mAutoCleanMode;

		if(!RefreshTape(cinfo.tapes, ltfsErr))
		{
			LtfsLgError("RefreshTape  failed, serial "<<serial_);
			return false;
		}

		if(!RefreshSlot(cinfo.slots, ltfsErr))
		{
			LtfsLgError("RefreshSlot  failed, serial "<<serial_);
			return false;
		}

		// check if we need to fire event for mail slot number change
		if(cinfo.mailSlots.size() <= 0){
			if(lastMailSlotNum_ != 0){
				//LtfsEvent(EVENT_LEVEL_WARNING, "TODO:", "Inventory finished. No mail slot found.");
			}
		}
		lastMailSlotNum_ = cinfo.mailSlots.size();

		if(!RefreshMailSlot(cinfo.mailSlots, ltfsErr))
		{
			LtfsLgError("RefreshMailSlot  failed, serial "<<serial_);
			return false;
		}

		if(true == bRefreshDrive){
			if(!RefreshDrive(cinfo.drives, ltfsErr))
			{
				LtfsLgError("RefreshDrive  failed, serial "<<serial_);
				return false;
			}
		}

#endif
		CheckHwMode(true);

		return true;
	}

	void LtfsChanger::SetLsSCSIInfo(ScsiInfo scsiInfo)
	{
		scsiAddr_ = scsiInfo.mScsiAddr;
		stDev_ = scsiInfo.mStDev;
		sgDev_ = scsiInfo.mSgDev;
	}

	ChangerHwMode LtfsChanger::GetHwMode(bool bRefresh)
	{
		if(bRefresh){
			CheckHwMode(true);
		}

		LtfsLgDebug("GetHwMode: hwMode = " << hwMode_);
		return hwMode_;
	}

	void LtfsChanger::CheckHwMode(bool bEvent)
	{
		ChangerHwMode newMode = RefreshHwMode();
		ChangerHwMode orgMode = hwMode_;
		hwMode_ = newMode;

		LtfsLgDebug("DDEBUG: CheckHwMode: orgMode = " << orgMode << ", hwMode_ = " << hwMode_);
		if(orgMode == hwMode_){
			return;
		}

		if(!bEvent){
			return;
		}

		LtfsLgEvent(EVENT_LEVEL_INFO, "Changer_HWMode_Changed", "Changer " << GetSerial() << " HW mode changed.");

		if(hwMode_ == CHANGER_HW_READY){
			//LtfsLgEvent(EVENT_LEVEL_INFO, "Lib_Conn", "Library " << serial_ << " connected.");
		}else if(hwMode_ == CHANGER_HW_DISSCONNECTED){
			//LtfsLgEvent(EVENT_LEVEL_CRITICAL, "Lib_Disconn", "Library " << serial_ << " disconnected.");
		}else if(hwMode_ == CHANGER_HW_ROBOTIC_BLOCKED){
			//LtfsLgEvent(EVENT_LEVEL_ERR, "Lib_Robotic_Blocked", "Changer " << GetSerial() << " robotic is blocked from moving.");
		}else if(hwMode_ == CHANGER_HW_SCSI_HANG){
			//LtfsLgEvent(EVENT_LEVEL_ERR, "TODO:Changer_SCSI_Hang", "Changer " << GetSerial() << " didn't  response to any SCSI command.");
		}else if(hwMode_ == CHANGER_HW_NO_USABLE_DRIVE){
			//LtfsLgEvent(EVENT_LEVEL_ERR, "TODO:Changer_No_Drive", "Changer " << GetSerial() << " has no usable drive.");
		}else if(hwMode_ == CHANGER_HW_TOO_MANY_TAPES){
			//LtfsLgEvent(EVENT_LEVEL_ERR, "TODO:Changer_Too_Many_Tapes", "Changer " << GetSerial() << " has too many tapes.");
		}else{
			LtfsLgError("Hardware mode for changer " << GetSerial() << " is not correct. Hardware mode: " << hwMode_);
		}
	}

	ChangerHwMode LtfsChanger::RefreshHwMode()
	{
		// check if the changer status is correct
		if(status_ == (int)CHANGER_STATUS_DISCONNECTED){
			return CHANGER_HW_DISSCONNECTED;
		}

		//TODO: to check if a changer's robotic is blocked

		//TODO: to check if a changer has SCSI hang problem

		// check if any available drives
		bool bDriveOk = false;
		for(LtfsDriveMap::iterator it = drives_.begin(); it != drives_.end(); it++){
			if(it->second.IsAvailable()){
				bDriveOk = true;
			}
		}
		if(!bDriveOk){
			return CHANGER_HW_NO_USABLE_DRIVE;
		}

		int tapeNum = GetTapeNum(true, false);
		int slotNum = GetSlotNum();

		if(tapeNum > slotNum){
			return CHANGER_HW_TOO_MANY_TAPES;
		}

		return CHANGER_HW_READY;
	}

	bool
	LtfsChanger::PhysicalSlotToLogicSlot(int phySlot, int& logSlot, ENUM_SLOT_TYPE& slotType, LtfsError& ltfsErr)
	{
		int mailSlotNum = mailslots_.size();

		// mail slot
		for(LtfsMailSlotMap::iterator iter = mailslots_.begin(); iter != mailslots_.end(); ++iter){
			if(iter->second.slotID_ == phySlot){
				logSlot = phySlot - mailSlotStart_ + 1;
				slotType = SLOT_MAIL_SLOT;
				return true;
			}
		}

		// storage slot
		for(LtfsSlotMap::iterator iter = slots_.begin(); iter != slots_.end(); ++iter){
			if(iter->second.slotID_ == phySlot){
				logSlot = phySlot - slotStart_ + mailSlotNum + 1;
				slotType = SLOT_STORAGE;
				return true;
			}
		}

		// drive
		for(LtfsDriveMap::iterator iter = drives_.begin(); iter != drives_.end(); ++iter){
			if(iter->second.slotID_ == phySlot){
				logSlot = phySlot - driveStart_ + 1;
				slotType = SLOT_DRIVE;
				return true;
			}
		}

		return false;
	}

	bool
	LtfsChanger::LogicSlotToPhysicalSlot(int logSlot, int& phySlot, ENUM_SLOT_TYPE slotType, LtfsError& ltfsErr)
	{
		int mailSlotNum = mailslots_.size();

		// mail slot
		for(LtfsMailSlotMap::iterator iter = mailslots_.begin(); SLOT_MAIL_SLOT == slotType && iter != mailslots_.end(); ++iter){
			if(iter->second.logicSlotID_ == logSlot){
				phySlot = logSlot + mailSlotStart_ - 1;
				return true;
			}
		}

		// storage slot
		for(LtfsSlotMap::iterator iter = slots_.begin(); SLOT_STORAGE == slotType && iter != slots_.end(); ++iter){
			if(iter->second.logicSlotID_ == logSlot){
				phySlot = logSlot + slotStart_ - mailSlotNum - 1;
				return true;
			}
		}

		// drive
		for(LtfsDriveMap::iterator iter = drives_.begin(); SLOT_DRIVE == slotType && iter != drives_.end(); ++iter){
			if(iter->second.logicSlotID_ == logSlot){
				phySlot = logSlot + driveStart_ - 1;
				return true;
			}
		}

		return false;
	}

	bool
	LtfsChanger::GetDriveList(vector<LtfsDriveInfo>& drives, LtfsError& ltfsErr)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		drives.clear();

		for(LtfsDriveMap::iterator iter=drives_.begin();
				iter!=drives_.end(); ++iter)
		{
			LtfsDriveInfo drive;
			if(!iter->second.GetMissing())
			{
				iter->second.GetDriveInfo(drive);
				//TODO: if the changer is disconnected, the dirve should be set to disconnected???
				if(status_ == CHANGER_STATUS_DISCONNECTED){
					//drive.mStatus = DRIVE_STATUS_DISCONNECTED;
				}

				drives.push_back(drive);
			}
		}
		LtfsLgDebug("GetDriveList  "<<drives.size());
		return true;
	}

	bool
	LtfsChanger::GetTapeList(vector<LtfsTapeInfo>& tapes, LtfsError& ltfsErr)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);
		string barcode;

		tapes.clear();

		for(LtfsDriveMap::iterator iter=drives_.begin();
				iter!=drives_.end(); ++iter)
		{
			barcode = iter->second.GetBarcode();
			if(!iter->second.GetMissing()&&!barcode.empty())
			{
				LtfsTapeInfo tape;
				iter->second.GetTapeInfo(tape);
				tapes.push_back(tape);
			}
		}

		LtfsLgDebug("LtfsChanger::GetTapeList  tape in drive  "<<tapes.size());

		for(LtfsSlotMap::iterator iter=slots_.begin();
				iter!=slots_.end(); ++iter)
		{
			barcode = iter->second.GetBarcode();
			if(!barcode.empty())
			{
				LtfsTapeInfo tape;
				iter->second.GetTapeInfo(tape);
				tapes.push_back(tape);
			}
		}

		LtfsLgDebug("LtfsChanger::GetTapeList  tape in slot  "<<tapes.size());

		for(LtfsMailSlotMap::iterator iter=mailslots_.begin();
				iter!=mailslots_.end(); ++iter)
		{
			barcode = iter->second.GetBarcode();
			if(!barcode.empty())
			{
				LtfsTapeInfo tape;
				iter->second.GetTapeInfo(tape);
				tapes.push_back(tape);
			}
		}

		LtfsLgDebug("GetTapeList  "<<tapes.size());

		return true;
	}

	bool
	LtfsChanger::GetSlotList(vector<LtfsSlotInfo>& slots, LtfsError& ltfsErr)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		slots.clear();

		for(LtfsSlotMap::iterator iter=slots_.begin();
				iter!=slots_.end(); ++iter)
		{
			LtfsSlotInfo slot;
			iter->second.GetSlotInfo(slot);

			slots.push_back(slot);
		}

		LtfsLgDebug("GetSlotList  "<<slots.size());

		return true;
	}

	bool
	LtfsChanger::GetMailSlotList(vector<LtfsMailSlotInfo>& mailslots, LtfsError& ltfsErr)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		mailslots.clear();

		for(LtfsMailSlotMap::iterator iter=mailslots_.begin();
				iter!=mailslots_.end(); ++iter)
		{
			LtfsMailSlotInfo mailSlot;
			iter->second.GetMailSlotInfo(mailSlot);

			mailslots.push_back(mailSlot);
		}

		LtfsLgDebug("GetMailSlotList  "<<mailslots.size());

		return true;
	}

	bool
	LtfsChanger::MoveCartridge(int srcSlot, int dstSlot, LtfsError& ltfsErr)
	{
		if(1){
			boost::mutex muSrc;
			boost::mutex muDst;
			boost::mutex* srcLock = GetDriveLockBySlotId(srcSlot);
			boost::mutex* dstLock = GetDriveLockBySlotId(dstSlot);
			LtfsLgDebug("srcSlot = " << srcSlot << ", srcLock = " << srcLock << ". dstSlot = " << dstSlot << ", dstLock = " << dstLock << ".");
			if(srcLock == NULL){
				srcLock = &muSrc;
			}
			if(dstLock == NULL){
				dstLock = &muDst;
			}
			// lock src/dst
			boost::lock_guard<boost::mutex> lock(*deviceMutex_);
			boost::lock_guard<boost::mutex> lockSrc(*srcLock);
			boost::lock_guard<boost::mutex> lockDst(*dstLock);

			if(missing_)
			{
				ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
				return false;
			}

#ifdef SIMULATOR
			int errID = 0;
			if(!Simulator::Instance()->MoveTape(serial_, srcSlot, dstSlot, errID))
			{
				ltfsErr.SetErrCode(errID);
				if(errID == ERR_CHANGER_NOT_READY){
					ltfsErr.AddStringParam(sgDev_);
				}else if(errID == ERR_MOVE_DST_FULL || errID == ERR_MAIL_SLOT_OPEN){
					ltfsErr.AddIntParam(dstSlot);
				}else if(errID == ERR_MOVE_SRC_EMPTY || errID == ERR_DRIVE_PREVENT_REMOVE_MEDIUM){
					ltfsErr.AddIntParam(srcSlot);
				}else if(errID == ERR_CHANGER_MOVE_FAILED){
					ltfsErr.AddIntParam(srcSlot);
					ltfsErr.AddIntParam(dstSlot);
				}
				LtfsLgError("MoveCartridge, MoveTape failed: "<<errID);
				return false;
			}
#else
			if(!ltfs_management::MoveCartridge(sgDev_, srcSlot, dstSlot, ltfsErr))
			{
				LtfsLgError("MoveCartridge, MoveTape failed: "<<ltfsErr.GetErrMsg());
				return false;
			}
#endif

			MoveTape(srcSlot, dstSlot);
		}

		// refresh drive cleaning status if we load/unload a tape
		RefreshDriveCleaningStatus(srcSlot);
		RefreshDriveCleaningStatus(dstSlot);

		return true;
	}

	void LtfsChanger::RefreshDriveCleaningStatus(int slotId)
	{
		if(SLOT_DRIVE == GetSlotType(slotId)){
			LtfsDriveMap::iterator iterDrive = drives_.find(slotId);
			if(iterDrive!=drives_.end()){
				// load/unload a tape, we need to refresh drive cleaning status
				int cleaningStatus = -1;
				LtfsError lfsErr;
				if(false == iterDrive->second.GetDriveCleaningStatus(cleaningStatus, lfsErr, true)){
					LtfsLgError("Failed to call GetDriveCleaningStatus for the drive.");
				}
			}
		}
	}

	bool
	LtfsChanger::Mount(const string& driveSerial, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("Mount, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.Mount(ltfsErr);
	}

	bool
	LtfsChanger::CheckTape(const string& driveSerial, CHECK_TAPE_FLAG flag, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("Mount, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.CheckTape(flag, ltfsErr);
	}

	bool
	LtfsChanger::UnMount(const string& driveSerial, UInt64_t& genIndex, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("UnMount, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		bool bRet = iter->second.UnMount(genIndex, ltfsErr);
		return bRet;
	}

	bool
	LtfsChanger::Format(const string& driveSerial, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("Format, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.Format(ltfsErr);
	}

	bool
	LtfsChanger::UnFormat(const string& driveSerial, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("UnFormat, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.UnFormat(ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeBarcode(const string& driveSerial, string& barcode, LtfsError& ltfsErr, bool bRefresh)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeBarcode, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeBarcode(barcode, ltfsErr, bRefresh);
	}

	bool
	LtfsChanger::GetLoadedTapeLtfsFormat(const string& driveSerial, int& ltfsFormat, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeLtfsFormat, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeLtfsFormat(ltfsFormat, ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeLoadCounter(const string& driveSerial, int& count, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeLoadCounter, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeLoadCounter(count, ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeWPFlag(const string& driveSerial, bool& bIsWP, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeWPFlag, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeWPFlag(bIsWP, ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeMediaType(const string& driveSerial, int& mediaType, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeMediaType, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeMediaType(mediaType, ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeGenerationIndex(const string& driveSerial, long long& index, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeGenerationIndex, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeGenerationIndex(index, ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeCapacity(const string& driveSerial, long long& freeCapacity, LtfsError& ltfsErr)
	{
		long long totalCapacity = 0;
		return GetLoadedTapeCapacity(driveSerial, freeCapacity, totalCapacity, ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeCapacity(const string& driveSerial, long long& freeCapacity, long long& totalCapacity, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeCapacity, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeCapacity(freeCapacity, totalCapacity, ltfsErr);
	}

	bool
	LtfsChanger::GetDriveCleaningStatus(const string& driveSerial, int& cleaningStatus, LtfsError& ltfsErr, bool bForceRefresh)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			LtfsLgError("GetDriveCleaningStatus, drive missing, driveSerial:"<<driveSerial);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetDriveCleaningStatus, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		bool bRet = iter->second.GetDriveCleaningStatus(cleaningStatus, ltfsErr, bForceRefresh);
		LtfsLgDebug("LtfsChanger::GetDriveCleaningStatus: cleaningStatus = " << cleaningStatus << ",bRet = " << bRet << ".");
		return bRet;
	}

	bool
	LtfsChanger::GetLoadedTapeStatus(const string& driveSerial, int& status, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeStatus, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeStatus(status, ltfsErr);
	}

	bool
	LtfsChanger::SetLoadedTapeStatus(const string& driveSerial, int status, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("SetLoadedTapeStatus, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.SetLoadedTapeStatus(status, ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeFaulty(const string& driveSerial, bool& faulty, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeFaulty, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeFaulty(faulty, ltfsErr);
	}

	bool
	LtfsChanger::SetLoadedTapeFaulty(const string& driveSerial, bool faulty, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("SetLoadedTapeFaulty, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.SetLoadedTapeFaulty(faulty, ltfsErr);
	}

	bool
	LtfsChanger::SetLoadedTapeGroup(const string& driveSerial, const string& uuid, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("SetLoadedTapeGroup, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.SetLoadedTapeGroup(uuid, ltfsErr);
	}

	bool
	LtfsChanger::SetLoadedTapeBarcode(const string& driveSerial, const string& barcode, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("SetLoadedTapeBarcode, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.SetLoadedTapeBarcode(barcode, ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeGroup(const string& driveSerial, string& uuid, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeGroup, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeGroup(uuid, ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeUUID(const string& driveSerial, string& tapeUUID, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeUUID, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeUUID(tapeUUID, ltfsErr);
	}

	bool
	LtfsChanger::SetLoadedTapeUUID(const string& driveSerial, const string& tapeUUID, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("SetLoadedTapeUUID, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.SetLoadedTapeUUID(tapeUUID, ltfsErr);
	}

	bool
	LtfsChanger::SetLoadedTapeDualCopy(const string& driveSerial, const string& dualCopy, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("SetLoadedTapeDualCopy, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.SetLoadedTapeDualCopy(dualCopy, ltfsErr);
	}

	bool
	LtfsChanger::GetLoadedTapeDualCopy(const string& driveSerial, string& dualCopy, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetLoadedTapeDualCopy, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetLoadedTapeDualCopy(dualCopy, ltfsErr);
	}

	bool
	LtfsChanger::GetWorkingStatus(const string& driveSerial, bool& working, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("GetWorkingStatus, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.GetWorkingStatus(working, ltfsErr);
	}

	bool
	LtfsChanger::IsTapeMounted(const string& driveSerial, bool& mounted, LtfsError& ltfsErr)
	{
		LtfsDriveMap::iterator iter;

		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

		if(!FindDriveBySerial(driveSerial, iter))
		{
			ltfsErr.SetErrCode(ERR_PARAM_DRIVE_SERIAL);
			LtfsLgError("IsTapeMounted, no drive been found, driveSerial:"<<driveSerial);
			return false;
		}

		return iter->second.IsTapeMounted(mounted, ltfsErr);
	}

	bool
	LtfsChanger::OpenMailSlot(int slotid, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);


		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_CHANGER_MISSING);
			return false;
		}

#ifdef SIMULATOR

		return false;
#else
		if(!ltfs_management::OpenMailSlot(sgDev_, slotid, ltfsErr))
		{
			LtfsLgError("MoveCartridge, MoveTape failed: "<<ltfsErr.GetErrMsg());
			return false;
		}

		return true;
#endif
	}

	bool LtfsChanger::IsMailSlotAvailable()
	{
#ifdef SIMULATOR
		return true;
#else
		LtfsError lfsErr;
		vector<ltfs_management::mailSlotElement> mailSlots;
		{
			boost::lock_guard<boost::mutex> lock(*deviceMutex_);
			if(false == ltfs_management::GetChangerMailSlots(sgDev_, mailSlots, lfsErr)){
				LtfsLgError("Failed to get mail slot info of changer to check if any mail slot available.");
				return false;
			}
		}

		for(unsigned int i = 0; i < mailSlots.size(); i++){
			if(true == mailSlots[i].mIsEmpty && false == mailSlots[i].mIsOpen){
				Refresh(false, lfsErr);
				return true;
			}
		}
#endif

		return false;
	}

	bool
	LtfsChanger::GetChangerMailSlots(vector<LtfsMailSlotInfo>& mailSlots, LtfsError& lfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
#ifdef SIMULATOR

#else
		vector<mailSlotElement> mailSlotE;
		mailSlots.clear();
		if(ltfs_management::GetChangerMailSlots(sgDev_, mailSlotE, lfsErr))
		{
			LtfsLgDebug("GetChangerMailSlots success  number is "<<mailSlotE.size());

			for(vector<mailSlotElement>::iterator iter=mailSlotE.begin();
					iter!=mailSlotE.end(); ++iter)
			{
				LtfsMailSlotInfo info;
				info.mSlotID = iter->mSlotID;
				info.mLogicSlotID = iter->mLogicSlotID;
				info.mIsEmpty = iter->mIsEmpty;
				info.mIsOpen = iter->mIsOpen;
				info.mBarcode = iter->mBarcode;
				mailSlots.push_back(info);
			}
			return true;
		}
#endif
		return false;
	}

	ENUM_SLOT_TYPE LtfsChanger::GetSlotType(int slotId)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		for(LtfsDriveMap::iterator iter=drives_.begin();
				iter!=drives_.end(); ++iter)
		{
			if(iter->second.GetSlotID() == slotId)
			{
				return SLOT_DRIVE;
			}
		}

		for(LtfsSlotMap::iterator iter=slots_.begin();
				iter!=slots_.end(); ++iter)
		{
			if(iter->second.GetSlotID() == slotId)
			{
				return SLOT_STORAGE;
			}
		}

		for(LtfsMailSlotMap::iterator iter=mailslots_.begin();
				iter!=mailslots_.end(); ++iter)
		{
			if(iter->second.GetSlotID() == slotId)
			{
				return SLOT_MAIL_SLOT;
			}
		}
		return SLOT_UNKNOWN;
	}

	string LtfsChanger::GetDriveSerialBySlotId(int slotId)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		for(LtfsDriveMap::iterator iter=drives_.begin();
				iter!=drives_.end(); ++iter)
		{
			if(iter->second.GetSlotID() == slotId)
			{
				return iter->second.GetSerial();
			}
		}
		return "";
	}

	boost::mutex* LtfsChanger::GetDriveLockBySlotId(int slotId)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		for(LtfsDriveMap::iterator iter=drives_.begin();
				iter!=drives_.end(); ++iter)
		{
			if(iter->second.GetSlotID() == slotId)
			{
				return iter->second.GetLockMutex();
			}
		}
		return NULL;
	}

	bool LtfsChanger::IsTapeOffline(int slotId)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		for(LtfsDriveMap::iterator iter=drives_.begin(); iter!=drives_.end(); ++iter)
		{
			if(iter->second.GetSlotID() == slotId){
				LtfsDriveInfo driveInfo;
				iter->second.GetDriveInfo(driveInfo);
				if(driveInfo.mStatus == DRIVE_STATUS_DISCONNECTED){
					return true;
				}
				return false;
			}
		}
		return false;
	}

	bool LtfsChanger::GetDriveInfoForTape(const string& barcode, LtfsDriveInfo& driveInfo)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		for(LtfsDriveMap::iterator iter=drives_.begin(); iter!=drives_.end(); ++iter)
		{
			if(iter->second.GetBarcode() == barcode){
				iter->second.GetDriveInfo(driveInfo);
				return true;
			}
		}
		return false;
	}

	bool LtfsChanger::IsTapeInDrive(const string& barcode)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		for(LtfsDriveMap::iterator iter=drives_.begin(); iter!=drives_.end(); ++iter)
		{
			if(iter->second.GetBarcode() == barcode){
				return true;
			}
		}
		return false;
	}

	int LtfsChanger::GetTapeNum(bool bIncludeDrive, bool bIncludeMailSlot)
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);

		int tapeNum = 0;
		for(LtfsSlotMap::iterator iter=slots_.begin(); iter!=slots_.end(); ++iter){
			if(!iter->second.IsEmpty()){
				tapeNum++;
			}
		}
		if(bIncludeDrive){
			for(LtfsDriveMap::iterator iter=drives_.begin(); iter!=drives_.end(); ++iter){
				if(!iter->second.IsEmpty()){
					tapeNum++;
				}
			}
		}
		if(bIncludeMailSlot){
			for(LtfsMailSlotMap::iterator iter=mailslots_.begin(); iter!=mailslots_.end(); ++iter){
				if(!iter->second.IsEmpty()){
					tapeNum++;
				}
			}
		}

		LtfsLgDebug("GetTapeNum: tapeNum = " << tapeNum);
		return tapeNum;
	}

	int LtfsChanger::GetSlotNum()
	{
		boost::shared_lock<boost::shared_mutex> lock(*mapMutex_);
		LtfsLgDebug("GetSlotNum: slotNum = " << slots_.size());
		return slots_.size();
	}

	bool LtfsChanger::IsTapeInMailSlot(const string& barcode, LtfsMailSlotInfo& mailSlotInfo)
	{
		LtfsError lfsErr;
		vector<LtfsMailSlotInfo> mailSlots;
		if(GetMailSlotList(mailSlots, lfsErr)){
			for(unsigned int i = 0; i < mailSlots.size(); i++){
				if(barcode == mailSlots[i].mBarcode){
					mailSlotInfo = mailSlots[i];
					return true;
				}
			}
		}
		return false;
	}

} /* namespace ltfs_management */
