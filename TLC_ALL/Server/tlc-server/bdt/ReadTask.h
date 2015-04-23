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
 * ReadTask.h
 *
 *  Created on: Dec 01, 2014
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class CacheManager;
    class FileOperationBitmap;


    class ReadTaskCallback
    {
    public:
        virtual void
        FinishReadTask(const unsigned long long number) = 0;
    };


    class ReadTask
    {
    public:
        ReadTask(
                const unsigned long long number,
                FileOperationInterface * source,
                FileOperationBitmap * file,
                ReadTaskCallback * callback);

        ~ReadTask();

        bool
        Prepare(
                off_t offset,
                size_t size);

        bool
        Truncate(off_t length);

        bool
        IsValid();

        bool
        IsRunning();

    private:
        unsigned long long number_;
        CacheManager * cache_;
        auto_ptr<FileOperationInterface> source_;
        auto_ptr<FileOperationBitmap> file_;

        boost::mutex mutex_;
        boost::condition_variable condition_;

        bool running_;
        bool success_;
        auto_ptr<boost::thread> thread_;
        vector<off_t> queueWait_;
        off_t current_;
        int errno_;

        bool
        CanWriteCache();

        void
        BackendTask();

        ReadTaskCallback * callback_;
    };

}
