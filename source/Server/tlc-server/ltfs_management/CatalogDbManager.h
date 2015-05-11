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
 * CatalogDbManager.h
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
#include "../lib/database/ConnectionPool.h"

using namespace sql;

namespace ltfs_management
{

	struct MetaFileInfo
	{
		string	mUuid;						// uuid of the file
		string	mFileName;
		string	mMetaFolder;				// folder of the file on meta
		bool	mCorrupted;					// the file is corrupted or not
	};

	struct TapeFileInfo
	{
		string	mUuid;
		string	mMetaFilePath;				// full path name on meta (including file name)
		unsigned long long mOffset;
		unsigned long long mSize;
	};

	struct BackupInfo
	{
		string 	mUuid;
		string 	mTapeFilePath;
		off_t 	mSize;
	};

	struct MetaFolderInfo
	{
		unsigned long long 	mId;
		string				mPath;			// meta folder path
	};

	class CatalogDbManager
	{
	public:
		static CatalogDbManager* Instance()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if(NULL == instance_)
			{
				instance_ = new CatalogDbManager();
			}
			return instance_;
		}
		static void Destroy()
		{
			boost::unique_lock<boost::mutex> lock(instanceMutex_);
			if( NULL != instance_)
			{
				delete instance_;
				instance_ = NULL;
			}
		}
		virtual ~CatalogDbManager();

	public:
		bool AddTapeFiles(const string& shareUuid, map<string, vector<TapeFileInfo> >& fileInfoMap);
		bool RenameMetaFile(const string& shareUuid, const string& uuid, const string& newPathName);
		bool RenameMetaFolder(const string& shareUuid, const string& oldFolder, const string& newFolder);
		bool DeleteUuid(const string& shareUuid, const string& uuid);//
		bool DeleteMetaFolder(const string& shareUuid, const string& folderPath);

		// Get path to be backup on tape
		bool GetPathForBackup(const string& uuid, string& backupPath);
		bool GetBackupInfo(const string& shareUuid, const string& uuid, map<string, BackupInfo>& backupInfo);
		bool SetFileCorrupted(const string& shareUuid, const string& shareName, const string& uuid, bool bCorrupted);

		bool GetMetaFilePath(const string& shareUuid, const string& uuid, string& metaFilePath);
		bool GetNextTapeFile(const string& shareUuid, const string& barcode, const string& curUuid, off_t& size, string& nextUuid);

		bool DeleteShare(const string& shareUuid);
		bool GetTotalSize(const string& shareUuid, off_t& size);

		bool DeleteTapeFiles(const string& shareUuid, const string& barcode, vector<string>& uuids);
		bool NeedDeleteFileOnTape(const string& shareUuid, const string& barcode);
		bool GetFilesToDelete(const string& shareUuid, const string& barcode, vector<string>& uuids, int limit = 0);

		bool DiagnoseCheckTape(const string& shareUuid, const string& barcode);
		bool TapeHasFile(const string& shareUuid, const string& barcode);

	private:
		CatalogDbManager();
		bool CreateTables(const string& shareUuid, const string& barcode);
		off_t GetTotalSize(const string& shareUuid, const string& barcode = "");
		void InitShareSizeMap(const string& shareUuid);

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
    	void DbThread();

	private:
		boost::shared_mutex 			rwMutex_;

		Driver*       					driver_;
		boost::shared_ptr<Connection>   connection_;

		static boost::mutex 			instanceMutex_;
		static CatalogDbManager*			instance_;
		boost::scoped_ptr<ConnectionPool> cPool_;
		map<string, bool> 				tableMap_;
		auto_ptr<boost::thread>			thread_;
		map<string, vector<string> >	deleteFiles_;
		map<string, vector<string> >	deleteFolders_;
		boost::mutex 					deleteMutex_;
		map<string, UInt64_t>			shareSizeMap_;
		boost::mutex 					shareSizeMapMutex_;
	};

} /* namespace ltfs_management */

