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
 * Error.h
 *
 *  Created on: Jul 2, 2012
 *      Author: More Zeng
 */

#pragma once


namespace tape
{

    class Error
    {
    public:
        Error();

        virtual
        ~Error();

        enum {
            ERROR_NO_ENTRY = 1,
            ERROR_ENTRY_FULL = 2,
            ERROR_ENTRY_EMPTY = 3,
        };

        int
        ErrorNumber()
        {
            return errorNumber_;
        }

        void
        SetError(int errorNumber,string errorMessage)
        {
            errorNumber_ = errorNumber;
            errorMessage_ = errorMessage;
        }

        string
        ErrorMessage()
        {
            return errorMessage_;
        }

    private:
        int errorNumber_;
        string errorMessage_;
    };

}
