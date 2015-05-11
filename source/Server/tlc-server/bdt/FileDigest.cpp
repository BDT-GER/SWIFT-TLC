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
 * FileDigest.cpp
 *
 *  Created on: Sep 26, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "FileDigest.h"


namespace bdt
{

    FileDigest::FileDigest()
    : offset_(0), size_(0), error_(false), finish_(false)
    {
        Configure * configure = Factory::GetConfigure();
        try {
            if ( configure->GetValueBool(Configure::DigestMD5Enable) ) {
                EnableDigest(DIGEST_MD5);
            }
        } catch(const std::exception & e) {
            LogError(e.what());
        }
        try {
            if ( configure->GetValueBool(Configure::DigestSHA1Enable) ) {
                EnableDigest(DIGEST_SHA1);
            }
        } catch(const std::exception & e) {
            LogError(e.what());
        }
    }


    FileDigest::~FileDigest()
    {
        for_each(
                engine_.begin(),
                engine_.end(),
                boost::bind(
                        &EVP_MD_CTX_destroy,
                        boost::bind(&EngineMap::value_type::second,_1) ) );
    }


    bool
    FileDigest::EnableDigest(int digest)
    {
        if ( digest < DIGEST_MIN || digest > DIGEST_MAX ) {
            return false;
        }

        EngineMap::iterator i = engine_.find(digest);
        if ( i == engine_.end() ) {
            if ( offset_ != 0 ) {
                return false;
            }

            EVP_MD * md = NULL;
            switch(digest) {
            case DIGEST_MD5:
                md = const_cast<EVP_MD *>(EVP_md5());
                break;
            case DIGEST_SHA1:
                md = const_cast<EVP_MD *>(EVP_sha1());
                break;
            default:
                break;
            }
            if ( NULL == md ) {
                return false;
            }

            EVP_MD_CTX * ctx = EVP_MD_CTX_create();
            if ( NULL == ctx ) {
                return false;
            }
            if ( 0 == EVP_DigestInit_ex(ctx,md,NULL) ) {
                EVP_MD_CTX_destroy(ctx);
                return false;
            }
            engine_.insert(EngineMap::value_type(digest,ctx));
            return true;
        } else {
            return true;
        }
    }


    bool
    FileDigest::FillPad(off_t size)
    {
        if ( size > offset_ ) {
            char pad[4096];
            memset(pad,0,sizeof(pad));
            do {
                size_t padlen = min(sizeof(pad),(size_t)(size - offset_));
                for_each(
                        engine_.begin(),
                        engine_.end(),
                        boost::bind(
                            &EVP_DigestUpdate,
                            boost::bind(&EngineMap::value_type::second,_1),
                            pad,
                            padlen) );
                offset_ += padlen;
            } while ( size > offset_ );
            return true;
        } else {
            return false;
        }
    }


    bool
    FileDigest::UpdateContent(off_t offset,const void * buffer,size_t length)
    {
        if ( error_ || finish_ ) {
            error_ = true;
            return false;
        }

        if ( offset > offset_ ) {
            FillPad(offset);
        }

        if ( offset == offset_ ) {
            offset_ += length;
            for_each(
                    engine_.begin(),
                    engine_.end(),
                    boost::bind(
                        &EVP_DigestUpdate,
                        boost::bind(&EngineMap::value_type::second,_1),
                        buffer,
                        length) );
            return true;
        }

        error_ = true;
        return false;
    }


    bool
    FileDigest::UpdateSize(off_t size)
    {
        if ( error_ || finish_ ) {
            error_ = true;
            return false;
        }

        if ( size >= offset_ ) {
            size_ = size;
            return true;
        } else {
            error_ = true;
            return false;
        }
    }


    static string
    HexOutput(unsigned char c)
    {
        char first = c / 16;
        char second = c % 16;
        if ( first <= 9 ) {
            first += '0';
        } else {
            first += ('a' - 10);
        }
        if ( second <= 9 ) {
            second += '0';
        } else {
            second += ('a' - 10);
        }
        string output = "00";
        output[0] = first;
        output[1] = second;
        return output;
    }


    bool
    FileDigest::GetDigest(int digest,string & checksum)
    {
        checksum.clear();

        if ( error_ ) {
            return false;
        }

        if ( size_ > offset_ ) {
            FillPad(size_);
        }

        DigestMap::iterator iterDigest = digest_.find(digest);
        if ( iterDigest != digest_.end() ) {
            checksum = iterDigest->second;
            return true;
        }

        EngineMap::iterator iterEngine = engine_.find(digest);
        if ( iterEngine == engine_.end() ) {
            return false;
        }

        unsigned char digestValue[EVP_MAX_MD_SIZE];
        unsigned int digestSize;
        EVP_DigestFinal_ex(iterEngine->second,digestValue,&digestSize);
        EVP_MD_CTX_destroy(iterEngine->second);
        engine_.erase(iterEngine);

        for ( unsigned int i = 0; i < digestSize; ++i ) {
            checksum = checksum + HexOutput(digestValue[i]);
        }
        digest_.insert(DigestMap::value_type(digest,checksum));
        finish_ = true;

        return true;
    }

}

