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
 * LtfsScsiDevice.cpp
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "LtfsScsiDevice.h"

namespace ltfs_management
{
	LtfsScsiDevice::LtfsScsiDevice():
			deviceMutex_(new boost::mutex())
	{
		// TODO Auto-generated constructor stub
		scsiAddr_ 	= "";
		vendor_ 	= "";
		product_ 	= "";
		version_ 	= "";
		stDev_ 		= "";
		sgDev_ 		= "";
		serial_		= "";
		status_ 	= 0;
		missing_	= false;
	}

	LtfsScsiDevice::~LtfsScsiDevice()
	{
		// TODO Auto-generated destructor stub
	}

	string
	LtfsScsiDevice::GetScsiAddr() const
	{
		return scsiAddr_;
	}

	boost::mutex* LtfsScsiDevice::GetLockMutex()
	{
		return deviceMutex_;
	}

	string
	LtfsScsiDevice::GetSerial() const
	{
		return serial_;
	}

} /* namespace ltfs_management */
