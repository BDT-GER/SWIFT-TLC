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
 * Changer.cpp
 *
 *  Created on: Aug 9, 2012
 *      Author: Sam Chen
 */


#include "cmn.h"
#include "../Changer.h"
#include "../../ltfs_management/TapeLibraryMgr.h"


namespace tape
{
    struct Changer::Detail
    {
        string setting;

        bool GetChanger(LtfsChangerInfo & changer, Error & error)
        {
            LtfsError err;
            vector<LtfsChangerInfo> changers;
            if ( ! LtfsLibraries::Instance()->GetChangerList(changers,err) ) {
                LtfsLogError("Failed to get changers from HAL");
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such changer: ") + setting );
                return false;
            }
            for ( vector<LtfsChangerInfo>::iterator i = changers.begin();
                    i != changers.end();
                    ++ i ) {
                if ( i->mSerial == setting ) {
                    changer = *i;
                    return true;
                }
            }
            LtfsLogError("No such changer in HAL: " << setting);
            error.SetError(
                    Error::ERROR_NO_ENTRY,
                    string("No such changer: ") + setting );
            return false;
        }

        bool GetDrive(LtfsDriveInfo & drive, int slotID, Error & error)
        {
            LtfsError err;
            vector<LtfsDriveInfo> drives;
            if ( ! LtfsLibraries::Instance()->GetDriveList(setting,drives,err) ) {
                LtfsLogError("Failed to get drives from HAL: " << setting);
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No drives for changer: ") + setting );
                return false;
            }
            for ( vector<LtfsDriveInfo>::iterator i = drives.begin();
                    i != drives.end();
                    ++ i ) {
                if ( i->mSlotID == slotID ) {
                    drive = *i;
                    return true;
                }
            }
            LtfsLogError("No such drive in HAL: "
                    << setting << ":" << slotID);
            error.SetError(
                    Error::ERROR_NO_ENTRY,
                    string("No such drive for changer: ") + setting
                    + " " + boost::lexical_cast<string>(slotID) );
            return false;
        }

    };


    Changer::Changer(const string & setting)
    : detail_(new Changer::Detail())
    {
        detail_->setting = setting;
    }


    Changer::Changer(const Changer & changer)
    : detail_(new Changer::Detail())
    {
        detail_->setting = changer.detail_->setting;
    }


    void
    Changer::Swap( Changer & changer)
    {
        swap( detail_, changer.detail_ );
    }


    Changer::~Changer()
    {
        delete detail_;
    }


    bool
    Changer::GetDriveList(vector<Drive> & drives, Error & error) const
    {
        drives.clear();

        LtfsError err;
        vector<LtfsDriveInfo> infos;
        if(LtfsLibraries::Instance()->GetDriveList(detail_->setting,infos,err)){
            LtfsLogDebug("drive number: " << infos.size());
            for(vector<LtfsDriveInfo>::iterator i = infos.begin();
                    i != infos.end();
                    ++ i ) {
                if ( i->mStatus == DRIVE_STATUS_OK ) {
                    LtfsLogDebug("Add drive " << i->mSlotID << " : " << i->mSerial);
                    drives.push_back(detail_->setting + " " + i->mSerial);
                }
            }
        }else{
            LtfsLogError("Failed to get drives from HAL:" << detail_->setting);
            ConvertError(error, err);
            return false;
        }

        return true;
    }


    bool
    Changer::GetSlotList(vector<Slot> & slots, Error & error) const
    {
        slots.clear();

        LtfsError err;
        vector<LtfsSlotInfo> infos;
        if(LtfsLibraries::Instance()->GetSlotList(detail_->setting,infos,err)){
            LtfsLogDebug("slot number: " << infos.size());
            for(vector<LtfsSlotInfo>::iterator i = infos.begin();
                    i != infos.end();
                    ++ i ) {
                LtfsLogDebug("Add slot " << i->mSlotID << " : " << i->mBarcode);
                slots.push_back(detail_->setting + " " + boost::lexical_cast<string>(i->mSlotID));
            }
        }else{
            LtfsLogError("Failed to get slots from HAL:" << detail_->setting);
            ConvertError(error, err);
            return false;
        }

        vector<LtfsMailSlotInfo> mailslots;
        if(LtfsLibraries::Instance()->GetMailSlotList(detail_->setting,mailslots,err)){
            LtfsLogDebug("mailslot number: " << mailslots.size());
            for(vector<LtfsMailSlotInfo>::iterator i = mailslots.begin();
                    i != mailslots.end();
                    ++ i ) {
                if ( i->mIsOpen ) {
                    LtfsLogDebug("Ignore opened mailslot " << i->mSlotID << " : " << i->mBarcode);
                    continue;
                }
                LtfsLogDebug("Add mailslot " << i->mSlotID << " : " << i->mBarcode);
                slots.push_back(detail_->setting + " " + boost::lexical_cast<string>(i->mSlotID));
            }
        }else{
            LtfsLogError("Failed to get mailslots from HAL:" << detail_->setting);
            ConvertError(error, err);
            return false;
        }

        return true;
    }


    bool
    Changer::GetCartridgeList(vector<Cartridge> & cartridges, Error & error) const
    {
        cartridges.clear();

        LtfsError err;
        vector<LtfsTapeInfo> infos;
        if ( LtfsLibraries::Instance()->GetTapeList( detail_->setting, infos, err ) ) {
            LtfsLogDebug("tape number: " << infos.size());
            for(vector<LtfsTapeInfo>::iterator i = infos.begin();
                    i != infos.end();
                    ++ i ) {
                LtfsLogDebug("Add tape " << i->mSlotID << " : " << i->mBarcode);
                cartridges.push_back(detail_->setting + " " + i->mBarcode);
            }
        }else{
            LtfsLogError("Failed to get cartridges from HAL:" << detail_->setting);
            ConvertError(error, err);
            return false;
        }

        return true;
    }


    bool
    Changer::MoveCartridge(
            Slot & slot,
            Cartridge & cartridge,
            Error & error)
    {
        int slotIDSrc;
        if ( !cartridge.GetSlotID(slotIDSrc,error) ) {
            LtfsLogError("Failed to get slot ID from HAL");
            return false;
        }
        int slotIDDst;
        if ( ! slot.GetSlotID(slotIDDst,error) ) {
            LtfsLogError("Failed to get slot ID from HAL");
            return false;
        }
        string barcode;
        if ( ! cartridge.GetBarcode(barcode,error) ) {
            LtfsLogError("Failed to get barcode from HAL");
        }

        string driveSerial = "";
        //LtfsLogDebug("LtfsLibraries::Instance()->GetSlotType(detail_->setting, slotIDSrc) = " << LtfsLibraries::Instance()->GetSlotType(detail_->setting, slotIDSrc) << ".");
        //LtfsLogDebug("LtfsLibraries::Instance()->GetSlotType(detail_->setting, slotIDDst) = " << LtfsLibraries::Instance()->GetSlotType(detail_->setting, slotIDDst) << ".");
        if(LtfsLibraries::Instance()->GetSlotType(detail_->setting, slotIDSrc) == SLOT_DRIVE
        		&& LtfsLibraries::Instance()->GetSlotType(detail_->setting, slotIDDst) != SLOT_DRIVE){
        	driveSerial = LtfsLibraries::Instance()->GetDriveSerialBySlotId(detail_->setting, slotIDSrc);
        	if(driveSerial == ""){
        		LtfsLogError("Failed to get drive serial when move tape " << barcode << " out of drive in slot " << slotIDSrc << ".");
        	}
        }

        LtfsError err;
        if ( LtfsLibraries::Instance()->MoveCartridge(
                detail_->setting, slotIDSrc, slotIDDst, err ) ) {
        	LtfsLogDebug("driveSerial = " << driveSerial);
			if(driveSerial != ""){
	            int cleaning;
	            /*int driveLogicId = slotIDSrc;
	            LtfsDriveInfo drive;
	            if(true == ltfs_management::TapeLibraryMgr::Instance()->GetDrive(driveSerial, drive)){
	            	driveLogicId = drive.mLogicSlotID;
	            }*/
	            if ( LtfsLibraries::Instance()->GetDriveCleaningStatus(
	                    detail_->setting, driveSerial,
	                    cleaning, err, true ) ) {
	            	LtfsLogDebug("cleaning = " << cleaning << ", CLEANING_REQUIRED = " << CLEANING_REQUIRED << ".");
                	// check if we need to fire the event
	            	LtfsLibraries::Instance()->CheckDriveCleaningEvent(detail_->setting, driveSerial, cleaning);
	                if ( CLEANING_REQUIRED == cleaning ) {
	                    //LtfsLogWarn("Drive " << driveLogicId
	                    //        << " requires cleaning.");
	                    ltfs_management::TapeLibraryMgr::Instance()->StartDriveCleaningTask(detail_->setting, driveSerial);
	                }
	            }
			}
            return true;
        }else{
            LtfsLogError("Failed to move cartridge: "
                    << slotIDSrc << "->" << slotIDDst);
            ConvertError(error, err);
            int code = err.GetErrCode();
            switch ( code ) {
                case ERR_MOVE_DST_FULL:
                    LtfsEvent(EVENT_LEVEL_ERR, "Tape_Move_Failed", "Failed to move tape " << barcode << " from "
                            << slotIDSrc << " to " << slotIDDst
                            << ". Destination slot is not empty.");
                    break;
                default:
                    break;
            }
            switch ( code ) {
                case ERR_MOVE_DST_FULL:
                case ERR_MOVE_SRC_EMPTY:
                case ERR_MAIL_SLOT_OPEN:
                case ERR_CHANGER_TRANSFER_FULL:
                    LtfsLibraries::Instance()->Refresh(err);
                    break;
                default:
                    break;
            }
            return false;
        }

        return false;
    }


    bool
    Changer::LoadCartridge(
            Drive & drive,
            Cartridge & cartridge,
            Error & error)
    {
        int slotIDSrc;
        if ( ! cartridge.GetSlotID(slotIDSrc,error) ) {
            LtfsLogError("Failed to get slot ID from HAL");
            return false;
        }
        int slotIDDst;
        if ( ! drive.GetSlotID(slotIDDst,error) ) {
            LtfsLogError("Failed to get slot ID from HAL");
            return false;
        }

        LtfsError err;
        if ( LtfsLibraries::Instance()->MoveCartridge(
                detail_->setting, slotIDSrc, slotIDDst, err ) ) {
            LtfsDriveInfo infoDrive;
            if ( detail_->GetDrive(infoDrive,slotIDDst,error) ) {
                int format;
                if ( LtfsLibraries::Instance()->GetLoadedTapeLtfsFormat(
                        detail_->setting, infoDrive.mSerial, format, err )
                        && (format == LTFS_VALID) ) {
                } else {
                    ltfs_management::TapeLibraryMgr::Instance()->UpdateTape(
                            detail_->setting, infoDrive.mSerial, err );
                }
            } else {
                ltfs_management::TapeLibraryMgr::Instance()->UpdateTape(
                        detail_->setting, infoDrive.mSerial, err );
            }
            int cleaning;
            if ( LtfsLibraries::Instance()->GetDriveCleaningStatus(
                    detail_->setting, infoDrive.mSerial,
                    cleaning, err, true ) ) {
            	// check if we need to fire the event
            	LtfsLibraries::Instance()->CheckDriveCleaningEvent(detail_->setting, infoDrive.mSerial, cleaning);
                if ( CLEANING_REQUIRED == cleaning ) {
                    //LtfsLogWarn("Drive " << infoDrive.mLogicSlotID
                    //        << " requires cleaning.");
                    ltfs_management::TapeLibraryMgr::Instance()->StartDriveCleaningTask(detail_->setting, infoDrive.mSerial);
                }
            }

            //ltfs_management::TapeLibraryMgr::Instance()->UpdateTapeWPFlag(
            //        detail_->setting, infoDrive.mSerial, err );

            return true;
        }else{
            LtfsLogError("Failed to load cartridge: "
                    << slotIDSrc << "->" << slotIDDst);
            ConvertError(error, err);
            int code = err.GetErrCode();
            switch ( code ) {
                case ERR_MOVE_DST_FULL:
                case ERR_MOVE_SRC_EMPTY:
                case ERR_MAIL_SLOT_OPEN:
                case ERR_CHANGER_TRANSFER_FULL:
                    LtfsLibraries::Instance()->Refresh(err);
                    break;
                default:
                    break;
            }
            return false;
        }
    }

}

