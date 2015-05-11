/* Copyright (c) 2012 BDT Media Automation GmbH
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
 * HandleManager.h
 *
 *  Created on: Oct 9, 2012
 *      Author: chento
 */

#ifndef HANDLEMANAGER_H_
#define HANDLEMANAGER_H_

namespace ltfs_management
{

	class HandleManager
	{
	public:
		static bool Bind(const string& path, int handle);
		static void Unbind(int handle);
		static bool Read(int handle, off_t offset, void * buffer, size_t & size);
		static bool Write(int handle, off_t offset, void * buffer, size_t & size);

		struct Location;
	private:
		HandleManager();
		virtual ~HandleManager();
		bool GetLocation(const string& path, struct Location& location);

	private:
		static HandleManager handleMgr_;

		typedef map<int,struct Location> ListLocationType;
		ListLocationType listLocation_;
	};

} /* namespace tape */
#endif /* HANDLEMANAGER_H_ */
