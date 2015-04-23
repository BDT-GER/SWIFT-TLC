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
 *should be tghe
 * FileCompare.h
 *
 *  Created on: Jan 16, 2013
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class FileCompare
    {
    public:
        FileCompare(int compare,int direction,const fs::path & prefix = "")
        : compare_(compare), direction_(direction), prefix_(prefix)
        {
        }

        ~FileCompare()
        {
        }

        enum {
//            COMPARE_PATHNAME = 1,
//            COMPARE_FOLDER_PATHNAME = 2,
            COMPARE_TIME_ACCESS = 3,
            COMPARE_TIME_MODIFY = 4,
//            COMPARE_TIME_CHANGE = 5,
        };

        enum {
            DIRECTION_LESS = 1,
            DIRECTION_LESS_EQUAL = 2,
            DIRECTION_GREATER = 3,
            DIRECTION_GREATER_EQUAL = 4,
        };

        bool
        CompareTime(bool exist1,bool exist2,time_t time1,time_t time2)
        {
            if ( ! exist1 ) {
                time1 = 1;
                time2 = 0;
                if ( ! exist2 ) {
                    time2 = 1;
                }
            } else if ( ! exist2 ) {
                time1 = 0;
                time2 = 1;
            }

            switch ( direction_ ) {
            case DIRECTION_LESS:
                return time1 < time2;
            case DIRECTION_LESS_EQUAL:
                return time1 <= time2;
            case DIRECTION_GREATER:
                return time1 > time2;
            case DIRECTION_GREATER_EQUAL:
                return time1 >= time2;
            default:
                assert(false);
            }

            return false;
        }

        bool
        operator() (const fs::path & path1, const fs::path & path2)
        {
            bool exist1 = true, exist2 = true;
            struct stat stat1, stat2;
            fs::path pathname1 = prefix_ / path1;
            if ( 0 != stat(pathname1.string().c_str(),&stat1) ) {
                exist1 = false;
            }
            fs::path pathname2 = prefix_ / path2;
            if ( 0 != stat(pathname2.string().c_str(),&stat2) ) {
                exist2 = false;
            }

            switch ( compare_ ) {
            case COMPARE_TIME_ACCESS:
                return CompareTime(
                        exist1, exist2, stat1.st_atime, stat2.st_atime );
            case COMPARE_TIME_MODIFY:
                return CompareTime(
                        exist1, exist2, stat1.st_mtime, stat2.st_mtime );
            default:
                assert(false);
                break;
            }

            return false;
        }

    private:
        int compare_;
        int direction_;
        fs::path prefix_;
    };

}

