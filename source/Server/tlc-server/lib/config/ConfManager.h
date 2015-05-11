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
 * ConfManager.h
 *
 *  Created on: Dec 24, 2014
 *      Author: chensa
 */

#pragma once

#include "stdafx.h"

namespace ltfs_config
{
	class ConfManager
	{
	public:
		ConfManager(const string& confFile);
		virtual	~ConfManager();
		bool GetString(const string& confitem, string& value);
	private:
		void ReadConf(const string& confFile);
		bool HandleLine(const string& line);

	private:
		string								curSection_;
		map<string, string>					confItemMap_;
	};

} /* namespace ltfs_config */

