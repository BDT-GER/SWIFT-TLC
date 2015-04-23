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
 * LtfsError.h
 *
 *  Created on: Jul 30, 2012
 *      Author: Sam Chen
 */

#pragma once

#include "stdafx.h"

namespace ltfs_management
{
	typedef struct _scsi_sense_code{
		int senseCode; //Sense Key
		int ascCode;  //Additional Sense Code
		int ascqCode; //Additional Sense Code Qualifier
	public:
		_scsi_sense_code()
		{
			senseCode = 0;
			ascCode = 0;
			ascqCode = 0;
		}
	}SCSI_SENSE_CODE;


#define ERR_GENERAL		0
#define ERR_CHANGER		100
#define ERR_DRIVE		200

	// general definition
	const int ERR_NO_ERR 						= (ERR_GENERAL+0);
	const int ERR_FAILED 						= (ERR_GENERAL+1);
	const int ERR_PARAM_ERR						= (ERR_GENERAL+2);
	const int ERR_PARAM_CHANGER_SERIAL			= (ERR_GENERAL+3);
	const int ERR_PARAM_DRIVE_SERIAL			= (ERR_GENERAL+4);

	// changer
	const int ERR_CHANGER_NOT_READY 			= (ERR_CHANGER+0);
	const int ERR_NO_CHANGER 					= (ERR_CHANGER+1);
	const int ERR_CHANGER_DISCONNECT 			= (ERR_CHANGER+2);
	const int ERR_GET_CHANGER_LIST 				= (ERR_CHANGER+3);
	const int ERR_MOVE_DST_FULL 				= (ERR_CHANGER+4);
	const int ERR_DRIVE_PREVENT_REMOVE_MEDIUM 	= (ERR_CHANGER+5);
	const int ERR_CHANGER_MOVE_FAILED 			= (ERR_CHANGER+6);
	const int ERR_CHANGER_MISSING				= (ERR_CHANGER+7);
	const int ERR_MAIL_SLOT_OPEN				= (ERR_CHANGER+8);
	const int ERR_GET_AUTOCLEAN_MODE_FAIL 		= (ERR_CHANGER+9);
	const int ERR_OPEN_MAILSLOT_ALLOW_REMOVAL_FAIL = (ERR_CHANGER+10);

	// drive
	const int ERR_MOUNT_EMPTY 					= (ERR_DRIVE+0);
	const int ERR_MOUNT_CREATE_MOUNT_POINT 		= (ERR_DRIVE+1);
	const int ERR_MOUNT_FAIL 					= (ERR_DRIVE+2);
	const int ERR_UNMOUNT_EMPTY 				= (ERR_DRIVE+3);
	const int ERR_UNMOUNT_FAIL 					= (ERR_DRIVE+4);
	const int ERR_LOAD_UNLOAD_FAIL 				= (ERR_DRIVE+5);
	const int ERR_FORMAT_EMPTY 					= (ERR_DRIVE+6);
	const int ERR_FORMAT_FAIL 					= (ERR_DRIVE+7);
	const int ERR_DRIVE_GET_CAPACITY_FAIL 		= (ERR_DRIVE+8);
	const int UPDATE_TAPE_INFO_FAIL 			= (ERR_DRIVE+9);
	const int ERR_DRIVE_GET_LOAD_COUNTER_FAIL 	= (ERR_DRIVE+10);
	const int ERR_DRIVE_GET_MEDIA_TYPE_FAIL 	= (ERR_DRIVE+11);
	const int ERR_DRIVE_GET_MEDIA_STATUS_FAIL 	= (ERR_DRIVE+12);
	const int ERR_MOUNT_FUSE 					= (ERR_DRIVE+13);
	const int ERR_MOUNT_OPEN_DRIVE 				= (ERR_DRIVE+14);
	const int ERR_MOUNT_LOAD_TAPE 				= (ERR_DRIVE+15);
	const int ERR_UNMOUNT_NOT_MOUNTED 			= (ERR_DRIVE+16);
	const int ERR_UNMOUNT_BUSY 					= (ERR_DRIVE+17);
	const int ERR_FORMAT_NOT_READY 				= (ERR_DRIVE+18);
	const int ERR_DRIVE_GET_TAPE_GROUP_FAIL 	= (ERR_DRIVE+19);
	const int ERR_DRIVE_SET_TAPE_GROUP_FAIL 	= (ERR_DRIVE+20);
	const int ERR_CHANGER_SEQ_MODE 				= (ERR_DRIVE+21);
	const int ERR_CHANGER_MODE_PARAM_CHANGED 	= (ERR_DRIVE+22);
	const int ERR_CHANGER_TRANSFER_FULL 		= (ERR_DRIVE+23);
	const int ERR_CHANGER_PREVENT_REMOVE 		= (ERR_DRIVE+24);
	const int ERR_OPEN_MAILSLOT_FAIL 			= (ERR_DRIVE+25);
	const int ERR_MOVE_SRC_EMPTY 				= (ERR_DRIVE+26);
	const int ERR_FORMAT_WRITE_PROTECT 			= (ERR_DRIVE+27);
	const int ERR_MOUNT_MOUNTPOINT_NOT_EMPTY 	= (ERR_DRIVE+28);
	const int ERR_MOUNT_FORMAT_CHECK 			= (ERR_DRIVE+29);
	const int ERR_DRIVE_MOUNTED 				= (ERR_DRIVE+30);
	const int ERR_DRIVE_EMPTY 					= (ERR_DRIVE+31);
	const int ERR_DRIVE_SET_CLOSE_FLAG_FAIL 	= (ERR_DRIVE+32);
	const int ERR_DRIVE_GET_FAULTY_FLAG_FAIL 	= (ERR_DRIVE+33);
	const int ERR_DRIVE_SET_FAULTY_FLAG_FAIL 	= (ERR_DRIVE+34);
	const int ERR_DRIVE_GET_TAPE_GENERATION_INDEX_FAIL = (ERR_DRIVE+35);
	const int ERR_DRIVE_SET_TAPE_STATUS_FAIL 	= (ERR_DRIVE+36);
	const int ERR_DRIVE_GET_TAPE_STATUS_FAIL 	= (ERR_DRIVE+37);
	const int ERR_MOVE_SRC_DST_ID_SAME 			= (ERR_DRIVE+38);
	const int ERR_MOVE_SRC_ID_INVALID 			= (ERR_DRIVE+39);
	const int ERR_MOVE_DST_ID_INVALID 			= (ERR_DRIVE+40);
	const int ERR_MOUNT_EOD_MISSING 			= (ERR_DRIVE+41);
	const int ERR_MOUNT_UNSUPPORTED_MEDIUM 		= (ERR_DRIVE+42);
	const int ERR_MOUNT_READ_PARTITION_FAIL 	= (ERR_DRIVE+43);
	const int ERR_DRIVE_MISSING					= (ERR_DRIVE+44);
	const int ERR_DRIVE_GET_LTFS_FORMAT_FAIL	= (ERR_DRIVE+45);
	const int ERR_DRIVE_GET_CAPACITY_NOT_LTFS	= (ERR_DRIVE+46);
	/*const int ERR_FORMAT_TAPE_MOUNTED_BUSY		= (ERR_DRIVE+47);
	const int ERR_FORMAT_TAPE_NOT_SUPPORTED		= (ERR_DRIVE+48);
	const int ERR_FORMAT_NO_TAPE				= (ERR_DRIVE+49);*/
	const int ERR_DRIVE_GET_WP_FLAG_FAIL		= (ERR_DRIVE+50);
	const int ERR_CHECK_MEDIUM_INUSE			= (ERR_DRIVE+51);
	const int ERR_CHECK_FAILED					= (ERR_DRIVE+52);
	const int ERR_CHECK_NO_MEDIUM				= (ERR_DRIVE+53);
	const int ERR_CHECK_EMPTY					= (ERR_DRIVE+54);
	const int ERR_UN_FORMAT_EMPTY				= (ERR_DRIVE+55);
	const int ERR_DRIVE_GET_TAPE_DUALCOPY_FAIL	= (ERR_DRIVE+56);
	const int ERR_DRIVE_SET_TAPE_DUALCOPY_FAIL	= (ERR_DRIVE+57);
	const int ERR_UN_FORMAT_FAIL 				= (ERR_DRIVE+58);
	const int ERR_UN_FORMAT_WRITE_PROTECT 		= (ERR_DRIVE+59);
	const int ERR_UN_FORMAT_NO_TAPE				= (ERR_DRIVE+60);
	const int ERR_DRIVE_GET_TAPE_BARCODE_FAIL	= (ERR_DRIVE+61);
	const int ERR_DRIVE_SET_TAPE_BARCODE_FAIL	= (ERR_DRIVE+62);
	const int ERR_DRIVE_GET_TAPE_UUID_FAIL		= (ERR_DRIVE+63);
	const int ERR_DRIVE_SET_TAPE_UUID_FAIL		= (ERR_DRIVE+64);
	const int ERR_EJECT_TAPE_MOUNTED			= (ERR_DRIVE+65);
	const int ERR_LOAD_NO_TAPE					= (ERR_DRIVE+66);
	const int ERR_UNLOAD_NO_TAPE				= (ERR_DRIVE+67);
	const int ERR_UNLOAD_TAPE_PREVENT			= (ERR_DRIVE+68);
	const int ERR_LOAD_TAPE_FAIL				= (ERR_DRIVE+69);
	const int ERR_UNLOAD_TAPE_FAIL				= (ERR_DRIVE+70);
	const int ERR_DRIVE_GET_MEDIUM_TYPE_FAIL	= (ERR_DRIVE+71);


	enum PARAM_TYPE
	{
		PARAM_INT,
		PARAM_UL,
		PARAM_STRING
	};

	typedef struct _ErrorParameter
	{
		union
		{
			int					iValue;
			unsigned long long	ulValue;
			char				chValue[512];
		}value;

		PARAM_TYPE pType;

	public:
		void SetStringParameter(const string& strValue)
		{
			int len = (strValue.length() <= 511)? strValue.length() : 511;

			memcpy(value.chValue, strValue.c_str(), len);
			value.chValue[len] = '\0';
			pType = PARAM_STRING;
		};
	}ErrParameter;

    class LtfsError
    {
    public:
    	LtfsError();

        virtual
        ~LtfsError();

        int
        GetErrCode() const;

        void
        SetErrCode(int errCode);

        void
        SetErrParams(const vector<ErrParameter>& params);

        vector<ErrParameter>
        GetErrParams() const;

        void
        ClearParams();

        void
        AddIntParam(int value);

        void
        AddUllParam(unsigned long long value);

        void
        AddStringParam(const string& value);

        string
        GetErrMsg() const;

        SCSI_SENSE_CODE GetSenseCode() const;
        void SetSenseCode(const SCSI_SENSE_CODE& senseCode);
        bool IsSenseCodeValid() const;

        bool
        IsErrRecoverable() const;

        void
        SetErrorRecoverable(bool bRecoverable);

    private:
    	int						m_errCode;
    	vector<ErrParameter>	m_params;
    	SCSI_SENSE_CODE			m_senseCode;
    	bool					m_validSenseCode;
    	bool					m_bErrRecoverable;
    };
}

