/*
 * ltfsTaskManagement.h
 *
 *  Created on: Aug 28, 2012
 *      Author: chento
 */

#ifndef __LTFSTASKMANAGEMENT_H__
#define __LTFSTASKMANAGEMENT_H__

#include "stdafx.h"
#include "ltfsTaskLoadTape.h"
#include "ltfsTaskAutoClean.h"
#include "ltfsTaskCheckTape.h"
#include "ltfsTaskAuditTape.h"
#include "ltfsTaskDeleteTapeFile.h"

#ifndef __LTFS_SOAP_H__
#define __LTFS_SOAP_H__
#include "ltfs.h"
#endif

namespace ltfs_soapserver
{
	enum DeleteFilesOnTapeStatus
	{
		DF_NOT_RUN = 0,
		DF_RUNNING = 1,
		DF_FINISHED = 2,
		DF_FAILED = 3
	};
	class TaskManagement
	{
	public:
		static
		TaskManagement* GetInstance();

		bool
		LoadTaskQueue();

		bool
		SaveTaskQueue(bool serverStoped = false);

		bool
		AddTask(Task* pTask, bool needSave=true);

		bool
		QueryStatus(struct QueryTaskRslt& result);

		bool
		QueryLoadStatus();

		bool
		CancelTask(string &uid, TaskType type);

		void
		TaskCleaner();

		void
		TaskTapeAuditTrigger();

		bool
		GetServerStop();

		bool IsTaskRunning(const string& barcode, TaskType taskType);
		bool GetRunningTaskTapes(TaskType taskType, vector<string>& runningTapes);

		DeleteFilesOnTapeStatus GetTapeDeleteFileStatus(const string& barcode);
		void SetTapeDeleteFileStatus(const string& barcode, DeleteFilesOnTapeStatus status);

	private:

		TaskManagement();
		~TaskManagement();

		void
		LoadTaskFrFile();

		void
		RecoverTask(boost::property_tree::ptree & taskTree);

		void
		RecoverFormat(boost::property_tree::ptree & formatTree);

		time_t
		GetLastStartTimeFromFile();

		void
		SetLastStartTimeToFile(time_t lastStartTime);

		void
		SaveTaskToFile();

		void RefreshTapeList(map<string, string>& tapeShareMap,
			map<string, time_t>& lastDeleteMap, map<string, UInt64_t>& pendingNumMap, time_t& refreshTime);
		void TaskTapeDeleteFile();

	private:
		static TaskManagement* 			 mInstance;
		static boost::mutex 			 mMutex;

		std::vector<Task*> 				 taskList_;
		boost::mutex 					 listMutex_;
		boost::scoped_ptr<boost::thread> threadPtr_;
		boost::scoped_ptr<boost::thread> auditPtr_;
		boost::scoped_ptr<boost::thread> deleteFilesPtr_;

		boost::mutex 			 		deleteFileOnTapeMapMutex_;
		map<string, DeleteFilesOnTapeStatus>	deleteFilesOnTapeMap_; //

		bool stop_;

	};
}


#endif /* LTFSTASKMANAGEMENT_H_ */
