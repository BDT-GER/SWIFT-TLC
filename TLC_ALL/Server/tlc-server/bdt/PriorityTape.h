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
 * PriorityTape.h
 *
 *  Created on: Jul 17, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class PriorityTape
    {
    public:
        PriorityTape();

        virtual
        ~PriorityTape();

        bool
        Request( const vector<string> & tapes,
                bool share,int timeout,int priority );

        bool
        Request(bool share,int timeout,int priority)
        {
            vector<string> tapes;
            return Request(tapes,share,timeout,priority);
        }

        void
        Release(bool share);

        bool
        Enable(bool enable);

        bool
        Enabled()
        {
            return enable_;
        }

        int
        Priority()
        {
            boost::lock_guard<boost::mutex> lock(mutex_);

            if ( data_.empty() ) {
                return -1;
            } else {
                return data_.begin()->priority;
            }
        }

        int
        Priority(vector<string>& tapes)
        {
            tapes.clear();

            boost::lock_guard<boost::mutex> lock(mutex_);

            if ( data_.empty() ) {
                return -1;
            } else {
                tapes = data_.begin()->tapes;
                return data_.begin()->priority;
            }
        }

        bool
        Busy(bool checkIdle = true)
        {
            static int idle = (int)Factory::GetConfigure()->GetValueSize(
                        Configure::TapeIdleTime );

            boost::lock_guard<boost::mutex> lock(mutex_);

            if ( reference_ > 0 ) {
                return true;
            } else {
                if ( ! checkIdle ) {
                    return false;
                }
                int duration =
                        (boost::posix_time::microsec_clock::local_time()
                            - time_).total_seconds();
                if ( duration < idle ) {
                    return true;
                } else {
                    return false;
                }
            }
        }

        void
        EnableDump(bool enable)
        {
            enableDump_ = enable;
        }

    private:
        bool
        RequestForShare(
                const vector<string> & tapes,
                int timeout, int priority );

        bool
        RequestNotShare(
                const vector<string> & tapes,
                int timeout, int priority );

        bool enable_;

        bool busy_;
        int reference_;
        boost::mutex mutex_;
        boost::condition_variable condition_;
        boost::posix_time::ptime time_;

        int priorityFile_;
        int idleFile_;
        boost::posix_time::ptime timeFile_;

        class RequestData
        {
        public:
            RequestData(
                    int _priority,
                    const boost::posix_time::ptime & _time,
                    const vector<string> & _tapes )
            : priority(_priority), time(_time), tapes(_tapes)
            {
            }

            RequestData(const RequestData & data)
            : priority(data.priority), time(data.time), tapes(data.tapes)
            {
            }

            RequestData &
            operator = (const RequestData & data)
            {
                priority = data.priority;
                time = data.time;
                tapes = data.tapes;
                return * this;
            }

            int priority;
            boost::posix_time::ptime time;
            vector<string> tapes;
        };

        class RequestDataCompare
        {
        public:
            bool
            operator() (
                    const RequestData & data1,
                    const RequestData & data2)
            {
                if ( data1.priority > data2.priority ) {
                    return true;
                }
                if ( data1.priority < data2.priority ) {
                    return false;
                }
                if ( data1.time > data2.time ) {
                    return true;
                }
                if ( data1.time < data2.time ) {
                    return false;
                }
                return data1.tapes > data1.tapes;
            }

            static bool
            Equal(
                    const RequestData & data1,
                    const RequestData & data2)
            {
                if ( data1.priority != data2.priority ) {
                    return false;
                }
                if ( data1.time != data2.time ) {
                    return false;
                }
                return data1.tapes == data2.tapes;
            }
        };

        bool CheckTape(RequestData & data,int & wait);

        typedef multiset<RequestData,RequestDataCompare> DataSet;
        DataSet data_;

        static boost::mutex mutexDump_;

        bool enableDump_;

        void
        DumpData(string output)
        {
#ifdef DEBUG
            if ( ! enableDump_ ) {
                return;
            }

            boost::lock_guard<boost::mutex> lock(mutexDump_);

            LogDebug(output << " " << busy_);
            for ( DataSet::iterator i = data_.begin();
                    i != data_.end();
                    ++ i) {
                LogDebug(i->priority << " " << i->time
                        << " " << boost::join(i->tapes,","));
            }
#endif
        }

    };

}

