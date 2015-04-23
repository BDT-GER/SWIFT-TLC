/* Copyright (c) 2014 BDT Media Automation GmbH
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
 * ReadTapeFileSimulator.cpp
 *
 *  Created on: Sep 3, 2012
 *      Author: chento
 */

#include "ReadTapeFileSimulator.h"

ReadTapeFileSimulator::ReadTapeFileSimulator(const fs::path & pth)
	    : path(pth), handle(-1)
{
	// TODO Auto-generated constructor stub

}

ReadTapeFileSimulator::~ReadTapeFileSimulator()
{
	// TODO Auto-generated destructor stub
	Close();
}

bool
ReadTapeFileSimulator::OpenFile(int flag)
{
	if ( !fs::exists(path) )
	{
		return false;
	}
	if ( handle >= 0 )
	{
		return false;
	}

	handle = ::open(path.string().c_str(),flag);
	return handle >= 0 ? true : false;
}

bool
ReadTapeFileSimulator::Read(off_t offset,void * buffer,size_t bufsize,size_t & size)
{
	if ( handle >= 0 )
	{
		ssize_t ret = ::pread(handle,buffer,bufsize,offset);
		if ( ret >= 0 )
		{
			size = ret;
			return true;
		}
		else
		{
			size = 0;
			return false;
		}
	}
	return false;
}

void
ReadTapeFileSimulator::Close()
{
	if(handle>=0)
	{
		handle = -1;
		::close(handle);
	}
}
