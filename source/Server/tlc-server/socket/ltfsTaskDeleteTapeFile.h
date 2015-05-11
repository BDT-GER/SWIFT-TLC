/*
 * ltfsTaskDeleteTapeFile.h
 *
 *  Created on: Nov 20, 2014
 *      Author: zhaona
 */

#ifndef __LTFSTASK_TAPE_DELETE_FILE_H__
#define __LTFSTASK_TAPE_DELETE_FILE_H__

#include "ltfsTask.h"

namespace ltfs_soapserver
{
	class TaskDeleteTapeFile: public Task
	{
	public:
		TaskDeleteTapeFile(const vector<string> &barcodes);
		virtual ~TaskDeleteTapeFile();
		virtual void Execute();

	private:
		vector<string> 	m_barcodes;
	};

} /* namespace ltfs_soapserver */
#endif /* __LTFSTASK_TAPE_DELETE_FILE_H__ */
