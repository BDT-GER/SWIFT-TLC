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
 * SocketServer.cpp
 *
 *  Created on: Oct 22, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "SocketServer.h"


namespace bdt
{

    SocketServer::SocketServer(const string & service)
    : service_(service),
      run_(true),
      thread_(boost::thread(&SocketServer::ServerTask,this))
    {
    }


    SocketServer::~SocketServer()
    {
        run_ = false;
        thread_.join();
        for ( ThreadList::iterator i = threads_.begin();
                i != threads_.end();
                ++ i ) {
            i->second->interrupt();
        }
        for ( ThreadList::iterator i = threads_.begin();
                i != threads_.end();
                ++ i ) {
            i->second->join();
            delete i->second;
            i->second = NULL;
        }
    }


    void
    SocketServer::ServerThread(int handle)
    {
        boost::thread::id current = boost::this_thread::get_id();

        while ( true ) {
            boost::unique_lock<boost::mutex> lock(mutexThread_);
            if ( threads_.find(current) != threads_.end() ) {
                break;
            }
            lock.unlock();
            boost::this_thread::yield();
        }

        ServiceThread(handle);

        boost::lock_guard<boost::mutex> lock(mutexThread_);
        ThreadList::iterator i = threads_.find(current);
        if ( i != threads_.end() ) {
            delete i->second;
            i->second = NULL;
            threads_.erase(i);
        } else {
            LogError(current);
        }
    }


    void
    SocketServer::ServerTask()
    {
        signal(SIGPIPE,SIG_IGN);

        int handleListen = Factory::SocketServerHandle(service_);
        if ( handleListen == -1 ) {
            LogError(service_);
            return;
        }

        while ( run_ ) {
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(handleListen,&fds);
            int ret = select(handleListen+1,&fds,NULL,NULL,&timeout);
            if ( ret <= 0 ) {
                continue;
            }
            if ( ! FD_ISSET(handleListen,&fds) ) {
                continue;
            }

            int handle = accept(handleListen,NULL,NULL);
            if ( handle >= 0 ) {
                boost::lock_guard<boost::mutex> lock(mutexThread_);
                boost::thread * thread = new boost::thread(
                        &SocketServer::ServerThread, this, handle);
                if ( ! threads_.insert( ThreadList::value_type(
                        thread->get_id(), thread ) ).second ) {
                    LogError(thread->get_id());
                }
            } else {
                LogError(service_ << " accept error");
            }
        }

        close(handleListen);

        return;
    }

}

