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
 * CfgfileSerialization.h
 *
 *  Created on: Oct 16, 2012
 *      Author: chento
 */

#pragma once

#include "../../../tinyxml/tinyxml.h"

namespace ltfs_management
{

	class CfgfileSerialization
	{
	public:
		CfgfileSerialization(const char* filename);

		virtual
		~CfgfileSerialization();

		bool
		SetCfgDetail(struct CfgDetail& detail);

		bool
		SetChangers(map<string,SimChanger>& changers);

		bool
		SaveToFile();

	private:
		bool
		SetDrives(TiXmlElement* parentElement, vector<struct DriveDetail>& drives);

		bool
		SetSlots(TiXmlElement* parentElement, vector<struct SlotDetail>& slots);

		bool
		SetMailSlots(TiXmlElement* parentElement, vector<struct MailSlotDetail>& mailslots);

		bool
		SetTapes(TiXmlElement* parentElement, vector<struct TapeDetail>& tapes);

	private:
		string filename_;

		TiXmlDocument* document_;
		TiXmlElement* rootElement_;
	};

} /* namespace ltfs_management */

