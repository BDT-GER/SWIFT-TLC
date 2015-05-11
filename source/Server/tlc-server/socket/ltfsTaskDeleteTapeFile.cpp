/*
 * ltfsTaskDeleteTapeFile.cpp
 *
 *  Created on: Nov 20, 2014
 *      Author: zhaona
 */

#include <fstream>
#include "ltfsTaskDeleteTapeFile.h"
#include "../ltfs_management/CatalogDbManager.h"
#include "ltfsTaskManagement.h"

using namespace ltfs_management;
using namespace ltfs_soapserver;

#define CHECK_TAPE_REQUEST_WAIT	1800  // time out for requesting resource to check tape,  30 minutes

namespace ltfs_soapserver
{
	TaskDeleteTapeFile::TaskDeleteTapeFile(const vector<string> &barcodes):
			Task(Type_AuditTape, "", barcodes)
	{
		groupID_ = "";
		m_barcodes = barcodes;
	}

	TaskDeleteTapeFile::~TaskDeleteTapeFile()
	{
	}

	void TaskDeleteTapeFile::Execute()
	{
		bool bRet = true;
		bool bNeedReleaseTape = false;
		string errMsg = "";
		LtfsError lfsErr;

		if(m_barcodes.size() <= 0 || m_barcodes[0] == ""){
			SocketError("Delete tape file for tape '" << m_barcodes[0] << "' failed: no tape with barcode found.");
			status_ = Status_Failed;
			bRet = false;
			MarkEnd();
			SaveStateToFile();
			return;
		}

		TapeLibraryMgr *pTapeMgr = TapeLibraryMgr::Instance();
		string barcode = m_barcodes[0];
		SocketInfo("Delete file on tape tape '" << m_barcodes[0] << "' started.");
		status_ = Status_Running;
		((TaskManagement*)manager_)->SetTapeDeleteFileStatus(barcode, DF_RUNNING);

		while(pTapeMgr != NULL){
			SocketInfo("Delete file on tape: requesting resource for the tape " << barcode);
			// request tape
			if(pTapeMgr->IsTapeInDrive(barcode)){
				if(true == pTapeMgr->RequestTape(m_barcodes[0], true, \
						bdt::ScheduleInterface::PRIORITY_CARTRIDGE_DELETE_FILE, CHECK_TAPE_REQUEST_WAIT)){
					bNeedReleaseTape = true;
					SocketInfo("Delete file on tape: requested resource for the tape " << barcode << " done.");
					break;
				}
			}
			sleep(10);
		}

		if(!bNeedReleaseTape){
			SocketError("Failed to request tape " << barcode << " to delete files on it.");
			errMsg = "Failed to request tape to delete files on it.";
			bRet = false;
			goto ERR_RETURN;
		}

		if(false == TapeLibraryMgr::Instance()->DeleteFilesOnTape(barcode)){
			SocketError("Failed to delete files in database for tape " << barcode);
			errMsg = "Failed to delete files in database for tape " + barcode + ".";
			bRet = false;
		}

ERR_RETURN:
		pTapeMgr = TapeLibraryMgr::Instance();
		if(bNeedReleaseTape){
			if(pTapeMgr == NULL || false == pTapeMgr->ReleaseTape(m_barcodes[0])){
				SocketError("Check Tape: Failed to release tape " << m_barcodes[0] << " after check it.");
			}
		}

		status_ = Status_Finish;
		if(false == bRet){
			((TaskManagement*)manager_)->SetTapeDeleteFileStatus(barcode, DF_FAILED);
			SocketError("Delete files on tape '" << m_barcodes[0] << "' failed: " << errMsg);
			SocketEvent(EVENT_LEVEL_ERR, "Tape_FileDelete_Failed", "Failed to delete files on tape " <<  m_barcodes[0] << "."); // Event/Notification
			status_ = Status_Failed;
		}else{
			((TaskManagement*)manager_)->SetTapeDeleteFileStatus(barcode, DF_FINISHED);
			SocketEvent(EVENT_LEVEL_INFO, "Tape_FileDelete_Finished", "Deleting files on tape " << m_barcodes[0] << " finished.");// Event/Notification
		}

		MarkEnd();
		SaveStateToFile();
		return;
	}

} /* namespace ltfs_soapserver */
