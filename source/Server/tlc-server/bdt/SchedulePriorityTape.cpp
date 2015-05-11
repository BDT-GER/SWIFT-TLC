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
 * SchedulePriorityTape.cpp
 *
 *  Created on: Jul 17, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "SchedulePriorityTape.h"
#include "PriorityTapeGroup.h"


namespace bdt
{

    SchedulePriorityTape::SchedulePriorityTape(ResourceTape * resource)
    : path_(Factory::GetTapeFolder()), resource_(resource),
      scheduleTime_(boost::posix_time::microsec_clock::local_time())
    {
        thread_.reset( new boost::thread(
                &SchedulePriorityTape::ScheduleTask, this) );
    }


    SchedulePriorityTape::~SchedulePriorityTape()
    {
        thread_->interrupt();
        thread_->join();

        for ( PriorityTapeMap::iterator i = tapes_.begin();
                i != tapes_.end();
                ++ i ) {
            delete i->second;
            i->second = NULL;
        }
    }


    bool
    SchedulePriorityTape::RequestResource(
            const vector<string> & tapes, vector<PriorityTape *> & priorities )
    {
        priorities.clear();

        for ( vector<string>::const_iterator i = tapes.begin();
                i != tapes.end();
                ++ i ) {
            PriorityTapeMap::iterator iter = tapes_.find(*i);
            if ( tapes_.end() == iter ) {
                iter = tapes_.insert( PriorityTapeMap::value_type(
                        *i, new PriorityTape() ) ).first;
            }
            priorities.push_back(iter->second);
        }

        return true;
    }


    bool
    SchedulePriorityTape::RequestTapes(
            const vector<string> & tapes, bool mount,
            bool share, int timeout, int priority )
    {
        string strTapes = boost::join(tapes,",");

        if ( priority < 0 ) {
            LogError(strTapes << " " << mount
                    << " " << share << " " << timeout << " " << priority);
            return false;
        }

        LogDebug("Request tapes: " << strTapes << " " << mount
                << " " << share << " " << timeout << " " << priority);

        boost::unique_lock<boost::mutex> lock(mutex_);

        vector<PriorityTape *> priorities;
        if ( ! RequestResource(tapes,priorities) ) {
            LogError(strTapes << " : " << false);
            return false;
        }

        Schedule(tapes,priority);

        lock.unlock();

        PriorityTapeGroup group(tapes,priorities);
        bool ret = group.Request(share,timeout,priority);
        LogDebug("Request " << strTapes << " : " << ret);
        if ( ret && mount ) {
            boost::this_thread::disable_interruption disable;
            if ( resource_->MountTapes(tapes) ) {
                LogDebug("Mount " << strTapes << " : " << true);
                return true;
            } else {
                LogDebug("Mount " << strTapes << " : " << false);
                ReleaseTapes(tapes,share);
                return false;
            }
            return false;
        } else {
            return ret;
        }
    }


    void
    SchedulePriorityTape::ReleaseTapes(
            const vector<string> & tapes, bool share )
    {
        LogDebug("Release tapes: " << boost::join(tapes,",") << " " << share);

        boost::lock_guard<boost::mutex> lock(mutex_);

        for ( vector<string>::const_iterator i = tapes.begin();
                i != tapes.end();
                ++ i ) {
            PriorityTapeMap::iterator iTape = tapes_.find(*i);
            if ( tapes_.end() == iTape ) {
                LogError(*i);
                continue;
            }
            if ( ! share ) {
                iTape->second->Enable(false);
            }
            iTape->second->Release(share);
        }

        Schedule();
    }


    typedef multimap<int,string,greater<int> > TapeMap;


    struct PriorityItem
    {
        int number;
        vector<string> tapes;
        PriorityTape * priority;
        bool busy;
        bool stop;
    };
    typedef map<string,PriorityItem> PriorityMap;


    static void
    StopTapes(PriorityMap & map,vector<string> tapes,int priority)
    {
        for ( vector<string>::iterator i = tapes.begin();
                i != tapes.end();
                ++ i ) {
            PriorityMap::iterator iter = map.find(*i);
            if ( map.end() == iter ) {
                LogError(*i);
                continue;
            }
            if ( iter->second.number > priority ) {
                LogWarn(*i << " " << iter->second.number << " " << priority);
                continue;
            }
            LogDebug("Stop tape " << *i);
            iter->second.stop = true;
        }
    }


    void
    SchedulePriorityTape::Schedule(const vector<string> & tapes, int priority)
    {
        LogDebug(priority << " " << boost::join(tapes,","));

        scheduleTime_ = boost::posix_time::microsec_clock::local_time();

        TapeMap tapeQueue;
        PriorityMap priorityQueue;
        for ( PriorityTapeMap::iterator i = tapes_.begin();
                i != tapes_.end();
                ++ i ) {
            if ( NULL == i->second ) {
                return;
            }
            bool busy = false;
            if ( i->second->Busy() ) {
                LogDebug("Busy tape " << i->first);
                busy = true;
            } else {
                i->second->Enable(false);
                if ( i->second->Busy() ) {
                    LogError("Busy tape " << i->first);
                    busy = true;
                }
            }

            vector<string> tapesInGroup;
            int numberPriority = i->second->Priority(tapesInGroup);
            if ( numberPriority < 0 && (! busy) ) {
                if ( resource_->StopTape(i->first) ) {
                    LogDebug("Success to stop tape " << i->first);
                } else {
                    LogDebug("Failure to stop tape " << i->first);
                }
            }

            bool toSchedule =
                    (tapes.end() != find(tapes.begin(),tapes.end(),i->first));

            PriorityItem priorityItem;
            priorityItem.number = numberPriority;
            priorityItem.tapes = tapesInGroup;
            if ( toSchedule && (priority > numberPriority) ) {
                LogDebug("Wait tape (origin): " << i->first
                        << " " << priorityItem.number
                        << " " << boost::join(priorityItem.tapes,","));
                priorityItem.number = priority;
                priorityItem.tapes = tapes;
            }
            priorityItem.priority = i->second;
            priorityItem.busy = busy;
            priorityItem.stop = false;
            LogDebug("Wait tape: " << i->first
                    << " " << priorityItem.number
                    << " " << boost::join(priorityItem.tapes,","));

            tapeQueue.insert( TapeMap::value_type(
                    priorityItem.number, i->first ) );
            priorityQueue.insert( PriorityMap::value_type(
                    i->first, priorityItem ) );
        }

        for ( TapeMap::iterator i = tapeQueue.begin();
                (i != tapeQueue.end()) && (i->first >=0 );
                ++ i ) {
            LogDebug("Handle tape: " << i->second << " " << i->first);

            PriorityMap::iterator item = priorityQueue.find(i->second);
            if ( priorityQueue.end() == item ) {
                LogError(i->second);
                continue;
            }
            if ( item->second.stop ) {
                LogDebug("Ignore stop tape " << item->first);
                continue;
            }
            if ( item->second.busy ) {
                item->second.priority->Enable(true);
                LogDebug("Enable busy tape " << item->first);
                continue;
            }

            ResourceTape::TapesInUseMap tapesInUse;
            vector<string> tapes;
            if ( item->second.tapes.empty() ) {
                tapes.push_back(item->first);
            } else {
                tapes = item->second.tapes;
            }
            int ret = resource_->StartTapes(tapes,tapesInUse, priority);
            LogDebug("Start tapes: " << boost::join(tapes,",") << " " << ret);

            if ( ret == ResourceTape::START_RETURN_NO_RESOURCE ) {

                for ( ResourceTape::TapesInUseMap::iterator iTape
                        = tapesInUse.begin();
                        iTape != tapesInUse.end();
                        ++ iTape ) {
                    for ( vector<string>::iterator iStop
                            = iTape->second.begin();
                            iStop != iTape->second.end();
                            ++ iStop ) {
                        PriorityMap::iterator iter(priorityQueue.find(*iStop));
                        if ( iter == priorityQueue.end() ) {
                            LogError("No exist tape " << *iStop);
                            continue;
                        }
                        if ( iter->second.number < i->first ) {
                            iter->second.stop = true;
                        }
                        if ( iter->second.busy ) {
                            LogDebug("Ignore busy tape " << *iStop);
                            continue;
                        }
                        if ( iter->second.number < i->first ) {
                            iter->second.stop = true;
                            if ( resource_->StopTape(*iStop) ) {
                                LogDebug("Success to stop tape " << *iStop);
                            } else {
                                LogDebug("Failure to stop tape " << *iStop);
                            }
                            StopTapes( priorityQueue,
                                    iter->second.tapes, iter->second.number );
                        }
                    }
                }

                ret = resource_->StartTapes(tapes,tapesInUse, priority);
                LogDebug("Start tapes: " <<boost::join(tapes,",") <<" " <<ret);
            }

            if ( ret == ResourceTape::START_RETURN_SUCCESS ) {
                item->second.priority->Enable(true);
                LogDebug("Enable the tape " << item->first);
                continue;
            }

        }

        return;
    }


    void
    SchedulePriorityTape::ScheduleTask()
    {
        try {
            int interval = 1000;
            while (true) {
                boost::this_thread::sleep(
                        boost::posix_time::milliseconds(interval) );

                boost::lock_guard<boost::mutex> lock(mutex_);

                boost::posix_time::ptime current =
                        boost::posix_time::microsec_clock::local_time();
                int duration = (current - scheduleTime_).total_milliseconds();
                if ( duration < 800 ) {
                    interval = 1000 - duration;
                    continue;
                }

                LogDebug("Schedule");
                Schedule();
                interval = 1000;
            }
        } catch (const boost::thread_interrupted & e) {
        }
    }

}
