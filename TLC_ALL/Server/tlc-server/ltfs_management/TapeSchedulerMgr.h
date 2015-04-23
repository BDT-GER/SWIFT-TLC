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
 * File:         TapeSchedulerMgr.h
 * Description:
 * Author:       Sam Chen
 * Created:      Mar 21, 2013
 *
 */


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
#include "TapeLibraryMgr.h"

#include "stdafx.h"

using namespace ltfs_management;

namespace ltfs_management
{
	struct SchDriveInfo
	{
		LtfsDriveInfo info;
		bool bIgnored;
	public:
		SchDriveInfo()
		{
			bIgnored = false;
		}
	};

	struct SchTapeInfo
	{
		string	barcode;
		time_t  reqTime;
	};

    class TapeSchedulerMgr
    {
    private:
        static TapeSchedulerMgr * Instance();
        static void Destroy();
    public:
    	SchResult SchRequestTape(const string& barcode, vector<string>& tapesInUse, int priority);
    	SchResult SchRequestTapes(const vector<string>& barcodes, map<string, vector<string> >& tapesInUse, int priority);
		bool SchReleaseTape(const string & barcode);
		bool SchReleaseTapes(const vector<string>& barcodes);
        bool BindTape(const string& barcode, const string& changerSerial, const string& driveSerial);
        bool UnbindTape(const string& barcode);

    private:
        TapeSchedulerMgr();
        ~TapeSchedulerMgr();

    	string TapeRequestedNoLock(const string& barcode);
    	bool GetDeviceInfoByTape(const string& barcode, LtfsChangerInfo& changerInfo, vector<SchDriveInfo>& driveList,
    			vector<TapeInfo>& tapeList, TapeInfo& requestTapeInfo);
    	bool GetChangerDriveList(const string& changerSerial, vector<SchDriveInfo>& driveList);
    	bool LoadTapeToDrive(const string& changerSerial, const TapeInfo& tapeInfo, const SchDriveInfo& driveInfo);
    	UInt64_t CountDriveScore(const TapeInfo& tapeInfo, const SchDriveInfo& driveInfo, int priority);
    	void SchRequestTapeThread(const string& changerSerial);
    	bool GetBusyTapesNolock(const string& barcode, vector<string>& busyTapes);
    	string GetTapeChangerSerial(const string& barcode);
    	bool SchReleaseTapeNoLock(const string & barcode);
    	void SetTapeSchStatus(const string& barcode, SchResult status, bool bCheck);
    	void SetTapeSchStatusNoLock(const string& barcode, SchResult status, bool bCheck);
    	void ReleaseFailTapesNoLock(const vector<string>& barcodes, map<string, vector<string> >& tapesInUse);
    	time_t GetCurMicroSecondTime();

    private:
        static TapeSchedulerMgr *         	instance_;
        static boost::mutex   				mutexInstance_;

        static boost::mutex   				mutexChangerBusyMap_;
		map<string, string>					schChangerBusyMap_;
		boost::mutex 						driveTapeMutex_;
		map<string, SchResult>				schTapeStatusMap_;

		map<string, string>					bindTapeMap_;   // barcode->drive serial
		map<string, SchTapeInfo>			driveTapeMap_;   // drive serial-> tape info
		map<string, UInt64_t>				lastReleaseMap_;

		friend TapeLibraryMgr* TapeLibraryMgr::Instance();
		friend void TapeLibraryMgr::Destroy();
    };
}

