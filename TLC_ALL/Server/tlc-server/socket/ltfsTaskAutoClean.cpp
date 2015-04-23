/*
 * ltfsTaskAutoClean.cpp
 *
 *  Created on: Dec 18, 2012
 *      Author: Sam Chen
 */

#include "ltfsTaskAutoClean.h"
#include "../ltfs_management/TapeLibraryMgr.h"

using namespace ltfs_management;

#define CLEAN_DRIVE_REQUEST_WAIT 	600   // 10 minutes

#ifdef LTFS_VSB_MODE
	#define NO_CLEAN_TAPE_EVENT_THROTTLE		24 * 60 * 60  // 1 day
	#define	CLEAN_TAPE_EXPIRED_EVENT_THROTTLE	24 * 60 * 60  // 1 day
	#define FIND_CLEAN_TAPE_WAIT				10 * 60  // 10 minutes
#endif


namespace ltfs_soapserver
{

	map<string, bool>				TaskAutoClean::driveCleaningTaskMap_;
	boost::mutex        			TaskAutoClean::driveCleaningTaskMutex_;

	TaskAutoClean::TaskAutoClean(const string& changerSerial, const vector<string> driveSerials):
			Task(Type_CleanDrive, changerSerial, driveSerials)
	{
		m_changerSerial = changerSerial;
		m_driveSerials = driveSerials;
		if(driveSerials.size() > 0){
			m_driveSerial = driveSerials[0];
		}
	}

	TaskAutoClean::~TaskAutoClean()
	{
	}

#ifndef LTFS_VSB_MODE
	string TaskAutoClean::FindValidCleaningTape()
	{
    	string cleanTape = "";
    	vector<TapeInfo> tapes;
    	LtfsError lfsErr;
    	// find a valid cleaning tape
    	if(false == TapeLibraryMgr::Instance()->GetTapeListForChanger(m_changerSerial, tapes, lfsErr)){
    		SocketError("Failed to get tape list to clean drive " << m_driveSerial << ".");
    	}
    	for(unsigned int i = 0; i < tapes.size(); i++){
    		if(tapes[i].mMediumType == MEDIUM_CLEANING && tapes[i].mStatus != TAPE_CLEAN_EXPIRED){
    			cleanTape = tapes[i].mBarcode;
    			break;
    		}
    	}
		return cleanTape;
	}
#else
	string TaskAutoClean::FindValidCleaningTape(int& cleaningStatus)
	{
    	string cleanTape = "";
    	TapeInfo tapeInfo;
    	LtfsError lfsErr;
    	// find a valid cleaning tape
    	if(false == TapeLibraryMgr::Instance()->GetTapeInDrive(m_driveSerial, tapeInfo, true)){
    		SocketError("Failed to get clean tape to clean drive " << m_driveSerial << ".");
    	}
    	else if(tapeInfo.mMediumType == MEDIUM_CLEANING && tapeInfo.mStatus != TAPE_CLEAN_EXPIRED){
			cleanTape = "CLN_TAPE";
			cleaningStatus = tapeInfo.mStatus;
		}

		return cleanTape;
	}
#endif

	void TaskAutoClean::Execute()
	{
		int driveLogicId = -1;
		int clnTapeStatus = TAPE_UNKNOWN;

		if(m_driveSerial == ""){
			SocketError("No drive specified, clean take will not run.");
			excludeByQuery_ = true;
			status_ = Status_Failed;
			MarkEnd();
			SaveStateToFile();
			return;
		}

		if(1){
			boost::mutex::scoped_lock lock(driveCleaningTaskMutex_);
			map<string, bool>::iterator it = driveCleaningTaskMap_.find(m_changerSerial + string(":") + m_driveSerial);
			if(it != driveCleaningTaskMap_.end() && it->second == true){
				SocketInfo("Auto cleaning task on drive " << m_driveSerial << " already running, will not start new task for it.");
				excludeByQuery_ = true;
				status_ = Status_Failed;
				MarkEnd();
				SaveStateToFile();
				return ;
			}
	    	driveCleaningTaskMap_[m_changerSerial + string(":") + m_driveSerial] = true;
		}

		SocketInfo("Starting auto cleaning task on drive " << m_driveSerial << " on changer " << m_changerSerial << ".");

    	LtfsError lfsErr;
		vector<LtfsDriveInfo> drives;
    	bool bNeedReleaseTape = false;
    	int retry = 0;
    	status_ = Status_Running;
    	SaveStateToFile();

		// get logic id of the drive
		if(true == TapeLibraryMgr::Instance()->GetDriveListForChanger(m_changerSerial, drives, lfsErr)){
			for(unsigned int i = 0; i < drives.size(); i++){
				if(drives[i].mSerial == m_driveSerial){
					driveLogicId = drives[i].mLogicSlotID;
					break;
				}
			}
		}

    	// find a valid cleaning tape
#ifndef LTFS_VSB_MODE
		string cleanTape = FindValidCleaningTape();
    	if(cleanTape == ""){
    		SocketError("No available clean tape found to clean drive " << m_driveSerial << " for changer " << m_changerSerial << ".");
    		SocketEvent(EVENT_LEVEL_WARNING, "Lib_No_Cln_Tape", "Drive " << driveLogicId << " requires cleaning. No cleaning tape found. Please insert a cleaning tape.");//TODO:EVENT
    		status_ = Status_Failed;
    		goto DRIVE_CLEANING_END;
    	}
#else
		string cleanTape = FindValidCleaningTape(clnTapeStatus);
		time_t lastNoTapeEvent = 0;
		time_t lastExpiredEvent = 0;
		while(cleanTape == ""){
			SocketWarn("No available clean tape found to clean drive " << m_driveSerial << ".");
			if(time(NULL) - lastNoTapeEvent > NO_CLEAN_TAPE_EVENT_THROTTLE){
				SocketEvent(EVENT_LEVEL_WARNING, "Lib_No_Cln_Tape", "Drive " << driveLogicId << " requires cleaning. No cleaning tape found. Please insert a cleaning tape.");//TODO:EVENT
				lastNoTapeEvent = time(NULL);
			}
			sleep(FIND_CLEAN_TAPE_WAIT);
			int cleaningStatus = 0;
			if(TapeLibraryMgr::Instance()->GetDriveCleaningStatus(m_changerSerial, m_driveSerial, cleaningStatus, lfsErr, true)){
				if(cleaningStatus != CLEANING_REQUIRED && cleaningStatus != CLEANING_IN_PROGRESS){
					SocketDebug("Cleaning of drive " << m_driveSerial << " finished.");
					status_ = Status_Finish;
					goto DRIVE_CLEANING_END;
				}
			}
			cleanTape = FindValidCleaningTape(clnTapeStatus);
			if(cleanTape != "" && clnTapeStatus == TAPE_CLEAN_EXPIRED){
				if(time(NULL) - lastExpiredEvent > CLEAN_TAPE_EXPIRED_EVENT_THROTTLE){
					SocketEvent(EVENT_LEVEL_WARNING, "Tape_Cln_Exp","Cleaning tape expired.");
					lastExpiredEvent = time(NULL);
				}
				SocketWarn("Cleaning tape " << cleanTape << " expired.");  // notification and event
			}
		}
#endif


#ifndef LTFS_VSB_MODE
    	// load clean tape to drive to start cleaning
    	if(false == TapeLibraryMgr::Instance()->BindTape(cleanTape, m_changerSerial, m_driveSerial)){
    		SocketError("Failed to load clean tape " << cleanTape << " to drive " << m_driveSerial << " to clean it.");
    		status_ = Status_Failed;
    		goto DRIVE_CLEANING_END;
    	}else if(false == TapeLibraryMgr::Instance()->RequestTape(cleanTape, false, \
    			bdt::ScheduleInterface::PRIORITY_DRIVE_CLEAN, CLEAN_DRIVE_REQUEST_WAIT)){
    		TapeLibraryMgr::Instance()->UnbindTape(cleanTape);
    		SocketError("Failed to load clean tape " << cleanTape << " to drive " << m_driveSerial << " to clean it.");
    		status_ = Status_Failed;
    		goto DRIVE_CLEANING_END;
    	}

    	// check if the tape in clean drive is expired
    	//if(false == LtfsLibraries::Instance()->GetLoadedTapeStatus(m_changerSerial, m_driveSerial, clnTapeStatus, lfsErr)){
    	if(false == TapeLibraryMgr::Instance()->GetLoadedTapeStatus(cleanTape, clnTapeStatus, lfsErr)){
    		SocketError("Failed to get status for clean tape " << cleanTape << ".");
    	}
		if(clnTapeStatus == TAPE_CLEAN_EXPIRED){
			//SocketEvent(EVENT_LEVEL_WARNING,"Tape_Cln_Exp","Cleaning tape " << cleanTape << " expired.");
			SocketEvent(EVENT_LEVEL_WARNING,"Tape_Cln_Exp","Cleaning tape expired.");
			SocketWarn("Cleaning tape " << cleanTape << " expired.");  // notification and event  TODO:EVENT
			if(false == TapeLibraryMgr::Instance()->SetLoadedTapeStatus(cleanTape, clnTapeStatus,lfsErr,true)){
				SocketError("Failed to update clean tape " << cleanTape << " in drive " << m_driveSerial << " to expired status.");
			}
			// release current clean tape and try to find another one
        	if(false == TapeLibraryMgr::Instance()->UnbindTape(cleanTape)
        	|| false == TapeLibraryMgr::Instance()->ReleaseTape(cleanTape)){
        		SocketError("Failed to release expired clean tape " << cleanTape << " in drive " << m_driveSerial << ".");
        	}
        	cleanTape = FindValidCleaningTape();
        	if(cleanTape == ""){
        		SocketError("No available clean tape found to clean drive " << m_driveSerial << " for changer " << m_changerSerial << ".");
        		SocketEvent(EVENT_LEVEL_WARNING, "Lib_No_Cln_Tape", "Drive " << driveLogicId << " requires cleaning. No cleaning tape found. Please insert a cleaning tape."); //TODO:EVENT
        		status_ = Status_Failed;
        		goto DRIVE_CLEANING_END;
        	}
		}
    	bNeedReleaseTape = true;
#endif

    	// here we start cleaning, fire an event for it
		if(driveLogicId < 0){
			SocketError("Failed to get logical slot id for drive " << m_driveSerial << ".");
		}else{
			//send event/alert
			SocketEvent(EVENT_LEVEL_INFO, "Drv_Cln_Start", "Cleaning of drive " << driveLogicId << " started."); // Event/Notification/Alert
		}

    	// wait a moment and check if drive is cleaned
    	sleep(30);
    	retry = 180; //  wait up to 180*10 seconds till we found drive cleaned or exit this task
        while(retry-- > 0){
        	int cleaningStatus = 0;
        	if(false == TapeLibraryMgr::Instance()->GetDriveCleaningStatus(m_changerSerial, m_driveSerial, cleaningStatus, lfsErr, true)){
        		SocketWarn("Failed to get drive cleaning status for drive " << m_driveSerial
        				<< ". Err: " << lfsErr.GetErrMsg() << ". Will retry later.");
        		sleep(10);
        		continue;
        	}
        	if(cleaningStatus != CLEANING_REQUIRED && cleaningStatus != CLEANING_IN_PROGRESS){
#ifdef LTFS_VSB_MODE
				SocketInfo("Succeed to clean drive " << m_driveSerial << ".");
#else
				SocketInfo("Succeed to use clean tape " << cleanTape << " to clean drive " << m_driveSerial << ".");
#endif
	    		status_ = Status_Finish;
				goto DRIVE_CLEANING_END;
        	}
        	sleep(10);
        }

        if(retry <= 0){
        	SocketError("Failed to use clean tape " << cleanTape << " to clean drive " << m_driveSerial << " on changer " << m_changerSerial);
        }

DRIVE_CLEANING_END:
#ifndef LTFS_VSB_MODE
        if(bNeedReleaseTape){
        	// release the clean tape
        	if(false == TapeLibraryMgr::Instance()->UnbindTape(cleanTape)
        	|| false == TapeLibraryMgr::Instance()->ReleaseTape(cleanTape)){
        		SocketError("Failed to release clean tape " << cleanTape << " which is used to clean drive " << m_driveSerial << ".");
        	}
        }
#endif
		if(status_ != Status_Failed){
			status_ = Status_Finish;
			//send event/alert
			if(driveLogicId >= 0){
				SocketEvent(EVENT_LEVEL_INFO, "Drv_Cln_Finished", "Cleaning of drive " << driveLogicId << " finished.");// Event/Notification/Alert
			}

		}else{
			//send event/alert
			if(driveLogicId >= 0){
				SocketEvent(EVENT_LEVEL_ERR, "Drv_Cln_Failed", "Failed to clean drive " << driveLogicId << ".");// Event/Notification/Alert
			}
		}
		MarkEnd();
		SaveStateToFile();
		if(1){
			boost::mutex::scoped_lock lock(driveCleaningTaskMutex_);
			driveCleaningTaskMap_[m_changerSerial + string(":") + m_driveSerial] = false;
		}

		return;
	}

} /* namespace ltfs_soapserver */
