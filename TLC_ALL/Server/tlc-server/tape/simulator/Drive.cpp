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
 *  Created on: Jul 2, 2012
 *      Author: More Zeng
 */

#include "../stdafx.h"
#include "../Drive.h"


#include "Factory.h"


namespace tape
{

    struct Drive::Detail
    {
        int changer;
        int drive;

        DriveImpl *
        GetDriveImpl(Error & error)
        {
            DriveImpl * impl = Factory::Instance()->GetDrive(changer,drive);
            if ( NULL == impl ) {
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such drive ")
                        + boost::lexical_cast<string>(drive)
                        + " in changer "
                        + boost::lexical_cast<string>(changer) );
            }
            return impl;
        }

        bool
        CheckCartridge(string & barcode)
        {
            DriveImpl * impl = Factory::Instance()->GetDrive(changer,drive);

            if ( impl->Empty() ) {
                return false;
            }

            barcode = impl->Barcode();
            if ( barcode.empty() ) {
                return false;
            }

            return true;
        }


    };


    Drive::Drive(const string & setting)
    : detail_(new Detail())
    {
        istringstream is(setting);
        int changer;
        int drive;
        is >> changer >> drive;

        detail_->changer = changer;
        detail_->drive = drive;
    }


    Drive::Drive(const Drive & drive)
    : detail_(new Detail())
    {
        detail_->changer = drive.detail_->changer;
        detail_->drive = drive.detail_->drive;
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
        SlotImpl * impl = detail_->GetDriveImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        slotID = impl->SlotID();
        return true;
    }


    bool
    Drive::GetEmpty(bool & empty, Error & error) const
    {
        SlotImpl * impl = detail_->GetDriveImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        empty = impl->Empty();
        return true;
    }


    bool
    Drive::GetBarcode(string & barcode, Error & error) const
    {
        SlotImpl * impl = detail_->GetDriveImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        barcode = impl->Barcode();
        return true;
    }


    fs::path
    Drive::GetLTFSFolder()
    {
        return DriveImpl::GetLTFSFolder();
    }


    bool
    Drive::Mount(Error & error)
    {
        DriveImpl * impl = detail_->GetDriveImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        return impl->Mount(error);
    }


    bool
    Drive::Unmount(Error & error)
    {
        DriveImpl * impl = detail_->GetDriveImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        return impl->Unmount(error);
    }


    bool
    Drive::Format(Error & error)
    {
        DriveImpl * impl = detail_->GetDriveImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        return impl->Format(error);
    }

}

