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
 * ScheduleProxyServer.cpp
 *
 *  Created on: Oct 19, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ScheduleProxyServer.h"
#include "SchedulePriorityTape.h"
#ifdef MORE_TEST
#include "ResourceTapeSimulator.h"
#else
#include "ResourceTapeLTFS.h"
#endif


namespace bdt
{

    string const ScheduleProxyServer::Service("Schedule");
    string const ScheduleProxyServer::Request("Schedule.Request");
    string const ScheduleProxyServer::Release("Schedule.Release");
    string const ScheduleProxyServer::Interrupt("Schedule.Interrupt");
    Configure * config = Factory::GetConfigure();
    static const time_t NUMBER_SECTIONS = 10;

    static map<string, time_t>			readStart_;
    static map<string, map<time_t, time_t> >	readTimeMap_;
    static boost::shared_mutex			readActMutex_;
    static time_t 						timeStart_ = time(NULL);


    void ClearExpiredRecords(const string& barcode);
    void ReadStart(const string& barcode);
    void ReadEnd(const string& barcode);


    ScheduleProxyServer::ScheduleProxyServer()
    : SocketServer(Service),
#ifdef MORE_TEST
      schedule_(new SchedulePriorityTape(new ResourceTapeSimulator())),
#else
      schedule_(new SchedulePriorityTape(new ResourceTapeLTFS())),
#endif
      account_(schedule_)
    {
        Factory::ResetSchedule(schedule_);
    }


    ScheduleProxyServer::~ScheduleProxyServer()
    {
    }


    unsigned int ScheduleProxyServer::RecentReadActPercentage(const string& barcode)
    {
    	boost::unique_lock<boost::shared_mutex> lock(readActMutex_);
    	ClearExpiredRecords(barcode);
		map<string, map<time_t, time_t> >::iterator itTape = readTimeMap_.find(barcode);
		if(itTape == readTimeMap_.end()){
			LogDebug("RecentReadActPercentage: no record found for tape " << barcode);
			return 0;
		}
	    static const time_t IgnoreWriteByReadCheckTime = Factory::GetConfigure()->GetValueSize(Configure::IgnoreWriteByReadCheckTime);
	    static const time_t SECONDS_PER_INDEX  =  (IgnoreWriteByReadCheckTime > NUMBER_SECTIONS)? (IgnoreWriteByReadCheckTime / NUMBER_SECTIONS) : 1;
    	time_t timeRange = SECONDS_PER_INDEX * NUMBER_SECTIONS;
    	if(timeRange > time(NULL) - timeStart_){
    		timeRange = time(NULL) - timeStart_;
    	}
    	int secNum = 0;
    	time_t timeRead = 0;
    	time_t indexStart = 0;
    	time_t indexEnd = 0;
    	for( map<time_t, time_t>::iterator it = itTape->second.begin(); it != itTape->second.end(); it++){
    		if(it == itTape->second.begin()){
    			indexStart = it->first;
    		}
    		indexEnd = it->first;
    		timeRead += it->second;
    		secNum++;
    	}
    	if(readStart_.find(barcode) != readStart_.end()){
    		struct timeval tvNow;
    		gettimeofday(&tvNow, NULL);
    		time_t tNow = tvNow.tv_sec * 1000000 + tvNow.tv_usec;
    		timeRead += tNow - readStart_[barcode];
    	}
		LogDebug("RecentReadActPercentage: timeRead = " << timeRead << ", timeRange = "
				<< (timeRange * 1000000) << ", secNum = " << secNum << ", indexStart = " << indexStart << ", indexEnd = " << indexEnd);
    	int percent = (timeRead * 100/(timeRange * 1000000));
    	LogDebug("RecentReadActPercentage, tape = " << barcode << ", percent = " << percent);
    	if(percent > 100){
    		percent = 100;
    	}
    	return percent;
    }

    void ClearExpiredRecords(const string& barcode)
    {
		map<string, map<time_t, time_t> >::iterator itTape = readTimeMap_.find(barcode);
		if(itTape == readTimeMap_.end()){
			LogDebug("RecentReadActPercentage: no record found for tape " << barcode);
			return;
		}
	    static const time_t IgnoreWriteByReadCheckTime = Factory::GetConfigure()->GetValueSize(Configure::IgnoreWriteByReadCheckTime);
	    static const time_t SECONDS_PER_INDEX  =  (IgnoreWriteByReadCheckTime > NUMBER_SECTIONS)? (IgnoreWriteByReadCheckTime / NUMBER_SECTIONS) : 1;
		time_t tNow = time(NULL);
		time_t curIndex = tNow / SECONDS_PER_INDEX;
		map<time_t, time_t>  tapeRecords = itTape->second;
		for( map<time_t, time_t>::iterator it = tapeRecords.begin(); it != tapeRecords.end();){
			//LogDebug("RecentReadActPercentage: curIndex = " << curIndex << "it->first = " << it->first);
			if(curIndex - it->first >= NUMBER_SECTIONS){
				//LogDebug("RecentReadActPercentage: erase " << it->first << "->" << it->second);
				tapeRecords.erase(it++);
			}else{
				it++;
			}
		}//for
		readTimeMap_[barcode] = tapeRecords;
    }

    void ReadStart(const string& barcode)
    {
    	boost::unique_lock<boost::shared_mutex> lock(readActMutex_);
		LogDebug("RecentReadActPercentage: start for tape " << barcode);
		struct timeval tvNow;
		gettimeofday(&tvNow, NULL);
    	readStart_[barcode] = tvNow.tv_sec * 1000000 + tvNow.tv_usec;
    }

    void ReadEnd(const string& barcode)
    {
    	boost::unique_lock<boost::shared_mutex> lock(readActMutex_);
    	ClearExpiredRecords(barcode);

    	if (readStart_.find(barcode) == readStart_.end()){
    		LogDebug("RecentReadActPercentage: Not found start record for " << barcode);
    		return;
    	}
	    static const time_t IgnoreWriteByReadCheckTime = Factory::GetConfigure()->GetValueSize(Configure::IgnoreWriteByReadCheckTime);
	    static const time_t SECONDS_PER_INDEX  =  (IgnoreWriteByReadCheckTime > NUMBER_SECTIONS)? (IgnoreWriteByReadCheckTime / NUMBER_SECTIONS) : 1;
	    //LogDebug("GetValueSize:   IgnoreWriteByReadCheckTime = " << IgnoreWriteByReadCheckTime << ", SECONDS_PER_INDEX = " << SECONDS_PER_INDEX);

		time_t curIndex = time(NULL) / SECONDS_PER_INDEX;
		struct timeval tvNow;
		gettimeofday(&tvNow, NULL);
		time_t tNow = tvNow.tv_sec * 1000000 + tvNow.tv_usec;
		map<time_t, time_t>  tapeRecords;
		map<string, map<time_t, time_t> >::iterator itTape = readTimeMap_.find(barcode);
		if(itTape != readTimeMap_.end()){
			tapeRecords = itTape->second;
		}
		if(tapeRecords.find(curIndex) == tapeRecords.end()){
			tapeRecords[curIndex] = 0;
		}
		time_t timeDiff = tNow - readStart_[barcode];
		/*static int chkNum = 0;
		if(timeDiff <= 0 && chkNum++ > 20){
			timeDiff = 1;
			chkNum = 0;
		}*/
		tapeRecords[curIndex] += timeDiff;
		LogDebug("RecentReadActPercentage: curIndex = " << curIndex << ", timeDiff = " << timeDiff
				<< ", tapeRecords[curIndex] = " << tapeRecords[curIndex] << ", tape = " << barcode);
		readTimeMap_[barcode] = tapeRecords;
		readStart_.erase(barcode);
    }

    class ScheduleReleaseMethod : public xmlrpc_c::method
    {
//        void ScheduleInterface::ReleaseTapes(
//                const vector<string> & tapes, bool share );

    public:
        ScheduleReleaseMethod(
                ScheduleInterface * schedule,ScheduleAccount * account)
        : schedule_(schedule), account_(account)
        {
            this->_signature = "b:sb";
            this->_help = "SchedulePriorityTape::Release";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            pid_t const pid(params.getInt(0));
            xmlrpc_c::carray const data(params.getArray(1));
            vector<string> tapes;
            for ( xmlrpc_c::carray::const_iterator i = data.begin();
                    i != data.end();
                    ++ i ) {
                tapes.push_back(xmlrpc_c::value_string(*i));
            }
            bool const share(params.getBoolean(2));
            LogDebug(boost::join(tapes,",") << " " << share);
            schedule_->ReleaseTapes(tapes,share);
            for ( vector<string>::iterator i = tapes.begin();
                    i != tapes.end();
                    ++ i ) {
                account_->DeleteSchedule(pid,*i,share);
                ReadEnd(*i);
            }
            * ret = xmlrpc_c::value_boolean(true);
        }

    private:
        ScheduleInterface * schedule_;
        ScheduleAccount * account_;

    };


    class ScheduleRequestMethod : public xmlrpc_c::method
    {
//        bool RequestTapes( const vector<string> & tapes, bool mount,
//                bool share, int timeout, int priority);

    public:
        ScheduleRequestMethod(
                ScheduleProxyServer * server,
                ScheduleInterface * schedule,
                ScheduleAccount * account)
        : server_(server), schedule_(schedule), account_(account)
        {
            this->_signature = "b:sii";
            this->_help = "SchedulePriorityTape::Request";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            pid_t const pid(params.getInt(0));
            string const seed(params.getString(1));
            boost::thread::id current = boost::this_thread::get_id();
            server_->InsertSeed(seed,current);
            LogDebug(pid << " " << seed);

            xmlrpc_c::carray const data(params.getArray(2));
            vector<string> tapes;
            for ( xmlrpc_c::carray::const_iterator i = data.begin();
                    i != data.end();
                    ++ i ) {
                tapes.push_back(xmlrpc_c::value_string(*i));
            }
            bool const mount(params.getBoolean(3));
            bool const share(params.getBoolean(4));
            int const timeout(params.getInt(5));
            int const priority(params.getInt(6));
            LogDebug(boost::join(tapes,",") << " " << mount
                    << " " << timeout << " " << priority);

            bool retValue = schedule_->RequestTapes(
                    tapes, mount, share, timeout, priority );
            LogDebug(boost::join(tapes,",") << " " << mount
                    << " " << timeout << " " << priority
                    << " : " << retValue);
    		//LogDebug("RecentReadActPercentage: retValue = " << retValue << ". " << boost::join(tapes,",") << ", priority = " << priority << ":" << ScheduleInterface::PRIORITY_PREREAD << ":" << ScheduleInterface::PRIORITY_READ);
            if ( retValue ) {
                for ( vector<string>::iterator i = tapes.begin();
                        i != tapes.end();
                        ++ i ) {
                    account_->InsertSchedule(pid,*i,share);
                    if(priority == ScheduleInterface::PRIORITY_PREREAD || priority == ScheduleInterface::PRIORITY_READ){
                		//LogDebug("RecentReadActPercentage: retValue = " << retValue << ". " << *i << ", priority = " << priority);
                    	ReadStart(*i);
                    }
                }
            }
            server_->RemoveSeed(seed);
            * ret = xmlrpc_c::value_boolean(retValue);
        }

    private:
        ScheduleProxyServer * server_;
        ScheduleInterface * schedule_;
        ScheduleAccount * account_;

    };


    class ScheduleInterruptMethod : public xmlrpc_c::method
    {

    public:
        ScheduleInterruptMethod( ScheduleProxyServer * server )
        : server_(server)
        {
            this->_signature = "b:s";
            this->_help = "Interrupt a previous call";
        }

        void
        execute(xmlrpc_c::paramList const & params,
                xmlrpc_c::value * const ret)
        {
            string const seed(params.getString(0));
            LogDebug(seed);
            bool retValue = server_->InterruptThread(seed);
            * ret = xmlrpc_c::value_boolean(retValue);
        }

    private:
        ScheduleProxyServer * server_;
    };


    bool
    ScheduleProxyServer::InterruptThread(const string & seed)
    {
        boost::thread::id id;

        {
            boost::lock_guard<boost::mutex> lock(mutex_);

            SeedList::iterator i = seeds_.find(seed);
            if ( i == seeds_.end() ) {
                return false;
            }

            id = i->second;
        }

        boost::lock_guard<boost::mutex> lock(mutexThread_);
        ThreadList::iterator i = threads_.find(id);
        if ( i == threads_.end() ) {
            return false;
        }

        i->second->interrupt();

        return true;
    }


    bool
    ScheduleProxyServer::InsertSeed(
            const string & seed, const boost::thread::id & id )
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        return seeds_.insert(SeedList::value_type(seed,id)).second;
    }


    bool
    ScheduleProxyServer::RemoveSeed(const string & seed)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        SeedList::iterator i = seeds_.find(seed);
        if ( i == seeds_.end() ) {
            return false;
        } else {
            seeds_.erase(i);
            return true;
        }
    }


    void
    ScheduleProxyServer::ServiceThread(int handle)
    {
        LogDebug(handle);

        try {
            xmlrpc_c::registry registry;
            xmlrpc_c::methodPtr const methodRelease(
                    new ScheduleReleaseMethod(schedule_,&account_));
            registry.addMethod(Release,methodRelease);
            xmlrpc_c::methodPtr const methodRequest(
                    new ScheduleRequestMethod(this,schedule_,&account_));
            registry.addMethod(Request,methodRequest);
            xmlrpc_c::methodPtr const methodInterrupt(
                    new ScheduleInterruptMethod(this));
            registry.addMethod(Interrupt,methodInterrupt);

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
