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
 * LtfsMailSlot.h
 *
 *  Created on: Nov 13, 2012
 *      Author: chento
 */

#pragma once

namespace ltfs_management
{
	class LtfsMailSlot : public LtfsSlot
	{
	public:
		friend class LtfsChanger;

		virtual
		~LtfsMailSlot();

		void
		SetMailSlotOpened(bool opened);

		void
		GetMailSlotInfo(LtfsMailSlotInfo& mailSlot);

	private:
		LtfsMailSlot(int slotid, int logicSlotId, bool isOpen);

	private:
		bool isOpen_;
	};

} /* namespace ltfs_management */

