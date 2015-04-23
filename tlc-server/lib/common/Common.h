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
 * Common.h
 *
 *  Created on: Dec 18th, 2013
 *      Author: Sam Chen
 */

#pragma once
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

const string COMM_DATA_PATH = "/opt/VS";
const string COMM_CACHE_PATH = COMM_DATA_PATH + "/vsCache";
const string COMM_META_CACHE_PATH = COMM_CACHE_PATH + "/meta";
const string COMM_DATA_CACHE_PATH = COMM_CACHE_PATH + "/diskCache";
const string COMM_MOUNT_PATH = COMM_DATA_PATH + "/vsMounts";
const string COMM_STORAGE_VFS_PATH = "/srv/node";
const string COMM_DIRECT_ACCESS_PATH = COMM_DATA_PATH + "/DirectTapeAccess";
const string COMM_APP_PATH = "/usr/VS";
const string COMM_APP_BIN_PATH = COMM_APP_PATH + "/vfs";
const string COMM_APP_CONF_PATH = "/etc/vs";
const string COMM_APP_SCRIPTS_PATH = COMM_APP_PATH + "/scripts";
const string COMM_BINARY_SERVER = COMM_APP_BIN_PATH + "/vfsserver";
const string COMM_BINARY_CLIENT = COMM_APP_BIN_PATH + "/vfsclient";
const string COMM_BINARY_SERVER_SIMULATOR = COMM_APP_BIN_PATH + "/vfsserver-simulator";
const string COMM_BINARY_CLIENT_SIMULATOR = COMM_APP_BIN_PATH + "/vfsclient-simulator";
const string COMM_VERSION_PATH = COMM_APP_PATH + "/version";
const string COMM_VS_LOG_PATH = "/var/log/vs";
const string COMM_DB_SERVER = "tcp://localhost:3306";
const string COMM_DEFDB_USER = "vsadmin";
const string COMM_DEFDB_PASS = "hello123";
const string COMM_DEF_ADMIN = "vsadmin";
const string COMM_SWIFT_SHARE_NAME = "vsnode";
const string COMM_AUDIT_OUTPUT_FOLDER = "/var/log/vs";
const string COMM_AUDIT_TOOL_FOLDER = "/usr/share/vs-tape-verify-tool";
const string COMM_AUDIT_TOOL_BIN = "TapeVerifyTool";
const string COMM_SWIFT_OBJECT_SERVER_CONF = "/etc/swift/object-server.conf";


string QuotaString(const string& strSrc);
string QuotaStringForSQL(const string& strSrc);
string GetFolderPath(const string& pathName);
string GetFileName(const string& pathName);
string GetTapeMountPoint(const string& barcode);
string GetPathFromUuid(const string& uuid);
string GetTapeFilePath(unsigned long long number, const string& barcode);
