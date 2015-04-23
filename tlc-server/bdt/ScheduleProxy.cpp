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
 * ScheduleProxy.cpp
 *
 *  Created on: Oct 19, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ScheduleProxy.h"


namespace bdt
{

    ScheduleProxy::ScheduleProxy()
//    : server_(new ScheduleProxyServer())
    {
        signal(SIGPIPE,SIG_IGN);
    }


    ScheduleProxy::~ScheduleProxy()
    {
    }


    void
    ScheduleProxy::RequestTapesThread(
            const string & seed,
            const vector<string> & tapes, bool mount,
            bool share, int timeout, int priority,
            bool * ret )
    {
        int handle = Factory::SocketClientHandle(ScheduleProxyServer::Service);
        if ( handle < 0 ) {
            LogError("GetHandle");
            * ret = false;
            return;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(ScheduleProxyServer::Request);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_int(getpid()));
        params.add(xmlrpc_c::value_string(seed));
        vector<xmlrpc_c::value> data;
        for ( vector<string>::const_iterator i = tapes.begin();
                i != tapes.end();
                ++ i ) {
            data.push_back(xmlrpc_c::value_string(*i));
        }
        params.add(xmlrpc_c::value_array(data));
        params.add(xmlrpc_c::value_boolean(mount));
        params.add(xmlrpc_c::value_boolean(share));
        params.add(xmlrpc_c::value_int(timeout));
        params.add(xmlrpc_c::value_int(priority));
        xmlrpc_c::rpcPtr rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        * ret = false;
        try {
            rpc->call(&client,&carriage);
            if ( ! rpc->isSuccessful() ) {
                xmlrpc_c::fault fault = rpc->getFault();
                LogError(fault.getCode() << ":" << fault.getDescription());
            } else {
                * ret = xmlrpc_c::value_boolean(rpc->getResult());
            }
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }

        close(handle);

        return;
    }


    bool
    ScheduleProxy::RequestTapes(
            const vector<string> & tapes, bool mount,
            bool share, int timeout, int priority )
    {
        string seed = Factory::GetService() + ":"
                + boost::lexical_cast<string>(boost::this_thread::get_id());

        LogDebug(seed << " " << boost::join(tapes,",") << " " << mount
                << " " << share << " " << timeout << " " << priority);

        bool ret = false;
        auto_ptr<boost::thread> request( new boost::thread(
                &ScheduleProxy::RequestTapesThread, this,
                seed,
                tapes, mount, share, timeout, priority,
                &ret ) );

        try {
            request->join();
        } catch ( const boost::thread_interrupted & e ) {
            LogWarn("Interrupt " << seed);
            int handle = Factory::SocketClientHandle(
                    ScheduleProxyServer::Service );
            if ( handle >= 0 ) {
                xmlrpc_c::clientXmlTransport_pstream transport(
                        xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                        .fd(handle));
                xmlrpc_c::client_xml client(&transport);
                string const method(ScheduleProxyServer::Interrupt);
                xmlrpc_c::paramList params;
                params.add(xmlrpc_c::value_string(seed));
                xmlrpc_c::rpcPtr rpc(method,params);
                xmlrpc_c::carriageParm_pstream carriage;
                rpc->call(&client,&carriage);
                if ( ! rpc->isSuccessful() ) {
                    xmlrpc_c::fault fault = rpc->getFault();
                    LogError(fault.getCode() << ":" << fault.getDescription());
                } else {
                    if ( xmlrpc_c::value_boolean(rpc->getResult()) ) {
                        LogWarn("Success to interrupt " << seed);
                    } else {
                        LogWarn("Failure to interrupt " << seed);
                    }
                }
                close(handle);
            } else {
                LogError("Cannot interrupt " << seed);
            }
        }

        if ( request->joinable() ) {
            request->join();
        }

        LogDebug(seed << " " << boost::join(tapes,",") << " " << mount
                << " " << share << " " << timeout << " " << priority
                << " : " << ret);

        return ret;
    }


    void
    ScheduleProxy::ReleaseTapes(const vector<string> & tapes,bool share)
    {
        LogDebug(boost::join(tapes,",") << " " << share);

        int handle = Factory::SocketClientHandle(ScheduleProxyServer::Service);
        if ( handle < 0 ) {
            LogError("GetHandle");
            return;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(ScheduleProxyServer::Release);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_int(getpid()));
        vector<xmlrpc_c::value> data;
        for ( vector<string>::const_iterator i = tapes.begin();
                i != tapes.end();
                ++ i ) {
            data.push_back(xmlrpc_c::value_string(*i));
        }
        params.add(xmlrpc_c::value_array(data));
        params.add(xmlrpc_c::value_boolean(share));
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
    }

}
