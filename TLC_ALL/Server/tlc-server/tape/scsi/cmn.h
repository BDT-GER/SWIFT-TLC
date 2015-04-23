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
 * cmn.h
 *
 *  Created on: Aug 143, 2012
 *      Author: Sam Chen
 */

#pragma once


#include "../stdafx.h"
#include "../../ltfs_management/stdafx.h"
#include "../../lib/ltfs_library/stdafx.h"
#include "../../lib/ltfs_library/LtfsError.h"
#include "../../lib/ltfs_library/LtfsDetails.h"
#include "../../lib/ltfs_library/LtfsTape.h"
#include "../../lib/ltfs_library/LtfsScsiDevice.h"
#include "../../lib/ltfs_library/LtfsSlot.h"
#include "../../lib/ltfs_library/LtfsDrive.h"
#include "../../lib/ltfs_library/LtfsMailSlot.h"
#include "../../lib/ltfs_library/LtfsChanger.h"
#include "../../lib/ltfs_library/LtfsLibraries.h"

using namespace ltfs_management;


namespace tape
{
    void ConvertError(Error& error, const LtfsError& err);
}
