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
 * CatalogDbManager.cpp
 *
 *  Created on: Nov 8, 2012
 *      Author: chento
 */


#include "stdafx.h"
#include "CatalogDbManager.h"
#include "TapeDbManager.h"
#include "../bdt/stdafx.h"
#include "../bdt/ExtendedAttribute.h"
#include "../bdt/Inode.h"
#include "../lib/database/DbLock.h"

using namespace bdt;
namespace ltfs_management
{
	#define CATALOG_DB_MAJOR_VERSION	1
	#define LIBRARY_DB_MINOR_VERSION	0
	static const string CATALOG_DB_VERSION_COMMENT	= "Original";

    CatalogDbManager * CatalogDbManager::instance_ = NULL;
    boost::mutex CatalogDbManager::instanceMutex_;

#define PREPARE_SQL(__str_sql__)\
	LtfsLogInfo("SQL = " << __str_sql__);\
	preStmt.reset(connection->prepareStatement(__str_sql__))

#define GET_CONNECTION_VOID(_TAG) \
	boost::shared_ptr< Connection > _TAG(\
		CreateConnection(),\
		boost::bind(&CatalogDbManager::ReleaseConnection, this, _1) );\
    	if(_TAG.get() == NULL){\
    		LtfsLogError("Failed to connect to database.");\
    		return;\
    	}
#define GET_CONNECTION_COTINUE(_TAG) \
	boost::shared_ptr< Connection > _TAG(\
		CreateConnection(),\
		boost::bind(&CatalogDbManager::ReleaseConnection, this, _1) );\
    	if(_TAG.get() == NULL){\
    		LtfsLogError("Failed to connect to database.");\
    		continue;\
    	}
#define GET_CONNECTION(_TAG, _rt) \
	boost::shared_ptr< Connection > _TAG(\
		CreateConnection(),\
		boost::bind(&CatalogDbManager::ReleaseConnection, this, _1) );\
    	if(_TAG.get() == NULL){\
    		LtfsLogError("Failed to connect to database.");\
    		return _rt;\
    	}

    CatalogDbManager::CatalogDbManager()
    {
    	tableMap_.clear();
    	cPool_.reset(new ConnectionPool());
    	CreateDatabase();
        InitDB();
        thread_.reset(new boost::thread(&CatalogDbManager::DbThread, this));
    }

    CatalogDbManager::~CatalogDbManager()
    {
    }

    string UUID2SQL(const string& uuid)
    {
    	string strRet = "";
    	for(unsigned int i = 0; i < uuid.length(); i++){
    		if(uuid[i] == '-' || uuid[i] == '/') {
    			strRet += string("_");
    		}else{
    			strRet += uuid[i];
    		}
    	}

    	return strRet;
    }

    void
    CatalogDbManager::InitDB()
    {
		try
		{
			if( !TableExists("Version") )
			{
				InitNewDB();
			}

		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("InitDB SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e)
		{
			LtfsLogError("InitDB : "<<e.what());
		}
    }

    bool
    CatalogDbManager::CreateDatabase()
    {
    	string checkSQL = "CREATE DATABASE IF NOT EXISTS CatalogDb";
    	boost::scoped_ptr< Statement > stmt;

    	try
    	{
        	boost::shared_ptr< Connection > connection(
        		cPool_->GetConnection(),
        		boost::bind(&CatalogDbManager::ReleaseConnection, this, _1));
        	if(connection.get() == NULL){
        		LtfsLogError("Failed to connect to database.");
        		return false;
        	}
    		stmt.reset( connection->createStatement() );
    		stmt->execute(checkSQL);

    		return true;
    	}
    	catch (const sql::SQLException & e)
    	{
    		LtfsLogError( "CreateDatabase SQLState:"<< e.getSQLState()<< "  ErrorCode:"<< e.getErrorCode() );
    	}
    	catch(const std::exception & e)
    	{
    		LtfsLogError("CreateDatabase : " << e.what());
    	}

    	return false;
    }

    void
    CatalogDbManager::ReleaseConnection(Connection * conn)
    {
    	cPool_->ReleaseConnection(conn);
    }

    Connection *
    CatalogDbManager::CreateConnection()
    {
    	Connection * connection = NULL;
    	try
    	{
    		for(int i = 10; i > 0 && !connection; --i)
    		{
    			connection = cPool_->GetConnection();
    			connection->setSchema("CatalogDb");
    		}
    	}
    	catch(const std::exception & e)
    	{
    		LtfsLogError("GetConnection failed: " << e.what());
    	}

    	return connection;
    }

	bool
	CatalogDbManager::TableExists(const string & tableName)
	{
		bool exist = false;
		boost::scoped_ptr<ResultSet> rs;
		boost::scoped_ptr<PreparedStatement> preStmt;
		string checkSQL = "SELECT table_name FROM information_schema.TABLES WHERE table_name =?;";
    	GET_CONNECTION(connection, false);

		try
		{
			LtfsLogDebug("TableExists " << checkSQL << ": " << tableName);
			preStmt.reset(connection->prepareStatement(checkSQL));
			preStmt->setString(1, tableName);
			rs.reset(preStmt->executeQuery());
			while (rs->next())
			{
				exist = true;
				LtfsLogDebug("TableExists, yes exists: " << ":" << rs->getString("table_name"));
				break;
			}
		}
		catch (SQLException& e)
		{
			exist = false;
			LtfsLogError("TableExist SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e)
		{
			exist = false;
			LtfsLogError("TableExist : "<<e.what());
		}

		return exist;
	}

	bool CatalogDbManager::CreateTables(const string& shareUuid, const string& barcode)
	{
		string sUuid = UUID2SQL(shareUuid);
        string strSQL;
        boost::scoped_ptr<Statement> stmt;
        GET_CONNECTION(connection, false);
		stmt.reset( connection->createStatement() );

		try{
			if(sUuid == ""){
				LtfsLogError("Create tables failed, share uuid is empty.");
				return false;
			}

			//Meta_File_YYY
			string tableName = "Meta_File_" + sUuid;
			LtfsLogInfo("CreateTables: tableName = " << tableName);
			if(tableMap_.find(tableName) == tableMap_.end()){
				if(!TableExists(tableName)){
					strSQL = "CREATE TABLE IF NOT EXISTS " + tableName + " (uuid BIGINT PRIMARY KEY  NOT NULL, \
							filename TEXT NOT NULL, meta_folder integer NOT NULL, corrupted BOOL, \
							size BIGINT NOT NULL) \
							ENGINE=InnoDB DEFAULT CHARSET=utf8;";
					LtfsLogInfo("CreateTables: strSQL = " << strSQL);
					stmt->execute(strSQL);
					tableMap_[tableName] = true;

					strSQL = "ALTER TABLE " + tableName + " ADD INDEX iMetaFileIndex (uuid, corrupted, meta_folder);";
					LtfsLogDebug("Creating Index for " << tableName << ". sql = " << strSQL);
					stmt->execute(strSQL);
				}
			}

			tableName = "Meta_Folder_" + sUuid;
			if(tableMap_.find(tableName) == tableMap_.end()){
				if(!TableExists(tableName)){
					strSQL = "CREATE TABLE IF NOT EXISTS " + tableName + " (id INTEGER PRIMARY KEY NOT NULL auto_increment, \
							path VARCHAR(512) ) ENGINE=InnoDB DEFAULT CHARSET=utf8;";
					LtfsLogInfo("CreateTables: strSQL = " << strSQL);
					stmt->execute(strSQL);
					tableMap_[tableName] = true;
				}
			}

			tableName = "File_" + barcode + "_" + sUuid;
			if(barcode != "" && tableMap_.find(tableName) == tableMap_.end()){
				if(!TableExists(tableName)){
					strSQL = "CREATE TABLE IF NOT EXISTS " + tableName + " (uuid BIGINT PRIMARY KEY  NOT NULL, \
							offset BIGINT NOT NULL, \
							flag integer NOT NULL, \
							size BIGINT NOT NULL) \
							ENGINE=InnoDB DEFAULT CHARSET=utf8;";
					LtfsLogInfo("CreateTables: strSQL = " << strSQL);
					stmt->execute(strSQL);
					tableMap_[tableName] = true;
					strSQL = "ALTER TABLE " + tableName + " ADD INDEX iTapeFileIndex (uuid, offset, flag);";
					LtfsLogDebug("Creating Index for " << tableName << ". sql = " << strSQL);
					stmt->execute(strSQL);
				}
			}

			return true;
		}
		catch (sql::SQLException& e){
			LtfsLogError("CreateTables \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e){
			LtfsLogError("CreateTables exception " << e.what());
		}
		return false;
	}

    void
    CatalogDbManager::InitNewDB()
    {
        string strSQL;
        boost::scoped_ptr<Statement> stmt;
    	GET_CONNECTION_VOID(connection);
		stmt.reset( connection->createStatement() );

    	try{
			strSQL = "CREATE TABLE Version (MajorVersion INTEGER PRIMARY KEY, MinorVersion INTEGER DEFAULT 0, Comment VARCHAR(128) DEFAULT NULL)";
			stmt->execute(strSQL);
			strSQL = "INSERT INTO Version Values(";
			strSQL += "'" + boost::lexical_cast<string>(CATALOG_DB_MAJOR_VERSION) + "'";
			strSQL += ",'" + boost::lexical_cast<string>(LIBRARY_DB_MINOR_VERSION) + "'";
			strSQL += ",'" + CATALOG_DB_VERSION_COMMENT + "')";
			stmt->execute(strSQL);
    	}
		catch (sql::SQLException& e){
			LtfsLogError("InitNewDB \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e){
			LtfsLogError("InitNewDB exception " << e.what());
		}
    }

    void CatalogDbManager::InitShareSizeMap(const string& shareUuid)
    {
	    boost::unique_lock<boost::mutex> lock(shareSizeMapMutex_);
    	if(shareSizeMap_.find(shareUuid) == shareSizeMap_.end()){
    		shareSizeMap_[shareUuid] = GetTotalSize(shareUuid);
    	}
    }

    bool CatalogDbManager::AddTapeFiles(const string& shareUuid, map<string, vector<TapeFileInfo> >& fileInfoMap)
    {
    	InitShareSizeMap(shareUuid);
    	bool bRet = true;
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
		boost::scoped_ptr<ResultSet> rs;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";
    	vector<string> folderList;
    	vector<string> fileNameList;
    	map<string, UInt64_t> numberSizeMap;
    	for(map<string, vector<TapeFileInfo> >::iterator itTape = fileInfoMap.begin(); itTape != fileInfoMap.end(); itTape++){
    		string barcode = itTape->first;
    		if(!CreateTables(sUuid, barcode)){
    			return false;
    		}
    	}

		string dbLockStr = "lock table Meta_File_" + sUuid + " write,  Meta_Folder_" + sUuid + " write ";
        vector<string> tapes;
        if(!TapeDbManager::Instance()->GetTapeGroupCartridgeList(shareUuid, tapes)){
        	LtfsLogError("Failed to get tape list to delete uuid from tape file tables");
        }else{
        	for(unsigned int i = 0; i < tapes.size(); i++){
        		string tapeTable = "File_" + tapes[i] + "_" + sUuid;
        		if(TableExists(tapeTable)){
        			dbLockStr += ", " + tapeTable + " write";
        		}
        	}
        }
    	DbLock dbLock(connection.get(), dbLockStr);

		try{
			string uuids = "(";
			map<string, vector<TapeFileInfo> >::iterator itTape = fileInfoMap.begin();
			if(itTape != fileInfoMap.end()){
				for(unsigned int i = 0; i < itTape->second.size(); i++){
					if(i != 0){
						uuids += ",";
					}
					uuids += itTape->second[i].mUuid;
				}
			}
			uuids += ")";
			strSQL = "select uuid, size from Meta_File_" + sUuid + " where uuid in " + uuids;
			PREPARE_SQL(strSQL);
			rs.reset(preStmt->executeQuery());
			while(rs->next()){
				numberSizeMap[boost::lexical_cast<string>(rs->getInt64("uuid"))] = rs->getInt64("size");
			}
		}
		catch (sql::SQLException& e){
			LtfsLogError("AddTapeFile \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e){
			LtfsLogError("AddTapeFile exception " << e.what());
		}

    	for(map<string, vector<TapeFileInfo> >::iterator itTape = fileInfoMap.begin(); itTape != fileInfoMap.end(); itTape++){
    		string barcode = itTape->first;
    		// check create database
    		if(!CreateTables(sUuid, barcode)){
    			return false;
    		}

			map<string, off_t> folerIdMap;
			for(unsigned int i = 0; i < itTape->second.size(); i++){
				TapeFileInfo fileInfo = itTape->second[i];
				regex matchPath("^\\s*(.*\\/)(\\S+.*)$");
				cmatch match;
				if(!regex_match(fileInfo.mMetaFilePath.c_str(), match, matchPath)){
					LtfsLogError("File path not correct: " << fileInfo.mMetaFilePath);
					folderList.push_back("");
					fileNameList.push_back("");
					continue;
				}
				string fileName = match[2];
				string metaFolder = match[1];
				folerIdMap[metaFolder] = 0;
				folderList.push_back(metaFolder);
				fileNameList.push_back(fileName);
			}
			for(map<string, off_t>::iterator it = folerIdMap.begin(); it != folerIdMap.end(); it++){
				try{
					long long metaFolderId = 0;
					// check and insert if folder not in database
					strSQL = "select id from Meta_Folder_" + sUuid + " where path='" + QuotaStringForSQL(it->first) + "'";
					PREPARE_SQL(strSQL);
					rs.reset(preStmt->executeQuery());
					if(rs->next()){
						metaFolderId = rs->getInt64("id");
					}else{
						strSQL = "insert into Meta_Folder_" + sUuid + "(path) values('" + QuotaStringForSQL(it->first) + "')";
						PREPARE_SQL(strSQL);
						preStmt->executeUpdate();

						strSQL = "select id from Meta_Folder_" + sUuid + " where path='" + QuotaStringForSQL(it->first) + "'";
						PREPARE_SQL(strSQL);
						rs.reset(preStmt->executeQuery());
						if(rs->next()){
							metaFolderId = rs->getInt64("id");
						}
					}
					folerIdMap[it->first] = metaFolderId;
				}
				catch (sql::SQLException& e){
					LtfsLogError("AddTapeFile \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
				}
				catch(std::exception& e){
					LtfsLogError("AddTapeFile exception " << e.what());
				}
			}

			for(unsigned int i = 0; i < itTape->second.size(); i++){
				TapeFileInfo fileInfo = itTape->second[i];
				string metaFolder = folderList[i];
				string fileName = fileNameList[i];
				if(fileNameList[i] == "" || metaFolder == ""){
					LtfsLogError("File path not correct: " << fileInfo.mMetaFilePath);
					continue;
				}
				long long metaFolderId = 0;
				if(folerIdMap.find(metaFolder) == folerIdMap.end()){
					LtfsLogError("metaFolder not found: " << metaFolder);
					continue;
				}
				metaFolderId = folerIdMap[metaFolder];
				try{
					for(unsigned int i = 0; i < tapes.size(); i++){
						string tapeTable = "File_" + tapes[i] + "_" + sUuid;
						LtfsLogDebug("tapeTable = " << tapeTable << ", barcode = " << barcode << ", tapes[" << i << "] = " << tapes[i]);
						if(tapes[i] != barcode && TableExists(tapeTable)){
							strSQL = "update " + tapeTable + " set flag=1 where uuid='" + fileInfo.mUuid + "'";
							PREPARE_SQL(strSQL);
							preStmt->executeUpdate();
						}
					}
					// insert/update into Meta_File
					strSQL = "replace into Meta_File_" + sUuid + " values('" + fileInfo.mUuid + "'";
					strSQL += ",'" + QuotaStringForSQL(fileName) + "'";
					strSQL += "," + boost::lexical_cast<string>(metaFolderId);
					strSQL += ", 0, " + boost::lexical_cast<string>(fileInfo.mSize) + ")";
					PREPARE_SQL(strSQL);
					preStmt->executeUpdate();

					strSQL = "replace into File_" + barcode + "_" + sUuid + " values('" + fileInfo.mUuid + "'";
					strSQL += "," + boost::lexical_cast<string>(fileInfo.mOffset);
					strSQL += ",0, " + boost::lexical_cast<string>(fileInfo.mSize) + ")";
					PREPARE_SQL(strSQL);
					preStmt->executeUpdate();

					{
						boost::unique_lock<boost::mutex> lock(shareSizeMapMutex_);
						LtfsLogDebug("GetTotalSize: add uuid: " << fileInfo.mUuid << ", shareUuid = " << shareUuid << ", size: " << fileInfo.mSize << ". shareSizeMap_[shareUuid] = " << shareSizeMap_[shareUuid]);
						shareSizeMap_[shareUuid] += fileInfo.mSize;
						if(numberSizeMap.find(fileInfo.mUuid) != numberSizeMap.end()){
							shareSizeMap_[shareUuid] -= numberSizeMap[fileInfo.mUuid];
							LtfsLogDebug("GetTotalSize: add uuid: " << fileInfo.mUuid << ", numberSizeMap[fileInfo.mUuid] = " << numberSizeMap[fileInfo.mUuid] << ". shareSizeMap_[shareUuid] = " << shareSizeMap_[shareUuid]);
						}
					}
				}
				catch (sql::SQLException& e){
					LtfsLogError("AddTapeFiles \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
				}
				catch(std::exception& e){
					LtfsLogError("AddTapeFiles exception " << e.what());
				}
			}//for
    	}//for
    	return bRet;
    }

	bool CatalogDbManager::SetFileCorrupted(const string& shareUuid, const string& shareName, const string& uuid, bool bCorrupted)
	{
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";


    	try
		{
			string metaFilePath = "";
			if(!GetMetaFilePath(shareUuid, uuid, metaFilePath)){
				LtfsLogError("Failed to get meta file path for " << uuid << " in share " << shareUuid);
			}
			if(metaFilePath != ""){
				string fullStorageFilePath = COMM_STORAGE_VFS_PATH + "/" + shareName + metaFilePath;
				if(bCorrupted){
					if(0 != truncate(fullStorageFilePath.c_str(), 0)){
						LtfsLogError("Failed to truncate file " << fullStorageFilePath);
					}
				}
				string fullMetaFilePath = COMM_META_CACHE_PATH + "/" + shareUuid + metaFilePath;
				auto_ptr<ExtendedAttribute> ea(new ExtendedAttribute(fullMetaFilePath));
				if (!ea->SetValue(Inode::ATTRIBUTE_CORRUPTED, &bCorrupted, sizeof(bCorrupted))) {
					LtfsLogError("Failed to set corrupted status for file " << fullMetaFilePath << " to " << bCorrupted);
				}
			}

			string dbLockStr = "lock table Meta_File_" + sUuid + " write;";
	        LtfsLogInfo("DBG: File 2");
			DbLock dbLock(connection.get(), dbLockStr);
			string strCorrupted = "0";
			if(bCorrupted){
				strCorrupted = "1";
			}
			strSQL = "update Meta_File_" + sUuid + " set corrupted=" + strCorrupted + " where uuid='" + uuid + "'";
			PREPARE_SQL(strSQL);
			preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetFileCorrupted \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetFileCorrupted exception " << e.what());
		}
		return false;
	}

	bool CatalogDbManager::RenameMetaFile(const string& shareUuid, const string& uuid, const string& newPathName)
	{
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
		boost::scoped_ptr<ResultSet> rs;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";

    	try
		{
    		string folderPath = GetFolderPath(newPathName) + "/";
    		string fileName = GetFileName(newPathName);

    		if(!TableExists("Meta_File_" + sUuid) || !TableExists("Meta_Folder_" + sUuid)){
    			return true;
    		}
    		string dbLockStr = "lock table Meta_File_" + sUuid + " write, Meta_Folder_" + sUuid + " write";
        	DbLock dbLock(connection.get(), dbLockStr);

    		string tableName = "Meta_Folder_" + sUuid;
    		strSQL = "select id from " + tableName + " where path='" + QuotaStringForSQL(folderPath) + "'";
			PREPARE_SQL(strSQL);
			rs.reset(preStmt->executeQuery());
			long long folderId = -1;
			if(rs->next()){
				folderId = rs->getInt64("id");
			}
			if(folderId <= 0){
				strSQL = "insert into " + tableName + "(path) values('" + QuotaStringForSQL(folderPath) + "')";
				PREPARE_SQL(strSQL);
				preStmt->executeUpdate();
	    		strSQL = "select id from " + tableName + " where path='" + QuotaStringForSQL(folderPath) + "'";
	    		PREPARE_SQL(strSQL);
				rs.reset(preStmt->executeQuery());
				if(rs->next()){
					folderId = rs->getInt64("id");
				}
			}
    		tableName = "Meta_File_" + sUuid;
    		if(TableExists(tableName)){
				strSQL = "update " + tableName + " set meta_folder=" + boost::lexical_cast<string>(folderId) + ", filename='";
				strSQL += QuotaStringForSQL(fileName) + "' where uuid='" + uuid + "'";
				PREPARE_SQL(strSQL);
				preStmt->executeUpdate();
    		}
            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("RenameMetaFile \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("RenameMetaFile exception " << e.what());
		}
		return false;
	}

	bool CatalogDbManager::RenameMetaFolder(const string& shareUuid, const string& oldFolder, const string& newFolder)
	{
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";

    	try
		{
    		string tableName = "Meta_Folder_" + sUuid;
    		if(!TableExists(tableName)){
    			return true;
    		}
    		string dbLockStr = "lock table " + tableName + " write";
        	DbLock dbLock(connection.get(), dbLockStr);

    		string folderOld = oldFolder;
    		string folderNew = newFolder;
    		if(folderOld[folderOld.length() - 1] != '/'){
    			folderOld += "/";
    		}
    		if(folderNew[folderNew.length() - 1] != '/'){
    			folderNew += "/";
    		}
            strSQL = "update " + tableName + " set path=replace(path, '" + QuotaStringForSQL(folderOld) + "', '" + QuotaStringForSQL(folderNew) + "')";
			PREPARE_SQL(strSQL);
            preStmt->executeUpdate();
            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("RenameMetaFolder \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("RenameMetaFolder exception " << e.what());
		}
		return false;
	}

	void CatalogDbManager::DbThread()
	{
        boost::unique_lock<boost::mutex> lock(deleteMutex_,boost::defer_lock);
		while(true){
            if ( boost::this_thread::interruption_requested() ) {
                return;
            }
            lock.lock();
            map<string, vector<string> > deleteFiles = deleteFiles_;
            map<string, vector<string> > deleteFolders = deleteFolders_;
            deleteFiles_.clear();
            deleteFolders_.clear();
            lock.unlock();
            if(deleteFiles.size() <= 0 && deleteFolders.size() <=0){
                boost::this_thread::sleep(boost::posix_time::milliseconds(100));
            	continue;
            }

            boost::scoped_ptr<PreparedStatement> preStmt;
            GET_CONNECTION_COTINUE(connection);
        	string strSQL = "";

    		string dbLockStr = "";
    		if(deleteFiles.size() >0){
				for(map<string, vector<string> >::iterator it = deleteFiles.begin(); it != deleteFiles.end(); it++){
					string tableName = "Meta_File_" + it->first;
					if(TableExists(tableName) && it->second.size() > 0){
						if(dbLockStr.length() <= 0){
							dbLockStr = "lock table ";
						}else{
							dbLockStr += ",";
						}
						dbLockStr += tableName + " write";
					}
				}
    		}
			if(deleteFolders.size() > 0){
				for(map<string, vector<string> >::iterator it = deleteFolders.begin(); it != deleteFolders.end(); it++){
					string tableName = "Meta_Folder_" + it->first;
					if(TableExists(tableName) && it->second.size() > 0){
						if(dbLockStr.length() <= 0){
							dbLockStr = "lock table ";
						}else{
							dbLockStr += ",";
						}
						dbLockStr += tableName + " write";
					}
				}
			}
        	DbLock dbLock(connection.get(), dbLockStr);
        	vector<string>  refreshShares;

			if(deleteFiles.size() >0){
				for(map<string, vector<string> >::iterator it = deleteFiles.begin(); it != deleteFiles.end(); it++){
					string tableName = "Meta_File_" + it->first;
					if(TableExists(tableName) && it->second.size() > 0){
						strSQL = "delete from " + tableName + " where uuid in (";
						for(unsigned int i = 0; i < it->second.size(); i++){
							strSQL += "'" + it->second[i] + "',";
						}
						strSQL[strSQL.length() - 1] = ')';
						try{
							PREPARE_SQL(strSQL);
							preStmt->executeUpdate();
						}catch (sql::SQLException& e){
							LtfsLogError("Delete File \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
						}catch(std::exception& e){
							LtfsLogError("Delete File exception " << e.what());
						}
						refreshShares.push_back(it->first);
					}//if
				}//for
			}//if


			if(deleteFolders.size() > 0){
				for(map<string, vector<string> >::iterator it = deleteFolders.begin(); it != deleteFolders.end(); it++){
					string tableName = "Meta_Folder_" + it->first;
					if(TableExists(tableName) && it->second.size() > 0){
						for(unsigned int i = 0; i < it->second.size(); i++){
							strSQL = "delete from Meta_Folder_" + tableName + " where path like '%" + QuotaStringForSQL(it->second[i]) + "%'";
							try{
								PREPARE_SQL(strSQL);
								preStmt->executeUpdate();
							}catch (sql::SQLException& e){
								LtfsLogError("Delete folder \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
							}catch(std::exception& e){
								LtfsLogError("Delete folder exception " << e.what());
							}
						}//for
					}//if
				}//for
			}//if

			dbLock.UnLockTables();
			//updater total size of shares
			for(unsigned int i = 0; i < refreshShares.size(); i++){
				off_t totalSize = GetTotalSize(refreshShares[i]);
				boost::unique_lock<boost::mutex> lock(shareSizeMapMutex_);
				shareSizeMap_[refreshShares[i]] = totalSize;
			}
		}//while
	}

	bool CatalogDbManager::DeleteUuid(const string& shareUuid, const string& uuid)
	{
		string sUuid = UUID2SQL(shareUuid);
        boost::unique_lock<boost::mutex> lock(deleteMutex_);
        map<string, vector<string> >::iterator it = deleteFiles_.find(sUuid);
        if(it != deleteFiles_.end()){
        	it->second.push_back(uuid);
        }else{
        	vector<string> uuids;
        	uuids.push_back(uuid);
        	deleteFiles_[sUuid] = uuids;
        }
		return true;
	}



	bool CatalogDbManager::DeleteShare(const string& shareUuid)
	{
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";

    	try
		{
    		strSQL = "drop table if exists ";
            vector<string> tapes;
            if(!TapeDbManager::Instance()->GetTapeGroupCartridgeList(shareUuid, tapes)){
            	LtfsLogError("Failed to get tape list to delete uuid from tape file tables");
            	return false;
            }else{
				for(unsigned int i = 0; i < tapes.size(); i++){
					strSQL += " File_" + tapes[i] + "_" + sUuid + ", ";
				}
            }
            strSQL += " Meta_File_" + sUuid + ", ";
            strSQL += " Meta_Folder_" + sUuid;
			PREPARE_SQL(strSQL);
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("DeleteShare \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("DeleteShare exception " << e.what());
		}
		return false;
	}

	bool CatalogDbManager::DeleteMetaFolder(const string& shareUuid, const string& folderPath)
	{
		string sUuid = UUID2SQL(shareUuid);
        boost::unique_lock<boost::mutex> lock(deleteMutex_);
        map<string, vector<string> >::iterator it = deleteFolders_.find(sUuid);
        if(it != deleteFolders_.end()){
        	it->second.push_back(folderPath);
        }else{
        	vector<string> folders;
        	folders.push_back(folderPath);
        	deleteFolders_[sUuid] = folders;
        }
		return true;
	}

	bool CatalogDbManager::GetPathForBackup(const string& uuid, string& backupPath)
	{
		backupPath = GetPathFromUuid(uuid);
		return true;
	}


	bool CatalogDbManager::GetBackupInfo(const string& shareUuid, const string& uuid, map<string, BackupInfo>& backupInfo)
	{
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
		boost::scoped_ptr<ResultSet> rs;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";
    	string filePath = "";
    	GetPathForBackup(uuid, filePath);

    	try
		{
    		backupInfo.clear();
    		// delete files in File_XXX
            vector<string> tapes;
            if(!TapeDbManager::Instance()->GetTapeGroupCartridgeList(shareUuid, tapes)){
            	LtfsLogError("GetBackupPath: Failed to get tape list.");
            	return false;
            }
            string dbLockStr = "";
            for(unsigned int i = 0; i < tapes.size(); i++){
            	string tableName = "File_" + tapes[i] + "_" + sUuid;
            	if(TableExists(tableName)){
					if(dbLockStr.length() <= 0){
						dbLockStr = "lock table ";
					}else{
						dbLockStr += ", ";
					}
					dbLockStr += tableName + " read";
            	}
            }
        	DbLock dbLock(connection.get(), dbLockStr);

            for(unsigned int i = 0; i < tapes.size(); i++){
            	string tableName = "File_" + tapes[i] + "_" + sUuid;
            	if(TableExists(tableName)){
					strSQL = "select size from " + tableName + " where uuid='" + uuid + "'";
					PREPARE_SQL(strSQL);
					rs.reset(preStmt->executeQuery());
					if(rs->next()){
						string fullPath = COMM_MOUNT_PATH + "/" + tapes[i] + "/" + filePath;
						BackupInfo item;
						item.mUuid = uuid;
						item.mSize = rs->getInt64("size");
						item.mTapeFilePath = fullPath;
						backupInfo[tapes[i]] = item;
					}
            	}
            }
            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("GetBackupPath \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("GetBackupPath exception " << e.what());
		}
		return false;
	}

	bool CatalogDbManager::GetMetaFilePath(const string& shareUuid, const string& uuid, string& metaFilePath)
	{
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
		boost::scoped_ptr<ResultSet> rs;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";

		try{
			if(TableExists("Meta_File_" + sUuid) && TableExists("Meta_Folder_" + sUuid)){
	            string dbLockStr = "lock table Meta_File_" + sUuid + " read, Meta_Folder_" + sUuid + " read";
	        	DbLock dbLock(connection.get(), dbLockStr);
				strSQL = "select filename, path from Meta_File_" + sUuid + " left join Meta_Folder_" \
				+ sUuid + " on Meta_File_" + sUuid + ".meta_folder=Meta_Folder_" + sUuid \
				+ ".id where Meta_File_" + sUuid + ".uuid='" + uuid + "'";
				PREPARE_SQL(strSQL);
				rs.reset(preStmt->executeQuery());
				if(rs->next()){
					string fileName = rs->getString("filename");
					string folderName = rs->getString("path");
					metaFilePath = folderName + fileName;
					return true;
				}
			}
		}
		catch (sql::SQLException& e){
			LtfsLogError("GetMetaFilePath \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e){
			LtfsLogError("GetMetaFilePath exception " << e.what());
		}

		return false;
	}
    bool CatalogDbManager::GetTotalSize(const string& shareUuid, off_t& size)
    {
	    boost::unique_lock<boost::mutex> lock(shareSizeMapMutex_);
    	if(shareSizeMap_.find(shareUuid) == shareSizeMap_.end()){
    		shareSizeMap_[shareUuid] = GetTotalSize(shareUuid);
    	}
    	size = shareSizeMap_[shareUuid];
    	LtfsLogDebug("GetTotalSize: " << shareUuid << ":" << size);
    	return true;
    }

    off_t CatalogDbManager::GetTotalSize(const string& shareUuid, const string& barcode)
    {
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
		boost::scoped_ptr<ResultSet> rs;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";
    	off_t size = 0;

		try{
			string tableName = "Meta_File_" + sUuid;
			if(barcode != ""){
				tableName = "File_" + barcode + "_" + sUuid;
			}
			if(!TableExists(tableName)){
				return 0;
			}
            string dbLockStr = "lock table " + tableName + " read";
        	DbLock dbLock(connection.get(), dbLockStr);
			strSQL = "select sum(size) as total from " + tableName;
			PREPARE_SQL(strSQL);
			rs.reset(preStmt->executeQuery());
			if(rs->next()){
				size = rs->getUInt64("total");
				return size;
			}
		}
		catch (sql::SQLException& e){
			LtfsLogError("GetTotalSize \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e){
			LtfsLogError("GetTotalSize exception " << e.what());
		}

		return 0;
    }

	bool CatalogDbManager::GetNextTapeFile(const string& shareUuid, const string& barcode, const string& curUuid, off_t& size, string& nextUuid)
	{
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
		boost::scoped_ptr<ResultSet> rs;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";

		try{
			string tableName = "File_" + barcode + "_" + sUuid;
            string dbLockStr = "lock table Meta_File_" + sUuid + " read, Meta_Folder_" + sUuid + " read, " + tableName + " read";
            LtfsLogInfo("DBG: File 9");
        	DbLock dbLock(connection.get(), dbLockStr);
			// get offset of cur file
			strSQL = "select offset from " + tableName + " where uuid='" + curUuid + "'";
			off_t curOffset = 0;
			PREPARE_SQL(strSQL);
			rs.reset(preStmt->executeQuery());
			if(rs->next()){
				curOffset = rs->getUInt64("offset");
			}
			// get uuid of next file
			strSQL = "select uuid from " + tableName + " where flag=0 and offset > ";
			strSQL += boost::lexical_cast<string>(curOffset) + " order by offset ASC limit 1";
			PREPARE_SQL(strSQL);
			rs.reset(preStmt->executeQuery());;
			if(rs->next()){
				nextUuid = rs->getString("uuid");
				strSQL = "select size from Meta_File_" + sUuid + " where uuid=" + nextUuid;
				PREPARE_SQL(strSQL);
				rs.reset(preStmt->executeQuery());
				if(rs->next()){
					size = rs->getUInt64("size");
					return true;
				}
			}
		}
		catch (sql::SQLException& e){
			LtfsLogError("GetNextTapeFile \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e){
			LtfsLogError("GetNextTapeFile exception " << e.what());
		}

		return false;
	}

	bool CatalogDbManager::NeedDeleteFileOnTape(const string& shareUuid, const string& barcode)
	{
		vector<string> uuids;
		if(GetFilesToDelete(shareUuid, barcode, uuids, 1) && uuids.size() > 0){
			return true;
		}
		return false;
	}


	bool CatalogDbManager::TapeHasFile(const string& shareUuid, const string& barcode)
	{
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
		boost::scoped_ptr<ResultSet> rs;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";
    	string tableName = "File_" + barcode + "_" + sUuid;
    	if(!TableExists(tableName)){
    		// table not exists, assume tape has files
    		return true;
    	}
        string dbLockStr = "lock table " + tableName + " read";
    	DbLock dbLock(connection.get(), dbLockStr);
		try{
			strSQL = "select uuid from " + tableName + " limit 1";
			PREPARE_SQL(strSQL);
			rs.reset(preStmt->executeQuery());
			while(rs->next()){
				return true;
			}
			return false;
		}
		catch (sql::SQLException& e){
			LtfsLogError("TapeHasFile \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e){
			LtfsLogError("TapeHasFile exception " << e.what());
		}

		// exception, assume tape has files
		return true;
	}

	bool CatalogDbManager::GetFilesToDelete(const string& shareUuid, const string& barcode, vector<string>& uuids, int limit)
	{
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
		boost::scoped_ptr<ResultSet> rs;
    	GET_CONNECTION(connection, false);
    	uuids.clear();
    	string strSQL = "";
    	string tableName = "File_" + barcode + "_" + sUuid;
    	if(!TableExists(tableName)){
    		LtfsLogDebug("No need to delete files on tape " << barcode);
    		return true;
    	}
        string dbLockStr = "lock table Meta_File_" + sUuid + " read, " + tableName + " read";
    	DbLock dbLock(connection.get(), dbLockStr);

		try{
			strSQL = "select uuid from " + tableName + " where uuid not in (select uuid from Meta_File_" + sUuid;
			strSQL += ") or flag=1 ";
			if(limit > 0){
				strSQL += " limit " + boost::lexical_cast<string>(limit);
			}
			PREPARE_SQL(strSQL);
			rs.reset(preStmt->executeQuery());
			while(rs->next()){
				uuids.push_back(rs->getString("uuid"));
			}
			return true;
		}
		catch (sql::SQLException& e){
			LtfsLogError("GetFilesToDelete \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e){
			LtfsLogError("GetFilesToDelete exception " << e.what());
		}

		return false;
	}

	bool CatalogDbManager::DeleteTapeFiles(const string& shareUuid, const string& barcode, vector<string>& uuids)
	{
		string sUuid = UUID2SQL(shareUuid);
		boost::scoped_ptr<PreparedStatement> preStmt;
		boost::scoped_ptr<ResultSet> rs;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";
    	string tableName = "File_" + barcode + "_" + sUuid;
    	if(uuids.size() <= 0){
    		return true;
    	}

		try{
            string dbLockStr = "lock table " + tableName + " write";
        	DbLock dbLock(connection.get(), dbLockStr);

			strSQL = "delete from " + tableName + " where uuid in (";
			for(unsigned int i = 0; i < uuids.size(); i++){
				strSQL += uuids[i] + ",";
			}
			strSQL[strSQL.length() - 1] = ')';
			LtfsLogDebug("delete files on tape sql: " << strSQL << ".");
			PREPARE_SQL(strSQL);
       		preStmt->executeUpdate();
			return true;
		}
		catch (sql::SQLException& e){
			LtfsLogError("NeedDeleteFileOnTape \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e){
			LtfsLogError("NeedDeleteFileOnTape exception " << e.what());
		}

		return false;
	}

	bool CatalogDbManager::DiagnoseCheckTape(const string& shareUuid, const string& barcode)
	{
		string sUuid = UUID2SQL(shareUuid);
		string mountPoint = COMM_MOUNT_PATH + "/" + barcode;

		boost::scoped_ptr<PreparedStatement> preStmt;
		boost::scoped_ptr<ResultSet> rs;
    	GET_CONNECTION(connection, false);
    	string strSQL = "";
    	string tapeTableName = "File_" + barcode + "_" + sUuid;
    	if(!TableExists(tapeTableName)){
    		LtfsLogInfo("Table not exists: " << tapeTableName);
    		return true;
    	}
    	string metaTableName = "Meta_File_" + sUuid;
    	if(!TableExists(metaTableName)){
    		LtfsLogInfo("Table not exists: " << metaTableName);
    		return true;
    	}
        string dbLockStr = "lock table " + metaTableName + " read, " + tapeTableName + " read";
    	DbLock dbLock(connection.get(), dbLockStr);

		try{
			strSQL = "select uuid from " + tapeTableName + " where uuid in (select uuid from Meta_File_" + sUuid;
			strSQL += ")";
			PREPARE_SQL(strSQL);
			rs.reset(preStmt->executeQuery());
			vector<string> uuids;
			while(rs->next()){
				uuids.push_back(rs->getString("uuid"));
			}
			vector<string> corruptedUuids;
			for(unsigned int i = 0; i < uuids.size(); i++){
				string tapeFile = GetTapeFilePath(boost::lexical_cast<unsigned long long>(uuids[i]), barcode);
				if(!fs::exists(tapeFile)){
					corruptedUuids.push_back(uuids[i]);
				}
			}
			if(corruptedUuids.size() >0 && TableExists("Meta_Folder_" + sUuid)){
	            dbLockStr = "lock table " + metaTableName + " read, Meta_Folder_" + sUuid + " read, " + tapeTableName + " read";
	            dbLock.UnLockTables();
	        	dbLock.LockTables(dbLockStr);
				strSQL = "select filename, path from Meta_File_" + sUuid + " left join Meta_Folder_" \
				+ sUuid + " on Meta_File_" + sUuid + ".meta_folder=Meta_Folder_" + sUuid \
				+ ".id where Meta_File_" + sUuid + ".uuid in (";
				for(unsigned int i = 0; i < corruptedUuids.size(); i++){
					if(i > 0){
						strSQL += ",";
					}
					strSQL += corruptedUuids[i];
				}
				strSQL += ")";
				PREPARE_SQL(strSQL);
				rs.reset(preStmt->executeQuery());
				vector<string> metaFiles;
				while(rs->next()){
					string fileName = rs->getString("filename");
					string folderName = rs->getString("path");
					metaFiles.push_back(COMM_META_CACHE_PATH + "/" + shareUuid + "/" + folderName + fileName);
				}
				dbLock.UnLockTables();
				for(unsigned int i = 0; i < metaFiles.size(); i++){
					long state = 1;
					auto_ptr<ExtendedAttribute> eaFile(new ExtendedAttribute(metaFiles[i]));
					LtfsLogDebug("Updating state for file " << metaFiles[i] << " to re-write it to tape.");
					if (!eaFile->SetValue(Inode::ATTRIBUTE_STATE, &state, sizeof(state))) {
						LtfsLogError("Failed to update state for file " << metaFiles[i] << " to re-write it to tape.");
					}
				}
				if(metaFiles.size() > 0){
					string needBackupFileListFile = COMM_META_CACHE_PATH + "/" + shareUuid + "/" + shareUuid + ".save";
					try{
						if(fs::exists(needBackupFileListFile)){
							fs::remove(needBackupFileListFile);
						}
					}catch(std::exception& e){
						LtfsLogError("delete file " << needBackupFileListFile << " exception: " << e.what());
					}
				}
			}
			return true;
		}
		catch (sql::SQLException& e){
			LtfsLogError("GetFilesToDelete \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e){
			LtfsLogError("GetFilesToDelete exception " << e.what());
		}
		return true;
	}


} /* namespace ltfs_management */
