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
 * CfgfileParser.h
 *
 *  Created on: Oct 10, 2012
 *      Author: chento
 */

#pragma once
#include "stdafx.h"

const string CFG_FILE = COMM_APP_CONF_PATH + "/simulator.xml";


namespace ltfs_management
{
	struct CfgDetail
	{
		size_t 						m_SizeReadErr;
		size_t 						m_SizeWriteErr;
		int 						m_NumReadErr;
		int 						m_NumWriteErr;
		int 						m_ReadDelay;
		int 						m_WriteDelay;
		int 						m_MoveDelay;
		int 						m_MoveFailure;

		int							m_MountDelay;
		int							m_MountError;
		int							m_UmontDelay;
		int							m_UmontError;
		int							m_FormatDelay;
		int							m_FormatError;
	};

	class CfgfileParser
	{
	public:
		CfgfileParser(const char* filename);

		virtual
		~CfgfileParser();

		bool
		ParserElement(struct CfgDetail& detail);

		bool
		ParserChangers(vector<SimChanger> &changers);

	private:
		bool
		ParserDrives(boost::property_tree::ptree & changerTree, SimChanger& changer);

		bool
		ParserTapes(boost::property_tree::ptree & changerTree, SimChanger& changer);

		bool
		ParserSlots(boost::property_tree::ptree & changerTree, SimChanger& changer);

		bool
		ParserMailSlots(boost::property_tree::ptree & changerTree, SimChanger& changer);

		bool
		ParserSize(string& str, size_t& size);

		void
		AutoMakeTapeDir(string& barcode);

	private:
		boost::property_tree::ptree	root_;
	};

} /* namespace ltfs_management */

