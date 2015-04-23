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
 * CmnFunc.h
 *
 *  Created on: Jul 30, 2012
 *      Author: Sam Chen
 */

#pragma once

#include "stdafx.h"
#include "LtfsError.h"
#include "LtfsDetails.h"

const string DIAGNOSIS_TAPE_LOG_PATH = COMM_VS_LOG_PATH + "/";

namespace ltfs_management
{

// SCSI commands definition
#define CMDread						0x08
#define	CMDwrite					0x0A
#define CMDwrite_filemarks			0x10
// SCSI command for changer
#define CMDMoveMedium				0xA5
#define CMDReadElementStatus		0xB8
#define CMDTestUnitReady			0x00
#define CMDOpenCloseMailSlot		0x1B
#define CMDModeSenseChanger			0x5A
#define CMDPreventAllowMediaRemoval	0x1E
// SCSI command for drive
#define CMDLoadUnload				0x1B
#define CMDLogSense					0x4D
#define CMDWriteAttribute			0x8D
#define CMDReadAttribute			0x8C
#define CMDReportSensity			0x44
#define CMDModeSense				0x1A
// SCSI command for changer and drive
#define CMDInquiry					0x12
#define CMDRequestSense				0x03

//time out value
#define DAT_INQUIRY_TIMEOUT				6000
#define LTFS_MOVEMEDIA_TIMEOUT		   	2280000
#define DAT_TESTUNITREADY_TIMEOUT      	60000
#define	LTO_READATTRIB_TIMEOUT			60000
#define LTO_WRITEATTRIB_TIMEOUT 		60000
#define LTO_OPEN_MAIL_SLOT_TIMEOUT 		60000
#define LTO_PREVENTALLOWMEDIA_TIMEOUT	60000
#define	CMD_DF_TIMEOUT					15*60 //15 minutes
#define	DAT_LOAD_TIMEOUT				60000

#define SENSE_EARLY_WARNING_EOM(b) (((b[2] & 0x4F) == 0x40) && (b[12] == 0x00) && (b[13] == 0x02))
#define SENSE_END_OF_MEDIA(b)      (((b[2] & 0x4F) == 0x4D) && (b[12] == 0x00) && (b[13] == 0x02))
#define SENSE_HAS_ILI_SET(b)       ((b[2] & 0x20) == 0x20)

//SCSI Status values:
#define S_NO_STATUS					0xFF
#define S_GOOD						0x00
#define S_CHECK_CONDITION			0x02
 // Driver Status values:
#define DS_GOOD						0x00

#define TO_STRING(val)	boost::lexical_cast<string>(val)

#ifndef LtfsLogDebug
#ifdef DO_UNIT_TEST
	#define LtfsLogDebug(_ostr) cout << _ostr << endl; LgDebug("vs", _ostr)
	#define LtfsLogInfo(_ostr) cout << _ostr << endl; LgInfo("vs", _ostr)
	#define LtfsLogWarn(_ostr) cout << _ostr << endl; LgInfo("vs", _ostr)
	#define LtfsLogError(_ostr) cout << _ostr << endl; LgError("vs", _ostr)
	#define LtfsLogFatal(_ostr) cout << _ostr << endl; LgFatal("vs", _ostr)
	#define LtfsEvent(level, eventId, msg) cout << msg << endl; CmnEvent("vs", level, eventId, msg)
#else
	#define LtfsLogDebug(_ostr) LgDebug("vs", _ostr)
	#define LtfsLogInfo(_ostr) LgInfo("vs", _ostr)
	#define LtfsLogWarn(_ostr) LgInfo("vs", _ostr)
	#define LtfsLogError(_ostr) LgError("vs", _ostr)
	#define LtfsLogFatal(_ostr) LgFatal("vs", _ostr)
	#define LtfsEvent(level, eventId, msg) CmnEvent("vs", level, eventId, msg)
#endif
#endif

	struct LSSCSI_INFO
	{
	public:
		LSSCSI_INFO()
		{
			scsiAddr = "";
			vendor = "";
			product = "";
			version = "";
			stDev = "";
			sgDev = "";
		}
		string	scsiAddr;
		string	vendor;
		string	product;
		string	version;
		string	stDev;
		string	sgDev;
	};

	typedef enum {
	  HOST_WRITE,
	  HOST_READ,
	  NO_TRANSFER,
	  UNKNOWN_DIRECTION
	} direction;

	typedef struct {
		int			   		fd;
		unsigned char	   	cdb[16];
		int			   		cdb_length;
		unsigned char	  	*data;
		int			   		data_length;
		direction		   	data_direction;
		int			   		actual_data_length;
		unsigned char	   	sensedata[128];
		int			   		sense_length;
		int			   		timeout_ms;
		SCSI_SENSE_CODE		senseCode;
	} scsi_cmd_io;

	void DebugPrintScsiCommand(scsi_cmd_io *ioCmd);
	void DebugPrintScsiCommandResult(scsi_cmd_io *ioCmd);

	int ExecScsiCommand(scsi_cmd_io *ioCmd, int retry = 0, int retryWait = 1);

	vector<string> GetCommandOutputLines(const string& cmd, int& status, int& runPid, int timeOut, bool bTotalTimeOut = false);
	vector<string> GetCommandOutputLines(const string& cmd, int& status, int timeOut, bool bTotalTimeOut);
	vector<string> GetCommandOutputLines(const string& cmd, int timeOut = 30, bool bTotalTimeOut = false);
	vector<string> PopenCmdOutputLines(const string& cmd);
	bool KillAll(pid_t pid);
	string GetCommandOutput(const string& cmd);

	int OpenDevice(const string& devName);
	int LockDevice(const string& devName);
	int UnlockDevice(int& lockFd);
	int CloseDevice(int& deviceFd);

	string GetStringFromBuf(unsigned char buf[], int len, bool withEmpty = false);
	string GetDriveSerial(const string& sgDev);
	bool GetLsscsiDrive(LSSCSI_INFO& info, const string& serial);
	string GetStDeviceFromSCSIAddr(const string& scsiAddr);

	bool RemoveFile(const fs::path removePath);
	fs::path GetLtfsFolder();
	bool IsMountPointMounted(const string& mountPoint);
	bool IsDriveMounted(const string& driveStDevName, const string& barcode = "");

	struct tapeElement{
		int 					mSlotID;
		int 					mLogicSlotID;
		TapeMediumType			mMediumType;
		string 					mBarcode;
	};
	struct driveElement{
		LSSCSI_INFO				mScsiInfo;
		int 					mSlotID;
		int 					mLogicSlotID;
		DriveStatus 			mStatus;
		DriveCleaningStatus 	mCleaningStatus;
		DriveInterfaceType 		mInterfaceType;
		bool 					mIsEmpty;
		bool					mAccessible;
		bool					mAbnormal;
		bool					mIsFullHight;      // the drive is full height or not
		int						mGeneration;      // lto generation of the drive, 1, 2, 3, 4, 5, 6
		string 					mBarcode;
		string					mSerial;
	public:
		driveElement()
		{
			mSlotID = 0;
			mLogicSlotID = 0;
			mStatus = DRIVE_STATUS_UNKNOWN;
			mCleaningStatus = CLEANING_UNKNOWN;
			mInterfaceType = INTERFACE_UNKNOWN;
			mIsEmpty = true;
			mAccessible = true;
			mAbnormal = false;
			mIsFullHight = false;      // the drive is full height or not
			mGeneration = 0;      // lto generation of the drive, 1, 2, 3, 4, 5, 6
			mBarcode = "";
			mSerial = "";
		}
	};
	struct slotElement{
		int 					mSlotID;
		int 					mLogicSlotID;
		bool 					mIsEmpty;
		bool					mAccessible;
		bool					mAbnormal;
		string					mBarcode;
	};
	struct mailSlotElement{
		int 					mSlotID;
		int 					mLogicSlotID;
		bool 					mIsEmpty;
		bool					mAccessible;
		bool					mAbnormal;
		bool					mIsOpen;   // the mail slot is open or not
		string					mBarcode;
	};
	struct ChangerInfo_{
		LSSCSI_INFO				mScsiInfo;
		ChangerStatus 			mStatus;
		unsigned long			mDriveStart;
		unsigned long 			mSlotStart;
		unsigned long 			mMailSlotStart;
		vector<driveElement> 	drives;
		vector<tapeElement> 	tapes;
		vector<slotElement> 	slots;
		vector<mailSlotElement> mailSlots;
		string					mSerial;
		bool 					mAutoCleanMode;		// auto clean mode for the changer is enabled or not
	};

	enum CHECK_TAPE_FLAG{
		FLAG_NO_RECOVERY = 0,
		FLAG_FULL_RECOVERY,
		FLAG_DEEP_RECOVERY
	};

	bool ChangerPreventMediaRemoval(const string& changerDevName, bool bPrevent);
	bool GetChangerList(vector<LSSCSI_INFO>& changers, LtfsError& error);
	bool GetChanger(const LSSCSI_INFO& info, ChangerInfo_& changer, LtfsError& error, bool bDriveScsi = false);
	bool GetChangerAutoCleanMode(const string& changerDevName, bool& bAutoClean, LtfsError& lfsErr);
	bool GetDriveInfo(const string& driveSerial, LSSCSI_INFO& scsiInfo, DriveStatus& driveStatus,
			DriveCleaningStatus& cleaningStatus, DriveInterfaceType& interfaceType);
	bool GetDriveStatus(const string& driveDevName, DriveStatus& driveStatus, DriveCleaningStatus& cleaningStatus);
	bool MoveCartridge(const string& devName, int srcSlotId, int dstSlotId, LtfsError& error);
	bool Mount(const string& barcode, const string& driveStDevName, LtfsError& error);
	bool CheckTape(const string& driveStDevName, const string& barcode, CHECK_TAPE_FLAG flag, LtfsError& error);
	bool UnMount(const string& barcode, const string& driveStDevName, LtfsError& error, const string& mountPath = "");
	bool Format(const string& driveStDevName, const string& barcode, LtfsError& error);
	bool UnFormat(const string& driveDevName, const string& driveStDevName, const string& barcode, LtfsError& error);
	bool GetLoadedTapeLtfsFormat(const string& driveDevName, const string& driveStDevName, LTFS_FORMAT& ltfsFormat, LtfsError& error);
	bool SetLoadedTapeLtfsFormat(const string& driveDevName, const string& driveStDevName, bool bLtfs, LtfsError& error);
	bool GetLoadedTapeLoadCounter(const string& driveDevName, const string& driveStDevName, UInt16_t& loadCounter, LtfsError& error);
	bool GetLoadedTapeMediaType(const string& driveDevName, const string& driveStDevName, TapeMediaType& mediaType, LtfsError& error);
	bool GetLoadedTapeMediumType(const string& driveDevName, const string& driveStDevName, TapeMediumType& mediumType, LtfsError& error);
	bool GetLoadedTapeWPFlag(const string& driveDevName, const string& driveStDevName, bool& bIsWP, LtfsError& error);
	bool GetLoadedTapeGenerationIndex(const string& driveDevName, const string& driveStDevName, UInt64_t& genIndex, LtfsError& error);
	bool GetLoadedTapeCapacity(const string& driveDevName,
			const string& driveStDevName, const string& barcode,
			Int64_t& freeCapacity, LtfsError& error, bool bIsLtfs = false);
	bool GetLoadedTapeCapacity(const string& driveDevName, const string& driveStDevName, const string& barcode, Int64_t& freeCapacity, Int64_t& totalCapacity, LtfsError& error, bool bIsLtfs = false);
	bool GetLoadedTapeStatus(const string& driveDevName, const string& driveStDevName, TapeStatus& tapeStatus, LtfsError& error);
	bool SetLoadedTapeStatus(const string& driveDevName, const string& driveStDevName, TapeStatus tapeStatus, LtfsError& error);
	bool GetLoadedTapeFaulty(const string& driveDevName, const string& driveStDevName, bool& bFaulty, LtfsError& error);
	bool SetLoadedTapeFaulty(const string& driveDevName, const string& driveStDevName, bool bFaulty, LtfsError& error);
	bool SetLoadedTapeGroup(const string& driveDevName, const string& driveStDevName, const string& uuid, LtfsError& error);
	bool GetLoadedTapeGroup(const string& driveDevName, const string& driveStDevName, string& uuid, LtfsError& error);
	bool SetLoadedTapeDualCopy(const string& driveDevName, const string& driveStDevName, const string& dualCopy, LtfsError& error);
	bool GetLoadedTapeDualCopy(const string& driveDevName, const string& driveStDevName, string& dualCopy, LtfsError& error);
	bool SetLoadedTapeUUID(const string& driveDevName, const string& driveStDevName, const string& uuid, LtfsError& error);
	bool GetLoadedTapeUUID(const string& driveDevName, const string& driveStDevName, string& uuid, LtfsError& error);
	bool SetLoadedTapeBarcode(const string& driveDevName, const string& driveStDevName, const string& barcode, LtfsError& error);
	bool GetLoadedTapeBarcode(const string& driveDevName, const string& driveStDevName, string& barcode, LtfsError& error, bool bForce = false);
	bool GetLoadedTapeAlertFlag(const string& driveDevName, const string& driveStDevName, TapeStatus &tapeStatus, bool& bFaulty, LtfsError& error, bool bForce = false);
	bool OpenMailSlot(const string& changerDevName, int mailSlotId, LtfsError& error);
	bool GetDriveInterfaceType(const string& driveDevName, DriveInterfaceType& interfaceType);
	bool GetChangerMailSlots(const string& changerDevName, vector<mailSlotElement>& mailSlots, LtfsError lfsErr);
	string GetTapeDiagnoseLog(const string& barcode);
	string GetDiagnoseTapeLogPath(const string& barcode);
	string GetMountedDriveMountPoint(const string& driveStDevName, const string& barcode);

	void DebugPrintChanger(const ChangerInfo_& changer, const string& indentStr = "");
	void DebugPrintDrive(const driveElement& drive, const string& indentStr = "");
	void DebugPrintTape(const tapeElement& tape, const string& indentStr = "");
	void DebugPrintSlot(const slotElement& slot, const string& indentStr = "");
	void DebugPrintMailSlot(const mailSlotElement& mailSlot, const string& indentStr = "");

}
