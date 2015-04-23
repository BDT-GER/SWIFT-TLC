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


ConnectionPool::ConnectionPool(int maxSize)
{
	maxSize_ = maxSize;
    curSize_ = 0;

	driver_ = get_driver_instance();

	Init(maxSize/2);
}


ConnectionPool::~ConnectionPool(void)
{
	Destroy();
}

void
ConnectionPool::Init(int size)
{
	boost::unique_lock<boost::mutex> lock(mutex_);
	Connection * conn ;

	for(int i = 0; i < size ;)
	{
        conn = this->CreateConnection();
        if(conn)
		{
            i++;
            connections_.push_back(conn);
            ++curSize_;
        }
        else
		{
            LtfsLogError("Init connection pool fail one!");
        }
    }
}

void
ConnectionPool::Destroy()
{
	boost::unique_lock<boost::mutex> lock(mutex_);

	for(deque<Connection *>::iterator pos = connections_.begin();
		pos != connections_.end();++pos)
	{
        TerminateConnection(*pos);
    }

    curSize_ = 0;
    connections_.clear();
}

Connection *
ConnectionPool::CreateConnection(const string& passwd, bool bLogErr)
{
	Connection *conn = NULL;

	try
	{
		 conn = driver_->connect(COMM_DB_SERVER, COMM_DEFDB_USER, passwd);
	}
	catch(const sql::SQLException & e)
	{
		if(bLogErr){
			LtfsLogWarn( "CreateConnection SQLState:"
				<< e.getSQLState()
				<< "  ErrorCode:"
				<< e.getErrorCode()
				<< ".   " << e.what());
		}
	}
	catch(const std::exception & e)
	{
		if(bLogErr){
			LtfsLogWarn("CreateConnection : " << e.what());
		}
	}

	return conn;
}

Connection *
ConnectionPool::CreateConnection()
{
	Connection *conn = CreateConnection("", false);
	if(NULL == conn){
		conn = CreateConnection(COMM_DEFDB_PASS);
	}
	if(NULL == conn){
		LtfsLogError("CreateConnection failed.");
	}
	return conn;
}

void
ConnectionPool::TerminateConnection(Connection * conn)
{
	if(conn)
	{
		delete conn;
	}
}

Connection *
ConnectionPool::GetConnection()
{
	boost::unique_lock<boost::mutex> lock(mutex_);
    Connection * conn = NULL;

    if(connections_.size() > 0)
	{
		//with available connection, return
        conn = connections_.front();
        connections_.pop_front();

        if(conn->isClosed())
		{
            LtfsLogDebug("GetConnection: a mysql conn has been closed.");
            delete conn;
            conn = CreateConnection();
        }

        if(conn == NULL)
		{
			//failed to create connection
            --curSize_;
        }
    }
    else
	{
        if(curSize_ < maxSize_)
		{
			//able to create new connection
            conn = CreateConnection();
            if(conn)
			{
                ++curSize_;
            }
        }
    }

	return conn;
}

void
ConnectionPool::ReleaseConnection(Connection * conn)
{
	if(conn)
	{
		boost::unique_lock<boost::mutex> lock(mutex_);
		connections_.push_back(conn);
	}
}
