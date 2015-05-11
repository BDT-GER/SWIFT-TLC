/*
 * ltfsTaskLoadTape.cpp
 *
 *  Created on: Sep 6, 2012
 *      Author: chento
 */

#include "ltfsTaskLoadTape.h"

using namespace ltfs_management;

namespace ltfs_soapserver
{

	TaskLoadTape::TaskLoadTape(bool first) :
			Task(Type_LoadTape), isFirst_(first)
	{
		// TODO Auto-generated constructor stub

	}

	TaskLoadTape::~TaskLoadTape()
	{
		// TODO Auto-generated destructor stub
	}

	void
	TaskLoadTape::Execute()
	{
		ltfs_management::LtfsError  ltfsError;
		SocketDebug("LoadTape execute");
		status_ = Status_Running;
		if(false == ltfs_management::TapeLibraryMgr::Instance()->Refresh(true, ltfsError))
		{
			SocketError("LoadTape  failed ");
			status_ = Status_Failed;
			MarkEnd();
			return ;
		}
		SocketDebug("LoadTape Refresh  done");
		status_ = Status_Finish;
		MarkEnd();
		SocketDebug("LoadTape Finish");
		return;
	}

} /* namespace ltfs_soapserver */
