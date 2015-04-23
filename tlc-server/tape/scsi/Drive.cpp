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
 * Drive.cpp
 *
 *  Created on: Aug 9, 2012
 *      Author: Sam Chen
 */


#include "cmn.h"
#include "../Drive.h"
#include "../../ltfs_management/TapeLibraryMgr.h"


namespace tape
{
    struct Drive::Detail
    {
        string changerSetting;
        string setting;

        bool GetDrive(LtfsDriveInfo & drive, Error & error)
        {
            LtfsError err;

            vector<LtfsDriveInfo> drives;
            if ( ! LtfsLibraries::Instance()->GetDriveList(
                    changerSetting, drives, err ) ) {
                LtfsLogError("Failed to get drives from HAL: " << changerSetting);
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such drive: ") + setting );
                return false;
            }

            for ( vector<LtfsDriveInfo>::iterator i = drives.begin();
                    i != drives.end();
                    ++ i ) {
                if ( i->mSerial == setting ) {
                    drive = *i;
                    return true;
                }
            }

            LtfsLogError("No such drive in HAL: "
                    << changerSetting << ":" << setting);
            error.SetError(
                    Error::ERROR_NO_ENTRY,
                    string("No such drive: ") + setting );
            return false;
        }

    };


    Drive::Drive(const string & setting)
    : detail_(new Detail())
    {
        istringstream is(setting);
        string changerSetting;
        string tapeSetting;
        is >> changerSetting >> tapeSetting;

        detail_->setting = tapeSetting;
        detail_->changerSetting = changerSetting;
    }


    Drive::Drive(const Drive & drive)
    : detail_(new Detail())
    {
        detail_->setting = drive.detail_->setting;
        detail_->changerSetting = drive.detail_->changerSetting;
    }


    void
    Drive::Swap( Drive & drive)
    {
        swap( detail_, drive.detail_ );
    }


    Drive::~Drive()
    {
        delete detail_;
    }


    bool
    Drive::GetSlotID(int & slotID, Error & error) const
    {
        LtfsDriveInfo drive;
        if(!detail_->GetDrive(drive, error)){
            return false;
        }

        slotID = drive.mSlotID;
        return true;
    }


    bool
    Drive::GetEmpty(bool & empty, Error & error) const
    {
        LtfsError ltfsErr;
        string barcode;
        if ( LtfsLibraries::Instance()->GetLoadedTapeBarcode(
                detail_->changerSetting, detail_->setting,
                barcode, ltfsErr ) ) {
            empty = false;
            return true;
        } else {
            if(ltfsErr.GetErrCode() == ERR_DRIVE_EMPTY) {
                empty = true;
                return true;
            } else {
                return false;
            }
        }
    }


    bool
    Drive::GetBarcode(string & barcode, Error & error) const
    {
        LtfsError ltfsErr;
        if ( LtfsLibraries::Instance()->GetLoadedTapeBarcode(
                detail_->changerSetting, detail_->setting,
                barcode, ltfsErr ) ) {
            return true;
        } else {
            if(ltfsErr.GetErrCode() == ERR_DRIVE_EMPTY) {
                barcode.clear();
                return true;
            } else {
                return false;
            }
        }
    }


    fs::path
    Drive::GetLTFSFolder()
    {
        return fs::path("/opt/LTFStor/ltfsMounts");
    }


    bool
    Drive::Mount(Error & error)
    {
        LtfsError ltfsErr;

        bool mount;
        if( LtfsLibraries::Instance()->IsTapeMounted(
                detail_->changerSetting, detail_->setting, mount, ltfsErr ) ) {
            if ( ! mount ) {
#if 0
                ltfs_management::TapeLibraryMgr::Instance()->UpdateTape(
                        detail_->changerSetting, detail_->setting, ltfsErr );
#endif
            }
        } else {
            LtfsLogError("Failed to get tape mount from HAL: "
                    << detail_->changerSetting << " : " << detail_->setting);
            mount = false;
        }

        if ( ! LtfsLibraries::Instance()->Mount(
                detail_->changerSetting, detail_->setting, ltfsErr ) ) {
            ConvertError(error, ltfsErr);
            int code = ltfsErr.GetErrCode();
            string barcode;
            if (LtfsLibraries::Instance()->GetLoadedTapeBarcode(
                    detail_->changerSetting,
                    detail_->setting,
                    barcode,
                    ltfsErr ) ) {
                switch ( code ) {
                    case ERR_MOUNT_FORMAT_CHECK:
                    case ERR_MOUNT_EOD_MISSING:
                    case ERR_MOUNT_READ_PARTITION_FAIL:
                    case ERR_MOUNT_FAIL:
                        ltfs_management::TapeLibraryMgr::Instance()
                                ->SetTapeFaulty( barcode, true );
                        break;
                    default:
                        break;
                }
                LtfsEvent(EVENT_LEVEL_ERR, "Tape_Mount_Failed", "Failed to mount tape " << barcode << ".");
            } else {
                LtfsLogError("Failed to get tape from HAL: "
                        << detail_->changerSetting
                        << " : " << detail_->setting);
            }
            return false;
        }

        if ( ! mount ) {
            ltfs_management::TapeLibraryMgr::Instance()->UpdateTape(
                    detail_->changerSetting, detail_->setting, ltfsErr );
        }

        return true;
    }


    bool
    Drive::Unmount(Error & error)
    {
        using namespace ltfs_management;

        LtfsDriveInfo drive;
        if(!detail_->GetDrive(drive, error)){
            return false;
        }

        LtfsError ltfsErr;

        ltfs_management::TapeLibraryMgr::Instance()->UpdateTape(
                detail_->changerSetting, detail_->setting, ltfsErr );

        string barcode = "";
        if(false == LtfsLibraries::Instance()->GetLoadedTapeBarcode(
                detail_->changerSetting,
                detail_->setting,
                barcode,
                ltfsErr )){
            LtfsLogError("Failed to get tape from HAL: "
                    << detail_->changerSetting
                    << " : " << detail_->setting);
        }

		UInt64_t genIndex = 0;
        if ( ! LtfsLibraries::Instance()->UnMount(
                detail_->changerSetting, detail_->setting, genIndex, ltfsErr ) ) {
            ConvertError(error, ltfsErr);
            if(barcode != ""){
                LtfsEvent(EVENT_LEVEL_ERR, "Tape_Unmount_Failed", "Failed to unmount tape " << barcode << ".");  //Event/Notification
            }
            return false;
        }
        // update tape generation index to database
        if(genIndex > 0 ){
        	if("" != barcode){
        		ltfs_management::TapeLibraryMgr::Instance()->UpdateTapeGeneraionIndex(barcode, genIndex);
        	}
        }

        TapeInfo tapeInfo;
    	if(false == ltfs_management::TapeLibraryMgr::Instance()->GetTape(barcode, tapeInfo)){
			LtfsLogError("Failed to get tape info for tape to update status to MAM for tape " << barcode << ".");
    	}else{
			// set tape status to MAM
			if(false == LtfsLibraries::Instance()->SetLoadedTapeStatus(detail_->changerSetting, detail_->setting, tapeInfo.mStatus, ltfsErr)){
				LtfsLogError("Failed to update tape status " << tapeInfo.mStatus << " for tape " << barcode << " to MAM.");
			}else{
				LtfsLogInfo("Succeed to update tape status " << tapeInfo.mStatus << " for tape " << barcode << " to MAM.");
			}
    	}

        int cleaning;
        if ( LtfsLibraries::Instance()->GetDriveCleaningStatus(
                detail_->changerSetting, detail_->setting,
                cleaning, ltfsErr, true ) ) {
        	// check if we need to fire the event
        	LtfsLibraries::Instance()->CheckDriveCleaningEvent(detail_->changerSetting, detail_->setting, cleaning);
            if ( CLEANING_REQUIRED == cleaning ) {
                //LtfsLogWarn("Drive " << drive.mLogicSlotID
                //        << " requires cleaning.");
                ltfs_management::TapeLibraryMgr::Instance()->StartDriveCleaningTask(detail_->changerSetting, detail_->setting);
            }
        }

        return true;
    }


    bool
    Drive::Format(Error & error)
    {
        LtfsError ltfsErr;
        if( ! LtfsLibraries::Instance()->Format(
                detail_->changerSetting, detail_->setting, ltfsErr ) ) {
            ConvertError(error, ltfsErr);
            int code = ltfsErr.GetErrCode();
            switch ( code ) {
                case ERR_FORMAT_FAIL:
                    string barcode;
                    if (LtfsLibraries::Instance()->GetLoadedTapeBarcode(
                            detail_->changerSetting,
                            detail_->setting,
                            barcode,
                            ltfsErr ) ) {
                        ltfs_management::TapeLibraryMgr::Instance()
                                ->SetTapeFaulty( barcode, true );
                    } else {
                        LtfsLogError("Failed to get tape from HAL: "
                                << detail_->changerSetting
                                << " : " << detail_->setting);
                    }
            }
            return false;
        }

        return true;
    }

}

