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
 * ExtendedAttribute.h
 *
 *  Created on: Mar 1, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    // CIFS limits the EA size to 64K, LTFS limits the size to 4096
    //(from ltfs source:     #define LTFS_MAX_XATTR_SIZE           4096)
	const int MAX_EA_VALUE_SIZE = 65536;

    class ExtendedAttribute
    {
    public:
        ExtendedAttribute(const fs::path & path);

        ExtendedAttribute(int handle);

        ~ExtendedAttribute();

        bool
        GetNameList(vector<string> & names);

        bool
        GetValue(const string & name,void * buf,int bufsize,int & size);

        bool
        GetStringValue(const string & name, string & value);

        bool
        SetValue(const string & name,const void * buf,int size);

        bool
        SetStringValue(const string & name, const string & value);

        bool
        DeleteName(const string & name);

        fs::path
        GetPath();
        bool MergeAttrTo(const fs::path& dstPath);
        bool MergeAttrTo(const fs::path & dstPath, const vector<string>& ignoredNames);

    private:
        fs::path path_;
        int handle_;
    };

}

