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
 * TapeManagerInterface.h
 *
 *  Created on: Oct 22, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class TapeManagerInterface
    {
    public:
        TapeManagerInterface()
        {
        }

        virtual
        ~TapeManagerInterface()
        {
        }


        enum TapeStatus {
            STATUS_UNKNOWN = 0,
            STATUS_MISSING,
            STATUS_EXPORT,
            STATUS_CORRUPT,
            STATUS_ONLINE,
            STATUS_OFFLINE,
            STATUS_IMPORT,
            STATUS_FULL,
        };

        virtual enum TapeStatus
        GetTapeStatus( const string & tape ) = 0;

        virtual bool
        SetTapeStatus( const string & tape, enum TapeStatus status ) = 0;


        virtual bool
        GetTapesUse(vector<string> & tapes,const fs::path & path,off_t size)
        = 0;

        virtual bool
        GetShareAvailableTapes(const string& uuid, vector<map<string, off_t> >& tapesList) = 0;
        virtual bool GetDriveNum(int& driveNum) = 0;
        virtual bool GetTapeActivity(const string& barcode, int& act, int& percentage) = 0;

        virtual bool
        SetTapesUse(
                const vector<string> & tapes,
                int fileNumber,
                off_t fileSize,
                off_t tapeSize ) = 0;

        bool
        GetTapeUse( string & tape, const fs::path & path, off_t size )
        {
            vector<string> tapes;
            bool ret = GetTapesUse(tapes,path,size);
            if ( tapes.empty() ) {
                tape.clear();
            } else {
                tape = tapes[0];
            }
            return ret;
        }

        bool
        SetTapeUse(
                const string & tape,
                int fileNumber,
                off_t fileSize,
                off_t tapeSize )
        {
            vector<string> tapes;
            tapes.push_back(tape);
            return SetTapesUse(tapes,fileNumber,fileSize,tapeSize);
        }


        /*virtual bool
        GetCapacity(
                const string & service,
                size_t & fileNumber,
                off_t & usedSize,
                off_t & freeSize ) = 0;*/



        virtual fs::path
        GetPath( const string & tape, const fs::path & path ) = 0;


        enum TapeState {
            STATE_IDLE = 0,
            STATE_READ,
            STATE_WRITE,
        };

        virtual bool
        CheckTapeState( const string & tape, enum TapeState state )
        {
            if ( STATE_IDLE == state ) {
                return true;
            }

            enum TapeStatus status = GetTapeStatus(tape);

            if ( STATE_READ == state ) {
                switch ( status ) {
                case STATUS_ONLINE:
                case STATUS_OFFLINE:
                case STATUS_CORRUPT:
                    return true;
                default:
                    return false;
                }
            }

            if ( STATE_WRITE == state ) {
                switch ( status ) {
                case STATUS_ONLINE:
                case STATUS_OFFLINE:
                    return true;
                default:
                    return false;
                }
            }

            return false;
        }

        virtual bool
        SetTapeStateNoLock( const string & tape, enum TapeState state )
        {
            return CheckTapeState(tape,state);
        }

        virtual bool
        LockTapeState( const string & tape )
        {
            return true;
        }

        virtual bool
        SetTapeState( const string & tape, enum TapeState state )
        {
            if ( LockTapeState(tape) ) {
                return SetTapeStateNoLock(tape,state);
            } else {
                return false;
            }
        }


        enum TapeAction {
            ACTION_IDLE = 0,
            ACTION_READ,
            ACTION_WRITE,
        };

        virtual bool
        SetTapeAction( const string & tape, enum TapeAction action )
        {
            return true;
        }

    };


    class TapeStateSwitch
    {
    public:
        TapeStateSwitch(
                TapeManagerInterface * tapeManager,
                TapeManagerInterface::TapeState state )
        : tapeManager_(tapeManager), state_(state)
        {
        }

        void
        AddTape(const string & tape)
        {
            tapes_.insert(tape);
        }

        void
        AddTapes(const vector<string> & tapes)
        {
            BOOST_FOREACH( const string & tape, tapes ) {
                tapes_.insert(tape);
            }
        }

        ~TapeStateSwitch()
        {
            BOOST_FOREACH( const string & tape, tapes_ ) {
                if ( ! tapeManager_->SetTapeState(tape,state_) ) {
                    LogError(tape << " " << state_);
                }
            }
        }

    private:
        TapeManagerInterface * tapeManager_;
        TapeManagerInterface::TapeState state_;
        set<string> tapes_;
    };


    class TapeActionSwitch
    {
    public:
        TapeActionSwitch(
                TapeManagerInterface * tapeManager,
                TapeManagerInterface::TapeAction action )
        : tapeManager_(tapeManager), action_(action)
        {
        }

        void
        AddTape(const string & tape)
        {
            tapes_.insert(tape);
        }

        void
        AddTapes(const vector<string> & tapes)
        {
            BOOST_FOREACH( const string & tape, tapes ) {
                tapes_.insert(tape);
            }
        }

        ~TapeActionSwitch()
        {
            BOOST_FOREACH( const string & tape, tapes_ ) {
                if ( ! tapeManager_->SetTapeAction(tape,action_) ) {
                    LogError(tape << " " << action_);
                }
            }
        }

    private:
        TapeManagerInterface * tapeManager_;
        TapeManagerInterface::TapeAction action_;
        set<string> tapes_;
    };

}

