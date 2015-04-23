/*
 * ltfsTaskAutoClean.h
 *
 *  Created on: Dec 18, 2012
 *      Author: Sam Chen
 */

#ifndef __LTFSTASKAUTOCLEAN_H__
#define __LTFSTASKAUTOCLEAN_H__

#include "ltfsTask.h"

namespace ltfs_soapserver
{
	class TaskAutoClean: public Task
	{
	public:
		TaskAutoClean(const string& changerSerial, const vector<string> driveSerials);
		virtual ~TaskAutoClean();
		virtual void Execute();

	private:
#ifdef LTFS_VSB_MODE
		string FindValidCleaningTape(int& cleaningStatus);
#else
		string FindValidCleaningTape();
#endif

	private:
		string									m_changerSerial;
		string									m_driveSerial;
		vector<string>							m_driveSerials;

		static map<string, bool>				driveCleaningTaskMap_;
        static boost::mutex        				driveCleaningTaskMutex_;
	};

} /* namespace ltfs_soapserver */
#endif /* __LTFSTASKAUTOCLEAN_H__ */
