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
 * TapeManagerEntity.h
 *
 *  Created on: Apr 25, 2013
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class TapeManagerEntity : public TapeManagerInterface
    {
    public:
        TapeManagerEntity(TapeManagerInterface * entity)
        : entity_(entity)
        {
        }

        virtual
        ~TapeManagerEntity()
        {
        }

        virtual enum TapeStatus
        GetTapeStatus( const string & tape )
        {
            return entity_->GetTapeStatus(tape);
        }

        virtual bool
        SetTapeStatus( const string & tape, enum TapeStatus status )
        {
            return entity_->SetTapeStatus(tape,status);
        }

        virtual bool
        GetTapesUse( vector<string> & tapes, const fs::path & path, off_t size )
        {
            return entity_->GetTapesUse(tapes,path,size);
        }

        virtual bool
        GetShareAvailableTapes(const string& uuid, vector<map<string, off_t> >& tapesList)
        {
        	return entity_->GetShareAvailableTapes(uuid, tapesList);
        }

        virtual bool
        SetTapesUse(
                const vector<string> & tapes,
                int fileNumber,
                off_t fileSize,
                off_t tapeSize )
        {
            return entity_->SetTapesUse(tapes,fileNumber,fileSize,tapeSize);
        }

        /*virtual bool
        GetCapacity(
                const string & service,
                size_t & fileNumber,
                off_t & usedSize,
                off_t & freeSize )
        {
            return entity_->GetCapacity(service,fileNumber,usedSize,freeSize);
        }*/

        virtual fs::path
        GetPath( const string & tape, const fs::path & path )
        {
            return entity_->GetPath(tape,path);
        }

        virtual bool
        GetDriveNum(int& driveNum)
        {
        	return entity_->GetDriveNum(driveNum);
        }
        virtual bool GetTapeActivity(const string& barcode, int& act, int& percentage)
        {
        	return entity_->GetTapeActivity(barcode, act, percentage);
        }

        virtual bool
        CheckTapeState( const string & tape, enum TapeState state )
        {
            return entity_->CheckTapeState(tape,state);
        }

        virtual bool
        SetTapeStateNoLock( const string & tape, enum TapeState state )
        {
            return entity_->SetTapeStateNoLock(tape,state);
        }

        virtual bool
        LockTapeState( const string & tape )
        {
            return entity_->LockTapeState(tape);
        }

        virtual bool
        SetTapeState( const string & tape, enum TapeState state )
        {
            return entity_->SetTapeState(tape,state);
        }

        virtual bool
        SetTapeAction( const string & tape, enum TapeAction action )
        {
            return entity_->SetTapeAction(tape,action);
        }

    protected:
        auto_ptr<TapeManagerInterface> entity_;

    };

}

