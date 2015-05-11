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
 * TapeDbManager.cpp
 *
 *  Created on: Nov 8, 2012
 *      Author: chento
 */


#include "stdafx.h"
#include "TapeDbManager.h"
#include "../lib/database/DbLock.h"

namespace ltfs_management
{
	#define LIBRARY_DB_MAJOR_VERSION	1
	#define LIBRARY_DB_MINOR_VERSION	0
	static const string LIBRARY_DB_VERSION_COMMENT	= "Original";

    TapeDbManager * TapeDbManager::instance_ = NULL;
    boost::mutex TapeDbManager::instanceMutex_;

#define GET_CONNECTION_VOID(_TAG) \
	boost::shared_ptr< Connection > _TAG(\
		CreateConnection(),\
		boost::bind(&TapeDbManager::ReleaseConnection, this, _1) );\
    	if(_TAG.get() == NULL){\
    		LtfsLogError("Failed to connect to database.");\
    		return;\
    	}
#define GET_CONNECTION(_TAG, _rt) \
	boost::shared_ptr< Connection > _TAG(\
		CreateConnection(),\
		boost::bind(&TapeDbManager::ReleaseConnection, this, _1) );\
    	if(_TAG.get() == NULL){\
    		LtfsLogError("Failed to connect to database.");\
    		return _rt;\
    	}
#define DB_LOCK_WRITE \
    boost::unique_lock<boost::shared_mutex> lock(rwMutex_);
#define DB_LOCK_READ \
    boost::shared_lock<boost::shared_mutex> lock(rwMutex_);

    TapeDbManager::TapeDbManager()
    {
    	cPool_.reset(new ConnectionPool());
    	CreateDatabase();
        InitDB();
    }

    TapeDbManager::~TapeDbManager()
    {
    }

    void
    TapeDbManager::InitDB()
    {
		try
		{
			if( !TableExists("Cartridges") )
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
    TapeDbManager::CreateDatabase()
    {
    	string checkSQL = "CREATE DATABASE IF NOT EXISTS Media";
    	boost::scoped_ptr< Statement > stmt;

    	try
    	{
        	boost::shared_ptr< Connection > connection(
        		cPool_->GetConnection(),
        		boost::bind(&TapeDbManager::ReleaseConnection, this, _1));
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
    TapeDbManager::ReleaseConnection(Connection * conn)
    {
    	cPool_->ReleaseConnection(conn);
    }

    Connection *
    TapeDbManager::CreateConnection()
    {
    	Connection * connection = NULL;
    	try
    	{
    		for(int i = 10; i > 0 && !connection; --i)
    		{
    			connection = cPool_->GetConnection();
    			connection->setSchema("Media");
    		}
    	}
    	catch(const std::exception & e)
    	{
    		LtfsLogError("GetConnection failed: " << e.what());
    	}

    	return connection;
    }

	bool
	TapeDbManager::TableExists(const string & tableName)
	{
		bool exist = false;
		boost::scoped_ptr<ResultSet> rs;
		boost::scoped_ptr<PreparedStatement> preStmt;
		string checkSQL = "SELECT table_name FROM information_schema.TABLES WHERE table_name =?;";
    	GET_CONNECTION(connection, false);

		try
		{
			LtfsLogDebug("TableExists " << checkSQL);
			preStmt.reset(connection->prepareStatement(checkSQL));
			preStmt->setString(1, tableName);
			rs.reset(preStmt->executeQuery());
			while (rs->next())
			{
				exist = true;
				LtfsLogDebug("TableExists, yes exists");
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

    void
    TapeDbManager::InitNewDB()
    {
        string strSQL;
        boost::scoped_ptr<Statement> stmt;
    	GET_CONNECTION_VOID(connection);
		stmt.reset( connection->createStatement() );

		{
			strSQL = "CREATE TABLE IF NOT EXISTS TapeGroup (TapeGroupUUID VARCHAR(36) PRIMARY KEY  NOT NULL, TapeGroupName TEXT) ENGINE=InnoDB DEFAULT CHARSET=utf8;";
			stmt->execute(strSQL);
		}

		{

			strSQL = "CREATE TABLE Drives (DriveSerial VARCHAR(16) PRIMARY KEY NOT NULL, DriveName VARCHAR(32) ) ENGINE=InnoDB DEFAULT CHARSET=utf8;";
			stmt->execute(strSQL);
		}

		{
			strSQL = "CREATE TABLE Cartridges (Barcode VARCHAR(10) PRIMARY KEY  NOT NULL , TapeGroupUUID VARCHAR(36),\
					MediaType INTEGER, Format INTEGER, Status INTEGER, UsedCapacity BIGINT , FreeCapacity BIGINT ,\
					FileNumber BIGINT , FileCapacity BIGINT , Faulty BOOL, \
					LoadCount BIGINT , GenerationNumber INTEGER, WriteProtect BOOL, Offline BOOL, DualCopy VARCHAR(10),\
					TapeUUID VARCHAR(36), LastMountTime BIGINT, LastAuditTime BIGINT \
					)";
			stmt->execute(strSQL);
		}

		{
			strSQL = "CREATE TABLE Version (MajorVersion INTEGER PRIMARY KEY, MinorVersion INTEGER DEFAULT 0, Comment VARCHAR(128) DEFAULT NULL)";
			stmt->execute(strSQL);
		}

		{
			strSQL = "INSERT INTO Version Values(";
			strSQL += "'" + boost::lexical_cast<string>(LIBRARY_DB_MAJOR_VERSION) + "'";
			strSQL += ",'" + boost::lexical_cast<string>(LIBRARY_DB_MINOR_VERSION) + "'";
			strSQL += ",'" + LIBRARY_DB_VERSION_COMMENT + "')";

			stmt->execute(strSQL);
        }
    }


	bool TapeDbManager::GetDriveList(vector<DriveInfo>& driveList)
    {
        string strSQL = "SELECT DriveSerial, DriveName FROM Drives";
		boost::scoped_ptr<ResultSet> rs;
		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Drives read";
    	DbLock dbLock(connection.get(), dbLockStr);

        try
        {
        	preStmt.reset(connection->prepareStatement(strSQL));
        	rs.reset(preStmt->executeQuery());

        	while (rs->next())
        	{
            	DriveInfo detail;

                detail.mDriveSerial 		= rs->getString("DriveSerial");
                detail.mDriveName			= rs->getString("DriveName");

                driveList.push_back(detail);
            }

            return true;
        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("GetDriveList \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e)
        {
            LtfsLogError("GetDriveList exception " << e.what());
        }

        LtfsLogDebug("GetDriveList Failed, SQL command: " << strSQL);
        return false;
    }

	bool TapeDbManager::GetDriveInfo(const string& driveSerial, DriveInfo& driveInfo)
    {
        string strSQL = "SELECT DriveSerial, DriveName FROM Drives WHERE DriveSerial='" + driveSerial + "'";
		boost::scoped_ptr<ResultSet> rs;
		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);

        if(driveSerial.empty())
        {
            LtfsLogWarn("GetDriveInfo driveSerial is empty");
            return false;
        }

        try
        {
    		string dbLockStr = "lock table Drives read";
        	DbLock dbLock(connection.get(), dbLockStr);

        	preStmt.reset(connection->prepareStatement(strSQL));
        	rs.reset(preStmt->executeQuery());

        	if( rs->rowsCount() <= 0 ||
        			!rs->next() )
        		return false;

            driveInfo.mDriveSerial      = rs->getString("DriveSerial");
            driveInfo.mDriveName		= rs->getString("DriveName");

            return true;

        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("GetDriveInfo \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e) {
            LtfsLogError("GetDriveInfo exception " << e.what());
        }

        LtfsLogDebug("GetDriveInfo failed, SQL command: " << strSQL);
        return false;
    }

	bool TapeDbManager::AddDrive(const DriveInfo& driveInfo)
    {
        string strSQL;
        boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);

        if(driveInfo.mDriveSerial.empty())
        {
            LtfsLogWarn("AddDrive drive serial is empty");
            return false;
        }

        try
        {
    		string dbLockStr = "lock table Drives write";
        	DbLock dbLock(connection.get(), dbLockStr);

            strSQL = "INSERT INTO Drives Values(";
            strSQL += "'" + driveInfo.mDriveSerial + "'";
            strSQL += ",'" + QuotaStringForSQL(driveInfo.mDriveName) + "'";
            strSQL += ")";

        	preStmt.reset(connection->prepareStatement(strSQL));
        	preStmt->executeUpdate();

            return true;
        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("AddDrive \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e){
            LtfsLogError("AddDrive exception " << e.what());
        }

        LtfsLogDebug("AddDrive Failed, SQL command: " << strSQL);
        return false;
    }

    bool
    TapeDbManager::GetCartridgeList(vector<CartridgeDetail>& cartridgeList, const string& changerSerial)
    {
        string strSQL = "SELECT Barcode, TapeGroupUUID, MediaType, Format, Status, UsedCapacity, \
                FreeCapacity, FileNumber, FileCapacity, Faulty, LoadCount, \
                GenerationNumber, WriteProtect, Offline, DualCopy, TapeUUID, LastMountTime, LastAuditTime FROM Cartridges";
        boost::scoped_ptr<ResultSet> rs;
        boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges read";
    	DbLock dbLock(connection.get(), dbLockStr);


        try
        {
           	preStmt.reset(connection->prepareStatement(strSQL));
			rs.reset(preStmt->executeQuery());

			while (rs->next())
			{
                CartridgeDetail detail;
                detail.mBarcode 			= rs->getString("Barcode");
                detail.mTapeGroupUUID		= rs->getString("TapeGroupUUID");
                detail.mMediaType           = rs->getInt("MediaType");
                detail.mFormat              = rs->getInt("Format");
                detail.mStatus              = rs->getInt("Status");
                detail.mUsedCapacity        = rs->getInt64("UsedCapacity");
                detail.mFreeCapacity        = rs->getInt64("FreeCapacity");
                detail.mFileNumber          = rs->getInt64("FileNumber");
                detail.mFileCapacity        = rs->getInt64("FileCapacity");
                detail.mFaulty              = rs->getBoolean("Faulty");

                if(tapeActivityMap_.find(detail.mBarcode) == tapeActivityMap_.end())
                {
                	tapeActivityMap_[detail.mBarcode] = (int)ACT_IDLE;
                }

                detail.mActivity = tapeActivityMap_[detail.mBarcode];

                if(tapeStateMap_.find(detail.mBarcode) == tapeStateMap_.end())
                {
                	tapeStateMap_[detail.mBarcode] = TAPE_STATE_IDLE;
                }

                detail.mState = tapeStateMap_[detail.mBarcode];
                detail.mLoadCount           = rs->getInt64("LoadCount");
                detail.mGenerationNumber    = rs->getInt("GenerationNumber");
                detail.mWriteProtect        = rs->getBoolean("WriteProtect");
                detail.mOffline		        = rs->getBoolean("Offline");
                detail.mDualCopy			= rs->getString("DualCopy");
                detail.mTapeUUID			= rs->getString("TapeUUID");
                detail.mLastMountTime       = boost::lexical_cast<time_t>(rs->getString("LastMountTime"));
                detail.mLastAuditTime       = boost::lexical_cast<time_t>(rs->getString("LastAuditTime"));
                cartridgeList.push_back(detail);
            }

            return true;
        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("GetCartridgeList \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e)
        {
            LtfsLogError("GetCartridgeList exception " << e.what());
        }

        LtfsLogDebug("GetCartridgeList Failed, SQL command: " << strSQL);
        return false;
    }

    bool
    TapeDbManager::GetCartridge(const string& barcode, CartridgeDetail& detail)
    {
        string strSQL = "SELECT Barcode, TapeGroupUUID, MediaType, Format, Status, UsedCapacity, \
                FreeCapacity, FileNumber, FileCapacity, Faulty, LoadCount, \
                GenerationNumber, WriteProtect, Offline, DualCopy, TapeUUID, LastMountTime, LastAuditTime FROM Cartridges WHERE Barcode='" + barcode + "'";
        boost::scoped_ptr<ResultSet> rs;
        boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges read";
    	DbLock dbLock(connection.get(), dbLockStr);

        if(barcode.empty())
        {
            LtfsLogWarn("GetCartridge barcode is empty");
            return false;
        }

        try
        {
        	preStmt.reset(connection->prepareStatement(strSQL));
			rs.reset(preStmt->executeQuery());

			if( rs->rowsCount() <= 0 ||
					!rs->next() )
				return false;

            detail.mBarcode 			= rs->getString("Barcode");
            detail.mTapeGroupUUID		= rs->getString("TapeGroupUUID");
            detail.mMediaType           = rs->getInt("MediaType");
            detail.mFormat              = rs->getInt("Format");
            detail.mStatus              = rs->getInt("Status");
            detail.mUsedCapacity        = rs->getInt64("UsedCapacity");
            detail.mFreeCapacity        = rs->getInt64("FreeCapacity");
            detail.mFileNumber          = rs->getInt64("FileNumber");
            detail.mFileCapacity        = rs->getInt64("FileCapacity");
            detail.mFaulty              = rs->getBoolean("Faulty");

            if(tapeActivityMap_.find(detail.mBarcode) == tapeActivityMap_.end())
            {
            	tapeActivityMap_[detail.mBarcode] = (int)ACT_IDLE;
            }
            detail.mActivity            = tapeActivityMap_[detail.mBarcode];

            if(tapeStateMap_.find(detail.mBarcode) == tapeStateMap_.end())
            {
            	tapeStateMap_[detail.mBarcode] = TAPE_STATE_IDLE;
            }
            detail.mState            	= tapeStateMap_[detail.mBarcode];

            detail.mLoadCount           = rs->getInt64("LoadCount");
            detail.mGenerationNumber    = rs->getInt("GenerationNumber");
            detail.mWriteProtect        = rs->getBoolean("WriteProtect");
            detail.mOffline		        = rs->getBoolean("Offline");
            detail.mDualCopy			= rs->getString("DualCopy");
            detail.mTapeUUID			= rs->getString("TapeUUID");
            detail.mLastMountTime       = boost::lexical_cast<time_t>(rs->getString("LastMountTime"));
            detail.mLastAuditTime       = boost::lexical_cast<time_t>(rs->getString("LastAuditTime"));

            return true;

        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("GetCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e)
        {
            LtfsLogError("GetCartridge exception " << e.what());
        }

        LtfsLogDebug("GetCartridge failed, SQL command: " << strSQL);
        return false;
    }

    bool
    TapeDbManager::DeleteCartridge(const string& barcode)
    {

        string strSQL = "DELETE  FROM Cartridges WHERE Barcode='" + barcode + "'";
        boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);

        if(barcode.empty())
        {
            LtfsLogWarn("DeleteCartridge barcode is empty");
            return false;
        }
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

        try
        {
        	preStmt.reset(connection->prepareStatement(strSQL));
        	preStmt->executeUpdate();

            return true;
        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("DeleteCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e)
        {
            LtfsLogError("DeleteCartridge exception " << e.what());
        }

        LtfsLogDebug("DeleteCartridge failed, SQL command: " << strSQL);
        return false;
    }

    bool
    TapeDbManager::ModifyCartridge(const string& barcode, CartridgeDetail& detail)
    {
        CartridgeDetail dtl;
        string strSQL;
        boost::scoped_ptr<PreparedStatement> preStmt;

        if(barcode.empty() || detail.mBarcode.empty())
        {
            LtfsLogWarn("ModifyCartridge barcode is empty");
            return false;
        }

        try
        {
            if(GetCartridge(barcode, dtl))
            {
            	GET_CONNECTION(connection, false);
        		string dbLockStr = "lock table Cartridges write";
            	DbLock dbLock(connection.get(), dbLockStr);

                //update
                strSQL = "UPDATE Cartridges SET ";
                strSQL += "Barcode='" + detail.mBarcode + "'";
                strSQL += " ,TapeGroupUUID='" + detail.mTapeGroupUUID + "'";
                strSQL += " ,MediaType=" + boost::lexical_cast<string>(detail.mMediaType);
                strSQL += " ,Format=" + boost::lexical_cast<string>(detail.mFormat);
                strSQL += " ,Status=" + boost::lexical_cast<string>(detail.mStatus);
                strSQL += " ,UsedCapacity=" + boost::lexical_cast<string>(detail.mUsedCapacity);
                strSQL += " ,FreeCapacity=" + boost::lexical_cast<string>(detail.mFreeCapacity);
                strSQL += " ,FileNumber=" + boost::lexical_cast<string>(detail.mFileNumber);
                strSQL += " ,FileCapacity=" + boost::lexical_cast<string>(detail.mFileCapacity);
                strSQL += " ,Faulty=" + boost::lexical_cast<string>(detail.mFaulty);
                strSQL += " ,LoadCount=" + boost::lexical_cast<string>(detail.mLoadCount);
                strSQL += " ,GenerationNumber=" + boost::lexical_cast<string>(detail.mGenerationNumber);
                strSQL += " ,WriteProtect=" + boost::lexical_cast<string>(detail.mWriteProtect);
                strSQL += " ,Offline=" + boost::lexical_cast<string>(detail.mOffline);
                strSQL += " ,DualCopy='" + detail.mDualCopy + "'";
                strSQL += " ,TapeUUID='" + detail.mTapeUUID + "'";
                strSQL += " ,LastMountTime=" + boost::lexical_cast<string>(detail.mLastMountTime);
                strSQL += " ,LastAuditTime=" + boost::lexical_cast<string>(detail.mLastAuditTime);
                strSQL += " WHERE Barcode='" + barcode + "'";

                preStmt.reset(connection->prepareStatement(strSQL));
                preStmt->executeUpdate();
            }
            else
            {
            	GET_CONNECTION(connection, false);
        		string dbLockStr = "lock table Cartridges write";
            	DbLock dbLock(connection.get(), dbLockStr);

                //insert
                strSQL = "INSERT INTO Cartridges Values(";
                strSQL += "'" + detail.mBarcode + "'";
                strSQL += ",'" + detail.mTapeGroupUUID + "'";
                strSQL += "," + boost::lexical_cast<string>(detail.mMediaType);
                strSQL += "," + boost::lexical_cast<string>(detail.mFormat);
                strSQL += "," + boost::lexical_cast<string>(detail.mStatus);
                strSQL += "," + boost::lexical_cast<string>(detail.mUsedCapacity);
                strSQL += "," + boost::lexical_cast<string>(detail.mFreeCapacity);
                strSQL += "," + boost::lexical_cast<string>(detail.mFileNumber);
                strSQL += "," + boost::lexical_cast<string>(detail.mFileCapacity);
                strSQL += "," + boost::lexical_cast<string>(detail.mFaulty);
                strSQL += "," + boost::lexical_cast<string>(detail.mLoadCount);
                strSQL += "," + boost::lexical_cast<string>(detail.mGenerationNumber);
                strSQL += "," + boost::lexical_cast<string>(detail.mWriteProtect);
                strSQL += "," + boost::lexical_cast<string>(detail.mOffline);
                strSQL += ",'" + detail.mDualCopy + "'";
                strSQL += ",'" + detail.mTapeUUID + "'";
                strSQL += "," + boost::lexical_cast<string>(detail.mLastMountTime);
                strSQL += "," + boost::lexical_cast<string>(detail.mLastAuditTime);
                strSQL += ")";

                preStmt.reset(connection->prepareStatement(strSQL));
                preStmt->executeUpdate();
            }

            return true;
        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("ModifyCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e)
        {
            LtfsLogError("ModifyCartridge exception " << e.what());
        }

        LtfsLogDebug("ModifyCartridge Failed, SQL command: " << strSQL);
        return false;
    }

    bool
    TapeDbManager::GetTapeGroupList(vector<string>& list)
    {
        string strSQL = "SELECT * FROM TapeGroup";
		boost::scoped_ptr<ResultSet> rs;
		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table TapeGroup read";
    	DbLock dbLock(connection.get(), dbLockStr);

        try
        {
        	preStmt.reset(connection->prepareStatement(strSQL));
        	rs.reset(preStmt->executeQuery());

        	while (rs->next())
            {
                list.push_back( rs->getString("TapeGroupUUID") );
            }

            return true;
        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("GetTapeGroupList \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e)
        {
            LtfsLogError("GetTapeGroupList exception " << e.what());
        }

        LtfsLogDebug("GetTapeGroupList Failed, SQL command: " << strSQL);
        return false;
    }

    bool TapeDbManager::GetTapeGroupDualCopy(const string& groupUUID)
    {
    	return false;
    }

    bool
    TapeDbManager::AddTapeGroup(const string& group, const string& name)
    {
        string strSQL;
		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);

        if(group.empty())
        {
            LtfsLogWarn("AddTapeGroup group is empty");
            return false;
        }
		string dbLockStr = "lock table TapeGroup write";
    	DbLock dbLock(connection.get(), dbLockStr);

        try
        {
            strSQL = "INSERT INTO TapeGroup Values(";
            strSQL += "'" + group + "'";
            strSQL += ",'" + QuotaStringForSQL(name) + "'";
            strSQL += ")";

            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("AddTapeGroup \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e)
        {
            LtfsLogError("AddTapeGroup exception " << e.what());
        }

        LtfsLogDebug("AddTapeGroup Failed, SQL command: " << strSQL);
        return false;
    }


    bool
    TapeDbManager::DeleteTapeGroup(const string& group)
    {
        string strSQL = "DELETE FROM TapeGroup WHERE TapeGroupUUID='"+group+"'";
		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);

        if(group.empty())
        {
            LtfsLogWarn("AddTapeGroup group is empty");
            return false;
        }
		string dbLockStr = "lock table TapeGroup write";
    	DbLock dbLock(connection.get(), dbLockStr);

        try
        {
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("DeleteTapeGroup \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e)
        {
            LtfsLogError("DeleteTapeGroup exception " << e.what());
        }

        LtfsLogDebug("DeleteTapeGroup Failed, SQL command: " << strSQL);
        return false;
    }

    bool
    TapeDbManager::GetTapeGroupCartridgeList(const string& group, vector<string>& list)
    {
        string strSQL = "SELECT Barcode FROM Cartridges WHERE TapeGroupUUID='"+group+"'";
		boost::scoped_ptr<ResultSet> rs;
		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);

        if(group.empty())
        {
            LtfsLogWarn("GetTapeGroupCartridgeList group is empty");
            return false;
        }
		string dbLockStr = "lock table Cartridges read";
    	DbLock dbLock(connection.get(), dbLockStr);

        try
        {
        	preStmt.reset(connection->prepareStatement(strSQL));
        	rs.reset(preStmt->executeQuery());

            LtfsLogDebug("GetTapeGroupCartridgeList successful, SQL command: " << strSQL << " group:"<< group << " count:"<< rs->rowsCount());

            while (rs->next())
            {
                list.push_back( rs->getString("Barcode"));
            }

            return true;
        }
		catch (sql::SQLException& e)
		{
			LtfsLogError("GetTapeGroupCartridgeList \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
        catch(std::exception& e)
        {
            LtfsLogError("GetTapeGroupCartridgeList exception " << e.what());
        }

        LtfsLogDebug("GetTapeGroupCartridgeList Failed, SQL command: " << strSQL);
        return false;
    }

    bool
    TapeDbManager::SetGroupUUIDForCartridge(const string& barcode, const string& group)
    {
    	string strSQL = "UPDATE Cartridges SET ";
        strSQL += "TapeGroupUUID='" + group + "'";
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetGroupUUIDForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetGroupUUIDForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetGroupUUIDForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }
    bool TapeDbManager::ChangeTapeBarcode(const string& oldBarcode, const string& newBarcode)
    {
    	string strSQL = "UPDATE Cartridges SET ";
        strSQL += "Barcode='" + newBarcode + "'";
        strSQL += " WHERE Barcode='" + oldBarcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
    	{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("ChangeTapeBarcode \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e){
			LtfsLogError("ChangeTapeBarcode exception " << e.what());
		}

    	LtfsLogWarn("ChangeTapeBarcode Failed, SQL command: " << strSQL);
    	return false;
    }


    bool
    TapeDbManager::SetTapeUUIDForCartridge(const string& barcode, const string& uuid)
    {
    	string strSQL = "UPDATE Cartridges SET ";
        strSQL += "TapeUUID='" + uuid + "'";
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetTapeUUIDForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetTapeUUIDForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetTapeUUIDForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

    bool
    TapeDbManager::SetDualCopyForCartridge(const string& barcode, const string& dualCopy)
    {
    	string strSQL = "UPDATE Cartridges SET ";
        strSQL += "DualCopy='" + dualCopy + "'";
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetDualCopyForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetDualCopyForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetDualCopyForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetStatusForCartridge(const string& barcode, int status)
    {
    	string strSQL = "UPDATE Cartridges SET ";
        strSQL += "Status=" + boost::lexical_cast<string>(status);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetStatusForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetStatusForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetStatusForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetLoadCountForCartridge(const string& barcode, long long loadCount)
    {
    	string strSQL = "UPDATE Cartridges SET ";
        strSQL += "LoadCount=" + boost::lexical_cast<string>(loadCount);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetLoadCountForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetLoadCountForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetLoadCountForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetWriteProtectForCartridge(const string& barcode, bool bWriteProtected)
	{
    	string strSQL = "UPDATE Cartridges SET ";
        strSQL += "WriteProtect=" + boost::lexical_cast<string>(bWriteProtected);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetWriteProtectForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetWriteProtectForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetWriteProtectForCartridge Failed, SQL command: " << strSQL);
    	return false;
	}

	bool
	TapeDbManager::SetOfflineForCartridge(const string& barcode, bool bOffline)
	{
    	string strSQL = "UPDATE Cartridges SET ";
        strSQL += "Offline=" + boost::lexical_cast<string>(bOffline);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetOfflineForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetOfflineForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetOfflineForCartridge Failed, SQL command: " << strSQL);
    	return false;
	}

	bool
	TapeDbManager::SetGenerationNumberForCartridge(const string& barcode, int generation)
    {
    	string strSQL = "UPDATE Cartridges SET ";
        strSQL += "GenerationNumber=" + boost::lexical_cast<string>(generation);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetGenerationNumberForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetGenerationNumberForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetGenerationNumberForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetUsedCapacityForCartridge(const string& barcode, long long usedCapacity)
    {
    	string strSQL = "UPDATE Cartridges SET ";
        strSQL += "UsedCapacity=" + boost::lexical_cast<string>(usedCapacity);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetUsedCapacityForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetUsedCapacityForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetUsedCapacityForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetFreeCapacityForCartridge(const string& barcode, long long freeCapacity)
    {

    	string strSQL = "UPDATE Cartridges SET ";
    	strSQL += "FreeCapacity=" + boost::lexical_cast<string>(freeCapacity);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetFreeCapacityForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetFreeCapacityForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetFreeCapacityForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetFileNumberForCartridge(const string& barcode, long fileNumber)
    {
    	string strSQL = "UPDATE Cartridges SET ";
    	strSQL += "FileNumber=" + boost::lexical_cast<string>(fileNumber);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetFileNumberForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetFileNumberForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetFileNumberForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetFileCapacityForCartridge(const string& barcode, long long fileCapacity)
    {
    	string strSQL = "UPDATE Cartridges SET ";
    	strSQL += "FileCapacity=" + boost::lexical_cast<string>(fileCapacity);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetFileCapacityForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetFileCapacityForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetFileCapacityForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetLastMountTimeForCartridge(const string& barcode, time_t lastMountTime)
    {
    	string strSQL = "UPDATE Cartridges SET ";
    	strSQL += "LastMountTime=" + boost::lexical_cast<string>(lastMountTime);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetLastMountTimeForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetLastMountTimeForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetLastMountTimeForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetLastAuditTimeForCartridge(const string& barcode, time_t lastAuditTime)
    {
    	string strSQL = "UPDATE Cartridges SET ";
    	strSQL += "LastAuditTime=" + boost::lexical_cast<string>(lastAuditTime);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetLastAuditTimeForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetLastAuditTimeForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetLastAuditTimeForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetFormatForCartridge(const string& barcode, int format)
    {
    	string strSQL = "UPDATE Cartridges SET ";
    	strSQL += "Format=" + boost::lexical_cast<string>(format);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetFormatForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetFormatForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetFormatForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetFaultyForCartridge(const string& barcode, bool faulty)
    {
    	if(faulty)
    		faulty = true;
    	else
    		faulty = false;

    	string strSQL = "UPDATE Cartridges SET ";
    	strSQL += "Faulty=" + boost::lexical_cast<string>(faulty);
        strSQL += " WHERE Barcode='" + barcode + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table Cartridges write";
    	DbLock dbLock(connection.get(), dbLockStr);

    	try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetFaultyForCartridge \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
    	catch(std::exception& e)
		{
			LtfsLogError("SetFaultyForCartridge exception " << e.what());
		}

    	LtfsLogWarn("SetFaultyForCartridge Failed, SQL command: " << strSQL);
    	return false;
    }

	bool
	TapeDbManager::SetStateForCartridge(const string& barcode, TapeState state)
    {
		DB_LOCK_WRITE;
    	if(barcode == ""){
    		return false;
    	}

    	tapeStateMap_[barcode] = state;
    	return true;
    }

	string TapeActivity2Str(TapeActivity act){
		switch(act){
			case ACT_IDLE:
				return "ACT_IDLE";
				break;
			case ACT_WAITING:
				return "ACT_WAITING";
				break;
			case ACT_FORMATTING:
				return "ACT_FORMATTING";
				break;
			case ACT_READING_LABEL:
				return "ACT_READING_LABEL";
				break;
			case ACT_WRITTING_LABEL:
				return "ACT_WRITTING_LABEL";
				break;
			case ACT_READING_DATA:
				return "ACT_READING_DATA";
				break;
			case ACT_WRITTING_DATA:
				return "ACT_WRITTING_DATA";
				break;
			case ACT_LISTING_FILES:
				return "ACT_LISTING_FILES";
				break;
			case ACT_DELETING_FILES:
				return "ACT_DELETING_FILES";
				break;
			case ACT_ACCESSING:
				return "ACT_ACCESSING";
				break;
			case ACT_DIAGNOSING:
				return "ACT_DIAGNOSING";
				break;
			case ACT_LOADING:
				return "ACT_LOADING";
				break;
			case ACT_UNLOADING:
				return "ACT_UNLOADING";
				break;
			case ACT_MOVING:
				return "ACT_MOVING";
				break;
			case ACT_MOUNTING:
				return "ACT_MOUNTING";
				break;
			case ACT_UNMOUNTING:
				return "ACT_UNMOUNTING";
				break;
			default:
				return "ACT_UNKNOWN";
				break;
		}
		return "ACT_UNKNOWN";
	}
	bool
	TapeDbManager::GetActivityForCartridge(const string& barcode, int& activity)
    {
    	DB_LOCK_READ;
    	if(barcode == ""){
    		return false;
    	}
    	activity = (int)ACT_IDLE;
    	if(tapeActivityMap_.find(barcode) != tapeActivityMap_.end()){
    		activity = tapeActivityMap_[barcode];
    	}
    	LtfsLogDebug("Getting activity for tape " << barcode << ":" << TapeActivity2Str((TapeActivity)activity) << ".");
    	return true;
    }

	bool
	TapeDbManager::SetActivityForCartridge(const string& barcode, int activity)
    {
    	DB_LOCK_WRITE;
    	if(barcode == ""){
    		return false;
    	}

    	LtfsLogDebug("Setting activity for tape " << barcode << " to " << TapeActivity2Str((TapeActivity)activity) << ".");
    	tapeActivityMap_[barcode] = activity;
    	return true;
    }

	bool
	TapeDbManager::SetGroupName(const string& group, const string& name)
	{
		string strSQL = "UPDATE TapeGroup SET ";
		strSQL += "TapeGroupName='" + QuotaStringForSQL(name) + "'";
		strSQL += " WHERE TapeGroupUUID='" + group + "'";

		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table TapeGroup write";
    	DbLock dbLock(connection.get(), dbLockStr);

		try
		{
            preStmt.reset(connection->prepareStatement(strSQL));
            preStmt->executeUpdate();

            return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("SetGroupName \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e)
		{
			LtfsLogError("SetGroupName exception " << e.what());
		}

		LtfsLogWarn("SetGroupName Failed, SQL command: " << strSQL);
		return false;
	}

	bool
	TapeDbManager::GetGroupName(const string& group, string& name)
	{
        string strSQL = "SELECT TapeGroupName FROM TapeGroup WHERE TapeGroupUUID='" + group + "'";
		boost::scoped_ptr<ResultSet> rs;
		boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table TapeGroup read";
    	DbLock dbLock(connection.get(), dbLockStr);

		try
		{
        	preStmt.reset(connection->prepareStatement(strSQL));
        	rs.reset(preStmt->executeQuery());

        	if( rs->rowsCount() <= 0 ||
        			!rs->next() )
        		return false;

        	name = rs->getString("TapeGroupName");
			return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("GetGroupName \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e)
		{
			LtfsLogError("GetGroupName exception " << e.what());
		}

		LtfsLogWarn("GetGroupName Failed, SQL command: " << strSQL);
		return false;
	}

	bool
	TapeDbManager::GetShareUUID(const string& name, string& uuid)
	{
		string strSQL = "SELECT TapeGroupUUID FROM TapeGroup WHERE TapeGroupName='" + QuotaStringForSQL(name) + "'";
        boost::scoped_ptr<ResultSet> rs;
        boost::scoped_ptr<PreparedStatement> preStmt;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table TapeGroup read";
    	DbLock dbLock(connection.get(), dbLockStr);

		try
		{
			preStmt.reset(connection->prepareStatement(strSQL));
			rs.reset(preStmt->executeQuery());

			if( rs->rowsCount() <= 0 ||
					!rs->next() )
				return false;

			uuid = rs->getString("TapeGroupUUID");
			return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("GetShareUUID \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e)
		{
			LtfsLogError("GetShareUUID exception " << e.what());
		}

		LtfsLogWarn("GetShareUUID Failed, SQL command: " << strSQL);
		return false;
	}

	bool
	TapeDbManager::CheckTapeGroupExistByName(const string name)
	{
		boost::scoped_ptr<ResultSet> rs;
		boost::scoped_ptr<PreparedStatement> preStmt;

		string strSQL;
    	GET_CONNECTION(connection, false);
		string dbLockStr = "lock table TapeGroup read";
    	DbLock dbLock(connection.get(), dbLockStr);

		try
		{
			strSQL = "SELECT TapeGroupUUID FROM TapeGroup WHERE (TapeGroupName LIKE '" + QuotaStringForSQL(name) + "')"; //nocase for share name

			preStmt.reset(connection->prepareStatement(strSQL));
			rs.reset(preStmt->executeQuery());

        	if( rs->rowsCount() <= 0 ||
        			!rs->next() )
        		return false;

			return true;
		}
		catch (sql::SQLException& e)
		{
			LtfsLogError("CheckTapeGroupExistByName \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
		}
		catch(std::exception& e)
		{
			LtfsLogError("CheckTapeGroupExistByName exception " << e.what());
		}

		LtfsLogWarn("Exit CheckTapeGroupExistByName Failed, SQL command: " << strSQL);

		return false;
	}

    bool IsTapeWritable(const CartridgeDetail & detail)
    {
        if ( detail.mFaulty ) {
            return false;
        }
        if ( detail.mWriteProtect ) {
            return false;
        }
        if ( detail.mFormat != LTFS_VALID ) {
            return false;
        }
        if ( detail.mOffline ) {
            return false;
        }
        if (detail.mStatus == TAPE_CLOSED){
            return false;
        }
        long long minTapeFree = TapeLibraryMgr::GetSizeMinTapeFree();
        if(detail.mFreeCapacity <= minTapeFree){
        	return false;
        }
        return true;
    }

    bool TapeDbManager::GetShareAvailableTapes(const string& uuid, vector<map<string, off_t> >& tapesList)
    {

		vector<string> barcodeList;
		if ( !GetTapeGroupCartridgeList(uuid,barcodeList) ) {
			LtfsLogError( "Failed to get cartridges from database"
					<< " for tape group " << uuid );
			return false;
		}

		long long minTapeFree = TapeLibraryMgr::GetSizeMinTapeFree();
		CartridgeDetail detail;
		CartridgeDetail dualTapeDetail;
		for ( vector<string>::iterator i = barcodeList.begin(); i != barcodeList.end(); ++ i ) {
			map<string, off_t> mapItem;
			if ( GetCartridge(*i,detail) ) {
				if ( !IsTapeWritable(detail) ) {
			    	LtfsLogDebug("GetShareAvailableTapes  tape not writable " << detail.mBarcode);
					continue;
				}

				if (GetTapeGroupDualCopy(uuid))
				{
					if (detail.mDualCopy == "")
					{
						LtfsLogError("Coupled Tape is gone for " << detail.mBarcode);
						continue;
					}
					if ( !GetCartridge(detail.mDualCopy, dualTapeDetail)
							|| detail.mTapeGroupUUID != dualTapeDetail.mTapeGroupUUID) {
						LtfsLogError("Failed to get cartridge from database: " << detail.mDualCopy);
						continue;
					}

					if ( !IsTapeWritable(dualTapeDetail)) {
						LtfsLogDebug("One of the coupled Tapes cann't be write: " << detail.mBarcode);
						continue;
					}
					mapItem[dualTapeDetail.mBarcode] = 0;
					if( dualTapeDetail.mFreeCapacity > minTapeFree){
						mapItem[dualTapeDetail.mBarcode] = dualTapeDetail.mFreeCapacity - minTapeFree;
					}
				}
				mapItem[detail.mBarcode] = 0;
				if( detail.mFreeCapacity > minTapeFree){
					mapItem[detail.mBarcode] = detail.mFreeCapacity - minTapeFree;
				}
		    	LtfsLogDebug("GetShareAvailableTapes adding tape " << detail.mBarcode << ":" << mapItem[detail.mBarcode]);
				tapesList.push_back(mapItem);
			} else {
				LtfsLogError("Failed to get cartridge from database: " << *i);
			}
		}

    	LtfsLogDebug("GetShareAvailableTapes  tapesList.szie: " << tapesList.size());
		return true;
    }

} /* namespace ltfs_management */
