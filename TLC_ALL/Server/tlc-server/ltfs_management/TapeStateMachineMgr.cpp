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
 * File:         TapeStateMachineMgr.cpp
 * Description:
 * Author:       Sam Chen
 * Created:      Mar 21, 2013
 *
 */

#include "TapeLibraryMgr.h"
#include "TapeStateMachineMgr.h"
#include "../ltfs_format/ltfsFormatDetails.h"

using namespace ltfs_management;

namespace ltfs_management
{
	TapeStateMachineMgr * TapeStateMachineMgr::instance_ = NULL;
	boost::mutex TapeStateMachineMgr::mutexInstance_;

	const string STM_CONDITION_TAPE_EMPTY 			= "tape_is_empty";
	const string STM_CONDITION_TAPE_FAULTY 			= "faulty=true";
	const string STM_CONDITION_TAPE_NO_FORMAT_TASK	= "no_format_task";

	TapeStateMachineMgr::TapeStateMachineMgr()
	{
		InitStateMap();
	}
	TapeStateMachineMgr::~TapeStateMachineMgr()
	{
		map<string, boost::timed_mutex*>::iterator it = stmTapeMutexMap_.begin();
		while(it != stmTapeMutexMap_.end()){
			delete it->second;
			it->second = NULL;
			stmTapeMutexMap_.erase(it);
			it = stmTapeMutexMap_.begin();
		}
		it = stmShareOpMutexMap_.begin();
		while(it != stmShareOpMutexMap_.end()){
			delete it->second;
			it->second = NULL;
			stmShareOpMutexMap_.erase(it);
			it = stmShareOpMutexMap_.begin();
		}
		it = stmTapeOpMap_.begin();
		while(it != stmTapeOpMap_.end()){
			delete it->second;
			it->second = NULL;
			stmTapeMutexMap_.erase(it);
			it = stmTapeOpMap_.begin();
		}
	}

	TapeStateMachineMgr *
    TapeStateMachineMgr::Instance()
    {
        boost::mutex::scoped_lock lock(mutexInstance_);

        if ( NULL == instance_ ) {
            instance_ = new TapeStateMachineMgr();
        }
        return instance_;
    }

    void
    TapeStateMachineMgr::Destroy()
    {
        boost::mutex::scoped_lock lock(mutexInstance_);

        if (instance_) {
            delete instance_;
            instance_ = NULL;
        }
    }

    string StmStateToStr(STM_STATE stFrom, STM_STATE stTo, STM_OPERATION operation)
    {
        return boost::lexical_cast<string>(stFrom) + string(":") + boost::lexical_cast<string>(stTo) + string(":") + boost::lexical_cast<string>(operation);
    }
    void TapeStateMachineMgr::InitStateMap()
    {
		// init tape state machine rule

    	stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_ASSIGNED_OPEN, STM_OP_RECYCLE)] = ""; //									Assigned Empty
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_ASSIGNED_ACTIVE, STM_OP_WRITE)] = ""; // Write(Label)						Assigned Active
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_ASSIGNED_CLOSED, STM_OP_XXX)] = ""; //									Assigned Closed
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_ASSIGNED_EXPORTED, STM_OP_EXPORT)] = ""; // (Label/Clean Caches)			Assigned Exported
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_UN_ASSIGNED, STM_OP_REVOKE)] = STM_CONDITION_TAPE_EMPTY; // (Label)								Unassigned
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_UN_ASSIGNED, STM_OP_UNASSIGN)] = ""; // (Label)							Unassigned
    	//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_RAW, STM_OP_XXX)] = ""; //												Raw
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_WRITE_PROTECT, STM_OP_INVENTORY)] = ""; //					WriteProtected
    	//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_ACCESSING, STM_OP_XXX)] = ""; //											Accessing
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_DIAGNOSING, STM_OP_DIAGNOSE)] = STM_CONDITION_TAPE_FAULTY; // (Label)					Diagnosing
    	//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //										Unsupported
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_CONFLICT, STM_OP_INVENTORY)] = ""; //								Conflict
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_OFFLINE, STM_OP_INVENTORY)] = ""; //									Offline
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_OFFLINE, STM_OP_EJECT)] = STM_CONDITION_TAPE_NO_FORMAT_TASK; //									Offline
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_READWRITE, STM_OP_READ)] = "";//STM_CONDITION_TAPE_IDLE; //								Reading
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_READWRITE, STM_OP_WRITE)] = "";
		  //stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_READWRITE, STM_OP_CHECK_OFFLINE)] = ""; //									Writting
		  //stmStateMap[StmStateToStr(STM_ST_ASSIGNED_OPEN, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_ASSIGNED_OPEN, STM_OP_RECYCLE)] = ""; // (Format/Clean Caches)
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_ASSIGNED_ACTIVE, STM_OP_XXX)] = ""; // Write(Label)
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_ASSIGNED_CLOSED, STM_OP_CLOSE)] = ""; // Close(Label)
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_ASSIGNED_EXPORTED, STM_OP_EXPORT)] = ""; // (Retention/Label/Clean Caches)
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_UN_ASSIGNED, STM_OP_UNASSIGN)] = ""; // (Label/Format)
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_RAW, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_WRITE_PROTECT, STM_OP_INVENTORY)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_ASSIGNED_ACCESSING, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_DIAGNOSING, STM_OP_DIAGNOSE)] = "faulty=true"; // (Label/Format)
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_CONFLICT, STM_OP_INVENTORY)] = ""; // (Label)
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_OFFLINE, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_OFFLINE, STM_OP_EJECT)] = STM_CONDITION_TAPE_NO_FORMAT_TASK; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_READWRITE, STM_OP_READ)] = "";//STM_CONDITION_TAPE_IDLE; //								Reading
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_READWRITE, STM_OP_WRITE)] = "";//STM_CONDITION_TAPE_IDLE; //
		  //stmStateMap[StmStateToStr(STM_ST_ASSIGNED_ACTIVE, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_ASSIGNED_OPEN, STM_OP_RECYCLE)] = ""; // (Format/Clean Caches)
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_ASSIGNED_ACTIVE, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_ASSIGN_CLOSED, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_ASSIGNED_EXPORTED, STM_OP_EXPORT)] = ""; // (Retention/Label/Clean Caches)
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_UN_ASSIGNED, STM_OP_UNASSIGN)] = ""; // (Label/Format)
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_RAW, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_WRITE_PROTECT, STM_OP_INVENTORY)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_ACCESSING, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_DIAGNOSING, STM_OP_DIAGNOSE)] = STM_CONDITION_TAPE_FAULTY; // (Label/Format)
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_CONFLICT, STM_OP_INVENTORY)] = ""; // (Label)
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_OFFLINE, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_OFFLINE, STM_OP_EJECT)] = STM_CONDITION_TAPE_NO_FORMAT_TASK; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_READWRITE, STM_OP_READ)] = "";//STM_CONDITION_TAPE_IDLE; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_READWRITE, STM_OP_WRITE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_ASSIGNED_CLOSED, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_ASSIGNED_OPEN, STM_OP_ASSIGN)] = ""; // (Format/Label)
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_ASSIGNED_OPEN, STM_OP_IMPORT)] = ""; // (Label/Create Caches)
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_XXX, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_ASSIGNED_CLOSED, STM_OP_IMPORT)] = ""; // (Label/Create Caches)
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_ASSIGNED_EXPORTED, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_UN_ASSIGNED, STM_OP_FORMAT)] = ""; // (Format/Label)
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_RAW, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_WRITE_PROTECT, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_ACCESSING, STM_OP_ACCESS)] = STM_CONDITION_TAPE_NO_FORMAT_TASK;// (Mount/Set Unavailable)
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_DIAGNOSING, STM_OP_DIAGNOSE)] = STM_CONDITION_TAPE_FAULTY; // (Label/Format)
		//stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_CONFLICT, STM_OP_INVENTORY)] = ""; // (Label)
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_OFFLINE, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_OFFLINE, STM_OP_EJECT)] = STM_CONDITION_TAPE_NO_FORMAT_TASK;
		  //stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_READWRITE, STM_OP_READ)] = "";//STM_CONDITION_TAPE_IDLE; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_READWRITE, STM_OP_CHECK_OFFLINE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_ASSIGNED_EXPORTED, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		  stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_ASSIGNED_OPEN, STM_OP_ASSIGN)] = ""; // (Label)
		  stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_ASSIGNED_OPEN, STM_OP_IMPORT)] = ""; // (Label/Create Caches)
		//stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_ASSIGNED_ACXTIVE, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_ASSIGNED_CLOSED, STM_OP_IMPORT)] = ""; // (Label/Create Caches)
		//stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_ASSIGNED_EXPORTED, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_UN_ASSIGNED, STM_OP_FORMAT)] = ""; // (Format)
		//stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_RAW, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_WRITE_PROTECT, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_ACCESSING, STM_OP_ACCESS)] = STM_CONDITION_TAPE_NO_FORMAT_TASK;//(Mount/Set Unavailable)
		  stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_DIAGNOSING, STM_OP_DIAGNOSE)] = STM_CONDITION_TAPE_FAULTY; // (Label/Format)
		//stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_CONFLICT, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_OFFLINE, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_OFFLINE, STM_OP_EJECT)] = STM_CONDITION_TAPE_NO_FORMAT_TASK;
		  //stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_READWRITE, STM_OP_READ)] = ""; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_READWRITE, STM_OP_CHECK_OFFLINE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_UN_ASSIGNED, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		  stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_ASSIGNED_OPEN, STM_OP_ASSIGN)] = ""; // (Format/Label)
		//stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_ASSIGNED_ACTIVE, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_ASSIGNED_CLOSED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_ASSIGNED_EXPORTED, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_UN_ASSIGNED, STM_OP_FORMAT)] = ""; // (Format/Label)
		//stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_RAW, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_WRITE_PROTECT, STM_OP_INVENTORY)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_ACCESSING, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_DIAGNOSING, STM_OP_DIAGNOSE)] = STM_CONDITION_TAPE_FAULTY; // (Label/Format)
		//stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_CONFLICT, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_OFFLINE, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_OFFLINE, STM_OP_EJECT)] = "";//STM_CONDITION_TAPE_IDLE; //
		  //stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_READWRITE, STM_OP_READ)] = ""; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_READWRITE, STM_OP_CHECK_OFFLINE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_RAW, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_ASSIGNED_OPEN, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_ASSIGNED_ACTIVE, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_ASSIGNED_CLOSED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_ASSIGNED_EXPORTED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_UN_ASSIGNED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_RAW, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_WRITE_PROTECT, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_ACCESSING, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_DIAGNOSING, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_UNSUPPORTED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_CONFLICT, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_OFFLINE, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_OFFLINE, STM_OP_EJECT)] = "";//STM_CONDITION_TAPE_IDLE; //
		  //stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_READWRITE, STM_OP_READ)] = "";//STM_CONDITION_TAPE_IDLE; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_READWRITE, STM_OP_CHECK_OFFLINE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_WRITE_PROTECT, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		  stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_ASSIGNED_OPEN, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_ASSIGNED_ACTIVE, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_ASSIGNED_CLOSED, STM_OP_INVENTORY)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_ASSIGNED_CLOSED, STM_OP_XXXX)] = ""; // Close Active Tape?
		  stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_ASSIGNED_EXPORTED, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_UN_ASSIGNED, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_RAW, STM_OP_INVENTORY)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_ACCESSING, STM_OP_DISCOVER_ONLINE)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_DIAGNOSING, STM_OP_DISCOVER_ONLINE)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_UN_SUPPORTED, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_CONFLICT, STM_OP_INVENTORY)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_OFFLINE, STM_OP_XXX)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_READWRITE, STM_OP_READ)] = ""; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_READWRITE, STM_OP_CHECK_OFFLINE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_OFFLINE, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		//stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_ASSIGNED_OPEN, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_ASSIGNED_ACTIVE, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_ASSIGNED_CLOSED, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_ASSIGNED_EXPORTED, STM_OP_UNACCESS)] = ""; //  (Unmount/Set Available)
		  stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_UN_ASSIGNED, STM_OP_UNACCESS)] = ""; //  (Unmount/Set Available)
		//stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_RAW, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_WRITE_PROTECT, STM_OP_INVENTORY)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_ACCESSING, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_DIAGNOSING, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_CONFLICT, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_OFFLINE, STM_OP_INVENTORY)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_READWRITE, STM_OP_READ)] = ""; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_READWRITE, STM_OP_CHECK_OFFLINE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_ACCESSING, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //


		  stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_ASSIGNED_OPEN, STM_OP_FINISH_DIAGNOSE)] = ""; // (Set Available)
		  stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_ASSIGNED_ACTIVE, STM_OP_FINISH_DIAGNOSE)] = ""; // (Set Available)
		  stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_ASSIGNED_CLOSED, STM_OP_FINISH_DIAGNOSE)] = ""; // (Set Available)
		  stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_ASSIGNED_EXPORTED, STM_OP_FINISH_DIAGNOSE)] = ""; // (Set Available)
		  stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_UN_ASSIGNED, STM_OP_FINISH_DIAGNOSE)] = ""; // (Set Available)
		  stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_RAW, STM_OP_FINISH_DIAGNOSE)] = ""; // (Set Available)
		  stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_WRITE_PROTECT, STM_OP_FINISH_DIAGNOSE)] = ""; //(Set Available)
		//stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_ACCESSING, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_CONFLICT, STM_OP_FINISH_DIAGNOSE)] = ""; // (Set Available)
		//stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_OFFLINE, STM_OP_XXX)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_READWRITE, STM_OP_READ)] = ""; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_READWRITE, STM_OP_CHECK_OFFLINE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_DIAGNOSING, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //


		//stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_ASSIGNED_OPEN, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_ASSIGNED_ACTIVE, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_ASSIGNED_CLOSED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_ASSIGNED_EXPORTED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_UN_ASSIGNED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_RAW, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_WRITE_PROTECT, STM_OP_FINISH_DIAGNOSE)] = ""; //(Set Available)
		//stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_ACCESSING, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_DIAGNOSING, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_CONFLICT, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_OFFLINE, STM_OP_INVENTORY)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_OFFLINE, STM_OP_EJECT)] = "";//STM_CONDITION_TAPE_IDLE; //
		  //stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_READWRITE, STM_OP_READ)] = ""; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_READWRITE, STM_OP_CHECK_OFFLINE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_UN_SUPPORTED, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		/*
		stmStateMap[StmStateToStr(STM_ST_MISSING, STM_ST_ASSIGNED_OPEN, STM_OP_DISCOVER_MISSING)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_MISSING, STM_ST_ASSIGNED_ACTIVE, STM_OP_DISCOVER_MISSING)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_MISSING, STM_ST_ASSIGNED_CLOSED, STM_OP_DISCOVER_MISSING)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_MISSING, STM_ST_ASSIGNED_EXPORTED, STM_OP_DISCOVER_MISSING)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_MISSING, STM_ST_UN_ASSIGNED, STM_OP_DISCOVER_MISSING)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_MISSING, STM_ST_RAW, STM_OP_DISCOVER_MISSING)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_MISSING, STM_ST_ACCESSING, STM_OP_DISCOVER_MISSING)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_MISSING, STM_ST_UN_SUPPORTED, STM_OP_DISCOVER_MISSING)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_MISSING, STM_ST_CONFLICT, STM_OP_DISCOVER_MISSING)] = ""; //
		*/

		  stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_ASSIGNED_OPEN, STM_OP_RECYCLE)] = ""; // (Format/Clean Caches)
		//stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_ASSIGNED_ACTIVE, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_ASSIGNED_CLOSED, STM_OP_XXX)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_ASSIGNED_EXPORTED, STM_OP_EXPORT)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_UN_ASSIGNED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_RAW, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_WRITE_PROTECT, STM_OP_FINISH_DIAGNOSE)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_ACCESSING, STM_OP_XXX)] = ""; //
		  stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_DIAGNOSING, STM_OP_DIAGNOSE)] = STM_CONDITION_TAPE_FAULTY; //
		//stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_CONFLICT, STM_OP_XXX)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_OFFLINE, STM_OP_EJECT)] = STM_CONDITION_TAPE_NO_FORMAT_TASK; //
		  //stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_READWRITE, STM_OP_READ)] = "";//STM_CONDITION_TAPE_IDLE; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_READWRITE, STM_OP_CHECK_OFFLINE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_CONFLICT, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		  stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_ASSIGNED_OPEN, STM_OP_STOP_READWRITE)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_ASSIGNED_ACTIVE, STM_OP_STOP_READWRITE)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_ASSIGNED_CLOSED, STM_OP_STOP_READWRITE)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_ASSIGNED_EXPORTED, STM_OP_STOP_READWRITE)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_UN_ASSIGNED, STM_OP_STOP_READWRITE)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_RAW, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_WRITE_PROTECT, STM_OP_STOP_READWRITE)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_ACCESSING, STM_OP_XXX)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_DIAGNOSING, STM_OP_DIAGNOSE)] = "faulty=true"; //
		//stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_CONFLICT, STM_OP_STOP_READWRITE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_OFFLINE, STM_OP_CHECK_OFFLINE)] = ""; //
		stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_READWRITE, STM_OP_READ)] = ""; //								Reading
		stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_READWRITE, STM_OP_WRITE)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_READWRITE, STM_ST_CHECKING, STM_OP_IMPORT_CHECK)] = ""; //

		  //stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_ASSIGNED_OPEN, STM_OP_RECYCLE)] = ""; // (Format/Clean Caches)
		//stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_ASSIGNED_ACTIVE, STM_OP_STOP_WRITE)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_ASSIGNED_CLOSED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_ASSIGNED_EXPORTED, STM_OP_FINISH_CHECK)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_UN_ASSIGNED, STM_OP_FINISH_CHECK)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_RAW, STM_OP_XXX)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_WRITE_PROTECT, STM_OP_FINISH_DIAGNOSE)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_ACCESSING, STM_OP_XXX)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_DIAGNOSING, STM_OP_DIAGNOSE)] = "faulty=true"; //
		//stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_UN_SUPPORTED, STM_OP_XXX)] = ""; //
		//stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_CONFLICT, STM_OP_XXX)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_OFFLINE, STM_OP_CHECK_OFFLINE)] = ""; //
		  //stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_READWRITE, STM_OP_READ)] = ""; //								Reading
		  //stmStateMap[StmStateToStr(STM_ST_CHECKING, STM_ST_CHECKING, STM_OP_CHECK_OFFLINE)] = ""; //
    }
    string ShareOpToStr(const string& shareUuid, STM_OPERATION stmOp)
    {
        if(shareUuid == ""){
            return "";
        }

        string strRet = "";;
        if(stmOp == STM_OP_IMPORT){ // || stmOp == STM_OP_EXPORT || stmOp == STM_OP_RECYCLE){
            strRet += "IMPORT_EXPORT_RECYCLE";
        }

        if(strRet != ""){
            strRet = boost::lexical_cast<string>(shareUuid) + string(":") + strRet;
        }

        return strRet;
    }

    bool TapeStateMachineMgr::StmRequestTape(const string& barcode, STM_OPERATION stmOperation, const string& shareUuid, int millTimeOut)
    {
        string strShareOp = ShareOpToStr(shareUuid, stmOperation);
        boost::system_time timeOut = boost::get_system_time() + boost::posix_time::milliseconds(millTimeOut);

        bool bRet = StmRequestTape(barcode, millTimeOut);
        if(bRet && strShareOp != ""){
            if(stmShareOpMutexMap_.find(strShareOp) == stmShareOpMutexMap_.end()){
                boost::timed_mutex *pMutex = new boost::timed_mutex();
                stmShareOpMutexMap_[strShareOp] = pMutex;
            }
            if( false == stmShareOpMutexMap_[strShareOp]->timed_lock(timeOut)){
                stmTapeMutexMap_[barcode]->unlock();
                bRet = false;
            }else{
                stmTapeOpMap_[barcode] = stmShareOpMutexMap_[strShareOp];
            }
        }

        LtfsLogDebug("StmRequestTape: finished requesting tape " << barcode << ", stmOperation = "
                << stmOperation << ", shareUuid = " << shareUuid << ", result: " << bRet << ".");
        return bRet;
    }

    bool TapeStateMachineMgr::StmRequestTape(const string& barcode, int millTimeOut)
    {
        LtfsLogDebug("StmRequestTape: start requesting tape " << barcode << ", time out in millisecond is " << millTimeOut);
        if(barcode == ""){
            return false;
        }

        if(stmTapeMutexMap_.find(barcode) == stmTapeMutexMap_.end()){
            boost::timed_mutex *pMutex = new boost::timed_mutex();
            stmTapeMutexMap_[barcode] = pMutex;
        }

        boost::system_time timeOut = boost::get_system_time() + boost::posix_time::milliseconds(millTimeOut);
        bool bRet = stmTapeMutexMap_[barcode]->timed_lock(timeOut);

        LtfsLogDebug("StmRequestTape: finished requesting tape " << barcode << ", result: " << bRet << ".");
        return bRet;
    }

    void TapeStateMachineMgr::StmReleaseTape(const string& barcode)
    {
        LtfsLogDebug("StmReleaseTape: " << barcode << " start.");
        if(barcode == ""){
            return;
        }

        if(stmTapeMutexMap_.find(barcode) == stmTapeMutexMap_.end()){
            LtfsLogDebug("StmReleaseTape: " << barcode << " error finished.");
            return;
        }

        map<string, boost::timed_mutex*>::iterator iter = stmTapeOpMap_.find(barcode);
        if(iter != stmTapeOpMap_.end() && iter->second != NULL){
            stmTapeOpMap_[barcode]->unlock();
            stmTapeOpMap_[barcode] = NULL;
        }

        stmTapeMutexMap_[barcode]->unlock();
        LtfsLogDebug("StmReleaseTape: " << barcode << " finished.");
    }

    bool TapeStateMachineMgr::StmGetTapeList(STM_OPERATION stmOperation, STM_STATE stmState, vector<TapeInfo>& tapes)
    {
        map<STM_OPERATION, STM_STATE> stmMap;
        stmMap[stmOperation] = stmState;

        return StmGetTapeList(stmMap, tapes);
    }

    bool TapeStateMachineMgr::StmGetTapeList(map<STM_OPERATION, STM_STATE> stmOpStateMap, vector<TapeInfo>& tapes)
    {
        vector<LtfsChangerInfo> changers;
        LtfsError lfsErr;
        if(false == TapeLibraryMgr::Instance()->GetChangerList(changers, lfsErr)){
            return false;
        }

        for(unsigned int i = 0; i < changers.size(); i++){
            vector<TapeInfo> lfsTapes;
            if(false == TapeLibraryMgr::Instance()->GetTapeListForChanger(changers[i].mSerial, lfsTapes, lfsErr)){
                return false;
            }
            for(unsigned int j = 0; j < lfsTapes.size(); j++){
                for(map<STM_OPERATION, STM_STATE>::iterator it = stmOpStateMap.begin(); it != stmOpStateMap.end(); it++){
                    if(StmTapeCanBeChangedTo(lfsTapes[j].mBarcode, it->second, it->first)){
                        tapes.push_back(lfsTapes[j]);
                        break;
                    }
                }
            }
        }
        return true;
    }

    STM_STATE TapeStateMachineMgr::GetTapeStmState(const string& barcode)
    {
        TapeInfo tapeInfo;
        if(false == TapeLibraryMgr::Instance()->GetTape(barcode, tapeInfo)){
            return STM_ST_INVALID;
        }

        STM_STATE stmState = GetTapeStmState(tapeInfo);
        return stmState;
    }


    STM_STATE TapeStateMachineMgr::GetTapeStmState(const TapeInfo& tapeInfo)
    {
    	TapeLibraryMgr::Instance()->LogDebugTapeInfo(tapeInfo);

        STM_STATE  stRet = STM_ST_UN_SUPPORTED;

        // state priority: STM_ST_OFFLINE > WP > UN_SUP > ACC > DIA > others
        if(tapeInfo.mOffline == true){
        	LtfsLogDebug("DDEBUG: tapeInfo.mOffline = " << tapeInfo.mOffline << ", (tapeInfo.mOffline == true) = " << (tapeInfo.mOffline == true));
            stRet = STM_ST_OFFLINE;
        }
        else if(tapeInfo.mWriteProtect == true){
            stRet = STM_ST_WRITE_PROTECT;
        }
        else if((tapeInfo.mMediumType != MEDIUM_DATA)
            || (tapeInfo.mMediaType != MEDIA_LTO6 && tapeInfo.mMediaType != MEDIA_LTO5)
            || (tapeInfo.mWriteProtect == true)
            || (tapeInfo.mStatus == TAPE_CLEAN_EXPIRED)
        ){
            stRet = STM_ST_UN_SUPPORTED;
        }
        else if(tapeInfo.mActivity == ACT_DIAGNOSING || tapeInfo.mState == TAPE_STATE_DIAGNOSING){
            stRet = STM_ST_DIAGNOSING;
        }
        else if(tapeInfo.mActivity == ACT_ACCESSING || tapeInfo.mState == TAPE_STATE_ACCESSING){
            stRet = STM_ST_ACCESSING;
        }
        else if(tapeInfo.mState == TAPE_STATE_READWRITE){
            stRet = STM_ST_READWRITE;
        }
        else if(tapeInfo.mStatus == TAPE_CONFLICT){
            stRet = STM_ST_CONFLICT;
        }
        else if(tapeInfo.mStatus == TAPE_OPEN && TapeLibraryMgr::Instance()->IsNativeTapeGroup(tapeInfo.mGroupID)){
            stRet = STM_ST_ASSIGNED_OPEN;
        }
        else if(tapeInfo.mStatus == TAPE_CLOSED && TapeLibraryMgr::Instance()->IsNativeTapeGroup(tapeInfo.mGroupID)){
            stRet = STM_ST_ASSIGNED_CLOSED;
        }
        else if(tapeInfo.mStatus == TAPE_ACTIVE && TapeLibraryMgr::Instance()->IsNativeTapeGroup(tapeInfo.mGroupID)){
            stRet = STM_ST_ASSIGNED_ACTIVE;
        }
        else if(tapeInfo.mStatus == TAPE_EXPORTED){
            stRet = STM_ST_ASSIGNED_EXPORTED;
        }
        else if(tapeInfo.mLtfsFormat == LTFS_VALID){
            stRet = STM_ST_UN_ASSIGNED;
        }
        else if(tapeInfo.mLtfsFormat != LTFS_VALID){
            stRet = STM_ST_RAW;
        }
        else{
            LtfsLogError("Tape state not match any, take it as invalid state.");
        }

        LtfsLogInfo("Tape state for " << tapeInfo.mBarcode << " is " << GetStmStateStr(stRet) << ".");
        return stRet;
    }

    string TapeStateMachineMgr::GetStmStateStr(STM_STATE stmState)
    {
    	static map<STM_STATE, string> sStateStrMap;
    	if(sStateStrMap.size() <= 0){
			sStateStrMap[STM_ST_INVALID] = "STM_ST_INVALID";
			sStateStrMap[STM_ST_ASSIGNED_OPEN] = "STM_ST_ASSIGNED_OPEN";
			sStateStrMap[STM_ST_ASSIGNED_ACTIVE] = "STM_ST_ASSIGNED_ACTIVE";
			sStateStrMap[STM_ST_ASSIGNED_CLOSED] = "STM_ST_ASSIGNED_CLOSED";
			sStateStrMap[STM_ST_ASSIGNED_EXPORTED] = "STM_ST_ASSIGNED_EXPORTED";
			sStateStrMap[STM_ST_UN_ASSIGNED] = "STM_ST_UN_ASSIGNED";
			sStateStrMap[STM_ST_RAW] = "STM_ST_RAW";
			sStateStrMap[STM_ST_WRITE_PROTECT] = "STM_ST_WRITE_PROTECT";
			sStateStrMap[STM_ST_ACCESSING] = "STM_ST_ACCESSING";
			sStateStrMap[STM_ST_DIAGNOSING] = "STM_ST_DIAGNOSING";
			sStateStrMap[STM_ST_UN_SUPPORTED] = "STM_ST_UN_SUPPORTED";
			sStateStrMap[STM_ST_CONFLICT] =  "STM_ST_CONFLICT";
			sStateStrMap[STM_ST_OFFLINE] =  "STM_ST_OFFLINE";
			sStateStrMap[STM_ST_READWRITE] =  "STM_ST_READWRITE";
    	}
    	if(sStateStrMap.find(stmState) != sStateStrMap.end()){
    		return sStateStrMap[stmState] + string("(") + boost::lexical_cast<string>(stmState) + string(")");
    	}
    	return "unknown";
    }
    string TapeStateMachineMgr::GetStmOperationStr(STM_OPERATION stmOp)
    {
    	static map<STM_OPERATION, string> sOpStrMap;
    	if(sOpStrMap.size() <= 0){
			sOpStrMap[STM_OP_WRITE] = "STM_OP_WRITE";
			sOpStrMap[STM_OP_READ] = "STM_OP_READ";
			sOpStrMap[STM_OP_STOP_READWRITE] = "STM_OP_STOP_READWRITE";
			sOpStrMap[STM_OP_RECYCLE] = "STM_OP_RECYCLE";
			sOpStrMap[STM_OP_ASSIGN] = "STM_OP_ASSIGN";
			sOpStrMap[STM_OP_INVENTORY] = "STM_OP_INVENTORY";
			sOpStrMap[STM_OP_CLOSE] = "STM_OP_CLOSE";
			sOpStrMap[STM_OP_IMPORT] = "STM_OP_IMPORT";
			sOpStrMap[STM_OP_EXPORT] = "STM_OP_EXPORT";
			sOpStrMap[STM_OP_UNACCESS] = "STM_OP_UNACCESS";
			sOpStrMap[STM_OP_REVOKE] = "STM_OP_REVOKE";
			sOpStrMap[STM_OP_UNASSIGN] = "STM_OP_UNASSIGN";
			sOpStrMap[STM_OP_FORMAT] = "STM_OP_FORMAT";
			sOpStrMap[STM_OP_ACCESS] = "STM_OP_ACCESS";
			sOpStrMap[STM_OP_DIAGNOSE] = "STM_OP_DIAGNOSE";
			sOpStrMap[STM_OP_EJECT] = "STM_OP_EJECT";
			sOpStrMap[STM_OP_CREATE_CACHE] = "STM_OP_CREATE_CACHE";
    	}

    	if(sOpStrMap.find(stmOp) != sOpStrMap.end()){
    		return sOpStrMap[stmOp] + string("(") + boost::lexical_cast<string>(stmOp) + string(")");
    	}
    	return "unknown";
    }

    bool TapeStateMachineMgr::StmTapeCanBeChangedTo(const string& barcode, STM_STATE stmState, STM_OPERATION operation)
    {
        STM_STATE curState = GetTapeStmState(barcode);
        TapeInfo tapeInfo;
        if(false == TapeLibraryMgr::Instance()->GetTape(barcode, tapeInfo)){
        	LtfsLogError("Failed get info of tape " << barcode << ". Change to state " \
        			<< GetStmStateStr(stmState) << " with operation " << GetStmOperationStr(operation) << " is not allowed.");
            return false;
        }

        string strCondition = "";
        map<string, string>::iterator it = stmStateMap.find(StmStateToStr(curState, stmState, operation));
        if(it != stmStateMap.end()){
        	bool bAllowed = true;
        	if(it->second != ""){
        		if(it->second.find(STM_CONDITION_TAPE_FAULTY) != string::npos){
        			if(tapeInfo.mFaulty == false){
        				bAllowed = false;
        				strCondition += STM_CONDITION_TAPE_FAULTY;
        			}
        		}

        		if(it->second.find(STM_CONDITION_TAPE_NO_FORMAT_TASK) != string::npos){
        			FormatThreadStatus formatStatus = FORMAT_THREAD_STATUS_FINISHED;
        			TapeLibraryMgr::Instance()->GetFormatStatus(tapeInfo.mBarcode, formatStatus);
        			if(formatStatus != FORMAT_THREAD_STATUS_FINISHED){
        				bAllowed = false;
        				strCondition += STM_CONDITION_TAPE_NO_FORMAT_TASK;
        			}
        		}

        		if(it->second.find(STM_CONDITION_TAPE_EMPTY) != string::npos){
        			if(tapeInfo.mGenerationIndex > 1 || tapeInfo.mFileNumber > 0){
        				bAllowed = false;
        				strCondition += STM_CONDITION_TAPE_EMPTY;
        			}
        		}
        	}
        	if(bAllowed){
				LtfsLogDebug("It is allowed to change tape " << barcode << " state machine status from " << GetStmStateStr(curState) << " to " << GetStmStateStr(stmState) \
						<< ". The operation is " << GetStmOperationStr(operation) << ".");
				return true;
        	}
        }

        string strLog = "It is not allowed to set tape " + barcode + string(" state machine status from ") + GetStmStateStr(curState) + \
        	string(" to ") + GetStmStateStr(stmState) + string(" by operation ") + GetStmOperationStr(operation) + string(".");
        if(strCondition != ""){
        	strLog += string(" Required condition: ") + strCondition;
        }
        LtfsLogDebug(strLog);
        return false;
    }

    bool TapeStateMachineMgr::StmTapeCanBeChangedTo(const string& barcode, STM_STATE stmState, const vector<STM_OPERATION> operations)
   {
	   STM_STATE curState = GetTapeStmState(barcode);
	   TapeInfo tapeInfo;
	   if(false == TapeLibraryMgr::Instance()->GetTape(barcode, tapeInfo)){
		LtfsLogError("Failed get info of tape " << barcode << ". Change to state " \
				<< GetStmStateStr(stmState) << " is not allowed.");
		   return false;
	   }

	   for(unsigned int i = 0; i < operations.size(); i++){
		  string strCondition = "";
		  map<string, string>::iterator it = stmStateMap.find(StmStateToStr(curState, stmState, operations[i]));
		  if(it != stmStateMap.end()){
			bool bAllowed = true;
			if(it->second != ""){
				if(it->second.find(STM_CONDITION_TAPE_FAULTY) != string::npos){
					if(tapeInfo.mFaulty == false){
						bAllowed = false;
						strCondition += STM_CONDITION_TAPE_FAULTY;
					}
				}

				if(it->second.find(STM_CONDITION_TAPE_NO_FORMAT_TASK) != string::npos){
					FormatThreadStatus formatStatus = FORMAT_THREAD_STATUS_FINISHED;
					TapeLibraryMgr::Instance()->GetFormatStatus(tapeInfo.mBarcode, formatStatus);
					if(formatStatus != FORMAT_THREAD_STATUS_FINISHED){
						bAllowed = false;
						strCondition += STM_CONDITION_TAPE_NO_FORMAT_TASK;
					}
				}

				if(it->second.find(STM_CONDITION_TAPE_EMPTY) != string::npos){
					if(tapeInfo.mGenerationIndex > 1 /*|| tapeInfo.mFileNumber > 0*/){
						bAllowed = false;
						strCondition += STM_CONDITION_TAPE_EMPTY;
					}
				}
			}
			if(bAllowed){
				LtfsLogDebug("It is allowed to change tape state machine status from " << GetStmStateStr(curState) << " to " << GetStmStateStr(stmState) \
						<< ". The operation is " << GetStmOperationStr(operations[i]) << ".");
				return true;
			}
		  }

		  string strLog = "It is not allowed to set tape state machine status from " + GetStmStateStr(curState) + \
			string(" to ") + GetStmStateStr(stmState) + string(" by operation ") + GetStmOperationStr(operations[i]) + string(".");
		  if(strCondition != ""){
			strLog += string(" Required condition: ") + strCondition;
		  }
	   }

	   return false;
   }
}

