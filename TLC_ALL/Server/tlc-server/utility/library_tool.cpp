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
 * lfs_tool.cpp
 *
 *  Created on: Nov 30, 2012
 *      Author: Sam Chen
 */


#define MAKE_LFS_TOOL

#include "stdafx.h"
#include "../bdt/TapeManagerProxyServer.h"
using namespace bdt;

void PrintHelp()
{
	cout << "library_tool  Action " << endl;
	cout << "    Action:  " << endl;
	cout << "             InsertTape	 		     ---- Help to insert tape into system." << endl;
}

void
Factory::ReleaseCacheManager()
{
    return;
}

bool ServerXmlRpc(const string& action)
{
    int handle = Factory::SocketClientHandle(
    		TapeManagerProxyServer::Service);
    if ( handle < 0 ) {
		cerr << "Failed to connect to vfsserver, please start the vs service first." << endl;
        return false;
    }
    xmlrpc_c::clientXmlTransport_pstream transport(
            xmlrpc_c::clientXmlTransport_pstream::constrOpt()
            .fd(handle));
    xmlrpc_c::client_xml client(&transport);
    string const method(action);
    xmlrpc_c::paramList params;
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

bool InsertTape()
{
    // open mail slot first
    if(!ServerXmlRpc(TapeManagerProxyServer::OpenMailSlot)){
    	cerr << "Failed to open mail slot for the library to insert tape." << endl;
    	return false;
    }
    cout << "Mail slot of the library has been open, please put in the tape and close the mail slot and type Y and press Enter here." << endl;
    string strCin = "";
	cin >> strCin;

	cout << strCin << endl;
    // start inventory
    if(!ServerXmlRpc(TapeManagerProxyServer::InventoryLibrary)){
    	cerr << "Failed to inventory for the new tape." << endl;
    	return false;
    }
    return true;
}
int main(int argc, char *argv[])
{
    if ( argc < 2 ) {
        PrintHelp();
        return 1;
    }

    string action(argv[1]);
    string strMsg;
    if(action == "InsertTape"){
		if(false == InsertTape()){
			cerr << " Failed to insert tape." << endl;
		}else{
			cout << " Succeed to insert tape." << endl;
		}
	}

    return 0;
}
