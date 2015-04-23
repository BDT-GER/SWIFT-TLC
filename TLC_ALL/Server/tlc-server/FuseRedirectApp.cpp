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
 * FuseRedirectApp.cpp
 *
 *  Created on: Feb 22, 2012
 *      Author: More Zeng
 */


#include <stdlib.h>

#include <iostream>

using namespace std;

#include "FuseHeader.h"
#include "FuseCallback.h"
#include "FuseRedirect.h"


static void UsageOutput(const char * base)
{
    std::cerr << base << " $SourceDirectory $TargetDirectory\n";
}

int main(int argc, char *argv[])
{
    int i;
    for ( i = 1; (i < argc) && (argv[i][0] == '-'); ++i ) {
    }

    if ( (argc - i) != 2 ) {
        UsageOutput(argv[0]);
        return 1;
    }

    umask(0);

    char * path = realpath(argv[i],NULL);
    if ( path == NULL ) {
        UsageOutput(argv[0]);
        return 1;
    }
    std::string pathname = path;
    free(path);

    for (int m=i+1; m<argc; ++m) {
        argv[m-1] = argv[m];
    }
    argc = argc - 1;

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

    FuseRedirect * redirect = new FuseRedirect(pathname);
    FuseCallback::Instance()->SetBase(redirect);

    int fuse_stat = fuse_main(args.argc, args.argv, &FuseCallback::FuseOperations, redirect->GetState());

    return fuse_stat;
}

