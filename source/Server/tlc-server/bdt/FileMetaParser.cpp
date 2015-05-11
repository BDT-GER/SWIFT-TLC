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
 * FileMetaParser.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: More Zeng
 */


#include "stdafx.h"
#include <boost/regex.hpp>
#include "FileMetaParser.h"
#include "MetaManager.h"


namespace bdt
{

    static const string SwiftKeySingle = "X-Object-Meta-Mtime";
    static const string SwiftKeyMultipleManifest = "X-Object-Manifest";
    static const string SwiftKeyName = "name";

    FileMetaParser::FileMetaParser()
    : isMultiple_(false), isManifest_(false)
    {
        Py_Initialize();
        module_ = PyImport_ImportModule("cPickle");
        function_ = PyObject_GetAttrString(module_,"loads");

    }

    FileMetaParser::~FileMetaParser()
    {
        Py_DECREF(function_);
        Py_DECREF(module_);
        Py_Finalize();
    }

    bool
    FileMetaParser::ParseSwiftMeta(const fs::path & path)
    {
        MetaManager * meta = Factory::GetMetaManager();
        auto_ptr<Inode> inode(meta->GetInode(path));
        if ( inode.get() == NULL ) {
            return false;
        }
        string swiftName = "user.swift.metadata";
        char buffer[1024];
        string swiftContent;
        for (int i=0; true; ++i) {
            string metaName = swiftName;
            if (i != 0) {
                metaName = swiftName + boost::lexical_cast<string>(i);
            }
            int size;
            if (! inode->GetExtendedAttribute(
                    metaName, buffer, sizeof(buffer) - 1, size )) {
                break;
            }
            buffer[size] = '\0';
            swiftContent += buffer;
        }
        try {
            return ParseSwift(swiftContent);
        } catch ( const std::exception & e ) {
            LogWarn("Un-expected exception to parse inode swift meta: "
                    <<  e.what());
            return false;
        }
    }

    bool
    FileMetaParser::ParseSwift(string meta)
    {
        static const boost::regex namePattern(".*/(\\d+[[.period.]]\\d+)/(\\d+)/(\\d+)/(\\d+)$");
        static const boost::regex manifestPattern(".*/(\\d+[[.period.]]\\d+)/(\\d+)/(\\d+)/$");

        PyObject * content = PyString_FromString(meta.c_str());
        PyObject * args = PyTuple_New(1);
        PyTuple_SetItem(args, 0, content);
        PyObject * result = PyObject_CallObject(function_,args);
        Py_DECREF(args);

        if ( result == NULL ) {
            LogWarn("Cannot parse meta: " << meta);
            return false;
        }

        PyObject * valueName = PyDict_GetItemString(result, SwiftKeyName.c_str());
        if ( valueName == NULL ) {
            return false;
        } else {
            name_ = PyString_AsString(valueName);
        }

        PyObject * valueSingle = PyDict_GetItemString(result, SwiftKeySingle.c_str());
        if ( valueSingle != NULL ) {
            isMultiple_ = false;
            isManifest_ = false;
            number_ = -1;
            total_ = 1;
        } else {
            isMultiple_ = true;
            isManifest_ = false;
            boost::smatch what;
            if ( boost::regex_match(
                    name_, what, namePattern, boost::match_extra ) ) {
                manifest_ = "/" + what[1] + "/" + what[2] + "/" + what[3] + "/";
                number_ = boost::lexical_cast<int>(what[4]);
                long long totalSize = boost::lexical_cast<long long>(what[2]);
                long long segmentSize = boost::lexical_cast<long long>(what[3]);
                total_ = (totalSize + segmentSize - 1) / segmentSize;
            } else {
                number_ = -1;
                total_ = 1;
                LogWarn("Un-expected name " << name_);
            }
        }

        PyObject * valueManifest = PyDict_GetItemString(result, SwiftKeyMultipleManifest.c_str());
        if ( valueManifest != NULL ) {
            isMultiple_ = true;
            isManifest_ = true;
            manifest_ = PyString_AsString(valueManifest);
            boost::smatch what;
            if ( boost::regex_match(
                    manifest_, what, manifestPattern, boost::match_extra ) ) {
                manifest_ = "/" + what[1] + "/" + what[2] + "/" + what[3] + "/";
                long long totalSize = boost::lexical_cast<long long>(what[2]);
                long long segmentSize = boost::lexical_cast<long long>(what[3]);
                total_ = (totalSize + segmentSize - 1) / segmentSize;
            } else {
                manifest_ = "";
                LogWarn("Un-expected manifest " << manifest_);
            }
        } else {
            manifest_ = "";
        }

        Py_DECREF(result);

        return true;
    }

}
