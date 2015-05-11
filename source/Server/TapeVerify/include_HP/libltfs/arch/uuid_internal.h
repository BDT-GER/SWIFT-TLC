/*
**  %Z% %I% %W% %G% %U%
**
**  ZZ_Copyright_BEGIN
**
**
**  Licensed Materials - Property of IBM
**
**  IBM Linear Tape File System Single Drive Edition Version 1.3.0.2 for Linux and Mac OS X
**
**  Copyright IBM Corp. 2010, 2013
**
**  This file is part of the IBM Linear Tape File System Single Drive Edition for Linux and Mac OS X
**  (formally known as IBM Linear Tape File System)
**
**  The IBM Linear Tape File System Single Drive Edition for Linux and Mac OS X is free software;
**  you can redistribute it and/or modify it under the terms of the GNU Lesser
**  General Public License as published by the Free Software Foundation,
**  version 2.1 of the License.
**
**  The IBM Linear Tape File System Single Drive Edition for Linux and Mac OS X is distributed in the
**  hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
**  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**  See the GNU Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
**  or download the license from <http://www.gnu.org/licenses/>.
**
**
**  ZZ_Copyright_END
**
*************************************************************************************
**
** COMPONENT NAME:  IBM Linear Tape File System
**
** FILE NAME:       arch/uuid_internal.h
**
** DESCRIPTION:     Prototypes for platform-specific UUID functions.
**
** AUTHOR:          Brian Biskeborn
**                  IBM Almaden Research Center
**                  bbiskebo@us.ibm.com
**
*************************************************************************************
*/

#ifndef __uuid_internal_h__
#define __uuid_internal_h__

#ifdef mingw_PLATFORM
#include "libltfs/arch/win/win_util.h"
#define ltfs_gen_uuid(uuid_str) gen_uuid_win(uuid_str)

#else
void gen_uuid_unix(char *uuid_str);
#define ltfs_gen_uuid(uuid_str) gen_uuid_unix(uuid_str)

#endif /* mingw_PLATFORM */

#endif /* __uuid_internal_h__ */

