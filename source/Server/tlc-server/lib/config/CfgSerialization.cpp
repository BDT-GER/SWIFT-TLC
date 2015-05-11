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
 * CfgSerialization.cpp
 *
 *  Created on: Nov 5, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "CfgSerialization.h"

namespace ltfs_config
{

	CfgSerialization::CfgSerialization(const char* filename)
	{
		filename_ 	 = filename;
		// TODO Auto-generated constructor stub
	}

	CfgSerialization::~CfgSerialization()
	{
		// TODO Auto-generated destructor stub
	}

	bool
	CfgSerialization::SetChild(const string& tag, const string& value)
	{
		try
		{
			boost::property_tree::ptree root;
			read_xml(filename_, root, boost::property_tree::xml_parser::trim_whitespace);

			root.put(tag, value);

			boost::property_tree::xml_writer_settings<char> settings('\t', 1, "utf8");
			write_xml(filename_,  root, std::locale(), settings);

			return true;
		}
		catch(std::exception& e)
		{
			CfgError("SetChild exception occured!  "<<e.what());
			return false;
		}

		return false;
	}

	bool
	CfgSerialization::DeleteNode(const string& parentNodeName, const string& nodeName)
	{
		try
		{
			boost::property_tree::ptree root;
			read_xml(filename_, root, boost::property_tree::xml_parser::trim_whitespace);

			//delete
			boost::property_tree::ptree &pTree = root.get_child(parentNodeName);
			pTree.erase(nodeName);

			boost::property_tree::xml_writer_settings<char> settings('\t', 1, "utf8");
			write_xml(filename_,  root, std::locale(), settings);
			return true;
		}
		catch(std::exception& e)
		{
			CfgError("Delete exception occured!  "<<e.what());
		}

		return false;
	}
} /* namespace ltfs_config */
