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
 * LtfsError.cpp
 *
 *  Created on: Jul 30, 2012
 *      Author: Sam Chen
 */

#include "stdafx.h"
#include "LtfsError.h"

namespace ltfs_management
{
	LtfsError::LtfsError()
	{
		m_params.clear();
		m_errCode = ERR_NO_ERR;
		m_bErrRecoverable = false;
	}

	LtfsError::~LtfsError()
	{
	}

    int
    LtfsError::GetErrCode() const
    {
    	return m_errCode;
    }

    void
    LtfsError::SetErrCode(int errCode)
    {
    	switch(errCode)
    	{
    	case ERR_CHANGER_NOT_READY:
    	case ERR_DRIVE_GET_CAPACITY_FAIL:
    	case ERR_CHANGER_MODE_PARAM_CHANGED:
    	case ERR_CHANGER_TRANSFER_FULL:
    		m_bErrRecoverable = true;
    		break;
    	default:
    		break;
    	}

    	m_errCode = errCode;
    }

    void
    LtfsError::SetErrParams(const vector<ErrParameter>& params)
    {
    	m_params = params;
    }

    vector<ErrParameter>
    LtfsError::GetErrParams() const
    {
    	return m_params;
    }

    string
    LtfsError::GetErrMsg() const
    {
    	string errMsg = "";

    	switch(m_errCode)
    	{
    	//general
    	case ERR_NO_ERR:
    		errMsg = "No error.";
    		break;
    	case ERR_FAILED:
    		errMsg = "Action failed";
    		break;
    	//changer
    	case ERR_CHANGER_NOT_READY:
    		errMsg = "Changer ({0}) is not ready to move medium.";
    		break;
    	case ERR_NO_CHANGER:
    		errMsg = "No changer is found.";
    		break;
    	case ERR_CHANGER_DISCONNECT:
    		errMsg = "Changer disconnected.";
    		break;
    	case ERR_GET_CHANGER_LIST:
    		errMsg = "Exception on getting changer list.";
    		break;
    	case ERR_MOVE_DST_FULL:
    		errMsg = "Medium destination element {0} is full.";
    		break;
    	case ERR_MOVE_SRC_EMPTY:
    		errMsg = "Medium source element empty.";
    		break;
    	case ERR_DRIVE_PREVENT_REMOVE_MEDIUM:
    		errMsg = "Drive media removal prevented state set.";
    		break;
    	case ERR_MAIL_SLOT_OPEN:
    		errMsg = "Failed to move tape to Mail slot {0}. Mail slot is already open, please close it first.";
    		break;
    	case ERR_GET_AUTOCLEAN_MODE_FAIL:
    		errMsg = "Failed to open mail slot. Failed to allow media removal for the changer.";
    		break;
    	case ERR_OPEN_MAILSLOT_ALLOW_REMOVAL_FAIL:
    		errMsg = "Failed to get auto clean mode for changer {0}.";
    		break;
    	case ERR_CHANGER_MOVE_FAILED:
    		errMsg = "Failed to move medium from slot {0} to slot {1}.";
    		break;
    	//drive
    	case ERR_MOUNT_EMPTY:
    		errMsg = "No medium in drive or medium has no barcode. Will not mount.";
    		break;
    	case ERR_MOUNT_CREATE_MOUNT_POINT:
    		errMsg = "Failed to create mount point to mount medium {0}.";
    		break;
    	case ERR_MOUNT_FAIL:
    		errMsg = "Failed to mount medium {0}.";
    		break;
    	case ERR_UNMOUNT_EMPTY:
    		errMsg = "No medium in drive or medium has no barcode. Will not unmount.";
    		break;
    	case ERR_UNMOUNT_FAIL:
    		errMsg = "Failed to unmount medium {0}.";
    		break;
    	case ERR_LOAD_UNLOAD_FAIL:
    		errMsg = "Fail to load/unload drive.";
    		break;
    	case ERR_FORMAT_EMPTY:
    		errMsg = "No medium in drive or medium has no barcode. Will not format.";
    		break;
    	case ERR_UN_FORMAT_EMPTY:
    		errMsg = "No medium in drive or medium has no barcode. Will not un format.";
    		break;
    	case ERR_FORMAT_WRITE_PROTECT:
    		errMsg = "Failed to format tape {0}: medium is write protected.";
    		break;
    	case ERR_UN_FORMAT_WRITE_PROTECT:
    		errMsg = "Failed to unformat tape {0}: medium is write protected.";
    		break;
    	case ERR_UN_FORMAT_NO_TAPE:
    		errMsg = "Failed to unformat tape {0}: no medium present.";
    		break;
    	case ERR_FORMAT_FAIL:
    		errMsg = "Failed to format medium {0}.";
    		break;
    	case ERR_UN_FORMAT_FAIL:
    		errMsg = "Failed to unformat medium {0}.";
    		break;
    	case ERR_DRIVE_GET_CAPACITY_FAIL:
    		errMsg = "Failed to get capacity info of the loaded tape.";
    		break;
    	case ERR_DRIVE_GET_CAPACITY_NOT_LTFS:
    		errMsg = "Tape {0} is a valid LTFS format tape and it is not mounted, will not get free size using SCSI command.";
    		break;
    	case UPDATE_TAPE_INFO_FAIL:
    		errMsg = "Failed to update information of medium {0}.";
    		break;
    	case ERR_DRIVE_GET_LOAD_COUNTER_FAIL:
    		errMsg = "Failed to get load counter of the loaded tape.";
    		break;
    	case ERR_DRIVE_GET_MEDIA_TYPE_FAIL:
    		errMsg = "Failed to get media type of the loaded tape.";
    		break;
    	case ERR_DRIVE_GET_WP_FLAG_FAIL:
    		errMsg = "Failed to get write protect flag of the loaded tape.";
    		break;
    	case ERR_DRIVE_GET_MEDIA_STATUS_FAIL:
    		errMsg = "Failed to get status of the loaded tape.";
    		break;
    	case ERR_MOUNT_FUSE:
    		errMsg = "Failed to mount the medium {0}. Fuse not started.";
    		break;
    	case ERR_MOUNT_OPEN_DRIVE:
    		errMsg = "Failed to mount the medium {0}. Failed to open drive device {1}.";
    		break;
    	case ERR_MOUNT_LOAD_TAPE:
    		errMsg = "Failed to mount the medium {0}. Failed to load tape.";
    		break;
    	case ERR_MOUNT_MOUNTPOINT_NOT_EMPTY:
    		errMsg = "Failed to mount the medium {0}: mountpoint is not empty.";
    		break;
    	case ERR_MOUNT_FORMAT_CHECK:
    		errMsg = "Failed to mount the medium {0}. Please format the medium or check it.";
    		break;
    	case ERR_UNMOUNT_BUSY:
    		errMsg = "Failed to umount the tape {0}. Device busy.";
    		break;
    	case ERR_MOUNT_READ_PARTITION_FAIL:
    		errMsg = "Failed to mount the tape {0}. Failed to read partition labels.";
    		break;
    	case ERR_MOUNT_EOD_MISSING:
    		errMsg = "Failed to mount the medium {0}. EOD of IP(0) is missing. The deep recovery is required.";
    		break;
    	case ERR_MOUNT_UNSUPPORTED_MEDIUM:
    		errMsg = "Failed to mount the tape {0}. Unsupported cartridge type.";
    		break;
    	case ERR_UNMOUNT_NOT_MOUNTED:
    		errMsg = "Failed to umount the tape {0}. Tape is not mounted.";
    		break;
    	case ERR_FORMAT_NOT_READY:
    		errMsg = "Failed to format the tape {0}. Resource temporarily unavailable.";
    		break;
    	case ERR_DRIVE_GET_TAPE_GROUP_FAIL:
    		errMsg = "Failed to get label for loaded tape in drive {0}.";
    		break;
    	case ERR_DRIVE_GET_LTFS_FORMAT_FAIL:
    		errMsg = "Failed to get ltfs format for loaded tape in drive {0}.";
    		break;
    	case ERR_DRIVE_SET_TAPE_GROUP_FAIL:
    		errMsg = "Failed to set label for loaded tape in drive {0}.";
    		break;
    	case ERR_DRIVE_SET_TAPE_DUALCOPY_FAIL:
    		errMsg = "Failed to set dual copy for loaded tape in drive {0}.";
    		break;
    	case ERR_DRIVE_GET_TAPE_DUALCOPY_FAIL:
    		errMsg = "Failed to get dual copy for loaded tape in drive {0}.";
    		break;
    	case ERR_DRIVE_SET_TAPE_BARCODE_FAIL:
    		errMsg = "Failed to set barcode for loaded tape in drive {0}.";
    		break;
    	case ERR_DRIVE_GET_TAPE_BARCODE_FAIL:
    		errMsg = "Failed to get barcode for loaded tape in drive {0}.";
    		break;
    	case ERR_DRIVE_SET_TAPE_UUID_FAIL:
    		errMsg = "Failed to set tape uuid for loaded tape in drive {0}.";
    		break;
    	case ERR_DRIVE_GET_TAPE_UUID_FAIL:
    		errMsg = "Failed to get tape uuid for loaded tape in drive {0}.";
    		break;
    	case ERR_CHANGER_SEQ_MODE:
    		errMsg = "Not ready, the media changer is in sequential mode.";
    		break;
    	case ERR_CHANGER_MODE_PARAM_CHANGED:
    		errMsg = "Changer mode parameters changed.";
    		break;
    	case ERR_CHANGER_TRANSFER_FULL:
    		errMsg = "Medium transfer element full.";
    		break;
    	case ERR_CHANGER_PREVENT_REMOVE:
    		errMsg = "Library media removal prevented state set";
    		break;
    	case ERR_OPEN_MAILSLOT_FAIL:
    		errMsg = "Failed to open mail slot.";
    		break;
    	case ERR_DRIVE_MOUNTED:
    		errMsg = "Drive {0} has mounted a tape. Operation on it is not permitted.";
    		break;
    	case ERR_CHECK_MEDIUM_INUSE:
    		errMsg = "Failed to run check tape on drive {0}. Medium is already mounted or in use.";
    		break;
    	case ERR_CHECK_NO_MEDIUM:
    		errMsg = "Failed to run check tape on drive {0}. No tape in drive.";
    		break;
    	case ERR_CHECK_FAILED:
    		errMsg = "Failed to run check tape on drive {0}.";
    		break;
    	case ERR_CHECK_EMPTY:
    		errMsg = "Failed to run check, no tape barcode is specified.";
    		break;
    	case ERR_DRIVE_EMPTY:
    		errMsg = "Drive is empty.";
    		break;
    	case ERR_DRIVE_SET_CLOSE_FLAG_FAIL:
    		errMsg = "Failed to update close flag for tape in drive {0}.";
    		break;
    	case ERR_DRIVE_SET_FAULTY_FLAG_FAIL:
    		errMsg = "Failed to update faulty flag for tape in drive {0}.";
    		break;
    	case ERR_DRIVE_GET_FAULTY_FLAG_FAIL:
    		errMsg = "Failed to get faulty flag for tape in drive {0}.";
    		break;
    	case ERR_DRIVE_GET_TAPE_GENERATION_INDEX_FAIL:
    		errMsg = "Failed to get generation index for tape in drive {0}.";
    		break;
    	case ERR_DRIVE_SET_TAPE_STATUS_FAIL:
    		errMsg = "Failed to update status for tape in drive {0}.";
    		break;
    	case ERR_DRIVE_GET_TAPE_STATUS_FAIL:
    		errMsg = "Failed to get status for tape in drive {0}.";
    		break;
    	case ERR_MOVE_SRC_DST_ID_SAME:
    		errMsg = "Source and destination ids are the same.";
    		break;
    	case ERR_MOVE_SRC_ID_INVALID:
    		errMsg = "Source slot id {0} is not valid.";
    		break;
    	case ERR_MOVE_DST_ID_INVALID:
    		errMsg = "Destination slot id {0} is not valid.";
    		break;
    	case ERR_EJECT_TAPE_MOUNTED:
    		errMsg = "EjectTape tape {0} failed. Tape is already mounted.";
    		break;
    	case ERR_LOAD_NO_TAPE:
    		errMsg = "LoadTape tape failed. No tape found.";
    		break;
    	case ERR_UNLOAD_NO_TAPE:
    		errMsg = "UnLoadTape tape failed. No tape found.";
    		break;
    	case ERR_UNLOAD_TAPE_PREVENT:
    		errMsg = "UnLoadTape tape {0} failed. Medium removal prevented.";
    		break;
    	case ERR_LOAD_TAPE_FAIL:
    		errMsg = "LoadTape tape failed.";
    		break;
    	case ERR_UNLOAD_TAPE_FAIL:
    		errMsg = "UnLoadTape tape failed.";
    		break;
    	case ERR_DRIVE_GET_MEDIUM_TYPE_FAIL:
    		errMsg = "Failed to get medium type for loaded tape in drive {0}.";
    		break;
    	default:
    		break;
    	}

    	for(unsigned int i = 0; i < m_params.size(); i++)
    	{
			string findStr = "{" + boost::lexical_cast<string>(i) + "}";
			int pos = errMsg.find(findStr);
			if(pos != -1)
			{
				string repStr = "";
				switch(m_params[i].pType)
				{
				case PARAM_INT:
					repStr = boost::lexical_cast<string>(m_params[i].value.iValue);
					break;
				case PARAM_UL:
					repStr = boost::lexical_cast<string>(m_params[i].value.ulValue);
					break;
				case PARAM_STRING:
					repStr = boost::lexical_cast<string>(m_params[i].value.chValue);
					break;
				}
				errMsg = errMsg.replace(pos, findStr.length(), repStr);
			}
    	}
    	return errMsg;
    }


    SCSI_SENSE_CODE LtfsError::GetSenseCode() const
    {
    	return m_senseCode;
    }
    void LtfsError::SetSenseCode(const SCSI_SENSE_CODE& senseCode)
    {
    	m_senseCode = senseCode;
    	m_validSenseCode = true;
    }
    bool LtfsError::IsSenseCodeValid() const
    {
    	return m_validSenseCode;
    }

    void LtfsError::ClearParams()
    {
    	m_params.clear();
    }

    void LtfsError::AddIntParam(int value)
    {
    	ErrParameter param;
    	param.pType = PARAM_INT;
    	param.value.iValue = value;
    	m_params.push_back(param);
    }

    void LtfsError::AddUllParam(unsigned long long value)
    {
    	ErrParameter param;
    	param.pType = PARAM_UL;
    	param.value.ulValue = value;
    	m_params.push_back(param);
    }

    void LtfsError::AddStringParam(const string& value)
    {
    	ErrParameter param;
    	param.SetStringParameter(value);
    	m_params.push_back(param);
    }

    bool LtfsError::IsErrRecoverable() const
    {
    	return m_bErrRecoverable;
    }
    void LtfsError::SetErrorRecoverable(bool bRecoverable)
    {
    	m_bErrRecoverable = bRecoverable;
    }
}
