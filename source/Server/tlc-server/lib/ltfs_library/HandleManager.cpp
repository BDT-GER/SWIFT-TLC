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
 * HandleManager.cpp
 *
 *  Created on: Oct 9, 2012
 *      Author: chento
 */


#ifdef SIMULATOR
#include "simulator/stdafx.h"
#include "simulator/Simulator.h"
#else
#include "stdafx.h"
#endif
#include "HandleManager.h"

namespace ltfs_management
{

#define SIZE_MARK 0x3200000

	HandleManager
	HandleManager::handleMgr_;

	struct HandleManager::Location
	{
		string m_TapeBarcode;
		size_t m_ReadSize;
		size_t m_WriteSize;
		size_t m_ReadSizeMark;
		size_t m_WriteSizeMark;
		boost::posix_time::ptime 		 m_TimeRead;
		boost::posix_time::ptime 		 m_TimeWrite;
	};

	HandleManager::HandleManager()
	{
		// TODO Auto-generated constructor stub

	}

	HandleManager::~HandleManager()
	{
		// TODO Auto-generated destructor stub
	}

	bool HandleManager::Bind(const string& path, int handle)
	{

#ifdef SIMULATOR
		struct Location location;

		if(handle<0 || path.empty())
			return false;

		if(!handleMgr_.GetLocation(path, location ))
			return true;

		ListLocationType::iterator locationItr = handleMgr_.listLocation_.find(handle);
		if(locationItr == handleMgr_.listLocation_.end())
		{
			handleMgr_.listLocation_.insert( ListLocationType::value_type(handle,location));
			return true;
		}
		else
		{
			errno = EIO;
			return false;
		}
#else
		return true;
#endif

	}

	void HandleManager::Unbind(int handle)
	{
#ifdef SIMULATOR
		ListLocationType::iterator locationItr = handleMgr_.listLocation_.find(handle);
		if(locationItr != handleMgr_.listLocation_.end())
		{
			handleMgr_.listLocation_.erase(locationItr);
		}
#endif
	}

	bool HandleManager::Read(int handle, off_t offset,void * buffer,size_t & size)
	{

#ifdef SIMULATOR
		do
		{
			ListLocationType::iterator locationItr = handleMgr_.listLocation_.find(handle);
			if(locationItr == handleMgr_.listLocation_.end())
				break;

			try
			{
				boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
				boost::posix_time::time_duration diff = now - locationItr->second.m_TimeRead;
				std::time_t seconds = diff.ticks()/boost::posix_time::time_duration::rep_type::ticks_per_second;

				size_t diffSize = locationItr->second.m_ReadSize - locationItr->second.m_ReadSizeMark;

				if(seconds >= 5
						|| diffSize >= SIZE_MARK)
				{
					locationItr->second.m_TimeRead = boost::posix_time::second_clock::local_time();
					locationItr->second.m_ReadSizeMark = (locationItr->second.m_ReadSize/SIZE_MARK)*SIZE_MARK;
					//check config

					if(Simulator::Instance()->Read(locationItr->second.m_TapeBarcode,
							locationItr->second.m_ReadSize+size))
					{
						locationItr->second.m_ReadSize += size;
						return true;
					}
					else
					{
						errno = EIO;

						return false;
					}
				}
				else
				{
					locationItr->second.m_ReadSize += size;
					return true;
				}
			}
			catch(...)
			{

			}

		}while(false);
#endif
		return true;
	}

	bool HandleManager::Write(int handle, off_t offset,void * buffer,size_t & size)
	{

#ifdef SIMULATOR
		do
		{
			ListLocationType::iterator locationItr = handleMgr_.listLocation_.find(handle);
			if(locationItr == handleMgr_.listLocation_.end())
				break;

			try
			{
				boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
				boost::posix_time::time_duration diff = now - locationItr->second.m_TimeWrite;
				std::time_t seconds = diff.ticks()/boost::posix_time::time_duration::rep_type::ticks_per_second;

				size_t diffSize = locationItr->second.m_WriteSize - locationItr->second.m_WriteSizeMark;

				if(seconds >= 5
						|| diffSize >= SIZE_MARK)
				{
					locationItr->second.m_TimeWrite = boost::posix_time::second_clock::local_time();
					locationItr->second.m_WriteSizeMark = (locationItr->second.m_WriteSize/SIZE_MARK)*SIZE_MARK;
					//check config

					if(Simulator::Instance()->Write(locationItr->second.m_TapeBarcode,
							locationItr->second.m_WriteSize+size))
					{
						locationItr->second.m_WriteSize += size;
						return true;
					}
					else
					{
						errno = EIO;
						return false;
					}
				}
				else
				{
					locationItr->second.m_WriteSize += size;
					return true;
				}


			}
			catch(...)
			{

			}

		}while(false);
#endif
		return true;
	}

	bool HandleManager::GetLocation(const string& path, struct Location& location)
	{
		//
#ifdef SIMULATOR
		;
		string subStr;
		string ltfsPath = GetLTFSFolder().string();

		if(path.find(ltfsPath) != 0)
		{
			return false;
		}

		subStr = path.substr(ltfsPath.length()+1);
		if(subStr.empty()){
			return false;
		}

		subStr = path.substr(ltfsPath.length()+1);
		if(subStr.empty())
		{
			return false;
		}

		int pos = subStr.find("/");
		if(pos == string::npos){
			return false;
		}

		location.m_TapeBarcode = subStr.substr(0, pos);
		location.m_ReadSize = 0;
		location.m_WriteSize = 0;
		location.m_ReadSizeMark = 0;
		location.m_WriteSizeMark = 0;
		location.m_TimeRead = boost::posix_time::second_clock::local_time();
		location.m_TimeWrite = boost::posix_time::second_clock::local_time();
#endif
		return true;
	}

} /* namespace tape */
