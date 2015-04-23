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
 * FileOperationDelay.h
 *
 *  Created on: Apr 24, 2012
 *      Author: More Zeng
 */

#pragma once


#include "FileOperation.h"


namespace bdt
{

    class FileOperationDelay : public FileOperation
    {
    public:
        FileOperationDelay(
                const fs::path & path,
                mode_t mode,
                int flags,
                int delayFirst,
                int delayRead,
                int delayWrite);

        FileOperationDelay(
                const fs::path & path,
                int flags,
                int delayFirst,
                int delayRead,
                int delayWrite);

        virtual
        ~FileOperationDelay();

        bool
        Read(off_t offset, void * buffer, size_t bufsize, size_t & size);

        bool
        Write(off_t offset, const void * buffer, size_t bufsize, size_t & size);

    private:
        int delayFirst_;
        int delayRead_;
        int delayWrite_;
    };

}

