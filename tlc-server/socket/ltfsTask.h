#ifndef __LTFSTASK_H__
#define __LTFSTASK_H__

#include "stdafx.h"
#include "../bdt/Factory.h"
#include "../ltfs_management/TapeLibraryMgr.h"
#include "../ltfs_management/TapeDbManager.h"

namespace ltfs_soapserver
{
	enum TaskType
	{
		Type_LoadTape = 0,
		Type_DelShare = 1,
		Type_Revoke   = 2,
		Type_Recycle  = 3,
		Type_Import   = 4,
		Type_CleanDrive  = 5,
		Type_CheckTape = 6,
		Type_Export	  = 7,
		Type_Eject	  = 8,
		Type_MannulFormat = 9,
		Type_DirectAccess = 10,
		Type_DeleteTape = 11,
		Type_AuditTape = 12,
		Type_DeleteTapeFile = 13,
		Type_TypeLast     		// this should be the last task type, no type can be added after this
	};

	enum StatusForQueue
	{
		Status_New,
		Status_Finished
	};

	enum StatusForTask
	{
		Status_Loading	  = 0,
		Status_Formatting = 1,
		Status_Creating	  = 2,
		status_Deleting   = 3,
		Status_Waiting    = 4,
		Status_Running	  = 5,
		Status_Ready  	  = 6,
		Status_Failed     = 7,
		Status_Finish     = 8,
		Status_Canceled	  = 9
	};

	struct TapeStatusPair
	{
		string 		  mBarcode;
		StatusForTask mStatus;
	};

	class Task
	{
	public:
		static string
		Status2String(StatusForTask status);

		static int
		String2Status(string& status);

		static int
		String2Type(string& type);

		Task(TaskType type);

		Task(TaskType type, const string& groupID, const vector<std::string> &barcodes);

		virtual
		~Task();

		void
		InitStatus(const vector<std::string> &barcodes);

		virtual void
		Execute()=0;

		bool
		Start(void* manager=NULL);

		virtual bool
		Cancel();

		string
		GetTypeStr() const;

		string
		GetStatusStr() const;

		string
		GetStartTimeStr() const;

		string
		GetEndTimeStr() const;

		string
		GetGroupID() const;

		TaskType
		GetType() const;

		StatusForTask
		GetStatus() const;

		bool GetExcludeByQueryFlag() const;

		boost::posix_time::ptime
		GetStartTime() const;

		boost::posix_time::ptime
		GetEndTime() const;

		void
		GetTapeList(vector<TapeStatusPair>& tapeList);

		StatusForQueue
		GetStatusForQueue() const;

		virtual string GetTaskProgressStr();

		void ForceDetachTask();
	protected:

		void
		MarkEnd();

		void
		SaveStateToFile();

		bool
		RequestResource(string& barcode, int priority, bool mount = false);

		bool
		FreeResource(string& barcode);

	protected:
		bool 							 cancel_;
		StatusForTask 					 status_;
		TaskType 				 		 type_;
		string 							 groupID_;
		boost::posix_time::ptime 		 timeStart_;
		boost::posix_time::ptime 		 timeEnd_;
		vector<TapeStatusPair> 			 barcodeList_;
		StatusForQueue 			 		 statusForQueue_;
		boost::scoped_ptr<boost::thread> threadPtr_;

		bool							 excludeByQuery_;
		string							 taskProgress_;  // progress of the whole task, 0% ~ 100%

		void*							 manager_;

	};
}
#endif
