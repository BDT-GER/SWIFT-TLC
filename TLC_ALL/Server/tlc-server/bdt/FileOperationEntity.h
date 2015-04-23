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
 * FileOperationEntity.h
 *
 *  Created on: Apr 24, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class FileOperationEntity : public FileOperationInterface
    {
    public:
        FileOperationEntity(FileOperationInterface * entity)
        : entity_(entity)
        {
        }

        virtual
        ~FileOperationEntity()
        {
        }

        virtual bool
        GetStat(struct stat & stat)
        {
            return entity_->GetStat(stat);
        }

        virtual bool
        Read(off_t offset,void * buffer,size_t bufsize,size_t & size)
        {
            return entity_->Read(offset,buffer,bufsize,size);
        }

        virtual bool
        Write(off_t offset,const void * buffer,size_t bufsize,size_t & size)
        {
            return entity_->Write(offset,buffer,bufsize,size);
        }

        virtual bool
        Sync(bool data)
        {
            return entity_->Sync(data);
        }

        virtual bool
        Truncate(off_t length)
        {
            return entity_->Truncate(length);
        }

    protected:
        auto_ptr<FileOperationInterface> entity_;
    };

}

