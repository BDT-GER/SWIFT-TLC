/* Copyright (c) 2012 BDT Media Automation GmbH
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
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/random.hpp>

namespace fs = boost::filesystem;

#define UInt64_t unsigned long long
#define UInt32_t unsigned long
#define Int64_t long long
#define UInt16_t unsigned int

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
