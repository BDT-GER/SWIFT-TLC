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
 * LtfsTape.cpp
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "LtfsDetails.h"
#include "LtfsTape.h"

namespace ltfs_management
{
	string LtfsTape::BarcodeEmpty = "";
	LtfsTape LtfsTape::NoBarcodeTape(BarcodeEmpty, MEDIUM_UNKNOWN);

	LtfsTape::LtfsTape(const string& barcode, int mediumType)
	{
		// TODO Auto-generated constructor stub
		barcode_ 	= barcode;
		mediumType_	= mediumType;
	}

	LtfsTape::~LtfsTape()
	{
		// TODO Auto-generated destructor stub
	}

	string
	LtfsTape::GetBarcode() const
	{
		return barcode_;
	}

	int
	LtfsTape::GetMediumType() const
	{
		return mediumType_;
	}
} /* namespace ltfs_management */
