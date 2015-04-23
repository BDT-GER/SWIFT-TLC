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
 * ConnectionPool.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: Sam Chen
 */
#include "ConnectionPool.h"
#include "DbLock.h"

DbLock::DbLock(Connection* conn)
{
	conn_ = conn;
	bLocked_ = false;
	lockStr_ = "";
}
DbLock::DbLock(Connection* conn, const string& lockStr)
{
	conn_ = conn;
	bLocked_ = false;
	lockStr_ = "";
	LockTables(lockStr);
}

DbLock::~DbLock(void)
{
	if(bLocked_){
		UnLockTables();
	}
}


bool DbLock::LockTables(const string& lockStr)
{
	try{
		if(lockStr != ""){
			lockStr_ = lockStr;
			boost::scoped_ptr<Statement> stmt;
			stmt.reset(conn_->createStatement());
			stmt->executeUpdate(lockStr);
			bLocked_ = true;
		}
		return true;
	}
	catch (sql::SQLException& e){
		LtfsLogError("LockTables \""<< lockStr <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
	}
	catch(std::exception& e){
		LtfsLogError("LockTables exception " << e.what());
	}
	return false;
}

bool DbLock::UnLockTables()
{
	if(conn_ == NULL){
		return false;
	}

	string strSQL = "unlock tables";
	try{
		if(bLocked_){
			boost::scoped_ptr<Statement> stmt;
			stmt.reset(conn_->createStatement());
			stmt->executeUpdate(strSQL);
			bLocked_ = false;
		}
		return true;
	}
	catch (sql::SQLException& e){
		LtfsLogError("UnLockTables \""<<strSQL <<"\" SQLState:"<<e.getSQLState() <<"  ErrorCode:"<<e.getErrorCode());
	}
	catch(std::exception& e){
		LtfsLogError("UnLockTables exception " << e.what());
	}
	return false;
}
