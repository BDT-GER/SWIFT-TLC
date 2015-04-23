/*
 * ltfsTaskCheckTape.cpp
 *
 *  Created on: Sep 6, 2012
 *      Author: chento
 */

#include "ltfsTaskCheckTape.h"
#include "../ltfs_format/ltfsFormatDetails.h"
///#include "../ltfs_management/SysConfMgr.h"
///#include "ltfsUtility.h"

using namespace ltfs_management;
using namespace ltfs_soapserver;

#define CHECK_TAPE_REQUEST_WAIT	1800  // time out for requesting resource to check tape,  30 minutes

namespace ltfs_soapserver
{
	TaskCheckTape::TaskCheckTape(const vector<string> &barcodes, bool bNeedLockTape):
			Task(Type_CheckTape, "", barcodes)
	{
		groupID_ = "";
		m_barcodes = barcodes;
		bNeedLockTape_ = bNeedLockTape;
	}

	TaskCheckTape::~TaskCheckTape()
	{
	}

	bool TaskCheckTape::CheckTape(CHECK_TAPE_FLAG flag)
	{
		LtfsError lfsErr;
		TapeLibraryMgr *pTapeMgr = TapeLibraryMgr::Instance();
		// Check the tape
		if(pTapeMgr == NULL || false == pTapeMgr->CheckTape(m_barcodes[0], flag, lfsErr)){
			SocketError("failed to check tape. Error: " << lfsErr.GetErrMsg());
			return false;
		}
		return true;
	}

	void TaskCheckTape::Execute()
	{
		bool bRet = true;
		bool bNeedReleaseTape = false;
		string errMsg = "";
		LtfsError lfsErr;

		if(m_barcodes.size() <= 0 || m_barcodes[0] == ""){
			SocketError("Check tape '" << m_barcodes[0] << "' failed: no tape with barcode found.");
			status_ = Status_Failed;
			bRet = false;
			MarkEnd();
			SaveStateToFile();
			return;
		}

		TapeLibraryMgr *pTapeMgr = TapeLibraryMgr::Instance();

		string barcode = m_barcodes[0];
		string dualCopy = "";
		string groupUUID = "";
		string shareName = "";
		vector<fs::path> failedFileList;
		TapeInfo tapeInfo;
		bool bNeedStartShare = false;

		if(TapeLibraryMgr::Instance()->GetTape(barcode, tapeInfo)){
			if(tapeInfo.mGroupID != "" && TapeLibraryMgr::Instance()->GetGroupName(tapeInfo.mGroupID, shareName) && shareName != ""){
				dualCopy = tapeInfo.mDualCopy;
				groupUUID = tapeInfo.mGroupID;
			}
		}else{
			SocketError("Failed to get tape info for " << barcode);
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

		pTapeMgr = TapeLibraryMgr::Instance();
    	if(false == TapeLibraryMgr::Instance()->SetTapeState(m_barcodes[0], TAPE_STATE_DIAGNOSING)){
			errMsg = "failed to set tape state to diagnosing.";
			bRet = false;
			goto ERR_RETURN;
    	}

		SocketInfo("Check tape '" << m_barcodes[0] << "' started.");
		status_ = Status_Running;

		SocketInfo("Check tape: requesting resource for the tape.");
		while(pTapeMgr != NULL){
			CartridgeDetail detail;
			if(pTapeMgr->GetCartridge(m_barcodes[0], detail) && !detail.mFaulty){
				SocketInfo("Tape " << m_barcodes[0]  << " become not faulty, do not need to check it.");
				bRet = true;
				goto ERR_RETURN;
			}
			// request tape
			if(true == pTapeMgr->RequestTape(m_barcodes[0], false, \
					bdt::ScheduleInterface::PRIORITY_DIAGNOSE_CARTRIDGE, CHECK_TAPE_REQUEST_WAIT)){
				break;
			}
			SocketWarn("Failed to load tape check, will retry.");
			sleep(5);
		}
		bNeedReleaseTape = true;

		SocketDebug("Check tape: unmounting the tape.");
		// should unmount the tape first?
		if(false == pTapeMgr->UnmountTape(m_barcodes[0])){
			SocketError("Check Tape: Failed to unmount tape " << m_barcodes[0] << " to diagnose it.");
			errMsg = "failed to unmount tape to check.";
			bRet = false;
			goto ERR_RETURN;
		}

		SocketInfo("Check tape: checking the tape.");
		// Check the tape
		if(false == CheckTape(FLAG_FULL_RECOVERY) && false == CheckTape(FLAG_DEEP_RECOVERY)){
			errMsg = "failed to check tape.";
			bRet = false;
			goto ERR_RETURN;
		}

		SocketInfo("Check tape: mounting the tape.");
		// succeed to diagnose the tape, try to mount it on
		if(true == pTapeMgr->MountTape(m_barcodes[0])){
			//the tape belongs to a share, check file between meta and tape
			if(groupUUID != ""){
				// if the tape share is dual copy, also need to check between two tapes
				if(dualCopy != ""){
					SocketInfo("Check tape: requesting resource for dual copy tape " << dualCopy << " for tape " << barcode << ".");
					if(TapeLibraryMgr::Instance()->RequestTape(dualCopy, true,
							bdt::ScheduleInterface::PRIORITY_DIAGNOSE_CARTRIDGE, CHECK_TAPE_REQUEST_WAIT)){
						SocketDebug("Check tape: checking tape consistency between " << barcode << " and " << dualCopy);
						if(!TapeLibraryMgr::Instance()->DiagnoseCheckDualCopyTape(barcode, dualCopy, groupUUID)){
							SocketError("Failed to check consistency between tape " << barcode << " and its dual copy " << dualCopy << ".");
						}else{
							//check file between meta and tape
							if(!TapeLibraryMgr::Instance()->ServiceDiagnoseTape(false, dualCopy, groupUUID, failedFileList)){
								SocketError("Failed to check consistency between meta and tape for " << dualCopy);
							}
						}
						TapeLibraryMgr::Instance()->ReleaseTape(dualCopy);
					}else{
						SocketError("Failed to request dual copy tape " << dualCopy << " to check consitency between its copy " << barcode);
					}
				}
				if(!TapeLibraryMgr::Instance()->ServiceDiagnoseTape(false, barcode, groupUUID, failedFileList)){
					SocketError("Failed to check consistency between meta and tape for " << barcode);
				}
			}

			// unmount the tape
			if(false == pTapeMgr->UnmountTape(m_barcodes[0])){
				SocketError("Check Tape: Failed to unmount tape " << m_barcodes[0] << " after mount it.");
			}else{
				// set tape to not faulty
				if(false == pTapeMgr->SetLoadedTapeFaulty(m_barcodes[0], false, lfsErr, true)){
					SocketError("Check Tape: Failed to set tape " << m_barcodes[0] << " to not faulty after successfully diagnose it.");
				}else{
					SocketInfo("Check Tape: Succeed to set tape " << m_barcodes[0] << " to not faulty after successfully diagnose it.");
				}
			}
		}

ERR_RETURN:
		pTapeMgr = TapeLibraryMgr::Instance();

		if(bNeedReleaseTape){
			if(pTapeMgr == NULL || false == pTapeMgr->ReleaseTape(m_barcodes[0])){
				SocketError("Check Tape: Failed to release tape " << m_barcodes[0] << " after check it.");
			}
		}

		// enable the share
		if(groupUUID != "" && bNeedStartShare){
			SocketInfo("Check tape: starting share " << shareName << ":" << groupUUID << " after diagnosing the tape " << m_barcodes[0] << ".");
		}

    	if(false == TapeLibraryMgr::Instance()->SetTapeState(m_barcodes[0], TAPE_STATE_IDLE)){
    		SocketWarn("Check tape: Failed to set tape state to idle.");
    	}

		for(vector<TapeStatusPair>::iterator iterBarcode=barcodeList_.begin(); iterBarcode!=barcodeList_.end(); ++iterBarcode){
				TapeLibraryMgr::Instance()->StmReleaseTape(iterBarcode->mBarcode);
		}

		status_ = Status_Finish;
		if(false == bRet){
			SocketError("Check tape '" << m_barcodes[0] << "' failed: " << errMsg);
			SocketEvent(EVENT_LEVEL_ERR, "Tape_Diagnose_Failed", "Failed to diagnose tape " <<  m_barcodes[0] << "."); // Event/Notification
			status_ = Status_Failed;
		}else{
			SocketEvent(EVENT_LEVEL_INFO, "Tape_Diagnose_Finished", "Diagnose tape " << m_barcodes[0] << " finished.");// Event/Notification
		}

		MarkEnd();
		SaveStateToFile();
		return;
	}

} /* namespace ltfs_soapserver */
