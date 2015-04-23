/*
 * ltfsTask.cpp
 *
 *  Created on: 2012-8-22
 *      Author: chento
 */

#include "ltfsTaskManagement.h"
#include "ltfsTask.h"

namespace ltfs_soapserver
{
	string
	Task::Status2String(StatusForTask status)
	{
		switch(status)
		{
		case Status_Loading:
			return "loading";
		case Status_Formatting:
			return "formatting";
		case Status_Creating:
			return "creating";
		case status_Deleting:
			return "deleting";
		case Status_Waiting:
			return "waiting";
		case Status_Running:
			return "running";
		case Status_Failed:
			return "failed";
		case Status_Finish:
			return "finish";
		case Status_Canceled:
			return "cancel";
		case Status_Ready:
			return "ready";
		default:
			return "";
		}
	}

	int
	Task::String2Status(string& status)
	{

		if(status == "loading")
			return Status_Loading;
		if(status == "formatting")
			return Status_Formatting;
		if(status == "creating")
			return Status_Creating;
		if(status == "deleting")
			return status_Deleting;
		if(status == "waiting")
			return Status_Waiting;
		if(status == "running")
			return Status_Running;
		if(status == "failed")
			return Status_Failed;
		if(status == "finish")
			return Status_Finish;
		if(status == "cancel")
			return Status_Canceled;
		if(status == "ready")
			return Status_Ready;

			return -1;
	}

	int
	Task::String2Type(string& type)
	{

		if(type == "Inventory")
			return Type_LoadTape;
		if(type == "DeleteShare")
			return Type_DelShare;
		if(type == "Revoke")
			return Type_Revoke;
		if(type == "Recycle")
			return Type_Recycle;
		if(type == "Import")
			return Type_Import;
		if(type == "Export")
			return Type_Export;
		if(type == "CleanDrive")
			return Type_CleanDrive;
		if(type == "DiagnoseTape")
			return Type_CheckTape;
		if(type == "Eject")
			return Type_Eject;
		if(type == "DirectAccess")
			return Type_DirectAccess;
		if(type == "MannulFormat")
			return Type_MannulFormat;
		if(type == "DeleteTape")
			return Type_DeleteTape;
		if(type == "AuditTape")
			return Type_AuditTape;
		if(type == "DeleteTapeFile")
			return Type_DeleteTapeFile;

			return -1;
	}

	Task::Task(TaskType type):
		type_(type)
	{
		statusForQueue_ = Status_New;
		timeStart_ 		= boost::posix_time::second_clock::local_time();
		timeEnd_ 		= timeStart_;
		cancel_ 		= false;
		status_ 		= Status_Waiting;
		excludeByQuery_ = false;
		manager_		= NULL;
		taskProgress_	= "";
	}

	Task::Task(TaskType type, const string& groupID, const vector<string> &barcodes):
		type_(type), groupID_(groupID)
	{
		statusForQueue_ = Status_New;
		timeStart_ 		= boost::posix_time::second_clock::local_time();
		timeEnd_ 		= timeStart_;
		cancel_ 		= false;
		status_ 		= Status_Waiting;
		excludeByQuery_ = false;
		manager_		= NULL;
		taskProgress_	= "";

		InitStatus(barcodes);
	}

	Task::~Task()
	{

	}

	void
	Task::InitStatus(const vector<std::string> &barcodes)
	{
		for(vector<std::string>::const_iterator iter = barcodes.begin();
				iter!=barcodes.end(); ++iter)
		{
			TapeStatusPair pair;
			pair.mBarcode = *iter;
			pair.mStatus = Status_Waiting;
			barcodeList_.insert(barcodeList_.end(), pair);
		}
	}

	bool
	Task::Start(void* manager)
	{
		threadPtr_.reset(new boost::thread(boost::bind(&Task::Execute, this)));
		if(manager != NULL){
			manager_ = manager;
		}
		return true;
	}

	bool
	Task::Cancel()
	{
		cancel_ = true;
		return true;
	}

	void
	Task::MarkEnd()
	{
		statusForQueue_ = Status_Finished;
		timeEnd_ 		= boost::posix_time::second_clock::local_time();
	}

	bool Task::GetExcludeByQueryFlag() const
	{
		return excludeByQuery_;
	}

	string
	Task::GetTypeStr() const
	{
		switch(type_)
		{
		case Type_LoadTape:
			return "Inventory";
		case Type_DelShare:
			return "DeleteShare";
		case Type_Revoke:
			return "Revoke";
		case Type_Recycle:
			return "Recycle";
		case Type_Import:
			return "Import";
		case Type_Export:
			return "Export";
		case Type_CleanDrive:
			return "CleanDrive";
		case Type_CheckTape:
			return "DiagnoseTape";
		case Type_Eject:
			return "Eject";
		case Type_DirectAccess:
			return "DirectAccess";
		case Type_MannulFormat:
			return "MannulFormat";
		case Type_DeleteTape:
			return "DeleteTape";
		case Type_AuditTape:
			return "AuditTape";
		case Type_DeleteTapeFile:
			return "DeleteTapeFile";
		}

		return "";
	}

	string
	Task::GetStatusStr() const
	{
		return Status2String(status_);
	}

	string
	Task::GetStartTimeStr() const
	{
		string strTime = boost::posix_time::to_iso_string(timeStart_);
		int pos = strTime.find('T');
		strTime.replace(pos,1,std::string(" "));
		strTime.replace(pos + 3,0,std::string(":"));
		strTime.replace(pos + 6,0,std::string(":"));

		strTime.replace(4,0,std::string("-"));
		strTime.replace(7,0,std::string("-"));

		return strTime;
	}

	string
	Task::GetEndTimeStr() const
	{
		string strTime = boost::posix_time::to_iso_string(timeEnd_);
		int pos = strTime.find('T');
		strTime.replace(pos,1,std::string(" "));
		strTime.replace(pos + 3,0,std::string(":"));
		strTime.replace(pos + 6,0,std::string(":"));

		strTime.replace(4,0,std::string("-"));
		strTime.replace(7,0,std::string("-"));

		return strTime;
	}

	string
	Task::GetGroupID() const
	{
		return groupID_;
	}

	TaskType
	Task::GetType() const
	{
		return type_;
	}

	StatusForTask
	Task::GetStatus() const
	{
		return status_;
	}

	boost::posix_time::ptime
	Task::GetStartTime() const
	{
		return timeStart_;
	}

	boost::posix_time::ptime
	Task::GetEndTime() const
	{
		return timeEnd_;
	}

	void
	Task::GetTapeList(vector<TapeStatusPair>& tapeList)
	{
		tapeList = barcodeList_;
	}

	StatusForQueue
	Task::GetStatusForQueue() const
	{
		return statusForQueue_;
	}

	void
	Task::SaveStateToFile()
	{
		if(NULL != manager_)
		{
			((TaskManagement*)manager_)->SaveTaskQueue();
		}

	}

	string Task::GetTaskProgressStr()
	{
		return taskProgress_;
	}

	bool
	Task::RequestResource(string& barcode, int priority,bool mount)
	{
		SocketDebug("RequestResource, barcode :" << barcode << " type:"<< GetTypeStr())

		while(!ltfs_management::TapeLibraryMgr::Instance()->RequestTape(barcode,mount,priority,1))
		{
			if(cancel_)
			{
		        SocketDebug("RequestResource, barcode :" << barcode << " type:"<< GetTypeStr() << " return false")
				return false;
			}
		}

		SocketDebug("RequestResource, barcode :" << barcode << " type:"<< GetTypeStr() << " return true")

		return true;
	}

	bool
	Task::FreeResource(string& barcode)
	{
		ltfs_management::TapeLibraryMgr::Instance()->ReleaseTape(barcode);

		SocketDebug("FreeResource, barcode :" << barcode);

		return true;
	}

	void
	Task::ForceDetachTask()
	{
		//threadPtr_->interrupt();
	}
}
