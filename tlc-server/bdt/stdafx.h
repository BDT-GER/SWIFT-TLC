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
 * stdafx.h
 *
 *  Created on: Feb 29, 2012
 *      Author: More Zeng
 */

#pragma once


#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>


#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

using namespace std;


#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace fs = boost::filesystem;


#include <utime.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <xmlrpc-c/client.hpp>
#include <xmlrpc-c/client_transport.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_pstream.hpp>

#include "../lib/common/Common.h"
#include "Factory.h"

