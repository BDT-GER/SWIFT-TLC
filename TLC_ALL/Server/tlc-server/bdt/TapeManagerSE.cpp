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
 * TapeManagerSE.cpp
 *
 *  Created on: Jul 24, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "TapeManagerSE.h"

const int CHECK_TAPE_STATE_REQUEST_TIME_OUT 		= 10000;  	// 10 seconds each time
const int CHECK_TAPE_STATE_REQUEST_RETRY_TIMES 		= 3;  		// 3 times

namespace bdt
{

    TapeManagerSE::TapeManagerSE()
    : folder_(Factory::GetTapeFolder())
    {
    }


    TapeManagerSE::~TapeManagerSE()
    {
    }


    bool
    TapeManagerSE::SetTapeStatus( const string & tape, enum TapeStatus status )
    {
#ifdef MORE_TEST
        assert(false);
        return false;
#else
        TapeInfo info;
		LtfsError error;

        switch ( status ) {
        case STATUS_CORRUPT:
			return TapeLibraryMgr::Instance()->SetLoadedTapeFaulty(tape, true, error, true);
            //return TapeLibraryMgr::Instance()->SetTapeFaulty(tape,true, -1); //bdt/TapeManagerSE.cpp: In member function �virtual bool bdt::TapeManagerSE::SetTapeStatus(const std::string&, bdt::TapeManagerI
            break;
        case STATUS_FULL:
            if ( TapeLibraryMgr::Instance()->GetTape(tape,info) ) {
                if ( info.mStatus == TAPE_OPEN || info.mStatus == TAPE_ACTIVE ) {
					return TapeLibraryMgr::Instance()->SetLoadedTapeStatus(tape, TAPE_CLOSED, error, true);
                    //return TapeLibraryMgr::Instance()->SetTapeStatus(tape,TAPE_CLOSED, -1); // //TODO: set mam?have resource?
                }
            }
            break;
        default:
            break;
        }

        return false;
#endif
    }

#ifdef LTFS_VSB_MODE
    enum TapeManagerInterface::TapeStatus
    TapeManagerSE::GetTapeStatus( const string & tape )
    {
    	LtfsDriveInfo driveInfo;
    	if(!TapeLibraryMgr::Instance()->GetDriveInfoForTape(tape, driveInfo) || driveInfo.mStatus == DRIVE_STATUS_DISCONNECTED){
    		return STATUS_MISSING;
    	}

    	TapeInfo info;
    	if(!TapeLibraryMgr::Instance()->GetTape(tape, info)){
    		return STATUS_MISSING;
    	}

		if ( info.mFaulty ) {
			return STATUS_CORRUPT;
		}

		if ( info.mStatus == TAPE_CONFLICT ) {
			return STATUS_MISSING;
		}

		if ( info.mOffline ) {
			return STATUS_MISSING;
		}

		if ( info.mStatus == TAPE_EXPORTED ) {
			return STATUS_EXPORT;
		}

		return STATUS_ONLINE;
    }
#else
    enum TapeManagerInterface::TapeStatus
    TapeManagerSE::GetTapeStatus( const string & tape )
    {
#ifdef MORE_TEST
        assert(false);
        return STATUS_MISSING;
#else
        vector<LtfsChangerInfo> changers;
        LtfsError error;
        if ( ! TapeLibraryMgr::Instance()->GetChangerList(changers,error) ) {
            return STATUS_EXPORT;
        }

        for ( vector<LtfsChangerInfo>::iterator i = changers.begin();
                i != changers.end();
                ++ i ) {
            enum TapeManagerInterface::TapeStatus status;
            if ( GetTapeStatus(tape,*i,status) ) {
                return status;
            }
        }

        return STATUS_EXPORT;
#endif
    }
#endif


    bool TapeManagerSE::GetShareAvailableTapes(const string& uuid, vector<map<string, off_t> >& tapesList)
    {
#ifdef MORE_TEST
        return false;
#else
        bool ret = TapeLibraryMgr::Instance()->GetShareAvailableTapes(uuid, tapesList);
        if(false == ret){
            LogError(uuid);
            return false;
        }
        return true;
#endif
    }

    bool
    TapeManagerSE::GetTapesUse(vector<string> & tapes,const fs::path & path,off_t size)
    {
        string strPath = path.string();
        int pos = strPath.find('/', 1);
        if(-1 == pos){
            LogError(path);
            return false;
        }

#ifdef MORE_TEST
        assert(false);
        return false;
#else
        string tmpTapeGroup = strPath.substr(1, pos - 1);
        bool ret = TapeLibraryMgr::Instance()->GetActiveTape(tapes,tmpTapeGroup,size);
        string tapeGroup = tmpTapeGroup;
        if(false == ret){
            LogError(boost::join(tapes,",") << " " << tapeGroup << " " << size);
            return false;
        }

        return true;
#endif
    }


    bool
    TapeManagerSE::SetTapesUse(const vector<string> & tapes,int fileNumber,off_t fileSize,off_t tapeUse)
    {
#ifdef MORE_TEST
        assert(false);
        return false;
#else
        bool ret = true;
        BOOST_FOREACH(const string & tape, tapes) {
            ret = TapeLibraryMgr::Instance()->SetTapeCapacity(tape,fileNumber,fileSize,tapeUse) && ret;
        }
        return ret;
#endif
    }


    fs::path
    TapeManagerSE::GetPath( const string & tape, const fs::path & path )
    {
        return folder_ / tape / path;
    }


    bool
    TapeManagerSE::GetTapeStatus(
            const string & tape,
#ifdef MORE_TEST
            tape::Changer & changer,
#else
            LtfsChangerInfo & changer,
#endif
            enum TapeStatus & status )
    {
#ifdef MORE_TEST
        assert(false);
        return false;
#else
        LtfsError error;
        string barcode;
        vector<TapeInfo> tapes;
        if ( ! TapeLibraryMgr::Instance()->GetTapeListForChanger(
                changer.mSerial, tapes, error ) ) {
            return false;
        }

        status = STATUS_EXPORT;
        for ( vector<TapeInfo>::iterator i = tapes.begin();
                i != tapes.end();
                ++ i ) {
            if ( i->mBarcode == tape ) {
                status = STATUS_OFFLINE;
                break;
            }
        }
        if ( STATUS_EXPORT == status ) {
            return false;
        }

        vector<LtfsDriveInfo> drives;
        if ( ! TapeLibraryMgr::Instance()->GetDriveListForChanger(
                changer.mSerial, drives, error ) ) {
            return false;
        }

        for ( vector<LtfsDriveInfo>::iterator i = drives.begin();
                i != drives.end();
                ++ i ) {
            if ( (! i->mIsEmpty) && (i->mBarcode == tape) ) {
                status = STATUS_ONLINE;
                break;
            }
        }

        TapeInfo info;
        if ( TapeLibraryMgr::Instance()->GetTape(tape,info) ) {
            if ( info.mFaulty ) {
                status = STATUS_CORRUPT;
                return true;
            }

            if ( info.mStatus == TAPE_CONFLICT ) {
                status = STATUS_MISSING;
                return true;
            }

            if ( info.mOffline ) {
                status = STATUS_MISSING;
                return true;
            }
            if ( info.mStatus == TAPE_EXPORTED ) {
                status = STATUS_EXPORT;
                return true;
            }
        }

        return true;
#endif
    }


    /*bool
    TapeManagerSE::GetCapacity(
            const string & service,
            size_t & fileNumber,
            off_t & usedSize,
            off_t & freeSize )
    {
#ifdef MORE_TEST
        assert(false);
        return false;
#else
        return TapeLibraryMgr::GetTapeGroupCapacity(service,fileNumber,usedSize,freeSize);
#endif
    }*/


    bool
    TapeManagerSE::CheckTapeState( const string & tape, enum TapeState state )
    {
#ifdef MORE_TEST
        assert(false);
        return false;
#else
        TapeLibraryMgr * mgr = TapeLibraryMgr::Instance();
        int retry = CHECK_TAPE_STATE_REQUEST_RETRY_TIMES;
        while ( ! mgr->StmRequestTape(tape, CHECK_TAPE_STATE_REQUEST_TIME_OUT) ) {
            -- retry;
            if ( 0 == retry ) {
            	LogDebug(tape << " cann't be locked.");
                return false;
            }
        }
        bool ret = true;
        switch ( state ) {
        case STATE_READ:
            if ( ! mgr->StmTapeCanBeChangedTo(tape,STM_ST_READWRITE,STM_OP_READ) ) {
                ret = false;
            }
            break;
        case STATE_WRITE:
            if ( ! mgr->StmTapeCanBeChangedTo(tape,STM_ST_READWRITE,STM_OP_WRITE) ) {
                ret = false;
            }
            break;
        default:
            break;
        }
        mgr->StmReleaseTape(tape);
        LogDebug(tape << " " << state <<  " : " << ret);
        return ret;
#endif
    }


    bool
    TapeManagerSE::SetTapeStateNoLock( const string & tape, enum TapeState state )
    {
#ifdef MORE_TEST
        assert(false);
        return false;
#else
        TapeLibraryMgr * mgr = TapeLibraryMgr::Instance();
        bool ret = true;
        switch ( state ) {
        case STATE_READ:
            if ( ! mgr->StmTapeCanBeChangedTo(tape,STM_ST_READWRITE,STM_OP_READ) ) {
                ret = false;
            }
            break;
        case STATE_WRITE:
            if ( ! mgr->StmTapeCanBeChangedTo(tape,STM_ST_READWRITE,STM_OP_WRITE) ) {
                ret = false;
            }
            break;
        default:
            break;
        }
        if ( ret ) {
            StateMap::iterator i = state_.insert( StateMap::value_type(tape,0) ).first;
            switch ( state ) {
            case STATE_READ:
            case STATE_WRITE:
                if ( 0 == i->second ) {
                    LogDebug(tape << " " << state);
                    ret = mgr->SetTapeState(tape,TAPE_STATE_READWRITE);
                }
                ++ i->second;
                break;
            case STATE_IDLE:
                -- i->second;
                if ( 0 == i->second ) {
                    LogDebug(tape);
                    ret = mgr->SetTapeState(tape,TAPE_STATE_IDLE);
                }
                break;
            default:
                break;
            }
        }
       // mgr->StmReleaseTape(tape);
        LogDebug(tape << " " << state <<  " : " << ret);
        return ret;
#endif
    }


    bool
    TapeManagerSE::LockTapeState( const string & tape )
    {
#ifdef MORE_TEST
        assert(false);
        return false;
#else
        TapeLibraryMgr * mgr = TapeLibraryMgr::Instance();
        return mgr->StmRequestTape(tape);
#endif
    }


    bool
    TapeManagerSE::SetTapeState( const string & tape, enum TapeState state )
    {
#ifdef MORE_TEST
        assert(false);
        return false;
#else
        TapeLibraryMgr * mgr = TapeLibraryMgr::Instance();
        int retry = 1;
        while ( ! mgr->StmRequestTape(tape) ) {
            -- retry;
            if ( 0 == retry ) {
                return false;
            }
        }

       // return SetTapeStateNoLock(tape,state);
        bool bRet = SetTapeStateNoLock(tape,state);
        mgr->StmReleaseTape(tape);

        return bRet;
#endif
    }

    bool
    TapeManagerSE::GetDriveNum(int& driveNum)
    {
#ifdef MORE_TEST
        return false;
#else
    	vector<LtfsChangerInfo> changers;
    	LtfsError error;
    	TapeLibraryMgr * mgr = TapeLibraryMgr::Instance();
    	driveNum = mgr->GetDriveNum();
    	LogDebug("GetDriveNum: driveNum = " << driveNum);
    	return true;
#endif
    }

    bool
    TapeManagerSE::GetTapeActivity(const string& barcode, int& act, int& percentage)
    {
#ifdef MORE_TEST
        return false;
#else
    	LtfsError error;
    	TapeLibraryMgr * mgr = TapeLibraryMgr::Instance();
    	mgr->GetTapeActivity(barcode, act, percentage);
    	LogDebug("GetTapeActivity: barcode = " << barcode << ", act = " << act << ", percentage = " << percentage);
    	return true;
#endif
    }


    bool
    TapeManagerSE::SetTapeAction( const string & tape, enum TapeAction action )
    {
#ifdef MORE_TEST
        assert(false);
        return false;
#else
        TapeLibraryMgr * mgr = TapeLibraryMgr::Instance();
        switch (action) {
        case ACTION_IDLE:
            return mgr->StopReadWriteTape(tape);
            break;
        case ACTION_READ:
            return mgr->StartReadTape(tape);
            break;
        case ACTION_WRITE:
            return mgr->StartWriteTape(tape);
            break;
        default:
            break;
        }
        return false;
#endif
    }

}
