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
 * FuseBDTApp.cpp
 *
 *  Created on: Mar 6, 2012
 *      Author: More Zeng
 */


#include "bdt/stdafx.h"
#include "bdt/ServiceServer.h"

using namespace bdt;

#include "FuseBDT.h"
#include "FuseCallback.h"


static void
UsageOutput(const char * base)
{
    cerr << base
            << " $TapeFolder $MetaFolder $CacheFolder $ShareUUID $ShareName"
            << " $TargetFolder [$OtherOptions]"
            << endl;
}


int main(int argc, char *argv[])
{
    if ( argc < 7 ) {
        UsageOutput(argv[0]);
        return 1;
    }

    umask(0);

    string tape(argv[1]);
    string meta(argv[2]);
    string cache(argv[3]);
    string service(argv[4]);
    string name(argv[5]);
    string target(argv[6]);
    fs::path folderTape(fs::system_complete(fs::path(tape)));
    fs::path folderMeta(fs::system_complete(fs::path(meta)));
    fs::path folderCache(fs::system_complete(fs::path(cache)));
    fs::path folderTarget(fs::system_complete(fs::path(target)));
    if ( ! fs::is_directory(folderTape) ) {
        cerr << "$TapeFolder: " << tape
                << " does not exist" << endl;
        return 2;
    }
    if ( ! fs::is_directory(folderCache) ) {
        cerr << "$CacheFolder: " << cache
                << " does not exist" << endl;
        return 2;
    }
    if ( ! fs::is_directory(folderMeta) ) {
        cerr << "$MetaFolder: " << meta
                << " does not exist" << endl;
        return 2;
    }
    if ( ! fs::is_directory(folderTarget) ) {
        cerr << "$TargetFolder: " << target
                << " does not exist" << endl;
        return 2;
    }

    for (int i=6; i<argc; ++i) {
        argv[i-5] = argv[i];
    }
    argc = argc - 5;

    struct fuse_args args = FUSE_ARGS_INIT(argc,argv);
    if ( fuse_opt_parse(&args,NULL,NULL,NULL) < 0 ) {
        cerr << "Fail to parse the fuse mount options" << endl;
        return 3;
    }
    if ( fuse_opt_add_arg(&args,"-oallow_other") < 0 ) {
        cerr << "Fail to add allow_other fuse mount option" << endl;
        return 3;
    }
    if ( fuse_opt_add_arg(&args,"-odefault_permissions") < 0 ) {
        cerr << "Fail to add default_permissions fuse mount option" << endl;
        return 3;
    }
    if ( fuse_opt_add_arg(&args,"-odirect_io") < 0 ) {
        cerr << "Fail to add direct_io fuse mount option" << endl;
        return 3;
    }
#if FUSE_VERSION >= 28
    if ( fuse_opt_add_arg(&args,"-obig_writes") < 0 ) {
        cerr << "Fail to add big_writes fuse mount option" << endl;
        return 3;
    }
#endif

    Factory::SetMetaFolder(folderMeta);
    Factory::SetCacheFolder(folderCache);
    Factory::SetTapeFolder(folderTape);
    Factory::SetService(service);
    Factory::SetName(name);

    string strPath = folderTape.string();
    int pos = strPath.rfind('/', 1);
    if(-1 != pos){
    	string uuid = strPath.substr(pos + 1, strPath.length() - 1);
        Factory::SetUuid(uuid);
    }


    FuseBDT * bdt = new FuseBDT();
    FuseCallback::Instance()->SetBase(bdt);
#if FUSE_VERSION >= 28
    FuseCallback::FuseOperations.flag_nullpath_ok = 0;
#endif

    int fuse_stat = fuse_main(
            args.argc, args.argv,
            &FuseCallback::FuseOperations, bdt->GetState() );

    return fuse_stat;
}
