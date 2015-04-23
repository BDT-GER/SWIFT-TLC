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
 *  Created on: Jul 3, 2012
 *      Author: More Zeng
 */

#pragma once


#include <vector>
#include <set>
#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <time.h>

using namespace std;

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/timer.hpp>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace fs = boost::filesystem;

#define UInt64_t unsigned long long

#include "../lib/common/Common.h"
#if 1
#include "../log/loggerManager.h"

using namespace ltfs_logger;

#define SocketDebug(msg); LgDebug("socket", msg);
#define SocketInfo(msg); LgInfo("socket", msg);
#define SocketWarn(msg); LgWarn("socket", msg);
#define SocketError(msg); LgError("socket", msg);
#define SocketFatal(msg); LgFatal("socket", msg);
#define SocketEvent(level, eventId, msg) CmnEvent("socket", level, eventId, msg)
#else
#define SocketDebug(msg);
#define SocketInfo(msg);
#define SocketWarn(msg);
#define SocketError(msg);
#define SocketFatal(msg);
#define SocketEvent(level, eventId, msg)
#endif
