/*
 * ltfsTaskCheckTape.h
 *
 *  Created on: Sep 6, 2012
 *      Author: chento
 */

#ifndef __LTFSTASKCHECK_TAPE_H__
#define __LTFSTASKCHECK_TAPE_H__

#include "ltfsTask.h"

namespace ltfs_soapserver
{
	class TaskCheckTape: public Task
	{
	public:
		TaskCheckTape(const vector<string> &barcodes, bool bNeedLockTape = false);
		virtual ~TaskCheckTape();
		virtual void Execute();

	private:
		bool SetTapeAvailable(bool bAvailable);
		bool CheckTape(CHECK_TAPE_FLAG flag);
	private:
		vector<string> 	m_barcodes;
		bool bNeedLockTape_;
	};

} /* namespace ltfs_soapserver */
#endif /* LTFSTASKEXPORT_H_ */
