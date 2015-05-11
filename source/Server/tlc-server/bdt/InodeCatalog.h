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
 * InodeCatalog.h
 *
 *  Created on: Nov 23, 2012
 *      Author: More Zeng
 */


#pragma once


#include "FileOperation.h"


namespace bdt
{

    class InodeCatalog : public InodeInterface
    {
    public:
        InodeCatalog(
                Inode * inode,
                const fs::path & pathCatalog,
                const fs::path & path )
        : inode_(inode), pathCatalog_(pathCatalog), path_(path)
        {
            assert( NULL != inode );
        }

        virtual
        ~InodeCatalog()
        {
        }

        static const string ATTRIBUTE_SERVICE;

        bool
        GetSize(off_t & size)
        {
            LogError(path_);
            assert(false);
            size = -1;
            return false;
        }

        static bool
        CheckService(const fs::path & path)
        {
            string service = Factory::GetService();

            auto_ptr<ExtendedAttribute> ea(new ExtendedAttribute(path));
            char buffer[1024];
            int buffersize;
            if ( ea->GetValue(
                    ATTRIBUTE_SERVICE, buffer, sizeof(buffer),buffersize) ) {
                buffer[buffersize] = '\0';
                return string(buffer) == service;
            } else {
                return false;
            }
        }

        static bool
        ResetService(const fs::path & path)
        {
            string service = Factory::GetService();

            auto_ptr<ExtendedAttribute> ea(new ExtendedAttribute(path));

            return ea->SetValue(
                    ATTRIBUTE_SERVICE, service.c_str(), service.size() );
        }

        bool
        SetSize(off_t size)
        {
            LogDebug(path_ << " : " << size);

            bool ret = true;
            if ( NULL != interface_.get() ) {
                ret = interface_->SetSize(size);
            }

            vector<string> tapes;
            if ( (! inode_->GetTapes(tapes)) || tapes.empty() ) {
                LogError(pathCatalog_ / path_);
                return false;
            }

            struct stat stat;
            if ( ! inode_->GetStat(stat) ) {
                LogError(pathCatalog_ / path_);
                return false;
            }

            BOOST_FOREACH( const string & tape, tapes ) {
                fs::path pathname = pathCatalog_ / tape / path_;
                if ( ! fs::exists(pathname) ) {
                    FileOperation file(pathname);
                    if ( ! file.CreateFile(O_RDWR,stat.st_mode,true) ) {
                        LogError(pathname);
                        return false;
                    }
                }

                if ( ! CheckService(pathCatalog_ / tape) ) {
                    LogError(tape);
                    if ( ! ResetService(pathCatalog_ / tape) ) {
                        LogError(tape);
                    }
                }

                if ( 0 != ::truncate(pathname.string().c_str(),size) ) {
                    LogError(pathname << " " << errno);
                    return false;
                }

                struct ::utimbuf times;
                times.actime = time(NULL);
                times.modtime = stat.st_mtime;
                if ( 0 != utime(pathname.string().c_str(),&times) ) {
                    LogError(pathname);
                }

                off_t offset = 0;
                if ( inode_->GetOffset(offset) ) {
                    ExtendedAttribute ea(pathname);
                    if (! ea.SetValue(
                            Inode::ATTRIBUTE_OFFSET,&offset,sizeof(offset))) {
                        LogError(pathname);
                    }
                } else {
                    LogDebug(pathname);
                }
            }

            return ret;
        }

        bool
        GetStat(struct stat & stat)
        {
            LogError(path_);
            assert(false);
            return false;
        }

    private:
        Inode * inode_;
        fs::path pathCatalog_;
        fs::path path_;
    };

}

