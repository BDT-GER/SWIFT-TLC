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
 * TapeDbManager.h
 *
 *  Created on: Nov 8, 2012
 *      Author: chento
 */


#pragma once


#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/metadata.h>
#include <cppconn/exception.h>
#include <mysql_connection.h>
#include <mysql_driver.h>

using namespace sql;

#include "TapeLibraryMgr.h"
#include "../ltfs_format/ltfsFormatThread.h"
#include "../lib/database/ConnectionPool.h"

namespace ltfs_management
{

	class TapeDbManager
	{
	public:
		static TapeDbManager*
		Instance()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if(NULL == instance_)
			{
				instance_ = new TapeDbManager();
			}
			return instance_;
		}

		static void
		Destroy()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if( NULL != instance_)
			{
				delete instance_;
				instance_ = NULL;
			}
		}

		virtual
		~TapeDbManager();

	public:
		bool
		GetCartridge(const string& barcode, CartridgeDetail& detail);

		bool
		DeleteCartridge(const string& barcode);

		bool
		ModifyCartridge(const string& barcode, CartridgeDetail& detail);

		bool
		GetTapeGroupList(vector<string>& list);

		bool GetTapeGroupDualCopy(const string& groupUUID);

		bool GetDriveList(vector<DriveInfo>& driveList);
		bool GetDriveInfo(const string& driveSerial, DriveInfo& driveInfo);
		bool AddDrive(const DriveInfo& driveInfo);

		bool
		AddTapeGroup(const string& group, const string& name);

		bool
		DeleteTapeGroup(const string& group);

		bool
		GetTapeGroupCartridgeList(const string& group, vector<string>& list);

		bool
		GetCartridgeList(vector<CartridgeDetail>& cartridgeList, const string& changerSerial = "");

		bool
		SetGroupUUIDForCartridge(const string& barcode, const string& group);

		bool
		SetTapeUUIDForCartridge(const string& barcode, const string& uuid);

		bool
		SetDualCopyForCartridge(const string& barcode, const string& dualCopy);

		bool
		ChangeTapeBarcode(const string& oldBarcode, const string& newBarcode);

		bool
		SetStatusForCartridge(const string& barcode, int status);

		bool
		SetLoadCountForCartridge(const string& barcode, long long loadCount);

		bool
		SetWriteProtectForCartridge(const string& barcode, bool bWriteProtected);

		bool
		SetOfflineForCartridge(const string& barcode, bool bOffline);

		bool
		SetGenerationNumberForCartridge(const string& barcode, int generation);

		bool
		SetUsedCapacityForCartridge(const string& barcode, long long usedCapacity);

		bool
		SetFreeCapacityForCartridge(const string& barcode, long long freeCapacity);

		bool
		SetFileNumberForCartridge(const string& barcode, long fileNumber);

		bool
		SetFileCapacityForCartridge(const string& barcode, long long fileCapacity);

		bool
		SetLastMountTimeForCartridge(const string& barcode, time_t lastMountTime);

		bool SetLastAuditTimeForCartridge(const string& barcode, time_t lastAuditTime);

		bool
		SetFormatForCartridge(const string& barcode, int format);

		bool
		SetFaultyForCartridge(const string& barcode, bool faulty);

		bool
		SetStateForCartridge(const string& barcode, TapeState state);

		bool
		SetActivityForCartridge(const string& barcode, int activity);

		bool
		GetActivityForCartridge(const string& barcode, int& activity);

		bool
		SetGroupName(const string& group, const string& name);

		bool
		GetGroupName(const string& group, string& name);

		bool
		GetShareUUID(const string& name, string& uuid);

		bool
		CheckTapeGroupExistByName(const string name);

		bool GetShareAvailableTapes(const string& uuid, vector<map<string, off_t> >& tapesList);

	private:
		TapeDbManager();

		void
		InitDB();

		void
		InitNewDB();

		bool
		CreateDatabase();

		Connection *CreateConnection();

		bool
		TableExists(const string & tableName);
		void ReleaseConnection(Connection * conn);

	private:
		boost::shared_mutex 			rwMutex_;

		Driver*       					driver_;
		boost::shared_ptr<Connection>   connection_;

		static boost::mutex 			instanceMutex_;
		static TapeDbManager*			instance_;
		map<string, int>				tapeActivityMap_;
		map<string, TapeState>			tapeStateMap_;
		boost::scoped_ptr<ConnectionPool> cPool_;

		friend class FormatThread;
		friend TapeLibraryMgr* TapeLibraryMgr::Instance();
		friend void TapeLibraryMgr::Destroy();
	};

} /* namespace ltfs_management */

