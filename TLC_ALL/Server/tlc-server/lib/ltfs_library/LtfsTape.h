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
 * LtfsTape.h
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#pragma once

namespace ltfs_management
{
	class LtfsTape
	{
	public:
		LtfsTape(const string& barcode, int mediumType);

		virtual
		~LtfsTape();

		string
		GetBarcode() const;

		int
		GetMediumType() const;

	public:
		static string BarcodeEmpty;
		static LtfsTape NoBarcodeTape;
	private:
		string 	barcode_;
		int		mediumType_;
	};

} /* namespace ltfs_management */

