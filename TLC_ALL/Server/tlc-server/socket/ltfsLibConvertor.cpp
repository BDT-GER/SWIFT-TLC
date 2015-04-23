/*
 * ltfsSCSI.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "../lib/config/CfgManager.h"
#include "../ltfs_management/TapeLibraryMgr.h"
#include "ltfsLibConvertor.h"
#include "ltfsTaskManagement.h"

#define SOAP_OK				0

namespace ltfs_soapserver
{
	bool CompareTapeGeneration(const TapeInfo& tape1, const TapeInfo& tape2)
	{
		return tape1.mMediaType > tape2.mMediaType;
	}

	bool
	CoupleTapes(vector<string> &barcodeList, vector<TapeInfo> &tapeList, struct ErrorReturn &errorReturn,  bool bNeedCouple)
	{
		bool bRet = true;
		int g5TapeCount = 0, g6TapeCount = 0;

		tapeList.clear();

		if (barcodeList.size() == 0)
			return bRet;

		//get tape info
		for(vector<string>::iterator iter=barcodeList.begin(); iter!=barcodeList.end(); ++iter)
		{
			TapeInfo tape;
			if (!TapeLibraryMgr::Instance()->GetTape(*iter, tape))
			{
				SocketError("Failed to get tape information.");
				bRet = false;
				return bRet;
			}
			if (tape.mMediaType == MEDIA_LTO6)
				g6TapeCount ++;
			else if (tape.mMediaType == MEDIA_LTO5)
				g5TapeCount ++;
			tape.mDualCopy = ""; //clear it before couple tape.
			tapeList.push_back(tape);
		}

		if (!bNeedCouple)  //not dual copy
		{
			return bRet;
		}

		//get driver list
		LtfsError error;
		vector<LtfsChangerInfo> changers;
		if (!TapeLibraryMgr::Instance()->GetChangerList(changers, error))
		{
			SocketError("Failed to get changer information.");
			bRet = false;
			return bRet;
		}

		vector<LtfsDriveInfo> driveList;
		vector<LtfsDriveInfo> drives;
		for (vector<LtfsChangerInfo>::iterator iter = changers.begin(); iter != changers.end(); iter++)
		{//get drives
			drives.clear();
			if (!TapeLibraryMgr::Instance()->GetDriveListForChanger(iter->mSerial, drives, error))
			{
				SocketError("Failed to get drive information.");
				bRet = false;
				return bRet;
			}
			for(vector<LtfsDriveInfo>::iterator it = drives.begin(); it != drives.end(); it++){
				if (it->mStatus != DRIVE_STATUS_DISCONNECTED){
					driveList.push_back(*it);
				}
			}
		}

		if (driveList.size() < 2)
		{
			SocketError("Dual copy must have two drivers or above.");
			bRet = false;
			return bRet;
		}

		int g6driveCount = 0, g5driveCount = 0;
		for (vector<LtfsDriveInfo>::iterator iter = driveList.begin(); iter != driveList.end(); iter++)
		{
			if (iter->mGeneration == 5)
			{
				g5driveCount++;
			}
			else if(iter->mGeneration == 6)
			{
				g6driveCount ++;
			}
		}
		SocketDebug("g5drive=" << g5driveCount << " g6drive=" << g6driveCount);
		if ((g6driveCount == 0 && g6TapeCount > 0)
				|| ((g6driveCount == 1 ) && !(g5TapeCount - g6TapeCount >= 0 && (g5TapeCount - g6TapeCount)%2 == 0))
				)
		{
			bRet = false;
			return bRet;
		}

		//sort
		sort(tapeList.begin(), tapeList.end(), CompareTapeGeneration);

		for(vector<TapeInfo>::iterator iter=tapeList.begin();iter!=tapeList.end(); ++iter)
		{
			SocketDebug("couple barcode:" << iter->mBarcode << " dualcopy:" << iter->mDualCopy);
			if (iter->mDualCopy != "")
				continue;
			if (g6driveCount == 1)
			{
				bool bFind = false;
				for(vector<TapeInfo>::iterator diter=tapeList.begin();diter!=tapeList.end(); ++diter)
				{
					if (diter->mDualCopy != "" || iter->mBarcode == diter->mBarcode)
						continue;
					if (iter->mMediaType != diter->mMediaType)
					{
						iter->mDualCopy = diter->mBarcode;
						diter->mDualCopy = iter->mBarcode;
						bFind = true;
						break;
					}
				}
				if (!bFind)
				{
					for(vector<TapeInfo>::iterator diter=tapeList.begin();diter!=tapeList.end(); ++diter)
					{
						if (diter->mDualCopy != "" || iter->mBarcode == diter->mBarcode)
							continue;
						if (iter->mMediaType == diter->mMediaType)
						{
							iter->mDualCopy = diter->mBarcode;
							diter->mDualCopy = iter->mBarcode;
							break;
						}
					}
				}
			}
			else
			{
				SocketDebug("barcode:" << iter->mBarcode << " type:" << iter->mMediaType);
				iter->mDualCopy = (iter+1)->mBarcode;
				(iter+1)->mDualCopy = iter->mBarcode;
			}
		}

		for(vector<TapeInfo>::iterator iter=tapeList.begin(); iter!=tapeList.end(); ++iter)
		{
			SocketDebug("sort barcode:" << iter->mBarcode << " dualcopy:" << iter->mDualCopy);
		}

		return bRet;
	}

	int
	LibConvertor::AddTape(std::string uid, struct BarcodesList barcodeList, bool import, struct ErrorReturn &result)
	{
		bool errOcurred = false;

		if(uid.empty())
		{
			result.errorCode = ERR_UUID_EMPTY;
			result.errorMsg	= "Share UUID is empty.";
			return SOAP_OK;
		}

		string shareName = "";//
		TapeLibraryMgr::Instance()->GetGroupName(uid, shareName);

		vector<string> lockedTapes;
		do
		{
			bool bDualCopy = TapeLibraryMgr::Instance()->GetTapeGroupDualCopy(uid);
			vector<TapeInfo> tapeList;
			if (!CoupleTapes(barcodeList.barcodes, tapeList, result, bDualCopy))
			{
				result.errorCode = ERR_ASSIGN_COUPLE_TAPES;
				result.errorMsg	= "Failed to assign tapes to share. Selected tapes don't match the requirement of dual copy share.";//TODO:review message
				SocketDebug("errorMsg:" << result.errorMsg);
				return SOAP_OK;
			}
			for(vector<string>::iterator iter=barcodeList.barcodes.begin();
					iter!=barcodeList.barcodes.end(); ++iter)
			{
				SocketEvent(EVENT_LEVEL_INFO, "Tape_Assign_Start", "Start to assign tape " << *iter << " to share " << shareName << ".");  //Event/Notification

				// get logic lock of the tape first
				if(false == TapeLibraryMgr::Instance()->StmRequestTape(*iter))
				{
					SocketError("Failed to get lock for tape " << *iter << ", will not format this tape for share " << uid);
					result.errorCode = ERR_ASSIGN_TAPE_LOCKED;
					result.errorMsg = string("Failed to assign the tape. Tape ") + *iter + string(" is busy.");
					SocketEvent(EVENT_LEVEL_ERR, "Tape_Assign_Failed", "Failed to assign tape " << *iter << " to share " << shareName << ".");  //Event/Notification
					errOcurred = true;
					break;
				}
				lockedTapes.push_back(*iter);

				// check if we can change the tape to target status
				if(false == TapeLibraryMgr::Instance()->
						StmTapeCanBeChangedTo(*iter, STM_ST_ASSIGNED_OPEN, STM_OP_ASSIGN))
				{
					SocketError("The tape " << *iter << " is not allowed to be changed to " << TapeLibraryMgr::Instance()->GetStmStateStr(STM_ST_ASSIGNED_OPEN)
							<< " state for assign share " << uid << " for the moment.");
					result.errorCode = ERR_ASSIGN_TAPE_USED;
					result.errorMsg = string("Failed to assign the tape. Tape ") + *iter + string(" cannot be used to assign to a share.");
					SocketEvent(EVENT_LEVEL_ERR, "Tape_Assign_Failed", "Failed to assign tape " << *iter << " to share " << shareName << ".");  //Event/Notification
					errOcurred = true;
					break;
				}
			}

			if(errOcurred)
				break;

			for(vector<TapeInfo>::iterator iter=tapeList.begin();
					iter!=tapeList.end(); ++iter)
			{
				TapeMamInfo mamInfo;
				mamInfo.mFaulty = false;
				mamInfo.mStatus = TAPE_OPEN;
				mamInfo.mTapeGroup = uid;
				mamInfo.mDualCopy = iter->mDualCopy;
				mamInfo.mPriority = bdt::ScheduleInterface::PRIORITY_ASSIGN_TAPES;
				mamInfo.mMask = MAM_INFO_MASK_UUID|MAM_INFO_MASK_STATUS|MAM_INFO_MASK_FAULTY|MAM_INFO_MASK_FORMAT|MAM_INFO_MASK_DUAL_COPY;

				if(!TapeLibraryMgr::Instance()->SetTapeMamInfo(iter->mBarcode, mamInfo))
				{
					SocketError("Failed to write tape " << iter->mBarcode << " mam information.");
				}
				SocketEvent(EVENT_LEVEL_INFO, "Tape_Assign_Finished", "Finished to assign tape " << iter->mBarcode << " to share " << shareName << ".");  //Event/Notification
			}

			result.errorCode = ERR_SUCCESS;
			result.errorMsg = "Success";
			SocketDebug("AddTape success");
		}while(false);

		if (!import || result.errorCode != ERR_SUCCESS) {
			SocketDebug("add tape release tape");
			for(vector<string>::iterator iter = lockedTapes.begin(); iter != lockedTapes.end(); ++iter)
			{
				TapeLibraryMgr::Instance()->StmReleaseTape(*iter);
			}
		}

		if(result.errorCode == ERR_SUCCESS)
		{
			SocketDebug("AddTape success");
		}
		else
		{
			SocketDebug("AddTape failed: " << result.errorMsg);
		}

		return SOAP_OK;
	}


	int LibConvertor::CreateShare(struct CreateShareRequest req, struct ErrorReturn &result)
	{
		result.errorCode = ERR_SUCCESS;
		result.errorMsg	= "Success";

		SocketDebug("Enter create share");

		//generate uuid for tape group
		string uuid = TapeLibraryMgr::Instance()->GenerUUIDByName(req.shareToAdd.name.c_str());

		vector<TapeInfo> tapeList;
		if (!CoupleTapes(req.barcodeList, tapeList, result, false))
		{
			result.errorCode = ERR_CREATE_SHARE_COUPLE_TAPES;
			result.errorMsg	= "Failed to create share. Selected tapes don't match the requirement of dual copy share."; //TODO:review message
			return SOAP_OK;
		}

		SocketDebug("barcode list: count = " << req.barcodeList.size());

		for (vector<string>::iterator biter = req.barcodeList.begin(); biter != req.barcodeList.end(); biter++)
		{
			SocketDebug(*biter);
		}

		if (TapeLibraryMgr::Instance()->CheckTapeGroupExistByName(req.shareToAdd.name))
		{
			result.errorCode = ERR_SHARE_EXISTS;
			result.errorMsg	= string("Share ") + req.shareToAdd.name + string(" exists.");
			SocketDebug("share name exist:" << req.shareToAdd.name);
			return SOAP_OK;
		}

		//get lock for tape

		vector<string> lockedTapes;

		SocketEvent(EVENT_LEVEL_INFO, "Share_Add_Start", "Start adding share " << req.shareToAdd.name << "." ); // Event/Notification

		int ret = -1;

		for(vector<string>::iterator iter= req.barcodeList.begin();
				iter!=req.barcodeList.end(); ++iter)
		{

			// get logic lock of the tape first
			if(false == TapeLibraryMgr::Instance()->StmRequestTape(*iter)){
				SocketError("Failed to get lock for tape " << *iter << ", will not format this tape for share " << uuid);
				result.errorCode = ERR_CREATE_SHARE_TAPE_LOCKED;
				result.errorMsg = string("Failed to create share. Tape ") + *iter + string(" is busy.");
				goto ADD_SHARE_RETURN;
			}
			lockedTapes.push_back(*iter);

			// check if we can change the tape to target status
			if(false == TapeLibraryMgr::Instance()->StmTapeCanBeChangedTo(*iter, STM_ST_ASSIGNED_OPEN, STM_OP_ASSIGN)){
				SocketError("The tape " << *iter << " is not allowed to be changed to " << TapeLibraryMgr::Instance()->GetStmStateStr(STM_ST_ASSIGNED_OPEN)
						<< " state for creating share " << uuid << " for the moment.");
				result.errorCode = ERR_CREATE_SHARE_TAPE_USED;
				result.errorMsg = string("Failed to create share. Tape ") + *iter + string(" cannot be used to create share.");
				goto ADD_SHARE_RETURN;
			}
		}


		do
		{
			//write share to database
			if(!TapeLibraryMgr::Instance()->AddTapeGroup(uuid, req.shareToAdd.name))
			{
				result.errorCode = ERR_SAVE_SHARE_INFO;
				result.errorMsg = string("Failed to add share ") + req.shareToAdd.name + string(" information into database.");
				break;
			}

			ret = TapeLibraryMgr::Instance()->MountShare(uuid, req.shareToAdd.name);
			if (ret != 0)
			{
				SocketError("Mount Share failed:" << req.shareToAdd.name << ret);
				result.errorCode = ERR_MOUNT_SHARE;
				result.errorMsg	= string("Failed to mount the share ") + req.shareToAdd.name + string(".");
				break;
			}

		}while(false);

		if (result.errorCode == ERR_SUCCESS)
		{
			if (req.barcodeList.size() > 0) {
				for(vector<TapeInfo>::iterator iter=tapeList.begin();
						iter!=tapeList.end(); ++iter)
				{
					TapeMamInfo mamInfo;
					mamInfo.mFaulty = false;
					mamInfo.mStatus = TAPE_OPEN;
					mamInfo.mTapeGroup = uuid;
					mamInfo.mDualCopy = iter->mDualCopy;
					mamInfo.mPriority = bdt::ScheduleInterface::PRIORITY_ASSIGN_TAPES;
					mamInfo.mMask = MAM_INFO_MASK_UUID|MAM_INFO_MASK_STATUS|MAM_INFO_MASK_FAULTY|MAM_INFO_MASK_FORMAT|MAM_INFO_MASK_DUAL_COPY;

					if(!TapeLibraryMgr::Instance()->SetTapeMamInfo(iter->mBarcode, mamInfo))
					{
						SocketError("Failed to update tape group and status for tape " << iter->mBarcode << " for share " << uuid << ". Will not format this tape for the share.");
					}
				}
			}
		}else
		{//restore database and samba configuration?
			TapeLibraryMgr::Instance()->DeleteTapeGroup(uuid);
		}
	ADD_SHARE_RETURN:
		// release the tape we locked before
		for(vector<string>::iterator iter = lockedTapes.begin(); iter != lockedTapes.end(); ++iter)
		{
			TapeLibraryMgr::Instance()->StmReleaseTape(*iter);
		}
		if(result.errorCode == ERR_SUCCESS){
			SocketDebug("AddShare success");
			SocketEvent(EVENT_LEVEL_INFO, "Share_Add_Finished", "Share " << req.shareToAdd.name << " created successfully." );   /// Event/Notification
		}else{
			SocketDebug("AddShare failed: " << result.errorMsg);
			SocketEvent(EVENT_LEVEL_ERR, "Share_Add_Failed", "Failed to add share " << req.shareToAdd.name << "." );   /// Event/Notification
		}

		return SOAP_OK;
	}
}



