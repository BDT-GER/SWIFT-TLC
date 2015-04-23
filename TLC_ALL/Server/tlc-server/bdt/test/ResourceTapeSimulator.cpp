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
 * ResourceTapeSimulator.cpp
 *
 *  Created on: 2013-3-21
 *      Author: more
 */


#include "stdafx.h"
#include "../PriorityTape.h"
#include "ResourceTapeSimulator.h"


namespace bdt
{

    ResourceTapeSimulator::ResourceTapeSimulator()
    : changer_(Factory::GetTapeLibraryManager())
    {
        changer_->Attach(this);
        Update();
    }


    ResourceTapeSimulator::~ResourceTapeSimulator()
    {
    }


    int
    ResourceTapeSimulator::StartTapes(
            const vector<string> & tapes,
            TapesInUseMap & tapesInUse,
            int priority)
    {
        //  We only support the first tape to ease
        const string & tape = tapes[0];

        LogDebug(tape);

        tapesInUse.clear();

#ifdef MORE_TEST
        assert(false);
        return START_RETURN_NO_USE;
#else
        boost::unique_lock<boost::mutex> lock(mutex_);

        PriorityMap::iterator iter = priorities_.find(tape);
        if ( iter == priorities_.end() ) {
            LogError(tape);
            return START_RETURN_NO_USE;
        }

        ChangerResource & resource = resources_[iter->second.changer];

        for ( size_t i = 0; i < resource.drive.size(); ++ i ) {
            if ( tape == resource.drive[i] ) {
                if ( resource.state[i] == DRIVE_STATE_STOPPED ) {
                    resource.state[i] = DRIVE_STATE_STARTED;
                }
                if ( resource.state[i] == DRIVE_STATE_STARTED ) {
                    return START_RETURN_SUCCESS;
                }
                if ( resource.state[i] == DRIVE_STATE_STARTING ) {
                    return START_RETURN_WAIT;
                }
                assert(false);
            }
        }

        if ( resource.driveBusy >= 0 ) {
            LogWarn(tape);
            return START_RETURN_BUSY;
        }

        for ( size_t i = 0; i < resource.drive.size(); ++ i ) {
            if ( resource.drive[i].empty() ) {
                if ( LoadTape(lock,iter->second.changer,i,tape) ) {
                    if ( resource.state[i] == DRIVE_STATE_STARTED ) {
                        return START_RETURN_SUCCESS;
                    } else {
                        return START_RETURN_WAIT;
                    }
                }
            }
        }

        for ( size_t i = 0; i < resource.drive.size(); ++ i ) {
            if ( resource.drive[i].empty() ) {
                continue;
            }
            if ( resource.state[i] == DRIVE_STATE_STOPPED ) {
                if ( LoadTape(lock,iter->second.changer,i,tape) ) {
                    if ( resource.state[i] == DRIVE_STATE_STARTED ) {
                        return START_RETURN_SUCCESS;
                    } else {
                        return START_RETURN_WAIT;
                    }
                }
            }
        }

        for ( size_t i = 0; i < resource.drive.size(); ++ i ) {
            if ( resource.drive[i].empty() ) {
                continue;
            }
            if ( resource.state[i] == DRIVE_STATE_STARTED ) {
                tapesInUse.push_back(resource.drive[i]);
            }
        }
        return START_RETURN_NO_RESOURCE;
#endif
    }


    bool
    ResourceTapeSimulator::StopTapes(const vector<string> & tapes)
    {
        //  We only support the first tape to ease
        const string & tape = tapes[0];

        LogDebug(tape);

#ifdef MORE_TEST
        assert(false);
        return true;
#else
        PriorityMap::iterator iter = priorities_.find(tape);
        if ( iter == priorities_.end() ) {
            LogError(tape << " does not exist");
            return true;
        }

        ChangerResource & resource = resources_[iter->second.changer];

        for ( size_t i = 0; i < resource.drive.size(); ++ i ) {
            if ( tape == resource.drive[i] ) {
                if ( resource.state[i] == DRIVE_STATE_STARTED ) {
                    resource.state[i] = DRIVE_STATE_STOPPED;
                    LogDebug(tape << " is stopped")
                    return true;
                }
                if ( resource.state[i] == DRIVE_STATE_STOPPED ) {
                    LogDebug(tape << " is stopped already")
                    return true;
                }
                if ( resource.state[i] == DRIVE_STATE_STARTING ) {
                    LogDebug(tape << " is starting and cannot be stopped")
                    return false;
                }
                assert(false);
            }
        }

        LogDebug(tape << " is not in a drive")
        return true;
#endif
    }


    void
    ResourceTapeSimulator::Update()
    {
        LogDebug("Update");

        while ( ! Refresh() ) {
            boost::this_thread::sleep(boost::posix_time::milliseconds(10));
        }
    }


    void
    ResourceTapeSimulator::ClearPriorities()
    {
#ifdef MORE_TEST
        assert(false);
#else
        bool remove = false;
        do {
            remove = false;
            for ( PriorityMap::iterator i = priorities_.begin();
                    i != priorities_.end();
                    ++i ) {
                if ( NULL == i->second.priority ) {
                    priorities_.erase(i);
                    remove = true;
                    break;
                }
            }
        } while ( remove );
#endif
    }


    void
    ResourceTapeSimulator::RefreshChanger(
            size_t indexChanger, tape::Changer & changer )
    {
#ifdef MORE_TEST
        assert(false);
#else
        ChangerResource & resource = resources_[indexChanger];

        vector<tape::Drive> drives;
        vector<tape::Cartridge> cartridges;
        tape::Error err;
        if ( ! changer.GetDriveList(drives,err) ) {
            LogError(err.ErrorNumber() << ":" << err.ErrorMessage());
            return;
        }
        if ( ! changer.GetCartridgeList(cartridges,err) ) {
            LogError(err.ErrorNumber() << ":" << err.ErrorMessage());
            return;
        }

        for ( size_t i = 0; i < drives.size(); ++i ) {
            string barcode;
            if ( ! drives[i].GetBarcode(barcode,err) ) {
                barcode.clear();
            }
            if ( resource.drive.size() > i ) {
                if ( resource.state[i] != DRIVE_STATE_STARTING ) {
                    LogDebug( resource.drive[i] << " " << barcode );
                    assert( resource.drive[i] == barcode );
                }
            } else {
                resource.drive.push_back(barcode);
                resource.state.push_back(DRIVE_STATE_STOPPED);
            }
        }

        for ( size_t i = 0; i < cartridges.size(); ++i ) {
            string barcode;
            if ( ! cartridges[i].GetBarcode(barcode,err) ) {
                LogError(err.ErrorNumber() << ":" << err.ErrorMessage());
                continue;
            }
            PriorityTapeChanger priority;
            priority.changer = indexChanger;
            priority.priority = NULL;
            PriorityMap::iterator i = priorities_.insert(
                            PriorityMap::value_type(barcode,priority) ).first;
            assert( i->second.changer == (int)indexChanger );
        }
#endif
    }


    bool
    ResourceTapeSimulator::Refresh()
    {
        LogDebug("Refresh");

#ifdef MORE_TEST
        assert(false);
        return true;
#else
        boost::lock_guard<boost::mutex> lock(mutex_);

        bool refresh = true;
        for ( vector<ChangerResource>::iterator i = resources_.begin();
                i != resources_.end();
                ++ i ) {
            if ( i->driveBusy >= 0 ) {
                refresh = false;
            }
        }
        if ( ! refresh ) {
            LogWarn("busy")
            return false;
        }

        vector<tape::Changer> changers;
        tape::Error err;
        if ( ! changer_->GetChangerList(changers,err) ) {
            LogError(err.ErrorNumber() << ":" << err.ErrorMessage());
            return false;
        }

        ClearPriorities();

        while ( resources_.size() < changers.size() ) {
            resources_.push_back(ChangerResource());
        }

        for ( size_t i = 0; i < changers.size(); ++ i ) {
            RefreshChanger(i,changers[i]);
        }

        return true;
#endif
    }


    bool
    ResourceTapeSimulator::LoadTape(
            boost::unique_lock<boost::mutex> & lock,
            size_t changer, size_t drive, const string & tape )
    {
#ifdef MORE_TEST
        assert(false);
        return true;
#else
        static bool syncMode = Factory::GetConfigure()->GetValueBool(
                Configure::ChangerSyncMode );

        ChangerResource & resource = resources_[changer];
        if ( resource.driveBusy >= 0 ) {
            LogError(changer << " : " << drive << " : " << resource.driveBusy);
            return false;
        }

        resource.drive[drive] = tape;
        resource.state[drive] = DRIVE_STATE_STARTING;
        resource.driveBusy = drive;

        if ( syncMode ) {
            lock.unlock();
            bool ret = LoadTapeTask(changer,tape,true);
            lock.lock();
            return ret;
        } else {
            boost::thread task( &ResourceTapeSimulator::LoadTapeTask, this,
                    changer, tape, true );
            return true;
        }
#endif
    }


    bool
    ResourceTapeSimulator::LoadTapeTask(
            size_t changer, const string & tape, bool reset )
    {
#ifdef MORE_TEST
        assert(false);
        return true;
#else
        boost::unique_lock<boost::mutex> lock(mutex_);

RetryLoadTapeTask:

        ChangerResource & resource = resources_[changer];
        size_t drive = resource.driveBusy;

        vector<tape::Changer> changers;
        vector<tape::Drive> drives;
        vector<tape::Cartridge> cartridges;
        tape::Error err;
        bool empty;
        string tapeDrive;
        int indexCartridge = -1;
        bool ret = false;

        if ( ! changer_->GetChangerList(changers,err) ) {
            LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
            abort();
        }
        if ( changer >= changers.size() ) {
            LogError(changer);
            abort();
        }

        if ( ! changers[changer].GetDriveList(drives,err) ) {
            LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
            abort();
        }
        if ( drive >= drives.size() ) {
            LogError(drive);
            abort();
        }
        if ( ! drives[drive].GetEmpty(empty,err) ) {
            LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
            abort();
        }
        if ( ! empty ) {
            if ( ! drives[drive].GetBarcode(tapeDrive,err) ) {
                LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
                abort();
            }
            if ( tape == tapeDrive ) {
                LogError(drive << " : " << tape);
                ret = true;
                goto LoadTapeTaskEnd;
            } else {
                lock.unlock();
                ret = UnloadTapeTask(changer,false,false);
                lock.lock();
                if ( ret ) {
                    goto RetryLoadTapeTask;
                } else {
                    goto LoadTapeTaskEnd;
                }
            }
        }

        if ( ! changers[changer].GetCartridgeList(cartridges,err) ) {
            LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
            abort();
        }
        for ( size_t i = 0; i < cartridges.size(); ++ i ) {
            string tapeCartridge;
            if ( ! cartridges[i].GetBarcode(tapeCartridge,err) ) {
                LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
                continue;
            }
            if ( tape == tapeCartridge ) {
                indexCartridge = i;
                break;
            }
        }
        if ( indexCartridge < 0 ) {
            LogError(changer << " " << drive);
            abort();
        }

        int trying;
        trying = 2;
        do {
            -- trying;
            resource.drive[drive] = tape;
            LogDebug("Begin to load tape: "
                    << changer
                    << " " << drive
                    << " " << tape);
            lock.unlock();
            ret = changers[changer].LoadCartridge(
                    drives[drive],
                    cartridges[indexCartridge],
                    err );
            lock.lock();
            LogDebug("Finish to load tape: "
                    << changer
                    << " " << drive
                    << " " << tape);
            if ( ! ret ) {
                LogWarn("Failure to load tape: "
                        << changer
                        << " " << drive
                        << " " << tape
                        << " " << err.ErrorNumber()
                        << ":" << err.ErrorMessage());
                resource.drive[drive].clear();
            }
        } while ( (! ret) && trying );

LoadTapeTaskEnd:

        if ( reset ) {
            if ( ! drives[drive].GetBarcode(tapeDrive,err) ) {
                LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
                abort();
            }
            resource.drive[drive] = tapeDrive;
            if ( ret ) {
                resource.state[drive] = DRIVE_STATE_STARTED;
            } else {
                resource.state[drive] = DRIVE_STATE_STOPPED;
            }
            resource.driveBusy = -1;
        }
        return ret;
#endif
    }


    bool
    ResourceTapeSimulator::UnloadTapeTask(
            size_t changer, bool isExport, bool reset )
    {
#ifdef MORE_TEST
        assert(false);
        return true;
#else
        boost::unique_lock<boost::mutex> lock(mutex_);

        ChangerResource & resource = resources_[changer];
        size_t drive = resource.driveBusy;

        vector<tape::Changer> changers;
        vector<tape::Drive> drives;
        vector<tape::Slot> slots;
        vector<tape::Cartridge> cartridges;
        tape::Error err;
        bool empty;
        string tape;
        int indexSlot = -1;
        int indexCartridge = -1;
        bool ret = false;

        if ( ! changer_->GetChangerList(changers,err) ) {
            LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
            abort();
        }
        if ( changer >= changers.size() ) {
            LogError(changer);
            abort();
        }

        if ( ! changers[changer].GetDriveList(drives,err) ) {
            LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
            abort();
        }
        if ( drive >= drives.size() ) {
            LogError(drive);
            abort();
        }
        if ( ! drives[drive].GetEmpty(empty,err) ) {
            LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
            abort();
        }
        if ( empty ) {
            LogError(changer << " : " << drive);
            ret = true;
            goto UnloadTapeTaskEnd;
        }
        if ( ! drives[drive].GetBarcode(tape,err) ) {
            LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
            abort();
        }

        if ( ! changers[changer].GetSlotList(slots,err) ) {
            LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
            abort();
        }
        for ( size_t i = 0; i < slots.size(); ++ i ) {
            if ( ! slots[i].GetEmpty(empty,err) ) {
                LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
                continue;
            }
            if ( empty ) {
                indexSlot = i;
                if ( ! isExport ) {
                    break;
                }
            }
        }
        if ( indexSlot < 0 ) {
            LogWarn("Failure to unload tape: no slot is empty");
            abort();
        }

        if ( ! changers[changer].GetCartridgeList(cartridges,err) ) {
            LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
            abort();
        }
        for ( size_t i = 0; i < cartridges.size(); ++ i ) {
            string tapeCartridge;
            if ( ! cartridges[i].GetBarcode(tapeCartridge,err) ) {
                LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
                continue;
            }
            if ( tape == tapeCartridge ) {
                indexCartridge = i;
                break;
            }
        }
        if ( indexCartridge < 0 ) {
            LogError(changer << " " << drive);
            abort();
        }

        LogDebug("Begin to unload tape: "
                << changer
                << " " << drive
                << " " << tape
                << " " << indexSlot);
        lock.unlock();
        ret = drives[drive].Unmount(err);
        if ( ret ) {
            int trying = 2;
            do {
                -- trying;
                ret = changers[changer].MoveCartridge(
                        slots[indexSlot],
                        cartridges[indexCartridge],
                        err );
                if ( ! ret ) {
                    LogWarn("Failure to unload tape: "
                            << changer
                            << " " << drive
                            << " " << tape
                            << " " << err.ErrorNumber()
                            << ":" << err.ErrorMessage());
                }
            } while ( (! ret ) && trying );
            if ( ! ret ) {
                drives[drive].Mount(err);
            }
        } else {
            LogWarn("Failure to umount tape: "
                    << changer
                    << " " << drive
                    << " " << tape
                    << " " << err.ErrorNumber()
                    << ":" << err.ErrorMessage());
        }
        lock.lock();
        LogDebug("Finish to unload tape: "
                << changer
                << " " << drive
                << " " << tape);

        if ( ! ret ) {
            LogWarn(err.ErrorNumber() << ":" << err.ErrorMessage());
        }

UnloadTapeTaskEnd:

        if ( reset ) {
            if ( ! drives[drive].GetBarcode(tape,err) ) {
                LogError(err.ErrorNumber() << " : " << err.ErrorMessage());
                abort();
            }
            resource.drive[drive] = tape;
            resource.state[drive] = DRIVE_STATE_STOPPED;
            resource.driveBusy = -1;
        }

        return ret;
#endif
    }


    bool
    ResourceTapeSimulator::MountTapes(const vector<string> & tapes)
    {
        //  We only support the first tape to ease
        const string & tape = tapes[0];

        LogDebug(tape);

#ifdef MORE_TEST
        assert(false);
        return true;
#else
        boost::lock_guard<boost::mutex> lock(mutex_);

        PriorityMap::iterator iter = priorities_.find(tape);
        if ( priorities_.end() == iter ) {
            return false;
        }
        if ( NULL == iter->second.priority ) {
            return false;
        }
        int changer = iter->second.changer;

        vector<tape::Changer> changers;
        vector<tape::Drive> drives;
        vector<tape::Cartridge> cartridges;
        tape::Error err;
        bool empty;

        if ( ! changer_->GetChangerList(changers,err) ) {
            return false;
        }
        if ( static_cast<size_t>(changer) >= changers.size() ) {
            return false;
        }

        if ( ! changers[changer].GetDriveList(drives,err) ) {
            return false;
        }
        for ( size_t i = 0; i < drives.size(); ++ i ) {
            if ( ! drives[i].GetEmpty(empty,err) ) {
                return false;
            }
            if ( ! empty ) {
                string barcodeDrive;
                if ( drives[i].GetBarcode(barcodeDrive,err) ) {
                    if ( tape == barcodeDrive ) {
                        if ( drives[i].Mount(err) ) {
                            return true;
                        } else {
                            LogWarn("Failure to mount tape: "
                                    << changer
                                    << " " << i
                                    << " " << tape
                                    << " " << err.ErrorNumber()
                                    << ":" << err.ErrorMessage());
                            return false;
                        }
                    }
                }
            }
        }

        return false;
#endif
    }


    bool
    ResourceTapeSimulator::UnMountTapes(const vector<string> & tapes)
    {
        //  We only support the first tape to ease
        const string & tape = tapes[0];

        LogDebug(tape);

#ifdef MORE_TEST
        assert(false);
        return START_RETURN_SUCCESS;
#else
        boost::lock_guard<boost::mutex> lock(mutex_);

        PriorityMap::iterator iter = priorities_.find(tape);
        if ( priorities_.end() == iter ) {
            return false;
        }
        if ( NULL == iter->second.priority ) {
            return false;
        }
        int changer = iter->second.changer;

        vector<tape::Changer> changers;
        vector<tape::Drive> drives;
        vector<tape::Cartridge> cartridges;
        tape::Error err;
        bool empty;

        if ( ! changer_->GetChangerList(changers,err) ) {
            return false;
        }
        if ( static_cast<size_t>(changer) >= changers.size() ) {
            return false;
        }

        if ( ! changers[changer].GetDriveList(drives,err) ) {
            return false;
        }
        for ( size_t i = 0; i < drives.size(); ++ i ) {
            if ( ! drives[i].GetEmpty(empty,err) ) {
                return false;
            }
            if ( ! empty ) {
                string barcodeDrive;
                if ( drives[i].GetBarcode(barcodeDrive,err) ) {
                    if ( tape == barcodeDrive ) {
                        if ( drives[i].Unmount(err) ) {
                            return true;
                        } else {
                            LogWarn("Failure to umount tape: "
                                    << changer
                                    << " " << i
                                    << " " << tape
                                    << " " << err.ErrorNumber()
                                    << ":" << err.ErrorMessage());
                            return false;
                        }
                    }
                }
            }
        }

        return false;
#endif
    }

}
