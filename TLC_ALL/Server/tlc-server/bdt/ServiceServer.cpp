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
 * ServiceServer.cpp
 *
 *  Created on: Nov 15, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ServiceServer.h"
#include "CacheManager.h"
#include "MetaManager.h"
#include "FileOperationTape.h"
//#include "FileOperationMeta.h"
//#include "FileOperationZero.h"
//#include "FileOperationCIFS.h"
//#include "FileOperationUnchange.h"
#include "TapeManagerStop.h"
//#include "FileDbProxyServer.h"
#include "../ltfs_management/TapeLibraryMgr.h"

using namespace ltfs_management;

static string const IMPORT_FILE_LIST_PATH = "/var/tmp/";
namespace bdt
{

    string const ServiceServer::Service("Client.");
    string const ServiceServer::ReleaseFile("Client.ReleaseFile");
    string const ServiceServer::ReleaseInode("Client.ReleaseInode");
    string const ServiceServer::ReleaseTape("Client.ReleaseTape");
    string const ServiceServer::GetCacheCapacity("Client.GetCacheCapacity");
    string const ServiceServer::SetThrottle("Client.SetThrottle");
    string const ServiceServer::Import("Client.Import");
    string const ServiceServer::SetName("Client.SetName");
    string const ServiceServer::StopTape("Client.StopTape");
    string const ServiceServer::SetCacheState("Client.SetCacheState");

    ServiceServer::ServiceServer()
    : SocketServer( Service + Factory::GetService() ),
      folderMeta_(Factory::GetMetaFolder() / Factory::GetService()),
      folderCache_(Factory::GetCacheFolder() / Factory::GetService())
    {
//        meta_.reset( new MetaManagerDisasterRecovery(
//                new MetaManagerCatalog(Factory::GetMetaManager()) ) );
    }


    ServiceServer::~ServiceServer()
    {
    }

//    class ServiceImportMethod : public xmlrpc_c::method
//    {
//    public:
//        ServiceImportMethod(MetaManager * meta)
//        : meta_(meta), metaRaw_(Factory::GetMetaManager())
//        {
//            this->_signature = "b:sss";
//            this->_help = "Import from a LTFS tape to meta";
//        }
//
//        void
//        execute(xmlrpc_c::paramList const & params,
//                xmlrpc_c::value * const ret)
//        {
//            int const action(params.getInt(0));
//            string const tape(params.getString(1));
//            string const pathTape(params.getString(2));
//            string const pathMeta(params.getString(3));
//            LogDebug(action << " : "
//                    << tape << " " << pathTape << " " << pathMeta);
//
//            bool retVal = false;
//            switch ( action ) {
//            case 0:
//                //  start to monitor the folder
//                break;
//            case 1:
//                retVal = Check( tape, pathTape, pathMeta );
//                break;
//            case 2:
//                retVal = ImportEmpty( tape, pathTape, pathMeta );
//                break;
//            case 3:
//                //  stop to monitor the folder
//                break;
//            case 4:
//                retVal = ImportContent( tape, pathTape, pathMeta );
//                break;
//#ifdef LTFS_VSB_MODE
//            case 6:
//            	retVal = CheckChanges(tape, pathTape, pathMeta);
//            	break;
//#endif
//            default:
//                break;
//            }
//            * ret = xmlrpc_c::value_boolean(retVal);
//        }
//
//    private:
//        bool
//        Check(
//                string const & tape,
//                fs::path const & pathTape,
//                fs::path const & pathMeta)
//        {
//            LogDebug(tape << " : " << pathTape << " : " << pathMeta);
//
//            if ( ! fs::exists(pathTape) ) {
//                LogError(pathTape);
//                return false;
//            }
//
//            auto_ptr<Inode> inode(metaRaw_->GetInode(pathMeta));
//            if ( NULL == inode.get() ) {
//                return true;
//            }
//
//            if ( fs::is_directory(pathTape) ) {
//                if ( inode->IsFile() ) {
//                    LogError(tape << " " << pathTape << " " << pathMeta);
//                    EventError("Import_File_Conflict",
//                            "Import of tape " << tape
//                            << " to share " << Factory::GetName()
//                            << " has been rejected."
//                            << " File " << pathMeta << " exists already.");
//                    return false;
//                } else {
//                    return CheckFolder(tape,pathTape,pathMeta);
//                }
//            } else {
//                string tapeMeta;
//                if ( inode->GetTape(tapeMeta) && (tape == tapeMeta) ) {
//                    LogInfo(tape << " " << pathTape << " " << pathMeta);
//                    return true;
//                } else {
//                    LogError(tape << " " << pathTape << " " << pathMeta);
//                    EventError("Import_File_Conflict",
//                            "Import of tape " << tape
//                            << " to share " << Factory::GetName()
//                            << " has been rejected."
//                            << " File " << pathMeta << " exists already.");
//                    return false;
//                }
//            }
//        }
//
//        bool
//        CheckFolder(
//                string const & tape,
//                fs::path const & pathTape,
//                fs::path const & pathMeta)
//        {
//            LogDebug(tape << " : " << pathTape << " : " << pathMeta);
//
//            try{
//				if ( ! fs::is_directory(pathTape) ) {
//					LogError(pathTape);
//					return false;
//				}
//
//				fs::directory_iterator end;
//				for ( fs::directory_iterator i(pathTape); i != end; ++ i ) {
//					if (! Check(tape,i->path(),pathMeta/i->path().filename().string())) {
//						return false;
//					}
//				}
//				return true;
//            }catch(const boost::filesystem::filesystem_error& e){
//            	LogError(e.what());
//            }catch(...){
//            	LogError("Exception.");
//            }
//            return false;
//        }
//
//        typedef map<long long,fs::path> OffsetMap;
//        typedef map<time_t,vector<fs::path> > TimeMap;
//
//        bool
//        ScanFolder(
//                fs::path const & folder,
//                OffsetMap & offsetFiles,
//                TimeMap & timeFiles)
//        {
//            bool ret = true;
//
//            try{
//
//            fs::directory_iterator end;
//            for ( fs::directory_iterator i(folder); i != end; ++ i ) {
//                if ( fs::is_directory(i->status()) ) {
//                    ret = ScanFolder(i->path(),offsetFiles,timeFiles) && ret;
//                } else {
//                    ExtendedAttribute ea(i->path());
//                    char buf[1024];
//                    int bufsize;
//                    if ( ea.GetValue( "user.ltfs.startblock",
//                            buf, sizeof(buf) - 1, bufsize ) ) {
//                        buf[bufsize] = '\0';
//                        try {
//                            string offset(buf);
//                            offsetFiles.insert( OffsetMap::value_type(
//                                    boost::lexical_cast<long long>(offset),
//                                    i->path() ) );
//                            LogDebug(i->path());
//                            continue;
//                        } catch ( const std::exception & e ) {
//                            LogError(i->path().string());
//                        }
//                    }
//                    vector<fs::path> paths;
//                    TimeMap::iterator iter = timeFiles.insert(
//                            TimeMap::value_type(
//                                fs::last_write_time(i->path()), paths )
//                            ).first;
//                    iter->second.push_back(i->path());
//                    LogDebug(i->path());
//                }
//            }
//            return true;
//
//            }catch(const boost::filesystem::filesystem_error& e){
//            	LogError(e.what());
//            }catch(...){
//            	LogError("Exception.");
//            }
//        }
//
//        bool
//        ImportFile(
//                string const & pathname,
//                string const & tape,
//                fs::path const & pathTape,
//                fs::path const & pathMeta)
//        {
//            if ( 0 != pathname.find(pathTape.string()) ) {
//                LogError(pathname);
//                return false;
//            }
//
//            string pathInode = pathname.substr(
//                    pathTape.string().size() );
//            bool ret = ImportInodeContent(
//                    tape, pathname, pathMeta / pathInode );
//            LogDebug(pathname << " : " << ret);
//            if ( ! ret ) {
//                EventError("", "Failed to import file from tape " << tape
//                        << " to " << pathInode
//                        << " for share " << Factory::GetName() << ".");
//            }
//            return ret;
//        }
//
//        bool
//        ImportContent(
//                string const & tape,
//                fs::path const & pathTape,
//                fs::path const & pathMeta)
//        {
//            bool ret = false;
//
//            if ( ! fs::is_directory(pathTape) ) {
//                ret = ImportInodeEmpty(tape,pathTape,pathMeta);
//                LogDebug(pathTape << " : " << ret);
//                if ( ret ) {
//                    ret = ImportInodeContent(tape,pathTape,pathMeta);
//                    LogDebug(pathTape << " : " << ret);
//                }
//                if ( ! ret ) {
//                    EventError("", "Failed to import file from tape " << tape
//                            << " to " << pathMeta
//                            << " for share " << Factory::GetName() << ".");
//                }
//                return true;
//            }
//
//            ret = ImportFolder(tape,pathTape,pathMeta);
//            LogDebug(pathTape << " : " << ret);
//
//            OffsetMap offsetFiles;
//            TimeMap timeFiles;
//            ret = ScanFolder(pathTape,offsetFiles,timeFiles);
//            LogDebug(pathTape << " : " << ret);
//
//            for ( OffsetMap::iterator i = offsetFiles.begin();
//                    i != offsetFiles.end();
//                    ++ i ) {
//                ImportFile(i->second.string(),tape,pathTape,pathMeta);
//            }
//
//            for ( TimeMap::iterator i = timeFiles.begin();
//                    i != timeFiles.end();
//                    ++ i ) {
//                for ( vector<fs::path>::iterator iter = i->second.begin();
//                        iter != i->second.end();
//                        ++ iter ) {
//                    ImportFile(iter->string(),tape,pathTape,pathMeta);
//                }
//            }
//
//            return true;
//        }
//
//        bool
//        ImportEmpty(
//                string const & tape,
//                fs::path const & pathTape,
//                fs::path const & pathMeta)
//        {
//            bool ret = false;
//
//            if ( ! meta_->CheckFreeCapacity() ) {
//                LogError("No free capacity")
//                return false;
//            }
//
//            if ( ! fs::is_directory(pathTape) ) {
//                ret = ImportInodeEmpty(tape,pathTape,pathMeta);
//                LogDebug(pathTape << " : " << ret);
//                if ( ! ret ) {
//                    EventError("", "Failed to import file from tape " << tape
//                            << " to " << pathMeta
//                            << " for share " << Factory::GetName() << ".");
//                }
//                return ret;
//            }
//
//            ret = ImportFolder(tape,pathTape,pathMeta);
//            LogDebug(pathTape << " : " << ret);
//            return ret;
//        }
//
//        bool
//        ImportFolder(
//                string const & tape,
//                fs::path const & pathTape,
//                fs::path const & pathMeta)
//        {
//            LogDebug(tape << " " << pathTape << " " << pathMeta);
//
//            try{
//				bool ret = true;
//				fs::directory_iterator end;
//				for ( fs::directory_iterator i(pathTape); i != end; ++ i ) {
//					fs::path target = pathMeta / i->path().filename().string();
//					if ( fs::is_directory(i->status()) ) {
//						if ( ImportInodeEmpty(tape,i->path(),target) ) {
//							ret = ImportFolder(tape,i->path(),target) && ret;
//						} else {
//							ret = false;
//						}
//					} else {
//						ret = ImportInodeEmpty(tape,i->path(),target) && ret;
//					}
//				}
//				return ret;
//            }catch(const boost::filesystem::filesystem_error& e){
//            	LogError(e.what());
//            }catch(...){
//            	LogError("Exception.");
//            }
//            return false;
//        }
//
//
//        bool AddUpdateFileDb(const fs::path & path)
//        {
//            int handle = Factory::SocketClientHandle(
//                    FileDbProxyServer::Service );
//            if ( handle < 0 ) {
//                LogError("GetHandle");
//                return false;
//            }
//
//            xmlrpc_c::clientXmlTransport_pstream transport(
//                    xmlrpc_c::clientXmlTransport_pstream::constrOpt()
//                    .fd(handle));
//            xmlrpc_c::client_xml client(&transport);
//            string const method(FileDbProxyServer::AddUpdateFile);
//            xmlrpc_c::paramList params;
//            vector<xmlrpc_c::value> data;
//            LogDebug("SEARCH: AddUpdateFileDb, path = " << path.string());
//            data.push_back(xmlrpc_c::value_string(path.string()));
//            params.add(xmlrpc_c::value_array(data));
//            xmlrpc_c::rpc rpc(method,params);
//            xmlrpc_c::carriageParm_pstream carriage;
//
//            bool ret = false;
//            try {
//                rpc.call(&client,&carriage);
//                if ( ! rpc.isSuccessful() ) {
//                    xmlrpc_c::fault fault = rpc.getFault();
//                    LogError("SEARCH: " << fault.getCode() << ":" << fault.getDescription());
//                } else {
//                    ret = xmlrpc_c::value_boolean(rpc.getResult());
//                }
//            } catch ( std::exception const & e ) {
//                LogError("SEARCH: " << e.what());
//            }
//
//            LogDebug("SEARCH: ret = " << ret);
//            close(handle);
//        	return ret;
//        }
//
//        bool
//        ImportInodeEmpty(
//                string const & tape,
//                fs::path const & pathTape,
//                fs::path const & pathMeta)
//        {
//            LogDebug(tape << " " << pathTape << " " << pathMeta);
//
//            if ( ! fs::exists(pathTape) ) {
//                LogError(pathTape);
//                return false;
//            }
//
//            auto_ptr<FileOperation> source(
//                    new FileOperationTape( pathTape, tape ) );
//            struct stat stat;
//            if ( ! source->GetStat(stat) ) {
//                LogError(pathTape);
//                return false;
//            }
//
//            auto_ptr<Inode> inode(metaRaw_->GetInode(pathMeta));
//
//            if ( fs::is_directory(pathTape) ) {
//                MetaManagerCatalog meta(Factory::GetMetaManager());
//                if ( ! meta.CreateTapeFolder(tape,pathMeta) ) {
//                    LogError(tape << " " << pathMeta);
//                }
//                if ( NULL == inode.get() ) {
//                    inode.reset( metaRaw_->CreateFolder(
//                            pathMeta, stat.st_mode | 0700 ) );
//                }
//
//                if ( NULL != inode.get() ) {
//                    if ( inode->IsFile() ) {
//                        LogError(pathMeta);
//                        return false;
//                    } else {
//                        return true;
//                    }
//                } else {
//                    LogError(pathMeta);
//                    return false;
//                }
//            }
//
//            if ( NULL != inode.get() ) {
//                if ( inode->IsFile() ) {
//                    string tapeInode;
//                    if ( inode->GetTape(tapeInode) && (tape == tapeInode) ) {
//                        return true;
//                    } else {
//                        LogError(pathMeta);
//                        return false;
//                    }
//                } else {
//                    LogError(pathMeta);
//                    return false;
//                }
//            }
//
//            inode.reset( metaRaw_->CreateFile(
//                    pathMeta, stat.st_mode | 0600 ) );
//            if ( NULL == inode.get() ) {
//                LogError(pathMeta);
//                return false;
//            }
//
//            ExtendedAttribute ea(pathTape);
//            char startblock[1024];
//            memset(startblock,0,sizeof(startblock));
//            int valuesize;
//            if ( ea.GetValue( "user.ltfs.startblock",
//                    startblock, sizeof(startblock), valuesize ) ) {
//                startblock[valuesize] = '\0';
//                off_t offset = boost::lexical_cast<off_t>(string(startblock));
//                if ( ! inode->SetOffset(offset) ) {
//                    LogError(pathMeta);
//                }
//            } else {
//                LogWarn(pathMeta);
//            }
//
//            bool ret = inode->SetSize(stat.st_size) && inode->SetTape(tape);
//            if ( ! ret ) {
//                LogError(pathMeta);
//                metaRaw_->DeleteInode(pathMeta);
//                return false;
//            }
//
//            struct ::utimbuf times;
//            times.actime = time(NULL);
//            times.modtime = stat.st_mtime;
//            utime(inode->Path().string().c_str(),&times);
//
//            inode.reset(metaRaw_->GetInode(pathMeta));
//            if ( NULL == inode.get() ) {
//                LogError(pathMeta);
//            } else {
//                if ( ! inode->SetSize(stat.st_size) ) {
//                    LogError(pathMeta);
//                }
//                time_t now = ::time(NULL);
//                ExtendedAttribute ea(inode->Path());
//                if ( ! ea.SetValue( Inode::ATTRIBUTE_MODTIME,
//                        &now, sizeof(now) ) ) {
//                    LogError(pathMeta);
//                }
//
//                // merge EA from tape to meta
//                ExtendedAttribute eaTape(pathTape);
//                vector<string> ignoredNames;
//                ignoredNames.push_back(Inode::ATTRIBUTE_MODTIME);
//                ignoredNames.push_back(Inode::ATTRIBUTE_TAPE);
//                if(!eaTape.MergeAttrTo(inode->Path(), ignoredNames)){
//                	LogError("Failed to merge EA from file " << pathTape.string() << " to file " << inode->Path().string());
//                }
//                if ( !eaTape.SetValue( Inode::ATTRIBUTE_TAPE, tape.c_str(), tape.size() ) ) {
//                    LogError("Failed to update " << Inode::ATTRIBUTE_TAPE << " for file " << pathTape);
//                }
//
//                // update tape barcode in the LTFS Stream EA fields
//                if(!ea.UpdateTapeToLtfsEA(tape)){
//                    LogError("SEARCH: Failed to update tape barcode to LTFS EA Stream for file " << inode->Path());
//                }
//                /*string eaStream = "";
//                ea.GetStringValue(Inode::ATTRIBUTE_PLUGIN_ORG, eaStream);
//                if(eaStream != ""){
//                	//"sdf||||||dsf||||LT1004L5,LT1003L5"
//                	regex matchEaStream("^(([^\\|]*\\|){10})(.*)$");
//                    cmatch match;
//                    if(regex_match(eaStream.c_str(), match, matchEaStream)){
//                    	eaStream = match[1] + tape;
//                    }else{
//                    	eaStream += "|" + tape;
//                    }
//                }else{
//                	eaStream = "||||||||||" + tape;
//                }
//                if ( !ea.SetValue(Inode::ATTRIBUTE_PLUGIN_ORG, eaStream.c_str(), eaStream.length()) ) {
//                	LogError("SEARCH: Failed to update tape barcode to LTFS Stream EA for file " << inode->Path());
//                }*/
//
//
//                if(false == AddUpdateFileDb(inode->Path())){
//                	LogError("SEARCH: Failed to add file " << inode->Path().string() << " to search database.");
//                }
//
//            }
////            if ( ! Factory::GetTapeManager()->SetTapeUse(
////                    tape, 1, stat.st_size, 0 ) ) {
////                LogError(tape << " " << stat.st_size);
////            }
//
//            return true;
//        }
//
//        bool
//        ImportInodeContent(
//                string const & tape,
//                fs::path const & pathTape,
//                fs::path const & pathMeta)
//        {
//            LogDebug(tape << " " << pathTape << " " << pathMeta);
//
//            if ( ! fs::exists(pathTape) ) {
//                LogError(pathTape);
//                return false;
//            }
//
//            if ( ! meta_->CheckFreeCapacity() ) {
//                LogError("No free capacity: " << pathMeta);
//                return false;
//            }
//
//            auto_ptr<FileOperation> source(
//                    new FileOperationTape( pathTape, tape ) );
//            struct stat stat;
//            if ( ! source->GetStat(stat) ) {
//                LogError(pathTape);
//                return false;
//            }
//
//            auto_ptr<Inode> inode(metaRaw_->GetInode(pathMeta));
//
//            if ( NULL == inode.get() ) {
//                LogError(pathMeta);
//                return false;
//            }
//
//            if ( ! inode->IsFile() ) {
//                LogError(pathMeta);
//                return false;
//            }
//
//            string tapeInode;
//            if ( inode->GetTape(tapeInode) && (tape == tapeInode) ) {
//                if ( FileOperationCIFS::CheckInode(inode.get()) ) {
//                    LogInfo(pathMeta);
//                    return true;
//                }
//                if ( 0 != fs::file_size(inode->Path()) ) {
//                    LogWarn(pathMeta);
//                }
//            } else {
//                LogError(pathMeta);
//                return false;
//            }
//
//            if ( ! source->OpenFile(O_RDONLY) ) {
//                LogError(pathTape);
//                return false;
//            }
//
//            bool checksum = false;
//            if (Factory::GetConfigure()->GetValueBool(
//                    Configure::DigestMD5Enable ) ) {
//                checksum = true;
//            }
//            if (Factory::GetConfigure()->GetValueBool(
//                    Configure::DigestSHA1Enable ) ) {
//                checksum = true;
//            }
//            checksum = false;
//
//            auto_ptr<FileOperationInterface> targetChecksum;
//            if ( checksum ) {
//                targetChecksum.reset( new FileOperationMeta(
//                        metaRaw_, pathMeta, new FileOperationZero(), "" ) );
//            } else {
//                targetChecksum.reset( new FileOperationZero() );
//            }
//            if ( ! targetChecksum->OpenFile(O_RDWR) ) {
//                LogError(pathMeta);
//                return false;
//            }
//
//            auto_ptr<FileOperationInterface> targetMeta;
//            targetMeta.reset( new FileOperationCIFS(
//                    inode.release(),
//                    new FileOperationUnchange(new FileOperation(pathTape)),
//                    0 ) );
//            if ( ! targetMeta->OpenFile(O_RDWR) ) {
//                LogError(pathTape);
//                return false;
//            }
//
//            off_t offset = 0;
//            const int BUFFER_SIZE = 512 * 1024;
//            off_t offsetEnd = stat.st_size - FileOperationCIFS::SizeInodeEnd;
//            if ( offsetEnd > 0 ) {
//                offsetEnd = (offsetEnd / BUFFER_SIZE) * BUFFER_SIZE;
//            } else {
//                offsetEnd = 0;
//            }
//            boost::scoped_array<char> buffer(new char[BUFFER_SIZE]);
//            size_t size;
//            while ( source->Read(offset,buffer.get(),BUFFER_SIZE,size) ) {
//                if ( 0 == size ) {
//                    targetChecksum.reset();
//                    targetMeta.reset();
//                    source.reset();
//
//                    inode.reset(meta_->GetInode(pathMeta));
//                    if ( NULL == inode.get() ) {
//                        LogError(pathMeta);
//                        return false;
//                    }
//                    if ( ! inode->SetTape(tape) ) {
//                        LogError(pathMeta);
//                        return false;
//                    }
//                    time_t now = ::time(NULL);
//                    struct ::utimbuf times;
//                    times.actime = now;
//                    times.modtime = stat.st_mtime;
//                    utime(inode->Path().string().c_str(),&times);
//                    if(!inode->SetSize(stat.st_size)){
//                    	LogWarn("Failed to call SetSize for " << inode->Path().string());
//                    }
//
//                    return true;
//                }
//
//                if (! targetChecksum->Write(offset,buffer.get(),size,size)) {
//                    break;
//                }
//                if ( ! targetMeta->Write(offset,buffer.get(),size,size) ) {
//                    break;
//                }
//                LogDebug(pathTape << " " << offset << " " << size);
//                offset += size;
//
//                if ( checksum ) {
//                    continue;
//                }
//                if ( offset >= FileOperationCIFS::SizeInode ) {
//                    offset = std::max<off_t>( offset, offsetEnd );
//                    continue;
//                }
//            }
//
//            targetChecksum.reset();
//            targetMeta.reset();
//            source.reset();
//            meta_->DeleteInode(pathMeta);
//            LogError(pathTape);
//            return false;
//        }
//#ifdef LTFS_VSB_MODE
//        void
//        ScanMeta(fs::path const & folder,
//        		fs::path const & pathTape){
//        	LogDebug(folder);
//        	try{
//				fs::directory_iterator end;
//				for ( fs::directory_iterator i(folder); i != end; ++ i ) {
//					fs::path pathMetaRoot = Factory::GetMetaFolder()/Factory::GetService();
//					string name = i->path().string();
//					if ( 0 != name.find(pathMetaRoot.string()) ) {
//						LogError(name);
//						continue;
//					}
//					string nameInode = name.substr(pathMetaRoot.string().size() );
//					fs::path tapeFilePath = pathTape/nameInode;
//					if (!fs::exists(tapeFilePath)) {
//						try{
//							LogDebug("ScanMeta:" << nameInode << "Path:" << i->path());
//							fs::path pathCache = Factory::GetCacheFolder()/Factory::GetService()/nameInode;
//
//							Factory::GetCacheManager()->ReleaseFileOperation(
//									pathCache, false );
//
//							Factory::GetCacheManager()->DeleteFolderOperation(pathCache);
//
//							if ( fs::exists(pathCache) ) {//force delete
//								fs::remove_all(pathCache);
//							}
//							meta_->DeleteInode(nameInode);
//							fs::remove_all(i->path());
//						}catch(...){
//							LogError("Failed to remove meta");
//						}
//					}else if ( fs::is_directory(i->status()) ) {
//						ScanMeta(i->path(), pathTape);
//					}
//				}
//            }catch(const boost::filesystem::filesystem_error& e){
//            	LogError(e.what());
//            }catch(...){
//            	LogError("Exception.");
//            }
//
//        	return ;
//        }
//
//        bool
//		ScanTapeFolder(string const & tape,
//				fs::path const & folder,
//				unsigned long & numberFile,
//				unsigned long & numberFolder,
//				off_t & sizeTotal,
//				OffsetMap & offsetFiles,
//				TimeMap & timeFiles)
//		{
//			numberFile = 0;
//			numberFolder = 0;
//			sizeTotal = 0;
//			off_t fileSize = 0;
//			try{
//
//			fs::directory_iterator end;
//			fs::path pathTape = Factory::GetTapeFolder()/tape;
//			for ( fs::directory_iterator i(folder); i != end; ++ i ) {
//				LogDebug("ScanTapeFolder:" << i->path().string());
//				if ( fs::is_directory(i->status()) ) {
//					LogDebug("ScanTapeFolder:" << i->path().string());
//					++ numberFolder;
//					unsigned long nFile, nFolder;
//					off_t sTotal;
//					string name = i->path().string();
//					if ( 0 != name.find(pathTape.string()) ) {
//						LogError(name);
//						continue;
//					}
//					string nameInode = name.substr(pathTape.string().size() );
//					auto_ptr<Inode> inode(metaRaw_->GetInode(nameInode));
//					if (NULL == inode.get()){
//						if (!ImportInodeEmpty(tape, i->path(), nameInode)){
//							LogDebug("ScanTapeFolder:" << i->path().string());
//							return false;
//						}
//					}
//					ScanTapeFolder(tape, i->path(),nFile,nFolder,sTotal,offsetFiles,timeFiles);
//					numberFile += nFile;
//					numberFolder += nFolder;
//					sizeTotal += sTotal;
//				}else {
//					LogDebug("--->ScanTapeFolder:" << i->path().string());
//					try {
//						fileSize = fs::file_size(i->path());
//					} catch ( std::exception const & e ) {
//						LogError( e.what() );
//					}
//					++ numberFile;
//					sizeTotal += fileSize;
//					string name = i->path().string();
//					if ( 0 != name.find(pathTape.string()) ) {
//						LogError(name);
//						continue;
//					}
//					string nameInode = name.substr(pathTape.string().size() );
//					auto_ptr<Inode> inode(metaRaw_->GetInode(nameInode));
//					bdt::ExtendedAttribute ea(i->path());
//					char buffer[1024];
//					int bufsize;
//					if ( NULL == inode.get() ) {
//						LogDebug("--->ScanTapeFolder:" << i->path().string());
//						if ( ea.GetValue( "user.ltfs.startblock",
//								buffer, sizeof(buffer) - 1, bufsize ) ) {
//							buffer[bufsize] = '\0';
//							off_t offset = boost::lexical_cast<off_t>(string(buffer));
//							offsetFiles.insert( OffsetMap::value_type(
//									boost::lexical_cast<long long>(offset),
//									i->path() ) );
//						} else {
//							vector<fs::path> paths;
//							TimeMap::iterator iter = timeFiles.insert(
//									TimeMap::value_type(
//										fs::last_write_time(i->path()), paths )
//									).first;
//							iter->second.push_back(i->path());
//						}
//						if (!ImportInodeEmpty(tape, i->path(), nameInode)){
//							LogDebug("--->ScanTapeFolder:" << i->path().string());
//							return false;
//						}
//					}else {
//						struct stat fileStat;
//
//						if (!inode->GetStat(fileStat)){
//							LogError("Filed to get file stat." << i->path());
//						}else {
//							LogDebug("--->ScanTapeFolder:" << i->path().string());
//							if (fileStat.st_size != fileSize || fileStat.st_mtime != last_write_time(i->path())){
//								LogDebug("--->ScanTapeFolder:" << i->path().string());
//								if ( ea.GetValue( "user.ltfs.startblock",
//										buffer, sizeof(buffer) - 1, bufsize ) ) {
//									buffer[bufsize] = '\0';
//									off_t offset = boost::lexical_cast<off_t>(string(buffer));
//									offsetFiles.insert( OffsetMap::value_type(
//											boost::lexical_cast<long long>(offset),
//											i->path() ) );
//								} else {
//									vector<fs::path> paths;
//									TimeMap::iterator iter = timeFiles.insert(
//											TimeMap::value_type(
//												fs::last_write_time(i->path()), paths )
//											).first;
//									iter->second.push_back(i->path());
//								}
//								meta_->DeleteInode(nameInode);
//								if (!ImportInodeEmpty(tape, i->path(), nameInode)){
//									LogDebug("--->ScanTapeFolder:" << i->path().string());
//									return false;
//								}
//							}else {
//								LogDebug("--->ScanTapeFolder:" << i->path().string());
//								string tapeInode;
//								if ( inode->GetTape(tapeInode) && (tape == tapeInode) ) {
//									if ( FileOperationCIFS::CheckInode(inode.get()) ) {
//										LogInfo(i->path());
//										continue;
//									}
//								}
//								if ( ea.GetValue( "user.ltfs.startblock",
//										buffer, sizeof(buffer) - 1, bufsize ) ) {
//									buffer[bufsize] = '\0';
//									off_t offset = boost::lexical_cast<off_t>(string(buffer));
//									offsetFiles.insert( OffsetMap::value_type(
//											boost::lexical_cast<long long>(offset),
//											i->path() ) );
//								} else {
//									vector<fs::path> paths;
//									TimeMap::iterator iter = timeFiles.insert(
//											TimeMap::value_type(
//												fs::last_write_time(i->path()), paths )
//											).first;
//									iter->second.push_back(i->path());
//								}
//							}
//						}
//					}
//				}
//			}
//			return true;
//
//	        }catch(const boost::filesystem::filesystem_error& e){
//	        	LogError(e.what());
//	        }catch(...){
//	        	LogError("Exception.");
//	        }
//	        return false;
//		}
//
//        bool
//		CheckChanges(
//				string const & tape,
//				fs::path const & pathTape,
//				fs::path const & pathMeta)
//		{
//			LogDebug(tape << " : " << pathTape << " : " << pathMeta);
//
//			if ( ! fs::exists(pathTape) ) {
//				LogError(pathTape);
//				return false;
//			}
//			LogDebug(tape << " : " << pathTape << " : " << pathMeta);
//			fs::path fileListPath = fs::path(IMPORT_FILE_LIST_PATH + string(".") + tape);
//
//			if (fs::exists(fileListPath)) {//delete list file if it exists.
//				fs::remove(fileListPath);
//			}
//
//			//delete meta if it's not exists in tape.
//			ScanMeta(Factory::GetMetaFolder()/Factory::GetService(), pathTape);
//
//			//scan tape, generate import file list.
//			OffsetMap offsetFiles;
//			TimeMap timeFiles;
//			unsigned long numberFile = 0;
//			unsigned long numberFolder = 0;
//			off_t sizeTotal = 0;
//
//			if (!ScanTapeFolder(tape, pathTape, numberFile, numberFolder, sizeTotal, offsetFiles, timeFiles)) {
//				LogError(pathTape);
//				return false;
//			}
//			LogDebug(tape << " : " << pathTape << " : " << pathMeta);
//			ofstream  fout(fileListPath.string().c_str(), ios::out);
//			if (!fout.is_open())
//			{
//				return false;
//			}
//			LogDebug(tape << " : " << pathTape << " : " << pathMeta);
//			for ( OffsetMap::iterator i = offsetFiles.begin();
//					i != offsetFiles.end();
//					++ i ) {
//				fout << i->second.string() <<endl;
//			}
//
//			for ( TimeMap::iterator i = timeFiles.begin();
//					i != timeFiles.end();
//					++ i ) {
//				for ( vector<fs::path>::iterator iter = i->second.begin();
//						iter != i->second.end();
//						++ iter ) {
//					ImportFile(iter->string(),tape,pathTape,pathMeta);
//					fout << iter->string() <<endl;
//				}
//			}
//			fout.close();
//
//			return true;
//		}
//#endif
//    private:
//        MetaManager * meta_;
//        MetaManager * metaRaw_;
//    };


    class ServiceReleaseFileMethod : public xmlrpc_c::method
    {
    public:
        ServiceReleaseFileMethod()
        : meta_(Factory::GetMetaManager())
        {
            this->_signature = "b:s";
            this->_help = "Release (delete) a file from cache";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const path(params.getString(0));
            LogDebug(path);

            bool retVal = meta_->ReleaseFile(path);
            LogDebug(path << " : " << retVal);
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    private:
        MetaManager * meta_;
    };


    class ServiceGetCacheCapacityMethod : public xmlrpc_c::method
    {
//        bool MetaManager::GetBackupList(vector<BackupItem> & list);

    public:
        ServiceGetCacheCapacityMethod()
        {
            this->_signature = "b:s";
            this->_help = "CacheManager::GetCacheSize";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            bool const isWrite(params.getBoolean(0));
            LogDebug(isWrite);

            off_t size = 0;
            vector<BackupItem> items;
            Factory::GetMetaManager()->GetBackupList(items);
            BOOST_FOREACH(const BackupItem & item, items) {
                size += item.size;
            }

            LogDebug(isWrite << " : " << size);
            * ret = xmlrpc_c::value_i8(size);
        }

    };


    class ServiceSetCacheStateMethod : public xmlrpc_c::method
    {
    public:
        ServiceSetCacheStateMethod()
        {
            this->_signature = "b:s";
            this->_help = "bool CacheManager::SetCacheState(int state)";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            int const state(params.getInt(0));
            LogDebug(state);

            bool retValue = Factory::GetCacheManager()->SetCacheState(state);
            if ( ! retValue ) {
                LogError(state);
            }
            * ret = xmlrpc_c::value_boolean(retValue);
        }

    };


    class ServiceSetThrottleMethod : public xmlrpc_c::method
    {
        //bool Throttle::Reset(int interval,long long valve);

    public:
        ServiceSetThrottleMethod()
        {
            this->_signature = "b:ii";
            this->_help = "Throttle::Reset";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            int const interval(params.getInt(0));
            long long const valve(params.getI8(1));

            LogDebug(interval << " " << valve);

            bool retVal = Factory::GetThrottle()->Reset(interval,valve);

            LogDebug(interval << " " << valve << " : " << retVal);
            * ret = xmlrpc_c::value_boolean(retVal);
        }

    };


    class ServiceSetNameMethod : public xmlrpc_c::method
    {
        //void Factory::SetName(const string & name);

    public:
        ServiceSetNameMethod()
        {
            this->_signature = "b:s";
            this->_help = "Factory::SetName";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const name(params.getString(0));

            LogDebug(name);

            Factory::SetName(name);

            * ret = xmlrpc_c::value_boolean(true);
        }

    };


    class ServiceStopTapeMethod : public xmlrpc_c::method
    {
        //bool TapeManagerStop::StopTape(const string & tape);

    public:
        ServiceStopTapeMethod()
        {
            this->_signature = "b:s";
            this->_help = "TapeManagerStop::StopTape";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const tape(params.getString(0));

            LogDebug(tape);

            TapeManagerInterface * interface = Factory::GetTapeManager();
            TapeManagerStop * stop = dynamic_cast<TapeManagerStop *>(
                    interface );
            bool retValue;
            if ( NULL == stop ) {
                retValue = false;
                LogError(tape);
            } else {
                retValue = stop->StopTape(tape);
            }

            * ret = xmlrpc_c::value_boolean(retValue);
        }

    };


//    class ServiceReleaseInodeMethod : public xmlrpc_c::method
//    {
//    public:
//        ServiceReleaseInodeMethod(
//                fs::path const & folderCache,
//                MetaManager * meta)
//        : folderCache_(folderCache), meta_(meta)
//        {
//            this->_signature = "b:s";
//            this->_help = "Release (delete) an inode from meta";
//        }
//
//        void
//        execute(xmlrpc_c::paramList const & params,
//                xmlrpc_c::value * const ret)
//        {
//            string const path(params.getString(0));
//            LogDebug(path);
//            fs::path pathCache = folderCache_ / path;
//
//            Factory::GetCacheManager()->ReleaseFileOperation(
//                    pathCache, false );
//
//            Factory::GetCacheManager()->DeleteFolderOperation(pathCache);
//
//            bool retVal = true;
//            if ( fs::exists(pathCache) ) {
//                //  Force to delete the file in cache
//                LogWarn(pathCache);
//                try {
//                    fs::remove_all(pathCache);
//                } catch (...) {
//                    LogError(pathCache);
//                    retVal = false;
//                }
//            }
//
//            if ( retVal ) {
//                auto_ptr<Inode> inode(meta_->GetInode(path));
//                if ( NULL != inode.get() ) {
//                    vector<string> tapes;
//                    inode->GetTapes(tapes);
//                    off_t size;
//                    inode->GetSize(size);
//                    retVal = meta_->DeleteInode(path);
//                    if ( retVal && (! tapes.empty()) ) {
//                        if ( ! Factory::GetTapeManager()->SetTapesUse(
//                                tapes, -1, -size, 0 ) ) {
//                            LogError(boost::join(tapes,",") << " " << size);
//                        }
//                    }
//                }
//            }
//
//            LogDebug(path << " : " << retVal);
//            * ret = xmlrpc_c::value_boolean(retVal);
//        }
//
//    private:
//        fs::path folderCache_;
//        MetaManager * meta_;
//    };


//    class ServiceReleaseTapeMethod : public xmlrpc_c::method
//    {
//    public:
//        ServiceReleaseTapeMethod(
//                fs::path const & folderMeta,
//                fs::path const & folderCache,
//                MetaManager * meta)
//        : folderMeta_(folderMeta), folderCache_(folderCache), meta_(meta)
//        {
//            this->_signature = "b:s";
//            this->_help = string("Release (delete) all files")
//                    + " from meta and cache for the tape";
//        }
//
//        void
//        execute(xmlrpc_c::paramList const & params,
//                xmlrpc_c::value * const ret)
//        {
//            string const tape(params.getString(0));
//            bool bExport(params.getBoolean(1));
//            LogDebug(tape);
//
//            bool deleted;
//            LogDebug("SEARCH: ReleaseTape, tape = " << tape << ", bExport = " << bExport);
//            bool retVal = ScanFolder(tape,folderMeta_,deleted, bExport);
//
//            LogDebug(tape << " : " << retVal);
//            * ret = xmlrpc_c::value_boolean(retVal);
//
////            CleanFolder(folderMeta_,false);
////            CleanFolder(folderCache_,false);
////            CleanFolder( Factory::GetMetaCatalogFolder() / tape, false );
//        }
//
//
//        void
//        CleanFolder(const fs::path & folder,bool me)
//        {
//        	try{
//				fs::directory_iterator end;
//				for ( fs::directory_iterator i(folder); i != end; ++ i ) {
//					if ( fs::is_directory(i->status()) ) {
//						CleanFolder(i->path(),true);
//					}
//				}
//				fs::directory_iterator i(folder);
//				if ( me && fs::is_empty(folder) ) {
//					try {
//						LogDebug(folder);
//						fs::remove(folder);
//					} catch (...) {
//						LogError(folder);
//					}
//				}
//            }catch(const boost::filesystem::filesystem_error& e){
//            	LogError(e.what());
//            }catch(...){
//            	LogError("Exception.");
//            }
//        }
//
//
//        bool
//        ScanFolder(string const & tape,const fs::path & folder,bool & deleted, bool bExport)
//        {
//            bool ret = true;
//            deleted = false;
//            static MetaManagerCatalog meta(Factory::GetMetaManager());
//
//            LogDebug("SEARCH: tape = " << tape << ", folder = " << folder.string() << ", bExport = " << bExport);
//            try{
//
//            fs::directory_iterator end;
//            for ( fs::directory_iterator i(folder); i != end; ++ i ) {
//                string pathname = i->path().string();
//                if ( 0 != pathname.find(folderMeta_.string()) ) {
//                    LogError(pathname);
//                    continue;
//                }
//                string pathInode = pathname.substr(
//                        folderMeta_.string().size() );
//
//                if ( fs::is_directory(i->status()) ) {
//                    bool deletedInFolder = false;
//                    ret = ScanFolder(tape,i->path(),deletedInFolder, bExport) && ret;
//                    if ( fs::is_empty(i->path()) ) {
//                        if ( ! meta.DeleteTapeFolder(tape,pathInode) ) {
//                            LogError(tape << " " << pathInode);
//                        }
//                    }
//                    deleted = deleted || deletedInFolder;
//                    if ( ! fs::is_empty(i->path()) ) {
//                        continue;
//                    }
//                    if ( (! deletedInFolder)
//                            && meta.ExistServiceInode(pathInode) ) {
//                        continue;
//                    }
//                }
//
//
//                if ( fs::is_directory(i->status()) ) {
//                    if ( meta_->DeleteInode(pathInode, bExport) ) {
//                        deleted = true;
//                        Factory::GetCacheManager()->DeleteFolderOperation(
//                                folderCache_ / pathInode );
//                    } else {
//                        LogError(pathname);
//                        ret = false;
//                    }
//                    continue;
//                }
//
//                auto_ptr<Inode> inode( meta_->GetInode(pathInode) );
//                if ( NULL == inode.get() ) {
//                    LogWarn(pathInode);
//                    continue;
//                }
//
//                boost::unique_lock<boost::mutex> lock(mutex_);
//                vector<string> tapes;
//                if ( ! inode->GetTapes(tapes) ) {
//                    LogWarn(pathInode);
//                    continue;
//                }
//                if ( 0 == count(tapes.begin(),tapes.end(),tape) ) {
//                    continue;
//                }
//                tapes.erase(
//                        remove(tapes.begin(),tapes.end(),tape), tapes.end() );
//                if ( ! inode->SetTapes(tapes) ) {
//                    LogWarn(pathInode << " " << tape
//                            << " " << boost::join(tapes,","));
//                }
//                if ( ! tapes.empty() ) {
//                    continue;
//                }
//                lock.unlock();
//
//                off_t size;
//                inode->GetSize(size);
//                inode.release();
//                if ( ! meta_->DeleteInode(pathInode,bExport) ) {
//                    LogError(pathname);
//                    ret = false;
//                    continue;
//                }
//#if 0
//                if (! Factory::GetTapeManager()->SetTapeUse(
//                        tape, -1, -size, 0 ) ) {
//                    LogError(tape << " " << size);
//                }
//#endif
//
//                deleted = true;
//
//                fs::path pathCache = folderCache_ / pathInode;
//                if ( ! fs::exists(pathCache) ) {
//                    continue;
//                }
////                if ( ! Factory::GetCacheManager()->IsUnusedFile(
////                        pathCache, false ) ) {
////                    LogError(pathCache);
////                    ret = false;
////                    continue;
////                }
//                if ( ! Factory::GetCacheManager()->ReleaseFileOperation(
//                        pathCache, false ) ) {
//                    LogError(pathCache);
//                }
//
//                if ( fs::exists(pathCache) ) {
//                    //  Force to delete the file in cache
//                    LogWarn(pathCache);
//                    try {
//                        fs::remove(pathCache);
//                    } catch (...) {
//                        LogError(pathCache);
//                        ret = false;
//                    }
//                }
//            }
//
//            return ret;
//
//            }catch(const boost::filesystem::filesystem_error& e){
//            	LogError(e.what());
//            }catch(...){
//            	LogError("Exception.");
//            }
//            return false;
//        }
//
//    private:
//        fs::path folderMeta_;
//        fs::path folderCache_;
//        MetaManager * meta_;
//        boost::mutex mutex_;
//    };


    void
    ServiceServer::ServiceThread(int handle)
    {
        LogDebug(handle);

        try {
            xmlrpc_c::registry registry;

            xmlrpc_c::methodPtr const methodReleaseFile(
                    new ServiceReleaseFileMethod());
            registry.addMethod(ReleaseFile,methodReleaseFile);

//            xmlrpc_c::methodPtr const methodReleaseInode(
//                    new ServiceReleaseInodeMethod(folderCache_,meta_.get()));
//            registry.addMethod(ReleaseInode,methodReleaseInode);

//            xmlrpc_c::methodPtr const methodReleaseTape(
//                    new ServiceReleaseTapeMethod(
//                        folderMeta_, folderCache_, meta_.get() ) );
//            registry.addMethod(ReleaseTape,methodReleaseTape);

            xmlrpc_c::methodPtr const methodGetCacheCapacity(
                    new ServiceGetCacheCapacityMethod());
            registry.addMethod(GetCacheCapacity, methodGetCacheCapacity );

            xmlrpc_c::methodPtr const methodSetThrottle(
                    new ServiceSetThrottleMethod());
            registry.addMethod(SetThrottle,methodSetThrottle);

//            xmlrpc_c::methodPtr const methodImport(
//                    new ServiceImportMethod(meta_.get()) );
//            registry.addMethod(Import,methodImport);

            xmlrpc_c::methodPtr const methodSetName(
                    new ServiceSetNameMethod() );
            registry.addMethod(SetName,methodSetName);

            xmlrpc_c::methodPtr const methodStopTape(
                    new ServiceStopTapeMethod() );
            registry.addMethod(StopTape,methodStopTape);

            xmlrpc_c::methodPtr const methodSetCacheState(
                    new ServiceSetCacheStateMethod() );
            registry.addMethod(SetCacheState,methodSetCacheState);

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

