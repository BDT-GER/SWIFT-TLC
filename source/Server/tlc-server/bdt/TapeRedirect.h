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
 * TapeRedirect.h
 *
 *  Created on: Mar 6, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{
    class TapeRedirect
    {
    public:
        bool
        AppendRedirect(const string & tape,int mode,const string & setting = "")
        {
            if ( ! IsModeValid(mode) ) {
                return false;
            }

            if ( MODE_DEFAULT == mode ) {
                tapeDefault_ = tape;
                return true;
            }

            TapeRedirectItem item;
            item.Tape = tape;
            item.Mode = mode;
            item.Setting = setting;
            items_.push_back(item);
            return true;
        }


        void
        Clear()
        {
            items_.clear();
        }


        string
        GetTape(const fs::path & path)
        {
            string name = path.string();
            for( size_t i = 0; i < items_.size(); ++i ) {
                string setting = items_[i].Setting;
                int mode = items_[i].Mode;
                string tape = items_[i].Tape;
                switch (mode) {
                case MODE_BEGIN_WITH: {
                    if ( name.substr(0,setting.size()) == setting ) {
                        return tape;
                    }
                    break;
                }
                case MODE_END_WITH: {
                    if ( name.size() > setting.size()
                            && name.substr(
                                    name.size() - setting.size(),
                                    setting.size() ) == setting ) {
                        return tape;
                    }
                    break;
                }
                case MODE_CONTAIN: {
                    if ( name.find(setting) != string::npos ) {
                        return tape;
                    }
                    break;
                }
                }
            }
            return tapeDefault_;
        }


        enum {
            MODE_BEGIN_WITH = 0,
            MODE_END_WITH,
            MODE_CONTAIN,
            MODE_DEFAULT,
        };


    private:
        typedef struct {
            string Tape;
            int Mode;
            string Setting;
        } TapeRedirectItem;
        vector<TapeRedirectItem> items_;

        string tapeDefault_;


        bool
        IsModeValid(int mode)
        {
            if ( mode >= MODE_BEGIN_WITH && mode <= MODE_DEFAULT ) {
                return true;
            } else {
                return false;
            }
        }
    };

}

