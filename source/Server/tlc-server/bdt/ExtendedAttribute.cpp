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
 * ExtendedAttribute.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"

#include <attr/xattr.h>
#include <boost/regex.hpp>


namespace bdt
{

    ExtendedAttribute::ExtendedAttribute(const fs::path & path)
    : path_(path), handle_(-1)
    {
        if ( ! fs::exists(path_) ) {
            LogError(path_);
        }
    }


    ExtendedAttribute::ExtendedAttribute(int handle)
    : handle_(handle)
    {
    }


    ExtendedAttribute::~ExtendedAttribute()
    {
        if ( handle_ >= 0 ) {
            ::close(handle_);
        }
    }


    fs::path
    ExtendedAttribute::GetPath()
    {
        if ( handle_ >= 0 ) {
            char proc[PATH_MAX];
            sprintf(proc, "/proc/self/fd/%d", handle_);
            char path[PATH_MAX+1];
            ssize_t size = sizeof(path);
            size = readlink(proc, path, sizeof(path));
            if ( size > 0 ) {
                path[size] = '\0';
                return path;
            }
        }
        return path_;
    }


    bool
    ExtendedAttribute::GetNameList(vector<string> & names)
    {
        names.clear();

        boost::scoped_array<char> buffer;
        int bufsize = 0;
        if ( handle_ >= 0 ) {
            bufsize = ::flistxattr( handle_, NULL, 0 );
        } else {
            bufsize = ::listxattr( path_.string().c_str(), NULL, 0 );
        }
        if ( bufsize < 0 ) {
            LogError(path_);
            return false;
        }
        if ( bufsize == 0 ) {
            return true;
        }

        buffer.reset( new char[bufsize] );
        int size = 0;
        if ( handle_ >= 0 ) {
            size = ::flistxattr( handle_, buffer.get(), bufsize );
        } else {
            size = ::listxattr(
                    path_.string().c_str(), buffer.get(), bufsize );
        }
        if ( size < 0 ) {
            LogError(path_);
            return false;
        }

        int offset = 0;
        while ( offset < size ) {
            names.push_back( buffer.get() + offset );
            offset += names.back().size() + 1;
            if (names.back().substr(0,5) != "user.") {
                names.pop_back();
            }
        }

        return true;
    }


    bool
    ExtendedAttribute::GetValue(const string & name,
            void * buf,int bufsize,int & size)
    {
        if ( handle_ >= 0 ) {
            size = ::fgetxattr( handle_, name.c_str(), buf, bufsize );
        } else {
            size = ::getxattr( path_.string().c_str(),
                    name.c_str(), buf, bufsize );
        }
        return size >= 0 ? true : false;
    }


    bool
    ExtendedAttribute::GetStringValue(const string & name, string & value)
    {
		try {
			boost::scoped_array<char> buffer(new char[MAX_EA_VALUE_SIZE]());
			int size = 0;
			if (GetValue(name, buffer.get(), MAX_EA_VALUE_SIZE, size)) {
				value = buffer.get();
				value.resize(size);
                return true;
			} else {
                return false;
            }
		} catch (const std::exception & e) {
            LogError(path_ << " " << name << " " << e.what());
			return false;
		}
    }


    bool
    ExtendedAttribute::SetValue(const string & name,const void * buf,int size)
    {
        if ( handle_ >= 0 ) {
            return (0 == ::fsetxattr( handle_, name.c_str(), buf, size, 0 ) );
        } else {
            return (0 == ::setxattr( path_.string().c_str(),
                    name.c_str(), buf, size, 0 ) );
        }
    }


    bool
    ExtendedAttribute::SetStringValue(const string & name,const string & value)
    {
        return SetValue( name, value.c_str(), value.size() );
    }


    bool
    ExtendedAttribute::DeleteName(const string & name)
    {
        if ( handle_ >= 0 ) {
            return (0 == ::fremovexattr( handle_, name.c_str() ));
        } else {
            return (0 == ::removexattr(
                    path_.string().c_str(), name.c_str() ));
        }
    }

    bool
    ExtendedAttribute::MergeAttrTo(const fs::path& dstPath)
    {
    	vector<string> ignoredNames;
    	ignoredNames.clear();
    	ignoredNames.push_back(Inode::ATTRIBUTE_NUMBER);
    	ignoredNames.push_back(Inode::ATTRIBUTE_STATE);
    	ignoredNames.push_back(Inode::ATTRIBUTE_BITMAP);
    	ignoredNames.push_back(Inode::ATTRIBUTE_SIZE);
    	ignoredNames.push_back(Inode::ATTRIBUTE_MD5);
    	ignoredNames.push_back(Inode::ATTRIBUTE_SHA1);
    	ignoredNames.push_back(Inode::ATTRIBUTE_BACKUP);
    	ignoredNames.push_back(Inode::ATTRIBUTE_CORRUPTED);
    	ignoredNames.push_back(Inode::ATTRIBUTE_ONLINE);

    	return MergeAttrTo(dstPath, ignoredNames);
    }


    bool
    ExtendedAttribute::MergeAttrTo(
            const fs::path & dstPath, const vector<string>& ignoredNames)
    {
        if ( ! fs::exists(dstPath) ) {
            LogError(dstPath);
            return false;
        }
        vector<string> names;
        if(!GetNameList(names)){
        	LogError("Failed to get extended attribute name list for " << path_.string());
        	return false;
        }


        LogDebug("Merging EA from file " << path_.string() << " to " << dstPath.string());
        for(unsigned int i = 0; i < names.size(); i++){
        	string eaName = names[i];
        	bool bIgnore = false;
        	for(unsigned int i = 0; i < ignoredNames.size(); i++){
        		if(ignoredNames[i] == eaName){
        			bIgnore = true;
        			break;
        		}
        	}
        	if(bIgnore){
        		LogDebug("Ignore setting EA " << eaName << " for file " << dstPath.string());
        		continue;
        	}

        	char buf[MAX_EA_VALUE_SIZE];
        	int size = 0;
        	if(!GetValue(names[i], buf, sizeof(buf), size)){
        		LogError("Failed to get extended attribute " << names[i] << " for file " << path_.string());
        		continue;
        	}
        	LogDebug("Setting EA " << eaName << " for file " << dstPath.string());
            int ret = ::setxattr( dstPath.string().c_str(), eaName.c_str(), buf, size, 0 );
            if(ret != 0){
            	LogError("Failed to set attribute " << eaName << " to file " << dstPath.string());
            }
        }
    	return true;
    }


}

