/*
 * ltfsTaskAuditTape.cpp
 *
 *  Created on: Nov 20, 2014
 *      Author: zhaona
 */

#include <fstream>
#include "ltfsTaskAuditTape.h"
#include "../ltfs_management/CatalogDbManager.h"

using namespace ltfs_management;
using namespace ltfs_soapserver;

#define CHECK_TAPE_REQUEST_WAIT	1800  // time out for requesting resource to check tape,  30 minutes

namespace ltfs_soapserver
{
	TaskAuditTape::TaskAuditTape(const vector<string> &barcodes, bool bNeedLockTape):
			Task(Type_AuditTape, "", barcodes)
	{
		groupID_ = "";
		mGroupName_ = "";
		m_barcodes = barcodes;
		bNeedLockTape_ = bNeedLockTape;
		pid_ = -1;
	}

	TaskAuditTape::~TaskAuditTape()
	{
		StopAudit();
	}

	bool TaskAuditTape::StopAudit()
	{
		SocketDebug("Auditing process id: " << pid_);
		if(pid_ != -1){
			string cmd = "kill -9 " + boost::lexical_cast<string>(pid_) + " 2>/dev/null 1>/dev/null";
			if(0 != ::system(cmd.c_str())){
				SocketError("Failed to stop running audit process " << pid_ << ", cmd = " << cmd);
				return false;
			}
			return true;
		}
		SocketInfo("Auditing process not started yet.");
		return true;
	}

	bool TaskAuditTape::HandleFile(const string& filePath)
	{
		if(filePath == ""){
			return true;
		}
		try{
			string uuid = "";
			//     //00/00/00/00/00/00/04/cc
			regex matchPath("^\\s*\\/\\/(\\w+)\\/(\\w+)\\/(\\w+)\\/(\\w+)\\/(\\w+)\\/(\\w+)\\/(\\w+)\\/(\\w+)\\s*$");
			cmatch match;
			if(!regex_match(filePath.c_str(), match, matchPath)){
				SocketError("Failed to get number from file path " << filePath);
				return false;
			}
			int number = 0;
			for(unsigned int i = 1; i <= 8; i++){
				std::stringstream str;
				str << match[i];
				int value;
				str >> std::hex >> value;
				int shift = 8 * (8 - i);
				number += (value << shift);
			}
			uuid = boost::lexical_cast<string>(number);
			bool bRet = CatalogDbManager::Instance()->SetFileCorrupted(groupID_, mGroupName_, uuid, true);
			return bRet;
		}catch(...){
			SocketError("Failed to handle corrupted file " << filePath);
		}
		return false;
	}

	void TaskAuditTape::Execute()
	{
		bool bRet = true;
		bool bNeedReleaseTape = false;
		string errMsg = "";
		LtfsError lfsErr;

		if(m_barcodes.size() <= 0 || m_barcodes[0] == ""){
			SocketError("Audit tape '" << m_barcodes[0] << "' failed: no tape with barcode found.");
			status_ = Status_Failed;
			bRet = false;
			MarkEnd();
			SaveStateToFile();
			return;
		}

		TapeLibraryMgr *pTapeMgr = TapeLibraryMgr::Instance();

		string barcode = m_barcodes[0];
		vector<fs::path> failedFileList;
		TapeInfo tapeInfo;
		if(TapeLibraryMgr::Instance()->GetTape(barcode, tapeInfo)){
			groupID_ = tapeInfo.mGroupID;
			if(!TapeLibraryMgr::Instance()->GetGroupName(groupID_, mGroupName_)){
				SocketError("Failed to get share name from uuid " << groupID_);
			}
		}else{
			SocketError("Failed to get tape info for " << barcode);
		}

		bool bStmNeedReleaseTape = false;
    	if(groupID_ == "" || mGroupName_ == ""){
			errMsg = "failed to get group uuid or group name for tape " + barcode + ". uuid: " + groupID_ + ", name: " + mGroupName_;
			bRet = false;
			goto ERR_RETURN;
    	}


		if (bNeedLockTape_) {
			for(vector<TapeStatusPair>::iterator iterBarcode=barcodeList_.begin();
				iterBarcode!=barcodeList_.end(); ++iterBarcode)
			{
				if(false == TapeLibraryMgr::Instance()->StmRequestTape(iterBarcode->mBarcode))
				{
					SocketError("Cann't get tape " << iterBarcode->mBarcode <<" lock when Recover CheckTape task");
					MarkEnd();
					return ;
				}
			}
		}
		bStmNeedReleaseTape = true;

		/*//TODO
		// check if we can change the tape to target status
		if(false == TapeLibraryMgr::Instance()->StmTapeCanBeChangedTo(barcode, STM_ST_DIAGNOSING, STM_OP_DIAGNOSE)){
			SocketError("The tape " << barcode << " is not allowed to be changed to " << TapeLibraryMgr::Instance()->GetStmStateStr(STM_ST_DIAGNOSING)
					<< " state for diagnosing.");
			status_ = Status_Failed;
			bRet = false;
			MarkEnd();
			SaveStateToFile();
			return;
		}*/

		pTapeMgr = TapeLibraryMgr::Instance();
    	if(false == TapeLibraryMgr::Instance()->SetTapeState(m_barcodes[0], TAPE_STATE_DIAGNOSING)){
			errMsg = "failed to set tape state to diagnosing.";
			bRet = false;
			goto ERR_RETURN;
    	}

		SocketInfo("Audit tape '" << m_barcodes[0] << "' started.");
		status_ = Status_Running;

		while(pTapeMgr != NULL){
			SocketInfo("Audit tape: requesting resource for the tape.");
			// request tape
			if(true == pTapeMgr->RequestTape(m_barcodes[0], false, \
					bdt::ScheduleInterface::PRIORITY_AUDIT_TAPE, CHECK_TAPE_REQUEST_WAIT)){
				bNeedReleaseTape = true;
				SocketInfo("Audit tape: requesting resource for the tape done.");
				break;
			}
			sleep(60 * 3);
		}

		SocketDebug("Audit tape: unmounting the tape.");
		// should unmount the tape first?
		if(false == pTapeMgr->UnmountTape(m_barcodes[0])){
			SocketError("Audit Tape: Failed to unmount tape " << m_barcodes[0] << " to audit it.");
		}

		{
			SocketInfo("Audit tape: auditing the tape.");
			LtfsDriveInfo driveInfo;
			if(!TapeLibraryMgr::Instance()->GetDriveInfoForTape(m_barcodes[0], driveInfo)){
				SocketError("Audit Tape: Failed to get drive info.");
				errMsg = "Failed to get drive info.";
				bRet = false;
				goto ERR_RETURN;
			}

			TapeLibraryMgr::Instance()->SetTapeActivity(barcode, ACT_AUDITING);
			string fileListPath = COMM_AUDIT_OUTPUT_FOLDER + "/" + m_barcodes[0] + ".auditor";
			string cmd = "cd " + COMM_AUDIT_TOOL_FOLDER + " && ./" + COMM_AUDIT_TOOL_BIN + " -f " + driveInfo.mStDev + " -g " + driveInfo.mSgDev + " -l " + fileListPath;
			//cmd += " -b 1000 -q 20000";//uncomment this line to enable simulating bad blocks
			cmd += " 2>&1 ";
			int ret = 0;
			bool bAuditRet = false;
			vector<string> outPuts = GetCommandOutputLines(cmd, ret, pid_, 60 * 60 * 24 * 7);
			TapeLibraryMgr::Instance()->SetTapeActivity(barcode, ACT_IDLE);
			pid_ = -1;
			for(unsigned int i = 0; i < outPuts.size(); i++){
				//Complete verification
				//Volume unmounted successfully
				//Blocks verified
				regex matchSucceed("^.*(Complete\\s+verification|Volume\\s+unmounted\\s+successfully|Blocks\\s+verified).*$");
				cmatch match;
				if(regex_match(outPuts[i].c_str(), match, matchSucceed)){
					bAuditRet = true;
					break;
				}
			}
			SocketInfo("Audit Tape: aduit result. Tape: " << m_barcodes[0] << ", cmd = " << cmd << ", bAuditRet = " << bAuditRet << ", ret = " << ret);
			for(unsigned int i = 0; i < outPuts.size(); i++){
				SocketDebug("AUDIT TOOL OUTPUT: " << outPuts[i]);
			}
			if(ret != 0 && bAuditRet == false){
				errMsg = "Failed to audit the tape.";
				bRet = false;
				goto ERR_RETURN;
			}
			try{
				ifstream inFile;
				if(fs::exists(fileListPath)){
					inFile.open(fileListPath.c_str());
					while(!inFile.eof()){
						string line = "";
						getline(inFile, line);
						SocketDebug("FILE CONTENT:    " << line);
						if(!HandleFile(line)){
							SocketError("Failed to handle corrupted file " << line);
						}
					}
					inFile.close();
				}
			}catch(...){
				SocketError("Failed to handle corrupted file list.");
				errMsg = "Failed to handle corrupted file list.";
				bRet = false;
				goto ERR_RETURN;
			}
		}


ERR_RETURN:
		pTapeMgr = TapeLibraryMgr::Instance();
		if(bNeedReleaseTape){
			if(pTapeMgr == NULL || false == pTapeMgr->ReleaseTape(m_barcodes[0])){
				SocketError("Check Tape: Failed to release tape " << m_barcodes[0] << " after check it.");
			}
		}

    	if(false == TapeLibraryMgr::Instance()->SetTapeState(m_barcodes[0], TAPE_STATE_IDLE)){
    		SocketWarn("Audit tape: Failed to set tape state to idle.");
    	}

		status_ = Status_Finish;
		if(false == bRet){
			SocketError("Audit tape '" << m_barcodes[0] << "' failed: " << errMsg);
			SocketEvent(EVENT_LEVEL_ERR, "Tape_Audit_Failed", "Failed to audit tape " <<  m_barcodes[0] << "."); // Event/Notification
			status_ = Status_Failed;
		}else{
			SocketEvent(EVENT_LEVEL_INFO, "Tape_Audit_Finished", "Audit tape " << m_barcodes[0] << " finished.");// Event/Notification
			TapeLibraryMgr::Instance()->SetLastAuditTime(m_barcodes[0], time(NULL));
		}

		MarkEnd();
		SaveStateToFile();
		return;
	}

} /* namespace ltfs_soapserver */
