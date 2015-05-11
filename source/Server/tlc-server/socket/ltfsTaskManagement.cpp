/*
 * ltfsTaskManagement.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "../tinyxml/tinyxml.h"
#include "../bdt/Factory.h"
#include "../ltfs_management/TapeLibraryMgr.h"
#include "../ltfs_management/TapeDbManager.h"
#include "../ltfs_format/ltfsFormatManager.h"
#include "ltfsTaskManagement.h"
#include <boost/thread/thread.hpp>
#include "../bdt/Configure.h"


#define XML_HEADER "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
const string XML_FILE  =  COMM_APP_BIN_PATH + "/tasks.xml";
const string LAST_TIME_FOR_TAPE_AUDIT  =  COMM_APP_BIN_PATH + "/LastTimeForTapeAudit";

using namespace ltfs_management;
using namespace bdt;

namespace ltfs_soapserver
{
	TaskManagement* TaskManagement::mInstance = NULL;
	boost::mutex TaskManagement::mMutex;

	TaskManagement::TaskManagement():stop_(false)
	{
		threadPtr_.reset(
				new boost::thread(boost::bind(&TaskManagement::TaskCleaner, this)));
		auditPtr_.reset(
				new boost::thread(boost::bind(&TaskManagement::TaskTapeAuditTrigger, this)));
		auditPtr_.reset(
				new boost::thread(boost::bind(&TaskManagement::TaskTapeDeleteFile, this)));
	}

	TaskManagement::~TaskManagement()
	{

	}

	TaskManagement*
	TaskManagement::GetInstance()
	{
		mMutex.lock();
		if(!mInstance)
		{
		  mInstance = new TaskManagement();
		}
		mMutex.unlock();
		return mInstance;
	}

	bool
	TaskManagement::LoadTaskQueue()
	{
		TaskLoadTape* pTask = new TaskLoadTape(true);

		AddTask(pTask, false);

		LoadTaskFrFile();
		return true;
	}

	bool
	TaskManagement::SaveTaskQueue(bool serverStoped)
	{
		boost::mutex::scoped_lock lock(listMutex_);
		if (stop_)
		{
			SocketDebug("Save task when Task Management was stopped!");
			return false;
		}

		if(serverStoped)
		{
			SocketDebug("Stop Task Management");
			stop_ = true;
		}

		SocketDebug("Start SaveTaskQueue");
//		boost::mutex::scoped_lock lock(listMutex_);
		SaveTaskToFile();
		SocketDebug("Finish SaveTaskQueue");

		// wait import task exit
		if (serverStoped)
		{
			std::vector<Task*> importTaskList;
			for(vector<Task*>::iterator iter=taskList_.begin();
				iter!=taskList_.end(); ++iter)
			{
				if((*iter)->GetExcludeByQueryFlag() || (*iter)->GetType() == Type_DeleteTapeFile){
					continue;
				}
				(*iter)->ForceDetachTask();
			}
		}

		SocketDebug("leave SaveTaskQueue");

		return true;
	}

	bool
	TaskManagement::AddTask(Task* pTask, bool needSave)
	{
		SocketDebug("AddTask "<<pTask->GetTypeStr());

		if(stop_)
		{
			delete pTask;
			return false;
		}

		if(pTask->Start(this))
		{
			boost::mutex::scoped_lock lock(listMutex_);
			taskList_.insert(taskList_.end(),pTask);

			if(needSave)
				SaveTaskToFile();
		}
		else
		{
			delete pTask;
			return false;
		}
		return true;
	}

	bool
	TaskManagement::QueryStatus(struct QueryTaskRslt& result)
	{
		boost::mutex::scoped_lock lock(listMutex_);

		result.taskStatus.clear();

		for(vector<Task*>::iterator iter=taskList_.begin();
				iter!=taskList_.end(); ++iter)
		{
			if((*iter)->GetExcludeByQueryFlag()){
				continue;
			}

			struct TaskStatus taskStatus;
			vector<TapeStatusPair> tapeList;

			taskStatus.type = (*iter)->GetTypeStr();
			taskStatus.status = (*iter)->GetStatusStr();
			taskStatus.startTime = (*iter)->GetStartTimeStr();
			taskStatus.endTime = (*iter)->GetEndTimeStr();
			taskStatus.shareUUID = (*iter)->GetGroupID();
			taskStatus.shareName = "";
			if("" != taskStatus.shareUUID){
				TapeLibraryMgr::Instance()->GetGroupName(taskStatus.shareUUID, taskStatus.shareName);
			}
			taskStatus.progress = (*iter)->GetTaskProgressStr();
			SocketDebug("taskStatus.progress = " << taskStatus.progress << ".");

			(*iter)->GetTapeList(tapeList);
			for(vector<TapeStatusPair>::iterator iterTape=tapeList.begin();
					iterTape!=tapeList.end(); ++iterTape)
			{
				struct BarcodeStatus status;
				status.barcode = iterTape->mBarcode;
				status.status = Task::Status2String(iterTape->mStatus);

				taskStatus.tapeStatus.push_back(status);
			}

			result.taskStatus.push_back(taskStatus);
		}

		return true;
	}

	bool
	TaskManagement::QueryLoadStatus()
	{
		SocketDebug("QueryLoadStatus start");

		boost::mutex::scoped_lock lock(listMutex_);

		for(vector<Task*>::iterator iter=taskList_.begin();
				iter!=taskList_.end(); ++iter)
		{
			if((*iter)->GetType() == Type_LoadTape)
			{
				if((*iter)->GetStatus() < Status_Failed)
				{
					SocketDebug("QueryLoadStatus return false");
					return false;
				}
			}

		}
		SocketDebug("QueryLoadStatus return true");
		return true;
	}

	bool
	TaskManagement::CancelTask(string &uid, TaskType type)
	{
		SocketDebug("CancelAddShare start");

		boost::mutex::scoped_lock lock(listMutex_);
		bool find = false;

		for(vector<Task*>::iterator iter=taskList_.begin();
				iter!=taskList_.end(); ++iter)
		{
			if((*iter)->GetType() == type
					&&(*iter)->GetGroupID() == uid)
			{
				find = true;
				(*iter)->Cancel();
			}

		}
		if(!find)
		{
			SocketDebug("CancelTask return false, not find");
			return false;
		}

		SocketDebug("CancelTask return true");
		return true;
	}

	void
	TaskManagement::TaskCleaner()
	{
		do
		{
			boost::this_thread::sleep(boost::posix_time::seconds(60));
			if(taskList_.size() == 0)
			{
				continue;
			}

			{
				boost::mutex::scoped_lock lock(listMutex_);
				for(vector<Task*>::iterator iter=taskList_.begin(); iter!=taskList_.end(); )
				{
					if((*iter)->GetStatusForQueue() == Status_Finished)
					{
						boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
						boost::posix_time::time_duration diff = now - (*iter)->GetEndTime();
						std::time_t seconds = diff.ticks()/boost::posix_time::time_duration::rep_type::ticks_per_second;

						if(seconds > 30)
						{
							delete (*iter);
							iter = taskList_.erase(iter);
							continue;
						}
					}
					++iter;
				}

				SaveTaskToFile();
			}
		}while(true);

	}
	bool TaskManagement::GetRunningTaskTapes(TaskType taskType, vector<string>& runningTapes)
	{
		boost::mutex::scoped_lock lock(listMutex_);
		runningTapes.clear();

		for(vector<Task*>::iterator iter=taskList_.begin();
				iter!=taskList_.end(); ++iter)
		{
			if((*iter)->GetType() != taskType){
				continue;
			}
			vector<TapeStatusPair> tapeList;
			(*iter)->GetTapeList(tapeList);
			for(vector<TapeStatusPair>::iterator iterTape=tapeList.begin();
					iterTape!=tapeList.end(); ++iterTape)
			{
				StatusForTask taskStatus = iterTape->mStatus;
				if(taskStatus != Status_Failed && taskStatus != Status_Finished && taskStatus != Status_Canceled){
					runningTapes.push_back(iterTape->mBarcode);
				}
			}
		}
		return true;
	}

	void TaskManagement::RefreshTapeList(map<string, string>& tapeShareMap, map<string, time_t>& lastDeleteMap, map<string, UInt64_t>& pendingNumMap, time_t& refreshTime)
	{
		vector<TapeInfo> tapes;
		LtfsError lfsErr;
		if(TapeLibraryMgr::Instance()->GetAllTapeList(tapes, lfsErr)){
			for(unsigned int i = 0; i < tapes.size(); i++){
				string barcode = tapes[i].mBarcode;
				if(lastDeleteMap.find(barcode) == lastDeleteMap.end()){
					lastDeleteMap[barcode] = 0;
					pendingNumMap[barcode] = 0;
				}
				tapeShareMap[barcode] = tapes[i].mGroupID;
			}
		}
		refreshTime = time(NULL);
	}

	void
	TaskManagement::TaskTapeDeleteFile()
	{
		map<string, string> tapeShareMap;
		map<string, time_t> lastDeleteMap;
		map<string, UInt64_t> pendingNumMap;
		time_t refreshTapeTime = time(NULL);
		RefreshTapeList(tapeShareMap, lastDeleteMap, pendingNumMap, refreshTapeTime);
        Configure * config = Factory::GetConfigure();
        off_t waitSize = config->GetValueSize(Configure::BackupWaitSize);
		UInt64_t triggerNum = config->GetValueSize(Configure::DeleteTapeFileTriggerNum);
		time_t triggerTimeDiff = config->GetValueSize(Configure::DeleteTapeFileTimeDiff);
		SocketInfo("triggerNum = " << triggerNum << ", triggerTimeDiff = " << triggerTimeDiff);
		do
		{
			if ( boost::this_thread::interruption_requested()){
				SocketInfo("TaskTapeDeleteFile task interrupted.");
				break;
			}

			boost::this_thread::sleep(boost::posix_time::seconds(60));
			if(time(NULL) - refreshTapeTime > 60*10){
				SocketDebug("TaskTapeDeleteFile: refresh tapes");
				RefreshTapeList(tapeShareMap, lastDeleteMap, pendingNumMap, refreshTapeTime);
			}

			for(map<string, UInt64_t>::iterator it = pendingNumMap.begin(); it != pendingNumMap.end(); it++){
				string barcode = it->first;
				time_t tNow = time(NULL);
				DeleteFilesOnTapeStatus runStaus = GetTapeDeleteFileStatus(barcode);
				if(runStaus == DF_RUNNING){
					lastDeleteMap[barcode] = time(NULL);
					SocketDebug("TaskTapeDeleteFile: task running " << barcode);
					continue;
				}else if(runStaus == DF_FAILED){
					lastDeleteMap[barcode] = 0;
				}
				if(it->second < triggerNum || tNow - lastDeleteMap[barcode] > triggerTimeDiff){
					it->second = TapeLibraryMgr::Instance()->GetPendingDeleteFileNum(barcode);
				}
				SocketDebug("TaskTapeDeleteFile: " << barcode << ",it->second = " << it->second << ", triggerNum = "
						<< triggerNum << ", tNow - lastDeleteMap[barcode] = " << tNow - lastDeleteMap[barcode]
						<< ", triggerTimeDiff = " << triggerTimeDiff);
				if(it->second <= 0 || (it->second < triggerNum && tNow - lastDeleteMap[barcode] < triggerTimeDiff)){
					continue;
				}
				SocketInfo("TaskTapeDeleteFile: add new " << barcode);
				vector<string> barcodes;
				barcodes.push_back(barcode);
				TaskDeleteTapeFile* pTask = new TaskDeleteTapeFile(barcodes);
				if(pTask == NULL || false == TaskManagement::GetInstance()->AddTask(pTask, false)){
					SocketError("Failed to add task to delete files on tape tape " << barcode << ".");
				}
				lastDeleteMap[barcode] = time(NULL);
				it->second = 0;
			}
		}while(true);
	}

	void
	TaskManagement::TaskTapeAuditTrigger()
	{
		do
		{
			boost::this_thread::sleep(boost::posix_time::seconds(60));

			bool bAuditorEnabled = bdt::Factory::GetConfigure()->GetValueBool(bdt::Configure::TapeAuditorEnable);
			if(!bAuditorEnabled){
				SocketDebug("Tape auditor is not enabled.");
				continue;
			}

			// restart any tasks can be restarted
			//RestartAuditorTasks();

			time_t tNow = time(NULL);
			time_t tLast = GetLastStartTimeFromFile();
			time_t diff = tNow - tLast;
			string runAt = bdt::Factory::GetConfigure()->GetValue(bdt::Configure::TapeAuditorRunAt);
			//02:03
			regex mathcAt("^\\s*(\\d+)\\:(\\d+)\\s*$");
			time_t runAtSecs = 0;
			time_t secondsADay = 60 * 60 * 24;
			cmatch match;
			if(regex_match(runAt.c_str(), match, mathcAt)){
				runAtSecs = boost::lexical_cast<long>(match[1]) * 3600 + boost::lexical_cast<long>(match[2]) * 60 + 0;;
			}
			int auditInterval = bdt::Factory::GetConfigure()->GetValueSize(bdt::Configure::TapeAuditorInterval );
			int auditMaxNum = bdt::Factory::GetConfigure()->GetValueSize(bdt::Configure::TapeAuditorMaxnum );
			vector<string> auditingTapes;
			if(!GetRunningTaskTapes(Type_AuditTape, auditingTapes)){
				SocketDebug("Failed to get running audit tasks.");
			}
			int runningNum = auditingTapes.size();
			SocketDebug("TaskTapeAuditTrigger: diff = " << diff << ", auditInterval = " << auditInterval << ", runningNum = " << runningNum\
					 << ", auditMaxNum = " << auditMaxNum << ", tNow % secondsADay = " << tNow % secondsADay << ", runAtSecs = " << runAtSecs\
					 << ", tLast = " << tLast);

			if(tLast <= 0 || (diff > secondsADay * auditInterval && runningNum < auditMaxNum && tNow % secondsADay > runAtSecs))  //3 days
			{
				while(runningNum < auditMaxNum){
					SocketDebug("getting next tape.");
					bool bFaulty = false;
					string barcode = TapeLibraryMgr::Instance()->GetNextTapeToAudit(auditingTapes, bFaulty);
					if(barcode == ""){
						SocketDebug("No next tape can be audited.");
						break;
					}
					vector<string> barcodes;
					barcodes.push_back(barcode);
					if(bFaulty){
						Task* pTask = new TaskCheckTape(barcodes);
						if(!pTask || !AddTask(pTask, false)){
							SocketWarn("Failed to add task to diagnose faulty tape " << barcode << " before audit it.");
						}
					}
					Task* pTask = new TaskAuditTape(barcodes);
					if(!pTask || !AddTask(pTask, false)){
						SocketWarn("Failed to add task to audit the tape " << barcode);
						continue;
					}
					auditingTapes.push_back(barcode);
					runningNum++;
					SocketInfo("Start audit task for tape " << barcode << ". Running audit tape num: " << runningNum);
				}
				//the tape audit get finish
				SetLastStartTimeToFile(tNow);
			}
		}while(true);

	}

	time_t
    TaskManagement::GetLastStartTimeFromFile()
    {
		if(!fs::exists(LAST_TIME_FOR_TAPE_AUDIT)){
			SetLastStartTimeToFile(time(NULL));
		}
          time_t startTime = 0;
          try{
                FILE* pFile = fopen(LAST_TIME_FOR_TAPE_AUDIT.c_str(), "r");
                if(pFile != NULL){
                      char buf[1024];
                      memset(buf, 0, 1024);
                      int nRead = fread(buf, 1022, sizeof(char), pFile);
                      string line = buf;
                      //line.resize(nRead);
                      startTime = boost::lexical_cast<time_t>(line);
                      fclose(pFile);
                      return startTime;
                }
          }catch(...){
        	  SocketError("Failed to get last audit start time.");
          }

          return 0;
    }

    void
    TaskManagement::SetLastStartTimeToFile(time_t lastStartTime)
    {
    	  time_t m_lastStartTime = lastStartTime;
          //fs::path  pathCfg = LAST_TIME_FOR_TAPE_AUDIT;
          //save last start time to file or tmp file
          //string cmd = "echo " + boost::lexical_cast<string>(m_lastStartTime) + " >" + pathCfg;
          //TODO: we should save this to general config file of Ltfstor but not in this way
          try{
                FILE* pFile = fopen(LAST_TIME_FOR_TAPE_AUDIT.c_str(), "w");
                if(pFile != NULL){
                      string line = boost::lexical_cast<string>(m_lastStartTime);
                      fwrite(line.c_str(), line.length(), sizeof(char), pFile);
                      fclose(pFile);
                      return;
                }
          }catch(...){
        	  SocketError("Failed to set last audit start time.");
          }
          SocketError("Failed to save last start time to " << LAST_TIME_FOR_TAPE_AUDIT.c_str());
    }



	bool
	TaskManagement::GetServerStop()
	{
		return stop_;
	}

	void
	TaskManagement::LoadTaskFrFile()
	{
		boost::property_tree::ptree	root;
		fs::path pFile(XML_FILE);
		string str;
		int taskStatus;

		try
		{
			if(!exists(pFile))
				return;

			read_xml(XML_FILE, root);

			boost::property_tree::ptree backend = root.get_child("backend_status");

			boost::property_tree::ptree tasks = backend.get_child("tasks");

			boost::property_tree::ptree formats = backend.get_child("formats");

			for(boost::property_tree::ptree::iterator iter=formats.begin();
					iter!=formats.end(); ++iter)
			{
				boost::property_tree::ptree taskTree = iter->second;

				RecoverFormat(taskTree);
			}

			for(boost::property_tree::ptree::iterator iter=tasks.begin();
					iter!=tasks.end(); ++iter)
			{
				boost::property_tree::ptree taskTree = iter->second;
				boost::property_tree::ptree tree;

				tree = taskTree.get_child("task_status");
				str = tree.data();
				taskStatus = Task::String2Status(str);
				if(taskStatus<0)
				{
					SocketWarn("Task Status wrong, status:"<<str);
					continue;
				}
				if(taskStatus>=Status_Failed)
					continue;

				RecoverTask(taskTree);
			}


		}
		catch(std::exception& e)
		{
			SocketError("LoadTaskFrFile exception, " << e.what());
		}
	}

	void
	TaskManagement::RecoverTask(boost::property_tree::ptree & taskTree)
	{
		Task* pTask = NULL;
		boost::property_tree::ptree tree;
		boost::property_tree::ptree tapesTree;
		string str;
		string groupid;
		int status;
		int type;
		vector<string> barcodes;
		vector<string>::iterator iterBarcode;

		SocketDebug("RecoverTask Start");

		tree = taskTree.get_child("type");
		str = tree.data();
		type = Task::String2Type(str);
		if(type<0 || type >= Type_TypeLast)
		{
			SocketWarn("Task type wrong, type:"<<str);
			return;
		}

		tree = taskTree.get_child("group_id");
		groupid = tree.data();

		tapesTree = taskTree.get_child("tapes");
		for(boost::property_tree::ptree::iterator iter=tapesTree.begin();
				iter!=tapesTree.end(); ++iter)
		{
			boost::property_tree::ptree tapeTree = iter->second;

			tree = tapeTree.get_child("status");
			str = tree.data();
			status = Task::String2Status(str);
			if(status>Status_Failed)
				continue;

			tree = tapeTree.get_child("barcode");
			str = tree.data();
			if(str.empty())
			{
				SocketWarn("Tape barcode wrong, empty");
				continue;
			}
			barcodes.push_back(str);
		}

		switch(type)
		{
		case Type_CheckTape:
			pTask = new TaskCheckTape(barcodes, true); //need get tape lock when recover CheckTape task
			break;
		case Type_AuditTape:
			iterBarcode = barcodes.begin();
			if(iterBarcode != barcodes.end())
			{
				pTask = new TaskAuditTape(barcodes, true); //need get tape lock when recover audit task
			}
			break;
		}

		if(!pTask || !AddTask(pTask, false))
		{
			SocketWarn("Add Task failed ");
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
	}

	void
	TaskManagement::RecoverFormat(boost::property_tree::ptree & formatTree)
	{
		string str;
		boost::property_tree::ptree tree;
		int priority;
		time_t startTime = 0;
		string barcode;
		int type;
		int status;
		Labels labels;

		SocketDebug("RecoverFormat Start");

		try{
			tree = formatTree.get_child("start_time");
			startTime = boost::lexical_cast<time_t>(tree.data());
		}catch(std::exception &e){
			SocketDebug("get format task format time error: " << e.what());
		}

		tree = formatTree.get_child("priority");
		priority = boost::lexical_cast<int>(tree.data());

		tree = formatTree.get_child("barcode");
		barcode = tree.data();

		tree = formatTree.get_child("status");
		status = boost::lexical_cast<int>(tree.data());

		tree = formatTree.get_child("type");
		type = boost::lexical_cast<int>(tree.data());

		tree = formatTree.get_child("lable_mark");
		labels.mMark = boost::lexical_cast<int>(tree.data());

		tree = formatTree.get_child("lable_faulty");
		str = tree.data();
		labels.mFaulty = str=="true"?true:false;

		tree = formatTree.get_child("lable_status");
		labels.mStatus = boost::lexical_cast<int>(tree.data());

		tree = formatTree.get_child("lable_group");
		labels.mGroupID = tree.data();

		TapeLibraryMgr::Instance()->
				StartFormat(barcode, (FormatType)type, priority, labels, startTime);
	}

	void
	TaskManagement::SaveTaskToFile()
	{
		SocketDebug("SaveTaskToFile start");

		TiXmlDocument* document = new TiXmlDocument();
		TiXmlElement* rootElement = new TiXmlElement("backend_status");

		try
		{
			document->Parse(XML_HEADER);
			document->LinkEndChild(rootElement);

			TiXmlElement* tasksElement = new TiXmlElement("tasks");

			for(vector<Task*>::iterator iter=taskList_.begin();
					iter!=taskList_.end(); ++iter)
			{
				if((*iter)->GetType() == Type_LoadTape)
					continue;

				TiXmlElement* taskElement = new TiXmlElement("task");
				TiXmlElement* element = new TiXmlElement("type");
				TiXmlText* content = new TiXmlText(((*iter)->GetTypeStr().c_str()));
				element->LinkEndChild(content);
				taskElement->LinkEndChild(element);

				element = new TiXmlElement("group_id");
				content = new TiXmlText(((*iter)->GetGroupID().c_str()));
				element->LinkEndChild(content);
				taskElement->LinkEndChild(element);

				element = new TiXmlElement("task_status");
				content = new TiXmlText(((*iter)->GetStatusStr().c_str()));
				element->LinkEndChild(content);
				taskElement->LinkEndChild(element);

				element = new TiXmlElement("task_progress");
				content = new TiXmlText(((*iter)->GetTaskProgressStr().c_str()));
				element->LinkEndChild(content);
				taskElement->LinkEndChild(element);

				TiXmlElement* tapesElement = new TiXmlElement("tapes");
				vector<TapeStatusPair> tapeList;
				(*iter)->GetTapeList(tapeList);
				for(vector<TapeStatusPair>::iterator iterTape=tapeList.begin();
						iterTape!=tapeList.end(); ++iterTape)
				{
					TiXmlElement* tapeElement = new TiXmlElement("tape");

					element = new TiXmlElement("barcode");
					content = new TiXmlText((iterTape->mBarcode.c_str()));
					element->LinkEndChild(content);
					tapeElement->LinkEndChild(element);

					element = new TiXmlElement("status");
					content = new TiXmlText((Task::Status2String(iterTape->mStatus).c_str()));
					element->LinkEndChild(content);
					tapeElement->LinkEndChild(element);

					tapesElement->LinkEndChild(tapeElement);
				}

				taskElement->LinkEndChild(tapesElement);
				tasksElement->LinkEndChild(taskElement);
			}
			rootElement->LinkEndChild(tasksElement);

			// backup format
			TiXmlElement* formatsElement = new TiXmlElement("formats");
			//using namespace ltfs_format;

			vector<FormatDetail> details;

			if(TapeLibraryMgr::Instance()->GetDetailForBackup(details)
					&& details.size()>0)
			{
				for(vector<FormatDetail>::iterator iterFormat=details.begin();
						iterFormat!=details.end(); ++iterFormat)
				{
					if(iterFormat->mStatus == FORMAT_THREAD_STATUS_FINISHED)
						continue;

					TiXmlElement* formatElement = new TiXmlElement("format");

					TiXmlElement* element = new TiXmlElement("start_time");
					TiXmlText* content = new TiXmlText(
							boost::lexical_cast<string>(iterFormat->mStartTime).c_str());
					element->LinkEndChild(content);
					formatElement->LinkEndChild(element);

					element = new TiXmlElement("priority");
					content = new TiXmlText(
							boost::lexical_cast<string>(iterFormat->mPriority).c_str());
					element->LinkEndChild(content);
					formatElement->LinkEndChild(element);

					element = new TiXmlElement("barcode");
					content = new TiXmlText(iterFormat->mBarcode.c_str());
					element->LinkEndChild(content);
					formatElement->LinkEndChild(element);

					element = new TiXmlElement("status");
					content = new TiXmlText(
							boost::lexical_cast<string>(iterFormat->mStatus).c_str());
					element->LinkEndChild(content);
					formatElement->LinkEndChild(element);

					element = new TiXmlElement("type");
					content = new TiXmlText(
							boost::lexical_cast<string>(iterFormat->mType).c_str());
					element->LinkEndChild(content);
					formatElement->LinkEndChild(element);

					element = new TiXmlElement("lable_mark");
					content = new TiXmlText(
							boost::lexical_cast<string>(iterFormat->mLabels.mMark).c_str());
					element->LinkEndChild(content);
					formatElement->LinkEndChild(element);

					element = new TiXmlElement("lable_faulty");
					content = new TiXmlText(
							(iterFormat->mLabels.mFaulty)?"true":"false");
					element->LinkEndChild(content);
					formatElement->LinkEndChild(element);

					element = new TiXmlElement("lable_status");
					content = new TiXmlText(
							boost::lexical_cast<string>(iterFormat->mLabels.mStatus).c_str());
					element->LinkEndChild(content);
					formatElement->LinkEndChild(element);

					element = new TiXmlElement("lable_group");
					content = new TiXmlText(iterFormat->mLabels.mGroupID.c_str());
					element->LinkEndChild(content);
					formatElement->LinkEndChild(element);

					formatsElement->LinkEndChild(formatElement);
				}
			}
			rootElement->LinkEndChild(formatsElement);

			document->SaveFile(XML_FILE.c_str());
		}
		catch(std::exception& e)
		{
			SocketError("SaveTaskToFile exception, " << e.what());
		}
		delete document;
	}
	bool TaskManagement::IsTaskRunning(const string& barcode, TaskType taskType)
	{
		for(vector<Task*>::iterator iter=taskList_.begin();
				iter!=taskList_.end(); ++iter)
		{
			if((*iter)->GetType() != taskType){
				continue;
			}
			vector<TapeStatusPair> tapeList;
			(*iter)->GetTapeList(tapeList);
			for(vector<TapeStatusPair>::iterator iterTape=tapeList.begin();
					iterTape!=tapeList.end(); ++iterTape)
			{
				if(iterTape->mBarcode != barcode){
					continue;
				}
				StatusForTask taskStatus = iterTape->mStatus;
				if(taskStatus != Status_Failed && taskStatus != Status_Finished && taskStatus != Status_Canceled){
					return true;
				}
			}
		}

		return false;
	}

	DeleteFilesOnTapeStatus TaskManagement::GetTapeDeleteFileStatus(const string& barcode)
	{
		DeleteFilesOnTapeStatus status = DF_NOT_RUN;
		boost::unique_lock<boost::mutex> lock(deleteFileOnTapeMapMutex_);
		if(deleteFilesOnTapeMap_.find(barcode) != deleteFilesOnTapeMap_.end()){
			status = deleteFilesOnTapeMap_[barcode];
		}
		return status;

	}
	void TaskManagement::SetTapeDeleteFileStatus(const string& barcode, DeleteFilesOnTapeStatus status)
	{
		boost::unique_lock<boost::mutex> lock(deleteFileOnTapeMapMutex_);
		deleteFilesOnTapeMap_[barcode] = status;
	}


}

