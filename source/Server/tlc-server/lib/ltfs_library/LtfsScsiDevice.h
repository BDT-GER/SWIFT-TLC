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
 * LtfsScsiDevice.h
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#pragma once

namespace ltfs_management
{
	struct ScsiInfo
	{
		string	mScsiAddr;
		string	mVendor;
		string	mProduct;
		string	mVersion;
		string	mStDev;
		string	mSgDev;
	};

	class LtfsScsiDevice
	{
	public:
		LtfsScsiDevice();

		virtual
		~LtfsScsiDevice();

		string
		GetScsiAddr() const;

		string
		GetSerial() const;

		virtual void
		SetMissing(bool missing=true) = 0;

		virtual bool
		GetMissing() = 0;

		boost::mutex*	GetLockMutex();

	protected:
		string	scsiAddr_;
		string	vendor_;
		string	product_;
		string	version_;
		string	stDev_;
		string	sgDev_;
		string	serial_;
		int		status_;

		bool	missing_;

	protected:
		boost::mutex*   deviceMutex_;
	};

} /* namespace ltfs_management */

