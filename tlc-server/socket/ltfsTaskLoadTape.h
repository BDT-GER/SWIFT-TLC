/*
 * ltfsTaskLoadTape.h
 *
 *  Created on: Sep 6, 2012
 *      Author: chento
 */

#ifndef LTFSTASKLOADTAPE_H_
#define LTFSTASKLOADTAPE_H_

#include "ltfsTask.h"

namespace ltfs_soapserver
{

	class TaskLoadTape: public Task
	{
	public:

		TaskLoadTape(bool first);

		virtual
		~TaskLoadTape();

		virtual void
		Execute();

	private:
		void
		LoadTape(vector<ltfs_management::TapeInfo>& tapes);

	private:
		bool isFirst_;
	};

} /* namespace ltfs_soapserver */
#endif /* LTFSTASKLOADTAPE_H_ */
