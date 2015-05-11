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
 * LtfsDrive.cpp
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#include "stdafx.h"
#ifdef SIMULATOR
#include "simulator/Simulator.h"
#else
#include "CmnFunc.h"
#endif
#include "LtfsError.h"
#include "LtfsDetails.h"
#include "LtfsScsiDevice.h"
#include "LtfsTape.h"
#include "LtfsSlot.h"
#include "LtfsDrive.h"

namespace ltfs_management
{
	LtfsDrive::LtfsDrive(int slotid, int logicSlotId, const string& serial, bool isFullHight,
			int generation, const string& vendor, const string& product, const string& stDev, const string& sgDev):
		LtfsScsiDevice(), LtfsSlot(slotid, logicSlotId)
	{
		// TODO Auto-generated constructor stub
		serial_	= serial;

		status_			= DRIVE_STATUS_UNKNOWN;
		cleaningStatus_ = CLEANING_UNKNOWN;
		interfaceType_ 	= INTERFACE_UNKNOWN;

		isFullHight_	= isFullHight;
		generation_		= generation;
		mounted_		= false;
		hasTape_		= false;
		hasTapeCheked_	= false;

		product_ = product;
		vendor_ = vendor;
		stDev_ = stDev;
		sgDev_ = sgDev;

		inited_ = false;
	}

	LtfsDrive::~LtfsDrive()
	{
		// TODO Auto-generated destructor stub
	}

	bool
	LtfsDrive::Refresh()
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
#ifdef SIMULATOR
		DriveDetail detail;

		if(!Simulator::Instance()->GetDriveInfo(serial_, detail))
		{
			LtfsLgError("Refresh, GetDriveInfo failed,  " << scsiAddr_);
			return false;
		}
		scsiAddr_ 	= detail.m_ScsiAddr;
		vendor_ 	= detail.m_Vendor;
		product_ 	= detail.m_Product;
		version_ 	= detail.m_Version;
		stDev_ 		= detail.m_StDev;
		sgDev_ 		= detail.m_SgDev;

		status_			= detail.m_Status;
		cleaningStatus_ = detail.m_CleaningStatus;
		interfaceType_ 	= detail.m_InterfaceType;
		generation_		= detail.m_Generation;
		//isFullHight_	= detail.m_IsFullHight;
		//accessible_		= detail.m_Accessible;
		//abnormal_		= detail.m_Abnormal;
		inited_ = true;

		return true;
#else
		LSSCSI_INFO scsiInfo;
		DriveStatus driveStatus;
		DriveCleaningStatus cleaningStatus;
		DriveInterfaceType interfaceType;

		if(!ltfs_management::GetDriveInfo(serial_, scsiInfo, driveStatus, cleaningStatus, interfaceType))
		{
			LtfsLgError("Refresh, GetDriveInfo failed,  " << scsiAddr_);
			return false;
		}

		scsiAddr_ 	= scsiInfo.scsiAddr;
		if(vendor_ == ""){
			vendor_ 	= scsiInfo.vendor;
		}
		if(product_ == ""){
			product_ 	= scsiInfo.product;
		}
		version_ 	= scsiInfo.version;
		stDev_ 		= scsiInfo.stDev;
		sgDev_ 		= scsiInfo.sgDev;

		if(status_ == DRIVE_STATUS_DISCONNECTED && driveStatus != DRIVE_STATUS_DISCONNECTED){
			//LtfsEvent(EVENT_LEVEL_INFO, "TODO:Drive_Connected", "Drive " << logicSlotID_ << " connected.");
		}
		status_			= driveStatus;
		cleaningStatus_ = cleaningStatus;
		interfaceType_ 	= interfaceType;

		if(!inited_){
			string barcode = GetBarcode();
			LtfsLgDebug("DDEBUG:  barcode = " << barcode);
			if(barcode != ""){
				string mountPoint = GetMountedDriveMountPoint(stDev_, GetBarcode());
				LtfsLgDebug("DDEBUG: GetMountedDriveMountPoint() = " << mountPoint);
				if(mountPoint != ""){
					mounted_ = true;
				}
			}
		}
		inited_ = true;

		return true;
#endif
	}


	void
	LtfsDrive::SetMissing(bool missing)
	{
		missing_ = missing;
		if(missing_){
			tape_ = NULL;
			status_ = DRIVE_STATUS_DISCONNECTED;
		}
	}

	bool
	LtfsDrive::GetMissing()
	{
		return missing_;
	}

	void
	LtfsDrive::GetDriveInfo(LtfsDriveInfo& drive)
	{
		drive.mSerial = serial_;
		drive.mScsiAddr = scsiAddr_;
		drive.mVendor = vendor_;
		drive.mProduct = product_;
		drive.mVersion = version_;
		drive.mStDev = stDev_;
		drive.mSgDev = sgDev_;

		drive.mStatus = status_;
		drive.mCleaningStatus = cleaningStatus_;
		drive.mInterfaceType = interfaceType_;

		drive.mSlotID = slotID_;
		drive.mLogicSlotID = logicSlotID_;
		drive.mBarcode = GetBarcode();

		drive.mIsFullHight = isFullHight_;
		drive.mGeneration = generation_;
		drive.mIsEmpty = (bool)IsEmpty();

		drive.mAccessible = accessible_;
		drive.mAbnormal = abnormal_;
	}

	bool
	LtfsDrive::Mount(LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("Mount, drive missing, :"<<serial_);
			return false;
		}

		string barcode = GetBarcode();
		if(barcode.empty())
		{
			ltfsErr.SetErrCode(ERR_MOUNT_EMPTY);
			LtfsLgError("Mount failed,  barcode empty , " << serial_);
			return false;
		}

		if(mounted_)
			return true;

#ifdef SIMULATOR
		int errID = 0 ;

		if(!Simulator::Instance()->Mount(serial_, errID))
		{
			ltfsErr.SetErrCode(errID);
			ltfsErr.AddStringParam(barcode);
			if(errID == ERR_MOUNT_OPEN_DRIVE){
				ltfsErr.AddStringParam(stDev_);
			}
			LtfsLgError("Mount failed,  " << serial_);
			return false;
		}

#else
		if(!ltfs_management::Mount(barcode, stDev_, ltfsErr))
		{
			LtfsLgError("Mount failed,  " << ltfsErr.GetErrMsg());
			return false;
		}
#endif
		mounted_ = true;

		return true;
	}

	bool
	LtfsDrive::CheckTape(CHECK_TAPE_FLAG flag, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("Mount, drive missing, :"<<serial_);
			return false;
		}

		string barcode = GetBarcode();
		if(barcode.empty())
		{
			ltfsErr.SetErrCode(ERR_MOUNT_EMPTY);
			LtfsLgError("CheckTape failed,  barcode empty , " << serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0 ;
		//TODO: add code to simulator check tape

#else
		if(!ltfs_management::CheckTape(stDev_, barcode, flag, ltfsErr))
		{
			LtfsLgError("CheckTape failed,  " << ltfsErr.GetErrMsg());
			return false;
		}
#endif
		return true;
	}

	bool
	LtfsDrive::UnMount(UInt64_t& genIndex, LtfsError& ltfsErr)
	{
		genIndex = 0;
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("UnMount, drive missing, :"<<serial_);
			return false;
		}

		string barcode = GetBarcode();
		if(barcode.empty())
		{
			ltfsErr.SetErrCode(ERR_UNMOUNT_EMPTY);
			LtfsLgError("Umount failed,  barcode empty , " << serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0 ;

		if(!Simulator::Instance()->Umount(serial_, errID))
		{
			ltfsErr.SetErrCode(errID);
			ltfsErr.AddStringParam(barcode);
			LtfsLgError("Umount failed,  " << serial_);
			return false;
		}
		long long gIndex = 0;
		Simulator::Instance()->GetTapeGenerationIndex(serial_, gIndex, errID);
		genIndex = gIndex;

#else
		string mountPoint = GetMountedDriveMountPoint(stDev_, barcode);
		if(!ltfs_management::UnMount(barcode, stDev_, ltfsErr, mountPoint))
		{
			LtfsLgError("UnMount failed,  " << ltfsErr.GetErrMsg());
			return false;
		}

		// get generation number index of the tape
		if(false == ltfs_management::GetLoadedTapeGenerationIndex(sgDev_, stDev_, genIndex, ltfsErr)){
			LtfsLgError("Failed to get generation number for tape " << barcode << " after it is unmounted.");
			genIndex = 0;
		}

#endif
		LtfsLgDebug("LtfsDrive::UnMount genIndex = " << genIndex << ".");
		mounted_ = false;

		// refresh drive cleaning status
		RefreshDriveCleaningStatus();

		return true;
	}

	bool
	LtfsDrive::Format(LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("Format, drive missing, :"<<serial_);
			return false;
		}

		string barcode = GetBarcode();
		if(barcode.empty())
		{
			ltfsErr.SetErrCode(ERR_FORMAT_EMPTY);
			LtfsLgError("Format failed,  barcode empty , " << ltfsErr.GetErrMsg());
			return false;
		}

#ifdef SIMULATOR
		int errID = 0 ;

		string uuid = "";
		if(!Simulator::Instance()->Format(uuid, serial_, errID))
		{
			ltfsErr.SetErrCode(errID);
			if(errID == ERR_DRIVE_MOUNTED){
				ltfsErr.AddStringParam(stDev_);
			}else{
				ltfsErr.AddStringParam(barcode);
			}
			LtfsLgError("Format failed,  " << serial_);
			return false;
		}
#else
		if(!ltfs_management::Format(stDev_, barcode, ltfsErr))
		{
			LtfsLgError("Format failed,  " << ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}


	bool
	LtfsDrive::UnFormat(LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("UnFormat, drive missing, :"<<serial_);
			return false;
		}

		string barcode = GetBarcode();
		if(barcode.empty())
		{
			ltfsErr.SetErrCode(ERR_UN_FORMAT_EMPTY);
			LtfsLgError("UnFormat failed,  barcode empty , " << ltfsErr.GetErrMsg());
			return false;
		}

#ifdef SIMULATOR
		/*int errID = 0 ;

		string uuid = "";
		if(!Simulator::Instance()->Format(uuid, serial_, errID))
		{
			ltfsErr.SetErrCode(errID);
			if(errID == ERR_DRIVE_MOUNTED){
				ltfsErr.AddStringParam(driveStDevName);
			}
			LtfsLgError("Format failed,  " << serial_);
			return false;
		}*/
#else
		if(!ltfs_management::UnFormat(sgDev_, stDev_, barcode, ltfsErr))
		{
			LtfsLgError("UnFormat failed,  " << ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeBarcode(string& barcode, LtfsError& ltfsErr, bool bRfresh)
	{
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeBarcode, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		TapeDetail tapeDetail;
		if(!Simulator::Instance()->GetTapeInfo(serial_, tapeDetail, errID))
		{

			ltfsErr.SetErrCode(errID);
			LtfsLgDebug("GetLoadedTapeBarcode failed, :"<<serial_);
			return false;
		}

		barcode = tapeDetail.m_Barcode;
#else
		if(tape_==NULL)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_EMPTY);
			LtfsLgDebug("GetLoadedTapeBarcode failed, :"<<serial_);
			return false;
		}

		barcode = GetBarcode();
#endif
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeLtfsFormat(int& ltfsFormat, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeLtfsFormat, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->GetTapeFormatType(serial_, ltfsFormat, errID))
		{
			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("GetLoadedTapeLtfsFormat failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::GetLoadedTapeLtfsFormat(sgDev_, stDev_, (LTFS_FORMAT&)ltfsFormat, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeLtfsFormat failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		LtfsLgDebug("GetLoadedTapeLtfsFormat, ltfsFormat :"<<ltfsFormat);
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeLoadCounter(int& count, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeLoadCounter, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		TapeDetail tapeDetail;
		if(!Simulator::Instance()->GetTapeInfo(serial_, tapeDetail, errID))
		{

			ltfsErr.SetErrCode(errID);
			if(ERR_DRIVE_MOUNTED == errID){
				ltfsErr.AddStringParam(stDev_);
			}
			LtfsLgError("GetLoadedTapeLoadCounter failed, :"<<serial_);
			return false;
		}

		count = 1;
#else
		if(!ltfs_management::GetLoadedTapeLoadCounter(sgDev_, stDev_, (unsigned int&)count, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeLoadCounter failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeWPFlag(bool& bIsWP, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeWPFlag, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		bIsWP = false;
		return true;
#else
		if(!ltfs_management::GetLoadedTapeWPFlag(sgDev_, stDev_, bIsWP, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeWPFlag failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}
#endif
		LtfsLgDebug("GetLoadedTapeWPFlag, bIsWP :"<< bIsWP);
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeMediaType(int& mediaType, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeMediaType, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->GetTapeMediaType(serial_, mediaType, errID))
		{
			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("GetLoadedTapeMediaType failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::GetLoadedTapeMediaType(sgDev_, stDev_, (TapeMediaType&)mediaType, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeMediaType failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		LtfsLgDebug("GetLoadedTapeMediaType, mediaType :"<<mediaType);
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeMediumType(int& mediumType, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeMediumType, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		/*int errID = 0;
		if(!Simulator::Instance()->GetLoadedTapeMediumType(serial_, mediaType, errID))
		{
			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("GetLoadedTapeMediumType failed, :"<<serial_);
			return false;
		}*/
		return false;

#else
		if(!ltfs_management::GetLoadedTapeMediumType(sgDev_, stDev_, (TapeMediumType&)mediumType, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeMediumType failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		LtfsLgDebug("GetLoadedTapeMediumType, mediumType :" << mediumType);
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeGenerationIndex(long long& index, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeGenerationIndex, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->GetTapeGenerationIndex(serial_, index, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("GetLoadedTapeGenerationIndex failed, :"<<serial_);
			return false;
		}


#else
		if(!ltfs_management::GetLoadedTapeGenerationIndex(sgDev_, stDev_, (long long unsigned int&)index, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeGenerationIndex failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}
	bool
	LtfsDrive::GetLoadedTapeCapacity(long long& freeCapacity, LtfsError& ltfsErr)
	{
		long long totalCapacity = 0;
		return GetLoadedTapeCapacity(freeCapacity, totalCapacity, ltfsErr);
	}

	bool
	LtfsDrive::GetLoadedTapeCapacity(long long& freeCapacity, long long& totalCapacity, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeCapacity, drive missing, :"<<serial_);
			return false;
		}

		string barcode = GetBarcode();
		if(barcode.empty())
		{
			ltfsErr.SetErrCode(ERR_DRIVE_EMPTY);
			LtfsLgError("GetLoadedTapeCapacity failed,  barcode empty , " << ltfsErr.GetErrMsg());
			return false;
		}


#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->GetLoadedTapeCapacity(serial_, totalCapacity, freeCapacity))
		{
			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("GetLoadedTapeCapacity failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::GetLoadedTapeCapacity(sgDev_, stDev_, barcode, freeCapacity, totalCapacity, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeCapacity failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeStatus(int& status, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeStatus, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->GetTapeStatus(serial_, status, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("GetLoadedTapeStatus failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::GetLoadedTapeStatus(sgDev_, stDev_, (TapeStatus&)status, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeStatus failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}


	bool LtfsDrive::RefreshDriveCleaningStatus()
	{
#ifdef SIMULATOR
		DriveDetail detail;

		if(!Simulator::Instance()->GetDriveInfo(serial_, detail))
		{
			LtfsLgError("Refresh, GetDriveInfo failed,  " << scsiAddr_);
			return false;
		}
		cleaningStatus_ = detail.m_CleaningStatus;//CLEANING_UNKNOWN;
#else
		ltfs_management::DriveStatus driveStatus;
		ltfs_management::DriveCleaningStatus  clnSt;
		if(false == ltfs_management::GetDriveStatus(sgDev_, driveStatus, clnSt)){
			LtfsLgError("RefreshDriveCleaningStatus failed.");
			return false;
		}
		cleaningStatus_ = clnSt;
#endif
		LtfsLgDebug("RefreshDriveCleaningStatus finished: cleaningStatus_ = " << cleaningStatus_ << ".");
		return true;
	}

	bool LtfsDrive::GetDriveCleaningStatus(int& cleaningStatus, LtfsError& ltfsErr, bool bForceRefresh)
	{
		if(cleaningStatus_ != CLEANING_UNKNOWN && false == bForceRefresh){
			cleaningStatus =  cleaningStatus_;
			LtfsLgDebug("GetDriveCleaningStatus: cleaningStatus = " << cleaningStatus << ".");
			return true;
		}

		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(false == RefreshDriveCleaningStatus()){
			LtfsLgError("GetDriveCleaningStatus failed.");
			return false;
		}
		cleaningStatus =  cleaningStatus_;
		LtfsLgDebug("GetDriveCleaningStatus: cleaningStatus = " << cleaningStatus << ".");
		return true;
	}

	bool
	LtfsDrive::SetLoadedTapeStatus(int status, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("SetLoadedTapeStatus, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->SetTapeStatus(serial_, status, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("SetLoadedTapeStatus failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::SetLoadedTapeStatus(sgDev_, stDev_, (TapeStatus)status, ltfsErr))
		{
			LtfsLgError("SetLoadedTapeStatus failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeFaulty(bool& faulty, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeFaulty, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->GetTapeFaulty(serial_, faulty, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("GetLoadedTapeFaulty failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::GetLoadedTapeFaulty(sgDev_, stDev_, faulty, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeFaulty failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}

	bool
	LtfsDrive::SetLoadedTapeFaulty(bool faulty, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("SetLoadedTapeFaulty, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->SetTapeFaulty(serial_, faulty, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("SetLoadedTapeFaulty failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::SetLoadedTapeFaulty(sgDev_, stDev_, faulty, ltfsErr))
		{
			LtfsLgError("SetLoadedTapeFaulty failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}

	bool
	LtfsDrive::SetLoadedTapeGroup(const string& uuid, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("SetLoadedTapeGroup, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->SetTapeGroupUUID(serial_, uuid, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("SetLoadedTapeGroup failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::SetLoadedTapeGroup(sgDev_, stDev_, uuid, ltfsErr))
		{
			LtfsLgError("SetLoadedTapeGroup failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeGroup(string& uuid, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeGroup, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->GetTapeGroupUUID(serial_, uuid, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("GetLoadedTapeGroup failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::GetLoadedTapeGroup(sgDev_, stDev_, uuid, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeGroup failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}


	bool
	LtfsDrive::SetLoadedTapeBarcode(const string& barcode, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("SetLoadedTapeBarcode, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		/*int errID = 0;
		if(!Simulator::Instance()->SetTapeGroupUUID(serial_, barcode, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("SetLoadedTapeBarcode failed, :"<<serial_);
			return false;
		}*/
		return true;

#else
		if(!ltfs_management::SetLoadedTapeBarcode(sgDev_, stDev_, barcode, ltfsErr))
		{
			LtfsLgError("SetLoadedTapeBarcode failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}
#endif
		return true;
	}

	bool
	LtfsDrive::SetLoadedTapeDualCopy(const string& dualCopy, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("SetLoadedTapeDualCopy, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->SetTapeDualCopy(serial_, dualCopy, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("SetLoadedTapeDualCopy failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::SetLoadedTapeDualCopy(sgDev_, stDev_, dualCopy, ltfsErr))
		{
			LtfsLgError("SetLoadedTapeDualCopy failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}

	bool
	LtfsDrive::GetLoadedTapeDualCopy(string& dualCopy, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeDualCopy, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->GetTapeDualCopy(serial_, dualCopy, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("GetLoadedTapeDualCopy failed, :"<<serial_);
			return false;
		}

#else
		if(!ltfs_management::GetLoadedTapeDualCopy(sgDev_, stDev_, dualCopy, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeDualCopy failed, :"<<ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}

	bool
	LtfsDrive::SetLoadedTapeUUID(const string& uuid, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("SetLoadedTapeUUID, drive missing, :" << serial_);
			return false;
		}

#ifdef SIMULATOR
		/*int errID = 0;
		if(!Simulator::Instance()->SetLoadedTapeUUID(serial_, uuid, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("SetLoadedTapeDualCopy failed, :"<<serial_);
			return false;
		}*/
		return true;

#else
		if(!ltfs_management::SetLoadedTapeUUID(sgDev_, stDev_, uuid, ltfsErr))
		{
			LtfsLgError("SetLoadedTapeUUID failed, :" << ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}


	bool
	LtfsDrive::GetLoadedTapeUUID(string& uuid, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetLoadedTapeUUID, drive missing, :" << serial_);
			return false;
		}

#ifdef SIMULATOR
		/*int errID = 0;
		if(!Simulator::Instance()->GetLoadedTapeUUID(serial_, uuid, errID))
		{

			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("GetLoadedTapeUUID failed, :"<<serial_);
			return false;
		}*/
		return true;

#else
		if(!ltfs_management::GetLoadedTapeUUID(sgDev_, stDev_, uuid, ltfsErr))
		{
			LtfsLgError("GetLoadedTapeUUID failed, :" << ltfsErr.GetErrMsg());
			return false;
		}

#endif
		return true;
	}

	bool
	LtfsDrive::GetWorkingStatus(bool& working, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("GetWorkingStatus, drive missing, :"<<serial_);
			return false;
		}

#ifdef SIMULATOR


		return false;
#else

		return false;
#endif
	}

	bool
	LtfsDrive::IsTapeMounted(bool& mounted, LtfsError& ltfsErr)
	{
		boost::lock_guard<boost::mutex> lock(*deviceMutex_);
		if(missing_)
		{
			ltfsErr.SetErrCode(ERR_DRIVE_MISSING);
			LtfsLgError("IsTapeMounted, drive missing, :"<<serial_);
			return false;
		}

#if 1
		mounted = mounted_;
		return true;
#else

#ifdef SIMULATOR
		int errID = 0;
		if(!Simulator::Instance()->IsMounted(serial_, mounted, errID))
		{
			ltfsErr.SetErrCode(ERR_FAILED);
			LtfsLgError("IsTapeMounted failed, :"<<serial_);
			return false;
		}

		return true;
#else
		mounted = ltfs_management::IsDriveMounted(stDev_);

		return true;
#endif
#endif
	}

	bool LtfsDrive::IsAvailable() const
	{
		if(status_ != (int)DRIVE_STATUS_OK){
			return false;
		}

		return true;
	}
} /* namespace ltfs_management */
