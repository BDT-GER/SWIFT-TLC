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
 * LtfsDetails.h
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#pragma once

namespace ltfs_management
{
	enum ChangerStatus
	{
		CHANGER_STATUS_UNKNOWN = 0,     //status not determined yet
		CHANGER_STATUS_CONNECTED,		//status is ready
		CHANGER_STATUS_BUSY,			//
		CHANGER_STATUS_DISCONNECTED		//
	};

	enum ChangerHwMode
	{
		CHANGER_HW_READY = 0,
		CHANGER_HW_DISSCONNECTED,
		CHANGER_HW_ROBOTIC_BLOCKED,
		CHANGER_HW_SCSI_HANG,
		CHANGER_HW_NO_USABLE_DRIVE,
		CHANGER_HW_TOO_MANY_TAPES
	};

	enum SystemHwMode
	{
		SYSTEM_HW_READY,
		SYSTEM_HW_NOT_READY,
		SYSTEM_HW_NEED_DIAGNOSE,
		SYSTEM_HW_DIAGNOSING,
		SYSTEM_HW_NO_LICENSE
	};

	enum DriveStatus
	{
		DRIVE_STATUS_UNKNOWN = 0, 		// drive status is not detected yet
		DRIVE_STATUS_OK,
		DRIVE_STATUS_ERROR,
		DRIVE_STATUS_DISCONNECTED
	};

	enum DriveCleaningStatus
	{
		CLEANING_UNKNOWN = 0,			// drive cleaning status is not detected yet
		CLEANING_NONE,
		CLEANING_REQUIRED,
		CLEANING_IN_PROGRESS
		//CLEANING_FAILED
	};

	enum DriveInterfaceType
	{
		INTERFACE_UNKNOWN = 0,
		INTERFACE_SAS,
		INTERFACE_FC
	};

	enum TapeStatus
	{
		TAPE_UNKNOWN = 0, 		// tape status not detected yet
		TAPE_OPEN,
		TAPE_ACTIVE,
		TAPE_CLOSED,
		TAPE_EXPORTED,
		TAPE_CLEAN_EXPIRED,
		//TAPE_MISSING,
		TAPE_CONFLICT
		//INVALID_CLEAN_TAPE
	};

	enum LTFS_FORMAT
	{
		LTFS_UNKNOWN = 0, 		// tape format is not dected yet
		LTFS_VALID,  			//tape is LTFS formatted already
		OTHER_DATA,  			//tape is not LTFS format but with data
		OTHER_EMPTY  			//tape is not LTFS format without any data
	};

	enum TapeMediaType
	{
		MEDIA_UNKNOWN = 0,			// media type not detected yet
		MEDIA_LTO6,
		MEDIA_LTO5,				// LTO5
		MEDIA_LTO4,
		MEDIA_LTO3,
		MEDIA_LTO2,
		MEDIA_LTO1,
		MEDIA_OTHERS			// other types
	};

	enum TapeMediumType
	{
		MEDIUM_UNKNOWN = 0,			// meduim type not detected yet
		MEDIUM_DATA,			// DATA medium
		MEDIUM_CLEANING,		// Cleaning medium
		MEDIUM_DIAGNOSTICS,		// Diagnostics Medium
		MEDIUM_WORM				// WORM Medium
	};

	struct LtfsChangerInfo
	{
		string 	mSerial;
		string	mVendor;
		string 	mProduct;
		string 	mVersion;
		int 	mStatus;
		int 	mDriveStart;
		int 	mSlotStart;
		int 	mMailSlotStart;
		bool	mAutoCleanMode;
		ChangerHwMode		mHwMode;
	};

	struct LtfsDriveInfo
	{
		string 	mSerial;
		string 	mVendor;
		string 	mProduct;
		string 	mVersion;
		string 	mScsiAddr;
		string 	mBarcode;
		string	mDriveName;
		string	mStDev;
		string	mSgDev;
		int 	mSlotID;
		int 	mLogicSlotID;
		int 	mStatus;
		int 	mCleaningStatus;
		int 	mInterfaceType;
		int		mGeneration;
		bool	mIsFullHight;
		bool 	mIsEmpty;
		bool	mAccessible;
		bool	mAbnormal;
	};

	struct LtfsSlotInfo
	{
		int 	mSlotID;
		int 	mLogicSlotID;
		bool 	mIsEmpty;
		string	mBarcode;
		bool	mAccessible;
		bool	mAbnormal;
	};

	struct LtfsMailSlotInfo
	{
		int 	mSlotID;
		int 	mLogicSlotID;
		bool 	mIsEmpty;
		bool	mIsOpen;
		bool	mAccessible;
		bool	mAbnormal;
		string	mBarcode;
	};

	enum ENUM_SLOT_TYPE
	{
		SLOT_STORAGE = 0,
		SLOT_DRIVE,
		SLOT_MAIL_SLOT,
		SLOT_UNKNOWN
	};

	struct LtfsTapeInfo
	{
		string 	mBarcode;
		int 	mSlotID;
		int 	mLogicSlotID;
		int		mMediumType;

	};

} /* namespace ltfs_management */

