/* Copyright (c) 2014 BDT Media Automation GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ReadTapeFileSimulator.h
 *
 *  Created on: Sep 3, 2012
 *      Author: chento
 */

#ifndef __READTAPEFILESIMULATOR_H__
#define __READTAPEFILESIMULATOR_H__

#include "stdafx.h"
#include "../FileOperationInterface.h"

class ReadTapeFileSimulator: public bdt::FileOperationInterface
{
public:
	ReadTapeFileSimulator(const fs::path & pth);
	virtual ~ReadTapeFileSimulator();

	virtual bool
	CreateFolder(mode_t mode,bool recur){return false;}

	virtual bool
	OpenFolder(){return false;}

	virtual bool
	ReadFolder(fs::path & path){return false;}

	virtual bool
	CreateFile(int flag,mode_t mode,bool recur){return false;}

	virtual bool
	OpenFile(int flag);

	virtual bool
	Read(off_t offset,void * buffer,size_t bufsize,size_t & size);

	virtual bool
	Write(off_t offset,const void * buffer,size_t bufsize,size_t & size){return false;}

	virtual bool
	GetStat(struct stat & stat){return false;}

	virtual bool
	Sync(bool data){return false;}

	virtual bool
	Truncate(off_t length){return false;}

	virtual bool
	Delete(){return false;}

	virtual bool
	GetTape(string & tape){return false;}

	virtual bool
	IsDirectIO(){return false;}

protected:
	void
	Close();
private:
	fs::path path;
	int handle;
};

#endif /* READTAPEFILESIMULATOR_H_ */
