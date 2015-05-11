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
 * Common.cpp
 *
 *  Created on: Dec 18th, 2013
 *      Author: Sam Chen
 */

#include <inttypes.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>
#include <iomanip>
#include <stdarg.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include "Common.h"


using namespace std;
string QuotaString(const string& strSrc)
{
	string strRet = "";

	for(unsigned int i = 0; i < strSrc.length(); i++){
		if( (strSrc[i] >= '0' && strSrc[i] <= '9')
			|| (strSrc[i] >= 'a' && strSrc[i] <= 'z')
			|| (strSrc[i] >= 'A' && strSrc[i] <= 'Z')
			){

			strRet += strSrc[i];

		}else{
			strRet += string("\\") + strSrc[i];
		}
	}

	return strRet;
}

string QuotaStringForSQL(const string& strSrc)
{
	string strRet = "";

	for(unsigned int i = 0; i < strSrc.length(); i++){
		if(strSrc[i] == '\'') {
			strRet += string("'") + strSrc[i];
		}else{
			strRet += strSrc[i];
		}
	}

	return strRet;
}


string GetFolderPath(const string& pathName)
{
	boost::regex matchPath("^\\s*(.*)\\/([^\\/]+)\\s*$");
	boost::cmatch match;
	if(boost::regex_match(pathName.c_str(), match, matchPath)){
		return match[1];
	}
	return "";
}

string GetFileName(const string& pathName)
{
	boost::regex matchPath("^\\s*(.*)\\/([^\\/]+)\\s*$");
	boost::cmatch match;
	if(boost::regex_match(pathName.c_str(), match, matchPath)){
		return match[2];
	}
	return "";
}

string GetTapeMountPoint(const string& barcode)
{
	return COMM_MOUNT_PATH + "/" + barcode;
}


string GetPathFromUuid(const string& uuid)
{
    unsigned long long current = boost::lexical_cast<unsigned long long>(uuid);
    string result;
    for ( int i=0; i<8; ++i ) {
        int leaf = current & 0xff;
        current = current >> 8;
        ostringstream os;
        os << setw(2) << setfill('0') << hex << leaf;
        if ( result.empty() ) {
            result = os.str();
        } else {
            result = os.str() + "/" + result;
        }
    }
    return result;
}

string GetTapeFilePath(unsigned long long number, const string& barcode)
{
	return COMM_MOUNT_PATH + "/" + barcode + "/" + GetPathFromUuid(boost::lexical_cast<string>(number));
}

