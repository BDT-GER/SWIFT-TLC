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
 * ScheduleTape.h
 *
 *  Created on: May 18, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class ScheduleTape : public ScheduleInterface
    {
    public:
        ScheduleTape(int delayElevator,int delayFile,int delayTape);

        virtual
        ~ScheduleTape();

        bool
        Request(const fs::path & path,off_t offset,size_t size,
                int timeout,int priority);

        bool
        Request(const fs::path & path,bool share,int timeout,int priority);

        void
        Release(const fs::path & path,bool share);

        bool
        GetTape(const fs::path & path, string & tape)
        {
            string pathname = path.string();
            if ( pathname.size() <= (folder_.size() + 3/* len("/tape/") */) ) {
                return false;
            }
            //if ( fs::slash<char>::value != pathname[folder_.size()] ) {
            if ( '/' != pathname[folder_.size()] ) {
                return false;
            }
            if ( 0 != pathname.find(folder_) ) {
                return false;
            }

            pathname = pathname.substr(folder_.size()+1);
            //string::size_type pos = pathname.find(fs::slash<char>::value);
            string::size_type pos = pathname.find('/');
            if ( string::npos == pos ) {
                return false;
            } else {
                tape = pathname.substr(0,pos);
                return true;
            }
        }

    private:
        string folder_;
        int delayElevator_;
        int delayPath_;
        int delayTape_;

        string currentTape_;
        boost::posix_time::ptime timeTape_;
        int referenceTape_;

        fs::path currentPath_;
        off_t offsetPath_;
        boost::posix_time::ptime timePath_;
        bool busyPath_;

        boost::mutex mutex_;
        boost::condition_variable condition_;

    };

}

