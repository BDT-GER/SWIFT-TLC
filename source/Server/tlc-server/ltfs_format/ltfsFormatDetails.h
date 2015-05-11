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
 * ltfsFormatDetails.h
 *
 *  Created on: Dec 7, 2012
 *      Author: chento
 */

#pragma once


namespace ltfs_management
{

	#define LABEL_STATUS 0x0001
	#define LABEL_FAULTY 0x0002
	#define LABEL_GROUP	 0x0004
	#define LABEL_DUAL_COPY	 0x0008
	#define LABEL_TAPE_ID	 0x0016

	enum FormatThreadStatus
	{
		FORMAT_THREAD_STATUS_WAITING,
		FORMAT_THREAD_STATUS_FORMATTING,
		FORMAT_THREAD_STATUS_FINISHED
	};

	enum TapeActivity
	{
		ACT_IDLE,
		ACT_WAITING,
		ACT_FORMATTING,
		ACT_READING_LABEL,
		ACT_WRITTING_LABEL,
		ACT_READING_DATA,
		ACT_WRITTING_DATA,
		ACT_LISTING_FILES,
		ACT_DELETING_FILES,
		ACT_ACCESSING,
		ACT_DIAGNOSING,
		ACT_AUDITING,
		ACT_LOADING,
		ACT_UNLOADING,
		ACT_MOVING,
		ACT_MOUNTING,
		ACT_UNMOUNTING
	};

	enum FormatType
	{
		FormatType_Format = 0,
		FormatType_Label,
		FormatType_Both
	};

	enum InventoryStatus
	{
		InventoryStatus_Finish = 0,
		InventoryStatus_Waiting,
		InventoryStatus_Running
	};

	struct Labels
	{
		int mMark;	//label mark
		int	mStatus;
		bool mFaulty;
		string mGroupID;
		string mDualCopy;
		string mTapeID;
	};

	struct FormatDetail
	{
		int	mPriority;
		string mBarcode;
		FormatThreadStatus mStatus;
		FormatType	mType;
		Labels mLabels;
		time_t mStartTime;   // start time in micro seconds
	};

}
