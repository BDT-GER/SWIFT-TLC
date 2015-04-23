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
** FILE NAME:       arch/time_internal.h
**
** DESCRIPTION:     Prototypes for platform-specific time functions.
**
** AUTHOR:          Michael A. Richmond
**                  IBM Almaden Research Center
**                  mar@almaden.ibm.com
**
*************************************************************************************
*/

#ifndef time_internal_h_
#define time_internal_h_

#ifdef mingw_PLATFORM
#include "libltfs/arch/win/win_util.h"
#else
#include <time.h>
typedef int64_t	ltfs_time_t;
struct ltfs_timespec {
	ltfs_time_t	tv_sec;
	long		tv_nsec;
};
#endif

#define LTFS_TIME_T_MAX (253402300799) /* 9999/12/31 23:59:59 UTC */
#define LTFS_TIME_T_MIN (-62167219200) /* 0000/01/01 00:00:00 UTC */
#define LTFS_TIME_OUT_OF_RANGE    (1)  /* Time stamp is out of range */

#define timer_sub(a, b, result) \
  do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
    if ((result)->tv_nsec < 0) { \
      --(result)->tv_sec; \
      (result)->tv_nsec += 1000000000; \
    } \
  } while (0)

ltfs_time_t ltfs_timegm(struct tm *t);
struct tm *ltfs_gmtime(const ltfs_time_t *timep, struct tm *result);
struct timespec timespec_from_ltfs_timespec(const struct ltfs_timespec *pSrc);
struct ltfs_timespec ltfs_timespec_from_timespec(const struct timespec *pSrc);

#ifdef __APPLE__
int get_osx_current_timespec(struct ltfs_timespec* now);
#endif

#ifdef __APPLE__
#define _get_current_timespec(timespec) get_osx_current_timespec(timespec)
#define get_localtime(time) localtime(time)
#elif defined(mingw_PLATFORM)
#define _get_current_timespec(timespec) get_win32_current_timespec(timespec)
#define get_localtime(time) get_win32_localtime(time)
#define get_gmtime(time) get_win32_gmtime(time)
#else
int get_unix_current_timespec(struct ltfs_timespec* now);
struct tm *get_unix_localtime(const ltfs_time_t *timep);
#define _get_current_timespec(timespec) get_unix_current_timespec(timespec)
#define get_localtime(time) get_unix_localtime(time);
#endif

static inline int normalize_ltfs_time(ltfs_time_t *t)
{
	int ret = LTFS_TIME_OUT_OF_RANGE;

	if (*t > (ltfs_time_t)LTFS_TIME_T_MAX)
		*t = (ltfs_time_t)LTFS_TIME_T_MAX;
	else if (*t < (ltfs_time_t)LTFS_TIME_T_MIN)
		*t = (ltfs_time_t)LTFS_TIME_T_MIN;
	else
		ret = 0;

	return ret;
}

static inline int get_current_timespec(struct ltfs_timespec* now)
{
	int ret;

	ret = _get_current_timespec(now);
	if (! ret)
		ret = normalize_ltfs_time(&now->tv_sec);

	return ret;
}

#ifndef gmtime_r
#define gmtime_r win_gmtime_r
#endif /* gmtime_r */


#endif /* time_internal_h_ */
