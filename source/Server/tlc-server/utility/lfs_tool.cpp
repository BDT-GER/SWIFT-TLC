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
 * lfs_tool.cpp
 *
 *  Created on: Nov 30, 2012
 *      Author: Sam Chen
 */


#define MAKE_LFS_TOOL

#include "stdafx.h"
#include "../lib/common/Common.h"
#include "../lib/ltfs_library/LtfsLibraries.h"
using namespace bdt;
using namespace ltfs_management;

void
Factory::ReleaseCacheManager()
{
    return;
}

void PrintHelp()
{
	cout << "lfs_tool  Action [deviceName/barcode] [Value]" << endl;
	cout << "    deviceName:  sg device name of the drive or changer, eg: /dev/sg0" << endl;
	cout << "    barcode:  barcode of the tape." << endl;
	cout << "    Action:  " << endl;
	cout << "             GetTapeFaulty deviceName             ---- Get Tape Faulty flag of the tape loaded in the drive." << endl;
	cout << "             SetTapeFaulty deviceName Value       ---- Set Tape Faulty flag of the tape. 1 -- Faulty, 0 --- Not faulty" << endl;
	cout << "             GetTapeLtfsFormat deviceName         ---- Get ltfs format of the loaded tape." << endl;
	cout << "             SetTapeLtfsFormat deviceName Value   ---- Set ltfs format of the loaded tape. 1 -- Is LTFS format. 0 -- Not LTFS format" << endl;
	cout << "             GetTapeMediaType deviceName          ---- Get media type of the loaded tape." << endl;
	cout << "             GetTapeMediumType deviceName         ---- Get medium type of the loaded tape." << endl;
	cout << "             GetTapeStatus deviceName             ---- Get status of the loaded tape." << endl;
	cout << "             SetTapeStatus deviceName Value       ---- Set ltfs status of the loaded tape. " << endl;
	cout << "                                           		" << TAPE_OPEN         << " -- Open. " << TAPE_CLOSED   << " -- Closed.  " << endl;
	cout << "                                           		" << TAPE_ACTIVE        << " -- Active. " << TAPE_EXPORTED << " -- Exported." << endl;
	cout << "                                           		" << TAPE_UNKNOWN        << " -- Unknown. " << TAPE_CONFLICT << " -- Conflict." << endl;
	cout << "             GetTapeGroup deviceName              ---- Get tape group uuid of the loaded tape." << endl;
	cout << "             SetTapeGroup deviceName Value        ---- Set tape group uuid of the loaded tape. 'Value' is the tape group uuid to be set." << endl;
	cout << "             GetTapeDualCopy deviceName              ---- Get tape dual copy of the loaded tape." << endl;
	cout << "             SetTapeDualCopy deviceName Value        ---- Set tape dual copy of the loaded tape. 'Value' is the tape dual copy to be set." << endl;
	cout << "             GetTapeBarcode deviceName              ---- Get tape barcode of the loaded tape." << endl;
	cout << "             SetTapeBarcode deviceName Value        ---- Set tape barcode of the loaded tape. 'Value' is the tape barcode to be set." << endl;
	cout << "             GetTapeLoadCounter deviceName        ---- Get load counter of the loaded tape." << endl;
	cout << "             GetTapeGenerationIndex deviceName    ---- Get generation index of the loaded tape." << endl;
	cout << "             ChangerPreventMediaRemoval deviceName ---- Set prevent media removal for changer." << endl;
	cout << "             ChangerAllowMediaRemoval deviceName   ---- Set allow media removal for changer." << endl;
	cout << "             GetDriveInterfaceType	deviceName 	   ---- Get interface type(FC/SAS) for the drive." << endl;
	cout << "             GetDriveStatus	deviceName 	   	   ---- Get status for the drive." << endl;
	cout << "             UnFormatTape barcode    	   		  ---- Unformat a tape." << endl;
	cout << "             UnFormatAllTapes	 		 ---- Unformat all tapes in the system." << endl;
}

bool UnForamtTape(const string& barcode)
{
	LtfsError ltfsErr;
	vector<LtfsChangerInfo> changers;
	if(false == LtfsLibraries::Instance()->GetChangerList(changers, ltfsErr)){
		return false;
	}
	for(unsigned int i = 0; i < changers.size(); i++){
		string changerSerial = changers[i].mSerial;
		// check if the tape is in this changer
		vector<LtfsTapeInfo> tapes;
		if(false == LtfsLibraries::Instance()->GetTapeList(changerSerial, tapes, ltfsErr)){
			continue;
		}
		int tapeSlotId = -1;
		for(unsigned int j = 0; j < tapes.size(); j++){
			if(tapes[j].mBarcode == barcode){
				tapeSlotId = tapes[j].mSlotID;
				break;
			}
		}
		if(tapeSlotId == -1){
			continue;
		}
		string emptyDrive = "";
		int emptySlotId = -1;
		vector<LtfsDriveInfo> drives;
		// find the drive with the tape or find an empty drive
		if(true == LtfsLibraries::Instance()->GetDriveList(changerSerial, drives, ltfsErr)){
			for(unsigned int j = 0; j < drives.size(); j++){
				if(drives[j].mBarcode == barcode){
					return LtfsLibraries::Instance()->UnFormat(changerSerial, drives[j].mSerial, ltfsErr);
				}
				if(drives[j].mIsEmpty){
					emptyDrive = drives[j].mSerial;
					emptySlotId = drives[j].mSlotID;
				}
			}
		}
		if(drives.size() <= 0){
			continue;
		}
		if(emptyDrive == ""){
			// find an empty slot, and move the tape in drive to slot
			vector<LtfsSlotInfo> slots;
			if(true == LtfsLibraries::Instance()->GetSlotList(changerSerial, slots, ltfsErr)){
				for(unsigned int j = 0; j < slots.size(); j++){
					if(slots[j].mIsEmpty){
						if(true == LtfsLibraries::Instance()->MoveCartridge(changerSerial, drives[0].mSlotID, slots[j].mSlotID, ltfsErr)){
							emptyDrive = drives[0].mSerial;
							emptySlotId = drives[0].mSlotID;
							break;
						}
					}
				}
			}
		}
		if(emptyDrive != ""){
			// move the tape to this drive
			if(true == LtfsLibraries::Instance()->MoveCartridge(changerSerial, tapeSlotId, emptySlotId, ltfsErr)){
				// unformat the tape
				return LtfsLibraries::Instance()->UnFormat(changerSerial, emptyDrive, ltfsErr);
			}
		}
		return false;
	}
	return false;
}

bool UnForamtAllTapes()
{
	LtfsError ltfsErr;
	vector<LtfsChangerInfo> changers;
	if(true == LtfsLibraries::Instance()->GetChangerList(changers, ltfsErr)){
		for(unsigned int i = 0; i < changers.size(); i++){
			string changerSerial = changers[i].mSerial;
			vector<LtfsTapeInfo> tapes;
			if(false == LtfsLibraries::Instance()->GetTapeList(changerSerial, tapes, ltfsErr)){
				cerr << "Failed to get tape list to unforamt tape for changer " << changerSerial << "." << endl;
				return false;
			}
			for(unsigned int j = 0; j < tapes.size(); j++){
				if(tapes[j].mMediumType != MEDIUM_DATA){
					cerr << "Tape " << tapes[j].mBarcode << " is not a data tape. Ignore unformat on it." << endl;
					continue;
				}
				if(false == UnForamtTape(tapes[j].mBarcode)){
					cerr << "Failed to unformat tape " << tapes[j].mBarcode << "." << endl;
				}else{
					cout << "Succeeded to unformat tape " << tapes[j].mBarcode << "." << endl;
				}
			}
		}
	}else{
		cerr << "Failed to get changer list to unformat all tapes." << endl;
		return false;
	}
	return true;
}

int main(int argc, char *argv[])
{
    if ( argc < 2 ) {
        PrintHelp();
        return 1;
    }

    string devBarcode = "";
    string action(argv[1]);
    if(action != "UnFormatAllTapes"){
    	if(argc < 3){
			PrintHelp();
			return 1;
    	}
    	devBarcode = argv[2];
    }

    LtfsError error;
    string strMsg;
    if(action == "GetTapeFaulty"){
    	bool bFaulty = false;
    	if(false == GetLoadedTapeFaulty(devBarcode, "", bFaulty, error)){
    		cerr << "Failed to get faulty flag for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}
    	strMsg = string("The loaded tape in drive ") + devBarcode + string(" is ");
    	if(bFaulty){
    		strMsg += string("faulty");
    	}else{
    		strMsg += string("not faulty");
    	}
    	cout << strMsg << "." << endl;
    }else if (action == "SetTapeFaulty"){
    	bool bFaulty = boost::lexical_cast<bool>((argv[3]));
    	if(false == SetLoadedTapeFaulty(devBarcode, "", bFaulty, error)){
    		cerr << "Failed to set faulty flag for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}
    	cout << "Succeed to set faulty flag for tape in drive " << devBarcode << "." << endl;
    }else if (action == "GetTapeLtfsFormat"){
    	LTFS_FORMAT ltfsFormat;
    	if(false == GetLoadedTapeLtfsFormat(devBarcode, "", ltfsFormat, error)){
    		cerr << "Failed to get LTFS format for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}else{
        	strMsg = string("The loaded tape in drive ") + devBarcode + string(" is ");
        	if(ltfsFormat != LTFS_VALID){
        		strMsg += string("not ");
        	}
    		strMsg += string("a LTFS format tape");
        	cout << strMsg << "." << endl;
    	}
    }else if (action == "GetTapeMediaType"){
    	TapeMediaType mediaType;
    	if(false == GetLoadedTapeMediaType(devBarcode, "", mediaType, error)){
    		cerr << "Failed to get media type for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}else{
    		enum TapeMediaType
    		{
    			MEDIA_UNKNOWN = 0,			// media type not detected yet
    			MEDIA_LTO6,
    			MEDIA_LTO5,				// LTO5
    			MEDIA_LTO4,
    			MEDIA_LTO3,
    			MEDIA_LTO2,
    			MEDIA_LTO1,
    			MEDIA_OTHERS			// other types
    		};
        	strMsg = string("The media type of loaded tape in drive ") + devBarcode + string(" is ");
        	switch(mediaType){
        	case MEDIA_UNKNOWN:
        		strMsg += string("MEDIA_UNKNOWN");
        		break;
        	case MEDIA_LTO6:
        		strMsg += string("MEDIA_LTO6");
        		break;
        	case MEDIA_LTO5:
        		strMsg += string("MEDIA_LTO5");
        		break;
        	case MEDIA_LTO4:
        		strMsg += string("MEDIA_LTO4");
        		break;
        	case MEDIA_LTO3:
        		strMsg += string("MEDIA_LTO3");
        		break;
        	case MEDIA_LTO2:
        		strMsg += string("MEDIA_LTO2");
        		break;
        	case MEDIA_LTO1:
        		strMsg += string("MEDIA_LTO1");
        		break;
        	case MEDIA_OTHERS:
        		strMsg += string("MEDIA_OTHERS");
        		break;
        	}
        	cout << strMsg << "." << endl;
    	}
    }else if (action == "GetTapeMediumType"){
    	TapeMediumType mediumType;
		if(false == GetLoadedTapeMediumType(devBarcode, "", mediumType, error)){
			cerr << "Failed to get medium type for tape in drive " << devBarcode << "." << endl;
			return 1;
		}else{
			strMsg = string("The medium type of loaded tape in drive ") + devBarcode + string(" is ");
			switch(mediumType){
			case MEDIUM_UNKNOWN:
				strMsg += string("MEDIUM_UNKNOWN");
				break;
			case MEDIUM_DATA:
				strMsg += string("MEDIUM_DATA");
				break;
			case MEDIUM_CLEANING:
				strMsg += string("MEDIUM_CLEANING");
				break;
			case MEDIUM_DIAGNOSTICS:
				strMsg += string("MEDIUM_DIAGNOSTICS");
				break;
			case MEDIUM_WORM:
				strMsg += string("MEDIUM_WORM");
				break;
			}
			cout << strMsg << "." << endl;
		}
    }else if (action == "SetTapeLtfsFormat"){
    	bool bLtfs = boost::lexical_cast<bool>((argv[3]));
    	if(false == SetLoadedTapeLtfsFormat(devBarcode, "", bLtfs, error)){
    		cerr << "Failed to set LTFS format for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}
    	cout << "Succeed to set LTFS format for tape in drive " << devBarcode << "." << endl;
    }else if (action == "GetTapeStatus"){
    	TapeStatus tapeStatus = TAPE_UNKNOWN;
    	if(false == GetLoadedTapeStatus(devBarcode, "", tapeStatus, error)){
    		cerr << "Failed to get status for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}
    	strMsg = string("The status of loaded tape in drive ") + devBarcode + string(" is ");
    	if(TAPE_UNKNOWN == tapeStatus){
    		strMsg += string("Unknown");
    	}else if(TAPE_OPEN == tapeStatus){
    		strMsg += string("Open");
    	}else if(TAPE_ACTIVE == tapeStatus){
    		strMsg += string("Active");
    	}else if(TAPE_CLOSED == tapeStatus){
    		strMsg += string("Closed");
    	}else if(TAPE_EXPORTED == tapeStatus){
    		strMsg += string("Exported");
    	}else if(TAPE_CLEAN_EXPIRED == tapeStatus){
    		strMsg += string("CleanTapeExpired");
		}else if(TAPE_CONFLICT == tapeStatus){
			strMsg += string("Conflict");
		}
    	cout << strMsg << "." << endl;
    }else if (action == "SetTapeStatus"){
    	TapeStatus tapeStatus = (TapeStatus)boost::lexical_cast<int>((argv[3]));
    	if(false == SetLoadedTapeStatus(devBarcode, "", tapeStatus, error)){
    		cerr << "Failed to set status for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}
    	cout << "Succeed to set status for tape in drive " << devBarcode << "." << endl;
    }else if (action == "GetTapeGroup"){
    	string uuid = "";
    	if(false == GetLoadedTapeGroup(devBarcode, "", uuid, error)){
    		cerr << "Failed to get tape group uuid for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}else{
    		cout << "The tape group uuid for loaded tape in drive " << devBarcode << " is \"" << uuid << "\"." << endl;
    	}
    }else if (action == "GetTapeDualCopy"){
    	string dualCopy = "";
    	if(false == GetLoadedTapeDualCopy(devBarcode, "", dualCopy, error)){
    		cerr << "Failed to get tape dual copy for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}else{
    		cout << "The tape dual copy for loaded tape in drive " << devBarcode << " is \"" << dualCopy << "\"." << endl;
    	}
    }else if (action == "GetTapeBarcode"){
    	string barcode = "";
    	if(false == GetLoadedTapeBarcode(devBarcode, "", barcode, error)){
    		cerr << "Failed to get tape barcode for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}else{
    		cout << "The tape barcode for loaded tape in drive " << devBarcode << " is \"" << barcode << "\"." << endl;
    	}
    }else if (action == "GetDriveStatus"){
    	DriveStatus driveStatus;
    	DriveCleaningStatus cleaningStatus;
    	if(false == GetDriveStatus(devBarcode, driveStatus, cleaningStatus)){
    		cerr << "Failed to get status for drive " << devBarcode << "." << endl;
    		return 1;
    	}else{
    		cout << "The status for drive " << devBarcode << " is " << driveStatus << ". cleaning status is " << cleaningStatus << endl;
    	}
    }else if (action == "SetTapeGroup"){
    	string uuid = "";
    	if(argc > 3){
    		uuid = (argv[3]);
    	}
    	if(false == SetLoadedTapeGroup(devBarcode, "", uuid, error)){
    		cerr << "Failed to tape group uuid for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}
    	cout << "Succeed to set tape group uuid for tape in drive " << devBarcode << "." << endl;
	}else if (action == "SetTapeDualCopy"){
    	string dualCopy = "";
    	if(argc > 3){
    		dualCopy = (argv[3]);
    	}
    	if(false == SetLoadedTapeDualCopy(devBarcode, "", dualCopy, error)){
    		cerr << "Failed to tape dual copy for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}
    	cout << "Succeed to set tape dual copy for tape in drive " << devBarcode << " to " << dualCopy << "." << endl;
	}else if (action == "SetTapeBarcode"){
    	string barcode = "";
    	if(argc > 3){
    		barcode = (argv[3]);
    	}
    	if(false == SetLoadedTapeBarcode(devBarcode, "", barcode, error)){
    		cerr << "Failed to tape barcode for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}
    	cout << "Succeed to set tape barcode for tape in drive " << devBarcode << " to " << barcode << "." << endl;
	}else if (action == "GetTapeLoadCounter"){
		UInt16_t loadCounter = 0;
    	if(false == GetLoadedTapeLoadCounter(devBarcode, "", loadCounter, error)){
    		cerr << "Failed to get load counter for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}else{
    		cout << "The load counter for loaded tape in drive " << devBarcode << " is " << dec << loadCounter << "." << endl;
    	}
    }else if (action == "GetTapeGenerationIndex"){
    	UInt64_t genIndex = 0;
    	if(false == GetLoadedTapeGenerationIndex(devBarcode, "", genIndex, error)){
    		cerr << "Failed to get generation index for tape in drive " << devBarcode << "." << endl;
    		return 1;
    	}else{
    		cout << "The generation index for loaded tape in drive " << devBarcode << " is " << genIndex << "." << endl;
    	}
    }else if (action == "ChangerPreventMediaRemoval" || action == "ChangerAllowMediaRemoval"){
    	bool bPrevent = (action == "ChangerPreventMediaRemoval")? true : false;
    	string actStr = "set prevent media removal";
    	if(!bPrevent){
    		actStr = "set allow media removal";
    	}
    	if(false == ChangerPreventMediaRemoval(devBarcode, bPrevent)){
    		cerr << "Failed to " << actStr << " for changer " << devBarcode << "." << endl;
    		return 1;
    	}else{
    		cerr << "Succeed to " << actStr << " for changer " << devBarcode << "." << endl;
    	}
    }else if (action == "GetDriveInterfaceType"){
    	DriveInterfaceType interfaceType;
    	if(false == GetDriveInterfaceType(devBarcode, interfaceType)){
    		cerr << "Failed to get interface type for drive " << devBarcode << "." << endl;
    		return 1;
    	}
    	string typeStr = "SAS";
    	if(interfaceType == INTERFACE_FC){
    		typeStr = "FC";
    	}
    	cout << "Interface type for drive " << devBarcode << " is " << typeStr << "." << endl;
	}else if (action == "ReInitTapeInDrive"){
    	if(false == SetLoadedTapeLtfsFormat(devBarcode, "", false, error)){
    		cerr << "Failed to set tape in drive " << devBarcode << " to not ltfs format." << endl;
    	}else{
    		cout << "Succeed to set tape in drive " << devBarcode << " to not ltfs format." << endl;
    	}
    	if(false == SetLoadedTapeStatus(devBarcode, "", TAPE_UNKNOWN, error)){
    		cerr << "Failed to set tape in drive " << devBarcode << " to unknow status." << endl;
    	}else{
    		cout << "Succeed to set tape in drive " << devBarcode << " to unknow status." << endl;
    	}
    	if(false == SetLoadedTapeFaulty(devBarcode, "", false, error)){
    		cerr << "Failed to set tape in drive " << devBarcode << " to not faulty." << endl;
    	}else{
    		cout << "Succeed to set tape in drive " << devBarcode << " to not faulty." << endl;
    	}
    	if(false == SetLoadedTapeGroup(devBarcode, "", "", error)){
    		cerr << "Failed to clear tape group uuid for tape in drive " << devBarcode << "." << endl;
    	}else{
    		cout << "Succeed to clear tape group uuid for tape in drive " << devBarcode << "." << endl;
    	}
    }
	else if(action == "UnFormatTape"){
		if(false == UnForamtTape(devBarcode)){
			cerr << " Failed to unformat tape " << devBarcode << "." << endl;
		}else{
			cout << " Succeed to unformat tape " << devBarcode << "." << endl;
		}
	}
	else if(action == "UnFormatAllTapes"){
		UnForamtAllTapes();
	}

    return 0;
}
