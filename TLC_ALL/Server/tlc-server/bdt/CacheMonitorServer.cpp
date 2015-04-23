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
 * CacheMonitorServer.cpp
 *
 *  Created on: Nov 2, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "CacheMonitorServer.h"
#include "ServiceServer.h"
#include "CacheManager.h"

#ifdef MORE_TEST
#else
#include "../ltfs_management/TapeLibraryMgr.h"
using namespace ltfs_management;
#endif


namespace bdt
{

    CacheMonitorServer::CacheMonitorServer()
    : folderCache_(Factory::GetCacheFolder()),
      folderMeta_(Factory::GetMetaFolder()),
      run_(true),
      minFreeSize_(0), maxFreeSize_(0),
      thread_(boost::thread(&CacheMonitorServer::ServerThread,this))
    {
        struct statfs stat;
        if ( 0 != statfs(folderCache_.string().c_str(),&stat) ) {
            LogWarn(folderCache_);
            return;
        }
        off_t sizeFileSystemOnePercent =
                (off_t)stat.f_bsize * stat.f_blocks / 100;

        Configure * config = Factory::GetConfigure();

        minFreeSize_ = config->GetValueSize(Configure::CacheFreeMinSize);
        off_t sizeMinFreePercent = sizeFileSystemOnePercent
                * config->GetValueSize(Configure::CacheFreeMinPercent);
        minFreeSize_ = max(minFreeSize_,sizeMinFreePercent);
        LogDebug(minFreeSize_);

        maxFreeSize_ = config->GetValueSize(Configure::CacheFreeMaxSize);
        off_t sizeMaxFreePercent = sizeFileSystemOnePercent
                * config->GetValueSize(Configure::CacheFreeMaxPercent);
        maxFreeSize_ = max(maxFreeSize_,sizeMaxFreePercent);
        LogDebug(maxFreeSize_);
    }


    CacheMonitorServer::~CacheMonitorServer()
    {
        run_ = false;
        thread_.join();
    }


    void
    CacheMonitorServer::ScanFolder(const fs::path & folder)
    {
        CacheManager * cache = Factory::GetCacheManager();
        fs::path folderMeta = Factory::GetMetaFolder() / Factory::GetService();

        try {
            fs::directory_iterator end;
            for ( fs::directory_iterator i(folder); i != end; ++ i ) {
                if ( fs::is_directory(i->status()) ) {
                    ScanFolder(i->path());
                } else {
                    struct stat stat;
                    if ( 0 != ::stat(i->path().string().c_str(),&stat) ) {
                        continue;
                    }

                    ExtendedAttribute ea(i->path());
                    unsigned long long number;
                    int valuesize;
                    if ( ! ea.GetValue(
                            Inode::ATTRIBUTE_NUMBER,
                            &number,
                            sizeof(number),
                            valuesize ) ) {
                        continue;
                    }
                    long state;
                    if ( ! ea.GetValue(
                            Inode::ATTRIBUTE_STATE,
                            &state,
                            sizeof(state),
                            valuesize) ) {
                        continue;
                    }
                    if ( state != Inode::StateBegin ) {
                        continue;
                    }
                    long long size = 0;
                    if ( ! ea.GetValue(
                            Inode::ATTRIBUTE_SIZE,
                            &size,
                            sizeof(size),
                            valuesize) ) {
                        continue;
                    }
                    if ( size <= 0 ) {
                        continue;
                    }

                    if ( ! cache->ExistFile(number) ) {
                        continue;
                    }

                    vector<FileInfo> files;
                    FileMap::iterator iter = files_.insert(
                            FileMap::value_type(stat.st_atime,files) ).first;
                    FileInfo file;
                    file.service = Factory::GetService();
                    file.path = i->path().string().substr(
                            folderMeta.string().size() );
                    iter->second.push_back(file);
                }
            }
        } catch ( const boost::filesystem::filesystem_error& e ) {
        	LogError(e.what());
        } catch ( const std::exception & e ) {
        	LogError(e.what());
        }
    }


    void
    CacheMonitorServer::ServerThread()
    {
        static int waitTime = Factory::GetConfigure()->GetValueSize(
                Configure::CachePurgeWaitTime );

        while ( run_ ) {
            boost::this_thread::sleep(boost::posix_time::seconds(waitTime));

#ifdef MORE_TEST
#else
            TapeLibraryMgr * mgr = TapeLibraryMgr::Instance();
            SystemHwMode mode = mgr->GetSystemHwMode();
            if ( mode == SYSTEM_HW_NEED_DIAGNOSE ) {
                continue;
            }
            if ( mode == SYSTEM_HW_DIAGNOSING ) {
                continue;
            }

            int action;
            if ( mgr->GetWriteCacheAction(action) ) {
                if ( ! mgr->SetWriteCacheAction(action) ) {
                    LogError("Failure to set write cache action: " << action);
                }
            } else {
                LogError("Failure to get write cache action");
            }
#endif

            if ( ! fs::exists(folderMeta_) || ! fs::is_directory(folderMeta_) ) {
                LogWarn(folderMeta_);
                continue;
            }
            if ( CheckFreeSize(minFreeSize_) ) {
                continue;
            }
            LogInfo(folderMeta_);

            fs::directory_iterator end;
            for ( fs::directory_iterator i(folderMeta_); i != end; ++ i ) {
                if ( ! fs::is_directory(i->status()) ) {
                    continue;
                }
                string service = i->path().filename().string();
                Factory::SetService(service);
                Factory::CreateCacheManager();
                ScanFolder(i->path());
                Factory::ReleaseCacheManager();
                Factory::SetService("");
            }

            for ( FileMap::iterator i = files_.begin();
                    i != files_.end();
                    ++ i ) {
                if ( CheckFreeSize(maxFreeSize_) ) {
                    break;
                }

                for ( vector<FileInfo>::iterator iter = i->second.begin();
                        iter != i->second.end();
                        ++ iter ) {
                    if ( CheckFreeSize(maxFreeSize_) ) {
                        break;
                    }

                    ReleaseFile(*iter);
                }
            }

            files_.clear();
        }
    }


    bool
    CacheMonitorServer::ReleaseFile(const FileInfo & file)
    {
        LogDebug(file.service << " " << file.path);

        int handle = Factory::SocketClientHandle(
                ServiceServer::Service + file.service );
        if ( handle < 0 ) {
            LogError("GetHandle");
            return false;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        string const method(ServiceServer::ReleaseFile);
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(file.path.string()));
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
    CacheMonitorServer::CheckFreeSize(off_t freesize)
    {
        struct statfs stat;
        if ( 0 != statfs(folderCache_.string().c_str(),&stat) ) {
            LogWarn(folderCache_);
            return true;
        }

        off_t sizeFree = (off_t)stat.f_bsize * stat.f_bfree;

        return sizeFree >= freesize ? true : false;
    }

}

