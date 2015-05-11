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
 * TapeManagerProxyServer.cpp
 *
 *  Created on: Oct 22, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "TapeManagerProxyServer.h"
#include "TapeManagerSE.h"
#ifdef MORE_TEST
#else
#include "../ltfs_management/TapeLibraryMgr.h"

using namespace ltfs_management;
#endif


namespace bdt
{

    string const TapeManagerProxyServer::Service("Tape");
    string const TapeManagerProxyServer::GetTapesUse("Tape.GetTapesUse");
    string const TapeManagerProxyServer::GetShareAvailableTapes("Tape.GetShareAvailableTapes");
    string const TapeManagerProxyServer::SetTapesUse("Tape.SetTapesUse");
    //string const TapeManagerProxyServer::GetCapacity("Tape.GetCapacity");
    string const TapeManagerProxyServer::GetTapeStatus("Tape.GetTapeStatus");
    string const TapeManagerProxyServer::SetTapeStatus("Tape.SetTapeStatus");
    string const TapeManagerProxyServer::CheckTapeState("Tape.CheckTapeState");
    string const TapeManagerProxyServer::SetTapeStateNoLock(
            "Tape.SetTapeStateNoLock");
    string const TapeManagerProxyServer::LockTapeState("Tape.LockTapeState");
    string const TapeManagerProxyServer::SetTapeState("Tape.SetTapeState");
    string const TapeManagerProxyServer::SetTapeAction("Tape.SetTapeAction");
    string const TapeManagerProxyServer::OpenMailSlot("Tape.OpenMailSlot");
    string const TapeManagerProxyServer::InventoryLibrary("Tape.InventoryLibrary");
    string const TapeManagerProxyServer::GetDriveNum("Tape.GetDriveNum");
    string const TapeManagerProxyServer::GetTapeActivity("Tape.GetTapeActivity");


    TapeManagerProxyServer::TapeManagerProxyServer()
    : SocketServer(Service),
      tape_(new TapeManagerSE())
    {
        Factory::ResetTapeManager(tape_);
    }


    TapeManagerProxyServer::~TapeManagerProxyServer()
    {
    }


    class TapeSetTapeStatusMethod : public xmlrpc_c::method
    {
        //bool SetTapeStatus(const string & tape,enum TapeStatus status);

    public:
        TapeSetTapeStatusMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "b:si";
            this->_help = "TapeManagerInterface::GetTapeStatus";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const tape(params.getString(0));
            int const status(params.getInt(1));
            bool retVal = tape_->SetTapeStatus(
                    tape, (TapeManagerInterface::TapeStatus)status );
            LogDebug(tape << " " << status << " : " << retVal);
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    private:
        TapeManagerInterface * tape_;

    };


    class TapeGetTapeStatusMethod : public xmlrpc_c::method
    {
        //enum TapeStatus GetTapeStatus( const string & tape );

    public:
        TapeGetTapeStatusMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "i:s";
            this->_help = "TapeManagerInterface::GetTapeStatus";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const tape(params.getString(0));
            LogDebug(tape);
            enum TapeManagerInterface::TapeStatus status =
                    tape_->GetTapeStatus(tape);
            LogDebug(tape << " : " << status);
            * ret = xmlrpc_c::value_int(status);
        }

    private:
        TapeManagerInterface * tape_;

    };


    class TapeGetTapesUseMethod : public xmlrpc_c::method
    {
//        bool GetTapesUse( vector<string> & tapes,
//                const fs::path & path, size_t size );

    public:
        TapeGetTapesUseMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "s:si";
            this->_help = "TapeManagerInterface::GetTapesUse";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const path(params.getString(0));
            long long const size(params.getI8(1));
            vector<string> tapes;
            if ( ! tape_->GetTapesUse(tapes,path,size) ) {
                tapes.clear();
            }
            LogDebug(path << " " << size << " : " << boost::join(tapes,","));

            vector<xmlrpc_c::value> data;
            for ( vector<string>::iterator i = tapes.begin();
                    i != tapes.end();
                    ++ i ) {
                data.push_back(xmlrpc_c::value_string(*i));
            }
            * ret = xmlrpc_c::value_array(data);
        }

    private:
        TapeManagerInterface * tape_;

    };


    class TapeGetShareAvailableTapesMethod : public xmlrpc_c::method
    {

    public:
        TapeGetShareAvailableTapesMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "s:si";
            this->_help = "TapeManagerInterface::GetShareAvailableTapes";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value_array * const ret)
        {
            string const uuid(params.getString(0));
            vector<map<string, off_t> > tapes;
            if ( ! tape_->GetShareAvailableTapes(uuid, tapes) ) {
                tapes.clear();
            }

            vector<xmlrpc_c::value> data;
            for ( vector<map<string, off_t> >::iterator i = tapes.begin(); i != tapes.end(); ++ i ) {
            	map<string, off_t> mapI = (*i);
            	xmlrpc_c::cstruct aa;
            	for(map<string, off_t>::iterator im = mapI.begin(); im != mapI.end(); im++){
					aa[im->first] = xmlrpc_c::value_i8(im->second);
            	}
                data.push_back(xmlrpc_c::value_struct(aa));
            }
            * ret = xmlrpc_c::value_array(data);
        }

    private:
        TapeManagerInterface * tape_;

    };


    class TapeSetTapesUseMethod : public xmlrpc_c::method
    {
//        bool SetTapesUse( const vector<string> & tapes,
//                int fileNumber, off_t size );

    public:
        TapeSetTapesUseMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "b:sii";
            this->_help = "TapeManagerInterface::SetTapesUse";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            vector<xmlrpc_c::value> const data(params.getArray(0));
            vector<string> tapes;
            for ( vector<xmlrpc_c::value>::const_iterator i = data.begin();
                    i != data.end();
                    ++ i ) {
                tapes.push_back(xmlrpc_c::value_string(*i));
            }
            long const fileNumber(params.getInt(1));
            long long const fileSize(params.getI8(2));
            long long const tapeSize(params.getI8(3));
            LogDebug(boost::join(tapes,",")
                    << " " << fileNumber << " " << fileSize
                    << " " << tapeSize);
            bool retVal = tape_->SetTapesUse(
                    tapes, fileNumber, fileSize, tapeSize );
            LogDebug(boost::join(tapes,",")
                    << " " << fileNumber << " " << fileSize
                    << " " << tapeSize << " : " << retVal);
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    private:
        TapeManagerInterface * tape_;

    };


    /*class TapeGetCapacityMethod : public xmlrpc_c::method
    {
        //bool GetCapacity(
        //      const string & service,
        //      size_t & fileNumber,
        //      off_t & usedSize,
        //      off_t & freeSize ) = 0;

    public:
        TapeGetCapacityMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "S:s";
            this->_help = "TapeManagerInterface::GetCapacity";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const service(params.getString(0));
            LogDebug(service);

            size_t fileNumber = 0;
            off_t usedSize = 0,freeSize = 0;
            bool retVal = tape_->GetCapacity(
                    service, fileNumber, usedSize, freeSize );
            LogDebug(service << " " << fileNumber
                    << " " << usedSize << " " << " " << freeSize
                    << " : " << retVal);

            map<string,xmlrpc_c::value> data;
            pair<string,xmlrpc_c::value> memberResult(
                    "Result", xmlrpc_c::value_boolean(retVal) );
            pair<string,xmlrpc_c::value> memberFileNumber(
                    "FileNumber", xmlrpc_c::value_int(fileNumber) );
            pair<string,xmlrpc_c::value> memberUsedSize(
                    "UsedSize", xmlrpc_c::value_i8(usedSize) );
            pair<string,xmlrpc_c::value> memberFreeSize(
                    "FreeSize", xmlrpc_c::value_i8(freeSize) );
            data.insert(memberResult);
            data.insert(memberFileNumber);
            data.insert(memberUsedSize);
            data.insert(memberFreeSize);
            * ret = xmlrpc_c::value_struct(data);
        }

    private:
        TapeManagerInterface * tape_;

    };*/


    class TapeCheckTapeStateMethod : public xmlrpc_c::method
    {
        //bool CheckTapeState( const string & tape, enum TapeState state );

    public:
        TapeCheckTapeStateMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "b:si";
            this->_help = "TapeManagerInterface::CheckTapeState";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const tape(params.getString(0));
            int const state(params.getInt(1));
            bool retVal = tape_->CheckTapeState(
                    tape, (TapeManagerInterface::TapeState)state );
            LogDebug(tape << " " << state << " : " << retVal);
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    private:
        TapeManagerInterface * tape_;

    };


    class TapeSetTapeStateNoLockMethod : public xmlrpc_c::method
    {
        //bool SetTapeStateNoLock( const string & tape, enum TapeState state );

    public:
        TapeSetTapeStateNoLockMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "b:si";
            this->_help = "TapeManagerInterface::SetTapeStateNoLock";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const tape(params.getString(0));
            int const state(params.getInt(1));
            bool retVal = tape_->SetTapeStateNoLock(
                    tape, (TapeManagerInterface::TapeState)state );
            LogDebug(tape << " " << state << " : " << retVal);
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    private:
        TapeManagerInterface * tape_;

    };


    class TapeLockTapeStateMethod : public xmlrpc_c::method
    {
        //bool LockTapeState( const string & tape );

    public:
        TapeLockTapeStateMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "b:si";
            this->_help = "TapeManagerInterface::LockTapeState";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const tape(params.getString(0));
            bool retVal = tape_->LockTapeState( tape );
            LogDebug(tape << " : " << retVal);
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    private:
        TapeManagerInterface * tape_;

    };


    class TapeSetTapeStateMethod : public xmlrpc_c::method
    {
        //bool SetTapeState( const string & tape, enum TapeState state );

    public:
        TapeSetTapeStateMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "b:si";
            this->_help = "TapeManagerInterface::SetTapeState";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const tape(params.getString(0));
            int const state(params.getInt(1));
            bool retVal = tape_->SetTapeState(
                    tape, (TapeManagerInterface::TapeState)state );
            LogDebug(tape << " " << state << " : " << retVal);
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    private:
        TapeManagerInterface * tape_;

    };


    class TapeSetTapeActionMethod : public xmlrpc_c::method
    {
        //bool SetTapeAction( const string & tape, enum TapeAction action );

    public:
        TapeSetTapeActionMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "b:si";
            this->_help = "TapeManagerInterface::SetTapeAction";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const tape(params.getString(0));
            int const action(params.getInt(1));
            bool retVal = tape_->SetTapeAction(
                    tape, (TapeManagerInterface::TapeAction)action );
            LogDebug(tape << " " << action << " : " << retVal);
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    private:
        TapeManagerInterface * tape_;

    };

    class TapeOpenMailSlotMethod : public xmlrpc_c::method
    {
    public:
        TapeOpenMailSlotMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "b:si";
            this->_help = "TapeManagerInterface::OpenMailSlot";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
#ifdef MORE_TEST
            bool retVal = false;
#else
            bool retVal = TapeLibraryMgr::Instance()->OpenMailSlot();
#endif
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    private:
        TapeManagerInterface * tape_;

    };


    class TapeInventoryLibraryMethod : public xmlrpc_c::method
    {
    public:
        TapeInventoryLibraryMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "b:si";
            this->_help = "TapeManagerInterface::SetTapeStateNoLock";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
#ifdef MORE_TEST
            bool retVal = false;
#else
        	LtfsError  lfsErr;
            bool retVal = TapeLibraryMgr::Instance()->Refresh(true, lfsErr);
#endif
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    private:
        TapeManagerInterface * tape_;

    };

    class TapeGetDriveNumMethod : public xmlrpc_c::method
    {
        //bool GetDriveNum(int& driveNum, int& imageNum)

    public:
        TapeGetDriveNumMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "S:s";
            this->_help = "TapeManagerInterface::GetDriveNum";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            int driveNum = 0;
            bool retVal = tape_->GetDriveNum(driveNum);
            LogDebug("driveNum = " << driveNum << ", retVal = " << retVal);

            map<string,xmlrpc_c::value> data;
            pair<string,xmlrpc_c::value> memberResult(
                    "Result", xmlrpc_c::value_boolean(retVal) );
            pair<string,xmlrpc_c::value> memberDriveNum(
                    "driveNum", xmlrpc_c::value_int(driveNum) );
            data.insert(memberResult);
            data.insert(memberDriveNum);
            * ret = xmlrpc_c::value_struct(data);
        }

    private:
        TapeManagerInterface * tape_;

    };


    class TapeGetTapeActivityMethod : public xmlrpc_c::method
    {
    public:
        TapeGetTapeActivityMethod(TapeManagerInterface * tape)
        : tape_(tape)
        {
            this->_signature = "S:s";
            this->_help = "TapeManagerInterface::GetTapeActivity";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const tape(params.getString(0));
            int percent = 0;
            int act = 0;
            bool retVal = tape_->GetTapeActivity(tape, act, percent);
            LogDebug("TapeGetTapeActivityMethod: tape = " << tape << ", act = " << act << ", percent = " << percent << ", retVal = " << retVal);

            map<string,xmlrpc_c::value> data;
            pair<string,xmlrpc_c::value> memberResult(
                    "Result", xmlrpc_c::value_boolean(retVal) );
            pair<string,xmlrpc_c::value> memberAct(
                    "activity", xmlrpc_c::value_int(act) );
            pair<string,xmlrpc_c::value> memberPercent(
                    "percentage", xmlrpc_c::value_int(percent) );
            data.insert(memberResult);
            data.insert(memberAct);
            data.insert(memberPercent);
            * ret = xmlrpc_c::value_struct(data);
        }

    private:
        TapeManagerInterface * tape_;

    };

    void
    TapeManagerProxyServer::ServiceThread(int handle)
    {
        LogDebug(handle);

        try {
            xmlrpc_c::registry registry;

            xmlrpc_c::methodPtr const methodSetTapeStatus(
                    new TapeSetTapeStatusMethod(tape_));
            registry.addMethod(SetTapeStatus,methodSetTapeStatus);

            xmlrpc_c::methodPtr const methodGetTapeStatus(
                    new TapeGetTapeStatusMethod(tape_));
            registry.addMethod(GetTapeStatus,methodGetTapeStatus);

            xmlrpc_c::methodPtr const methodGetTapesUse(
                    new TapeGetTapesUseMethod(tape_));
            registry.addMethod(GetTapesUse,methodGetTapesUse);

            xmlrpc_c::methodPtr const methodSetTapesUse(
                    new TapeSetTapesUseMethod(tape_));
            registry.addMethod(SetTapesUse,methodSetTapesUse);

            /*xmlrpc_c::methodPtr const methodGetCapacity(
                    new TapeGetCapacityMethod(tape_));
            registry.addMethod(GetCapacity,methodGetCapacity);*/

            xmlrpc_c::methodPtr const methodCheckTapeState(
                    new TapeCheckTapeStateMethod(tape_));
            registry.addMethod(CheckTapeState,methodCheckTapeState);

            xmlrpc_c::methodPtr const methodLockTapeState(
                    new TapeLockTapeStateMethod(tape_));
            registry.addMethod(LockTapeState,methodLockTapeState);

            xmlrpc_c::methodPtr const methodSetTapeStateNoLock(
                    new TapeSetTapeStateNoLockMethod(tape_));
            registry.addMethod(SetTapeStateNoLock,methodSetTapeStateNoLock);

            xmlrpc_c::methodPtr const methodSetTapeState(
                    new TapeSetTapeStateMethod(tape_));
            registry.addMethod(SetTapeState,methodSetTapeState);

            xmlrpc_c::methodPtr const methodSetTapeAction(
                    new TapeSetTapeActionMethod(tape_));
            registry.addMethod(SetTapeAction,methodSetTapeAction);

            xmlrpc_c::methodPtr const methodOpenMailSlot(
                    new TapeOpenMailSlotMethod(tape_));
            registry.addMethod(OpenMailSlot, methodOpenMailSlot);

            xmlrpc_c::methodPtr const methodInventoryLibrary(
                    new TapeInventoryLibraryMethod(tape_));
            registry.addMethod(InventoryLibrary,methodInventoryLibrary);

            xmlrpc_c::methodPtr const methodGetDriveNum(
            		new TapeGetDriveNumMethod(tape_));
            registry.addMethod(GetDriveNum,methodGetDriveNum);

            xmlrpc_c::methodPtr const methodGetTapeActivity(
            		new TapeGetTapeActivityMethod(tape_));
            registry.addMethod(GetTapeActivity ,methodGetTapeActivity);

            xmlrpc_c::serverPstreamConn server(
                    xmlrpc_c::serverPstreamConn::constrOpt()
                    .socketFd(handle)
                    .registryP(&registry));

            bool disconnect;
            server.runOnce(&disconnect);
            if ( disconnect ) {
                LogError("Disconnect");
            }
        } catch ( std::exception const & e ) {
            cerr << e.what() << endl;
        }

        close(handle);
    }
}
