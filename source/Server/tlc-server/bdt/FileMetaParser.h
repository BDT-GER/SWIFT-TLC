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
 * InodeHandler.h
 *
 *  Created on: Nov 5, 2014
 *      Author: More Zeng
 */


#pragma once


#include <Python.h>


namespace bdt
{

    class FileMetaParser
    {
    public:
        FileMetaParser();

        ~FileMetaParser();

        bool ParseSwiftMeta(const fs::path & path);

        string GetName()
        {
            return name_;
        }

        string GetManifest()
        {
            return manifest_;
        }

        int GetNumber()
        {
            return number_;
        }

        int GetTotal()
        {
            return total_;
        }

        bool IsMultiple()
        {
            return isMultiple_;
        }

        bool IsManifest()
        {
            return isManifest_;
        }

    private:
        PyObject * module_;
        PyObject * function_;

        string name_;
        string manifest_;
        bool isMultiple_;
        bool isManifest_;
        int number_;
        int total_;

        bool ParseSwift(string meta);
    };

}

