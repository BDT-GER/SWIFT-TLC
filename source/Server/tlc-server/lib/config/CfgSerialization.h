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
 * CfgSerialization.h
 *
 *  Created on: Nov 5, 2012
 *      Author: chento
 */

#pragma once

namespace ltfs_config
{

	class CfgSerialization
	{
	public:
		CfgSerialization(const char* filename);

		virtual
		~CfgSerialization();

		bool
		SetChild(const string& tag, const string& value);

		bool
		DeleteNode(const string& parentNodeName, const string& nodeName);
	private:
		string filename_;
	};

} /* namespace ltfs_config */

