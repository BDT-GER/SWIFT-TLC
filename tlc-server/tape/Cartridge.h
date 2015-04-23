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
 * Cartridge.h
 *
 *  Created on: Jul 2, 2012
 *      Author: More Zeng
 */

#pragma once


namespace tape
{

    class Cartridge
    {
    public:
        Cartridge(const string & setting);

        Cartridge(const Cartridge & cartridge);

        Cartridge &
        operator = (const Cartridge & cartridge)
        {
            Cartridge temp(cartridge);
            Swap(temp);
            return * this;
        }

        void
        Swap(Cartridge & cartridge);

        virtual
        ~Cartridge();

        bool
        GetBarcode(string & barcode, Error & error);

        bool
        GetSlotID(int & slotID, Error & error);

    private:
        struct Detail;
        Detail * detail_;

    };

}
