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
 * FileDigest.h
 *
 *  Created on: Sep 26, 2012
 *      Author: More Zeng
 */


#pragma once

#include <openssl/evp.h>


namespace bdt
{

    class FileDigest
    {
    public:
        FileDigest();

        ~FileDigest();

        enum {
            DIGEST_MIN  = 1,
            DIGEST_MD5  = 1,
            DIGEST_SHA1 = 2,
            DIGEST_MAX  = 2,
        };

        bool
        EnableDigest(int digest);

        bool
        UpdateContent(off_t offset,const void * buffer,size_t length);

        bool
        UpdateSize(off_t size);

        bool
        GetDigest(int digest,string & checksum);

        bool
        IsValid()
        {
            return ! error_;
        }

    private:
        typedef map<int,EVP_MD_CTX *> EngineMap;
        EngineMap engine_;

        typedef map<int,string> DigestMap;
        DigestMap digest_;

        off_t offset_;
        off_t size_;

        bool error_;
        bool finish_;

        bool
        FillPad(off_t size);

    };

}

