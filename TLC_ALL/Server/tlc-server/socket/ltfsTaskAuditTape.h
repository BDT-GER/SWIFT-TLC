/*
 * ltfsTaskAuditTape.h
 *
 *  Created on: Nov 20, 2014
 *      Author: zhaona
 */

#ifndef __LTFSTASK_TAPE_AUDITOR_H__
#define __LTFSTASK_TAPE_AUDITOR_H__

#include "ltfsTask.h"

namespace ltfs_soapserver
{
	class TaskAuditTape: public Task
	{
	public:
		TaskAuditTape(const vector<string> &barcodes, bool bNeedLockTape = false);
		virtual ~TaskAuditTape();
		virtual void Execute();

	private:
		bool HandleFile(const string& filePath);
		bool StopAudit();
	private:
		vector<string> 	m_barcodes;
		bool bNeedLockTape_;
		int	 pid_;
		string	mGroupName_;
	};

} /* namespace ltfs_soapserver */
#endif /* __LTFSTASK_TAPE_AUDITOR_H__ */
