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
 * ReadManager.h
 *
 *  Created on: Dec 02, 2014
 *      Author: More Zeng
 */


#pragma once


#include "ReadTask.h"


namespace bdt
{

    class MetaDatabase;
    class FileOperationBitmap;


    struct ReadItem
    {
        string tape;
        off_t offset;
        ReadTask * task;
        bool preRead;
    };


    class ReadManager : public ReadTaskCallback
    {
    public:
        ReadManager();

        ~ReadManager();

        bool
        NeedRead(const unsigned long long number);

        bool
        TruncateRead(const unsigned long long number,off_t length);

        bool
        ReadPrepare(const unsigned long long number,off_t length = -1);

        bool
        ReadBegin(
                const unsigned long long number,
                string & tape,
                int timeout);

        bool
        CheckRead(const unsigned long long number,off_t offset,size_t size);

        bool
        CheckWrite(const unsigned long long number,off_t offset,size_t size);

        bool
        ReadEnd(const unsigned long long number);

        void
        FinishReadTask(const unsigned long long number);

    private:
        CacheManager * cache_;
        auto_ptr<MetaDatabase> database_;

        boost::mutex mutex_;
        typedef map<unsigned long long,ReadItem> ReadMap;
        ReadMap items_;

        boost::mutex mutexPreRead_;
        typedef map<string,boost::thread *> PreReadMap;
        PreReadMap preReadItems_;

        void
        StartPreRead(
                const string & tape,
                const unsigned long long number,
                off_t offset);

        void
        StopPreRead(const string & tape);

        void
        PreReadTask(
                const string & tape,
                const unsigned long long number,
                off_t offset);

        bool
        PreRead(
                const string & tape,
                const unsigned long long number,
                FileOperationBitmap * file,
                off_t offset,
                off_t offsetEnd);
    };

}

