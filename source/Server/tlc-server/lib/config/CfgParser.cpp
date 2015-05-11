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
 * CfgParser.cpp
 *
 *  Created on: Nov 5, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "CfgParser.h"

namespace ltfs_config
{

	CfgParser::CfgParser(const char* filename)
	{
		// TODO Auto-generated constructor stub
		try
		{
			read_xml(filename, root_);
		}
		catch(std::exception& e)
		{
			CfgError("Exception occured! "<<e.what());
		}
	}

	CfgParser::~CfgParser()
	{
		// TODO Auto-generated destructor stub
	}

	bool
	CfgParser::ParserSize(string& str, size_t& size)
	{
		try
		{
			size = 0;

			if(str.length() > 1)
			{
				string strUnit = str.substr(str.length()-1);
				string strSize = str.substr(0, str.length()-1);

				if(strUnit == "K" || strUnit == "k")
				{
					size = (size_t)boost::lexical_cast<double>(strSize)*0x400;
				}
				else if(strUnit == "M" || strUnit == "m")
				{
						size = (size_t)boost::lexical_cast<double>(strSize)*0x100000;
				}
				else if(strUnit == "G" || strUnit == "g")
				{
						size = (size_t)boost::lexical_cast<double>(strSize)*0x40000000;
				}
				else if(strUnit == "T" || strUnit == "t")
				{
						size = (size_t)boost::lexical_cast<double>(strSize)*0x10000000000;
				}
				else
					size = (size_t)boost::lexical_cast<double>(str);
			}
			else if(str.length()==1)
			{
				size = (size_t)boost::lexical_cast<double>(str);
			}
			return true;
		}catch(std::exception& e)
		{
			CfgError("ParserSize exception occured!  " << e.what());
		}

		return false;
	}

	bool
	CfgParser::GetChildValue(const string& child, string& value)
	{
		try
		{
			CfgDebug("GetChildValue  child  "<<child);

			boost::property_tree::ptree tree = root_.get_child(child);

			value = tree.data();

			CfgDebug("GetChildValue  value  "<<value);
		}
		catch(std::exception& e)
		{
			CfgWarn("GetChildValue exception occured!  "<<e.what());
			return false;
		}

		return true;
	}

} /* namespace ltfs_config */
