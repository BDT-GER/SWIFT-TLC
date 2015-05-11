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
 * DriveImpl.h
 *
 *  Created on: Aug 6, 2012
 *      Author: More Zeng
 */

#pragma once

#include "SlotImpl.h"


namespace tape
{

    class DriveImpl : public SlotImpl
    {
    public:
        DriveImpl(int slotID)
        : SlotImpl(slotID)
        {
        }

        virtual
        ~DriveImpl()
        {
            Error error;
            Unmount(error);
        }

        static fs::path
        GetTapeFolder()
        {
            return fs::path("/opt/LTFStor/ltfsTapes");
        }

        static fs::path
        GetLTFSFolder()
        {
            return fs::path("/opt/LTFStor/ltfsTapes");
        }

        bool
        Mount(Error & error)
        {
            string barcode = Barcode();
            if ( barcode.empty() ) {
                error.SetError(
                        Error::ERROR_ENTRY_EMPTY,
                        string("No tape for format in drive ")
                        + boost::lexical_cast<string>(SlotID()) );
                return false;
            }

            fs::path pathTape = GetTapeFolder() / barcode;
            fs::path pathLTFS = GetLTFSFolder() / barcode;
            if ( ! fs::exists(pathTape) ) {
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No tape ")
                        + barcode
                        + " for format in drive "
                        + boost::lexical_cast<string>(SlotID()) );
                return false;
            }
            try {
                if ( ! fs::is_symlink(pathLTFS) ) {
                    if ( fs::exists( "/tmp/.bdt.mount_error") ) {
                        error.SetError(
                                Error::ERROR_NO_ENTRY,
                                "Force error for debug" );
                        return false;
                    }
                    fs::create_symlink(pathTape,pathLTFS);
                }
            } catch(...) {
                return false;
            }
            return true;
        }


        bool
        Unmount(Error & error)
        {
            string barcode = Barcode();
            if ( barcode.empty() ) {
                return true;
            }

            fs::path pathTape = GetTapeFolder() / barcode;
            fs::path pathLTFS = GetLTFSFolder() / barcode;
            if ( ! fs::exists(pathLTFS) ) {
                return true;
            }
            return fs::remove(pathLTFS);
        }


        bool
        Format(Error & error)
        {
            string barcode = Barcode();
            if ( barcode.empty() ) {
                error.SetError(
                        Error::ERROR_ENTRY_EMPTY,
                        string("No tape in drive ")
                        + boost::lexical_cast<string>(SlotID()) );
                return false;
            }

            fs::path pathTape = GetTapeFolder() / barcode;
            fs::path pathLTFS = GetLTFSFolder() / barcode;
            if ( fs::exists(pathLTFS) ) {
                error.SetError(
                        Error::ERROR_ENTRY_FULL,
                        string("Already mounting tape in drive ")
                        + boost::lexical_cast<string>(SlotID()) );
                return false;
            }
            if ( fs::exists(pathTape) ) {
                if ( ! fs::remove_all(pathTape) ) {
                    error.SetError(
                            Error::ERROR_ENTRY_FULL,
                            string("Fail to erase tape for format in drive ")
                            + boost::lexical_cast<string>(SlotID()) );
                    return false;
                }
            }
            return fs::create_directory(pathTape);
        }
    };

}

