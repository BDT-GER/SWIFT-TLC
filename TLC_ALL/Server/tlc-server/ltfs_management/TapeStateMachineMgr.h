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
 * File:         TapeStateMachineMgr.h
 * Description:
 * Author:       Sam Chen
 * Created:      Mar 21, 2013
 *
 */
//******************************************************************************


#pragma once


#include "../lib/ltfs_library/stdafx.h"
#include "../lib/ltfs_library/LtfsError.h"
#include "../lib/ltfs_library/LtfsDetails.h"
#include "../lib/ltfs_library/LtfsTape.h"
#include "../lib/ltfs_library/LtfsScsiDevice.h"
#include "../lib/ltfs_library/LtfsSlot.h"
#include "../lib/ltfs_library/LtfsDrive.h"
#include "../lib/ltfs_library/LtfsMailSlot.h"
#include "../lib/ltfs_library/LtfsChanger.h"
#include "../lib/ltfs_library/LtfsLibraries.h"

#include "stdafx.h"
#include "TapeLibraryMgr.h"


using namespace ltfs_management;

namespace ltfs_management
{
    class TapeStateMachineMgr
    {
    private:
        static TapeStateMachineMgr * Instance();
        static void Destroy();
    public:
        bool StmRequestTape(const string& barcode, int millTimeOut = STM_LOCK_DEF_TIMEOUT);
        bool StmRequestTape(const string& barcode, STM_OPERATION stmOperation, const string& shareUuid = "", int millTimeOut = STM_LOCK_DEF_TIMEOUT);
        void StmReleaseTape(const string& barcode);
        bool StmGetTapeList(STM_OPERATION stmOperation, STM_STATE stmState, vector<TapeInfo>& tapes);
        bool StmGetTapeList(map<STM_OPERATION, STM_STATE> stmOpStateMap, vector<TapeInfo>& tapes);
        bool StmTapeCanBeChangedTo(const string& barcode, STM_STATE stmState, STM_OPERATION stmOperation);
        bool StmTapeCanBeChangedTo(const string& barcode, STM_STATE stmState, const vector<STM_OPERATION> operations);
        string GetStmStateStr(STM_STATE stmState);
        string GetStmOperationStr(STM_OPERATION stmOp);
        STM_STATE GetTapeStmState(const string& barcode);

    private:
        TapeStateMachineMgr();
        ~TapeStateMachineMgr();
        void InitStateMap();
        STM_STATE GetTapeStmState(const TapeInfo& tapeInfo);

    private:
		map<string, boost::timed_mutex*> 	stmTapeMutexMap_;
		map<string, boost::timed_mutex*> 	stmShareOpMutexMap_;
		map<string, boost::timed_mutex*> 	stmTapeOpMap_;
        map<string, string>					stmStateMap;
        boost::mutex        				mutexBuffer_;
        boost::mutex       					mutexTape_;

        static boost::mutex   				mutexInstance_;
        static TapeStateMachineMgr *        instance_;

		friend TapeLibraryMgr* TapeLibraryMgr::Instance();
		friend void TapeLibraryMgr::Destroy();
    };

}


