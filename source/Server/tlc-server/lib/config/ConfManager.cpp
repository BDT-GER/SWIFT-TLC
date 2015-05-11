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
 * ConfManager.cpp
 *
 *  Created on: Nov 5, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "ConfManager.h"

namespace ltfs_config
{
	ConfManager::ConfManager(const string& confFile)
	{
		ReadConf(confFile);
		curSection_ = "";
	}

	ConfManager::~ConfManager()
	{
	}

	void ConfManager::ReadConf(const string& confFile)
	{
		try{
			if(!fs::exists(confFile)){
				return;
			}
			ifstream inFile;
			inFile.open(confFile.c_str());
			while(!inFile.eof()){
				string line = "";
				getline(inFile, line);
				if(!HandleLine(line)){
					CfgError("Failed to handle corrupted file " << line);
				}
			}
			inFile.close();
		}catch(...){
			CfgError("Failed to parser conf file " << confFile);
		}
	}

	bool ConfManager::HandleLine(const string& line)
	{
		regex matchComment("^\\s*\\#(.*)\\s*$");
		regex matchEmpty("^\\s*$");
		regex matchSection("^\\s*\\[(.*)\\]\\s*$");
		regex matchKeyValue("^\\s*(\\S+)\\s*\\=\\s*(.*)\\s*$");
		cmatch match;
		if(regex_match(line.c_str(), match, matchComment)
		|| regex_match(line.c_str(), match, matchEmpty)){
			return true;
		}
		if(regex_match(line.c_str(), match, matchSection)){
			curSection_ = match[1];
			return true;
		}
		if(regex_match(line.c_str(), match, matchKeyValue)){
			if(curSection_ != ""){
				confItemMap_[curSection_ + "." + match[1]] = match[2];
			}
			return true;
		}

		return false;
	}

	bool ConfManager::GetString(const string& confitem, string& value)
	{
		map<string, string>::iterator it = confItemMap_.find(confitem);
		if(it == confItemMap_.end()){
			CfgDebug("confitem " << confitem << " not found in configure file.");
			return false;
		}
		value = it->second;
		return true;
	}
} /* namespace ltfs_config */
