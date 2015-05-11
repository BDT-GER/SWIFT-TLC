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
 * ChangerImpl.h
 *
 *  Created on: Aug 3, 2012
 *      Author: More Zeng
 */

#pragma once


namespace tape
{

    class ChangerImpl
    {
    public:
        ChangerImpl(int changer)
        : changer_(changer),
          numberDrive_(0), numberSlot_(0), numberCartridge_(0)
        {
        }

        virtual
        ~ChangerImpl()
        {
        }

        int NumberDrive()
        {
            return numberDrive_;
        }

        void SetNumberDrive(int numberDrive)
        {
            assert( numberDrive == (numberDrive_ + 1) );
            numberDrive_ = numberDrive;
        }

        int NumberSlot()
        {
            return numberSlot_;
        }

        void SetNumberSlot(int numberSlot)
        {
            assert( numberSlot == (numberSlot_ + 1) );
            numberSlot_ = numberSlot;
        }

        int NumberCartridge()
        {
            return numberCartridge_;
        }

        void SetNumberCartridge(int numberCartridge)
        {
            assert( numberCartridge == (numberCartridge_ + 1) );
            numberCartridge_ = numberCartridge;
        }

    private:
        int changer_;
        int numberDrive_;
        int numberSlot_;
        int numberCartridge_;
    };

}

