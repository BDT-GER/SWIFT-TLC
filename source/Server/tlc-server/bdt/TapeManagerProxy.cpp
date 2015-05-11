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
 * TapeManagerProxy.cpp
 *
 *  Created on: Oct 22, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "TapeManagerProxy.h"


namespace bdt
{

    TapeManagerProxy::TapeManagerProxy()
    : folder_(Factory::GetTapeFolder()),
      service_(Factory::GetService())
    {
        signal(SIGPIPE,SIG_IGN);
    }


    TapeManagerProxy::~TapeManagerProxy()
    {
    }


    bool
    TapeManagerProxy::SetTapeStatus(const string & tape,enum TapeStatus status)
    {
        LogDebug(tape << " " << status);

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::SetTapeStatus);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(tape));
        params.add(xmlrpc_c::value_int(status));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                ret = xmlrpc_c::value_boolean(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        return ret;
    }


    enum TapeManagerInterface::TapeStatus
    TapeManagerProxy::GetTapeStatus( const string & tape )
    {
        LogDebug(tape);

        enum TapeStatus status = STATUS_UNKNOWN;

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return status;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::GetTapeStatus);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(tape));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                status = static_cast<enum TapeManagerInterface::TapeStatus>(
                        static_cast<int>(
                        xmlrpc_c::value_int(rpc.getResult()) ) );
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        return status;
    }


    bool
    TapeManagerProxy::GetShareAvailableTapes(const string& uuid,
    		vector<map<string, off_t> >& tapesList)
    {
        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::GetTapesUse);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(uuid));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                vector<xmlrpc_c::value> data = xmlrpc_c::value_array(rpc.getResult()).vectorValueValue();
                for ( vector<xmlrpc_c::value>::iterator i = data.begin(); i != data.end(); ++ i ) {
                	xmlrpc_c::cstruct cdata = xmlrpc_c::value_struct(*i).cvalue();
                	map<string, off_t> mapItem;
                	for(map<std::string, xmlrpc_c::value>::iterator iv = cdata.begin(); iv != cdata.end(); iv++){
                		mapItem[xmlrpc_c::value_string(iv->first)] = xmlrpc_c::value_i8(iv->second);
                	}
            		tapesList.push_back(mapItem);
                }
                ret = true;
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);
        return ret;
    }

    bool
    TapeManagerProxy::GetTapesUse(
            vector<string> & tapes,
            const fs::path & path,
            off_t size)
    {
        LogDebug(path << " " << size);

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::GetTapesUse);
        xmlrpc_c::paramList params;
        string pathname =
                "/" + service_ + path.string();
                //fs::slash<char>::value + service_ + path.string();
        params.add(xmlrpc_c::value_string(pathname));
        params.add(xmlrpc_c::value_i8(size));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        tapes.clear();
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                vector<xmlrpc_c::value> data = xmlrpc_c::value_array(
                        rpc.getResult() ).vectorValueValue();
                for ( vector<xmlrpc_c::value>::iterator i = data.begin();
                        i != data.end();
                        ++ i ) {
                    tapes.push_back(xmlrpc_c::value_string(*i));
                }
                ret = (! tapes.empty());
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        LogDebug(path << " " << size << " : " << boost::join(tapes,","));

        return ret;
    }


    bool
    TapeManagerProxy::SetTapesUse(
            const vector<string> & tapes,
            int fileNumber,
            off_t fileSize,
            off_t tapeSize)
    {
        LogDebug(boost::join(tapes,",")
                << " " << fileNumber << " " << tapeSize);

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::SetTapesUse);
        xmlrpc_c::paramList params;
        vector<xmlrpc_c::value> data;
        for ( vector<string>::const_iterator i = tapes.begin();
                i != tapes.end();
                ++ i ) {
            data.push_back(xmlrpc_c::value_string(*i));
        }
        params.add(xmlrpc_c::value_array(data));
        params.add(xmlrpc_c::value_int(fileNumber));
        params.add(xmlrpc_c::value_i8(fileSize));
        params.add(xmlrpc_c::value_i8(tapeSize));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                ret = xmlrpc_c::value_boolean(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        return ret;
    }


    /*bool
    TapeManagerProxy::GetCapacity(
            const string & service,
            size_t & fileNumber,
            off_t & usedSize,
            off_t & freeSize )
    {
        LogDebug(service);

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::GetCapacity);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(service));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                map<string,xmlrpc_c::value> data(
                        xmlrpc_c::value_struct(rpc.getResult()) );
                ret = xmlrpc_c::value_boolean(data["Result"]);
                fileNumber = xmlrpc_c::value_int(data["FileNumber"]);
                usedSize = xmlrpc_c::value_i8(data["UsedSize"]);
                freeSize = xmlrpc_c::value_i8(data["FreeSize"]);
                LogDebug(service << " " << fileNumber << " " << usedSize
                        << " " << freeSize << " : " << ret);
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        return ret;
    }*/


    fs::path
    TapeManagerProxy::GetPath( const string & tape, const fs::path & path )
    {
        return folder_ / tape / path;
    }


    bool
    TapeManagerProxy::CheckTapeState(const string & tape,enum TapeState state)
    {
        LogDebug(tape << " " << state);

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::CheckTapeState);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(tape));
        params.add(xmlrpc_c::value_int(state));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                ret = xmlrpc_c::value_boolean(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        return ret;
    }


    bool
    TapeManagerProxy::SetTapeStateNoLock(
            const string & tape, enum TapeState state )
    {
        LogDebug(tape << " " << state);

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::SetTapeStateNoLock);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(tape));
        params.add(xmlrpc_c::value_int(state));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                ret = xmlrpc_c::value_boolean(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        return ret;
    }


    bool
    TapeManagerProxy::LockTapeState( const string & tape )
    {
        LogDebug(tape);

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::LockTapeState);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(tape));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                ret = xmlrpc_c::value_boolean(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        return ret;
    }


    bool
    TapeManagerProxy::SetTapeState(const string & tape,enum TapeState state)
    {
        LogDebug(tape << " " << state);

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::SetTapeState);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(tape));
        params.add(xmlrpc_c::value_int(state));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                ret = xmlrpc_c::value_boolean(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        return ret;
    }


    bool
    TapeManagerProxy::SetTapeAction(const string & tape,enum TapeAction action)
    {
        LogDebug(tape << " " << action);

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::SetTapeAction);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(tape));
        params.add(xmlrpc_c::value_int(action));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        bool ret = false;
        try {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() ) {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                ret = xmlrpc_c::value_boolean(rpc.getResult());
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        return ret;
    }
    bool
    TapeManagerProxy::GetDriveNum(int& driveNum)
    {
    	bool ret = false;

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return ret;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::GetDriveNum);
        xmlrpc_c::paramList params;
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        try
        {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() )
            {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            }
            else
            {
                map<string,xmlrpc_c::value> data(
                        xmlrpc_c::value_struct(rpc.getResult()) );
                ret = xmlrpc_c::value_boolean(data["Result"]);
                driveNum = xmlrpc_c::value_int(data["driveNum"]);
                LogDebug("GetDriveNum: driveNum = " << driveNum);
            }
        }
        catch ( std::exception const & e )
        {
            LogError(e.what());
        }

        close(handle);

    	return ret;
    }

    bool
    TapeManagerProxy::GetTapeActivity(const string& barcode, int& act, int& percentage)
    {
    	bool ret = false;

        int handle = Factory::SocketClientHandle(
                TapeManagerProxyServer::Service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return ret;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(TapeManagerProxyServer::GetTapeActivity);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(barcode));
        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        try
        {
            rpc.call(&client,&carriage);
            if ( ! rpc.isSuccessful() )
            {
                xmlrpc_c::fault fault = rpc.getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            }
            else
            {
                map<string,xmlrpc_c::value> data(
                        xmlrpc_c::value_struct(rpc.getResult()) );
                ret = xmlrpc_c::value_boolean(data["Result"]);
                act = xmlrpc_c::value_int(data["activity"]);
                percentage = xmlrpc_c::value_int(data["percentage"]);
                LogDebug("GetTapeActivity: percentage = " << percentage << ", act = " << act);
            }
        }
        catch ( std::exception const & e )
        {
            LogError(e.what());
        }

        close(handle);

    	return ret;
    }

}

