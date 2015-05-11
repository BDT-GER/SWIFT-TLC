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
** FILE NAME:       mrsw.h
**
** DESCRIPTION:     Multi-reader single-writer lock implementation.
**
** AUTHORS:         Mark A. Smith
**                  IBM Almaden Research Center
**                  mark1smi@us.ibm.com
**
*************************************************************************************
*/

#ifndef __MRSW_H__
#define __MRSW_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MultiReaderSingleWriter {
	ltfs_mutex_t write_exclusive_mutex;
	ltfs_mutex_t reading_mutex;
	ltfs_mutex_t read_count_mutex;
	uint32_t read_count;
	uint32_t writer; //if there is a write lock acquired
} MultiReaderSingleWriter;

NOTRACE_FN static inline int
init_mrsw(MultiReaderSingleWriter *mrsw)
{
	int ret;
	mrsw->read_count = 0;
	mrsw->writer = 0;
	ret = ltfs_mutex_init(&mrsw->read_count_mutex);
	if (ret)
		return -ret;
	ret = ltfs_mutex_init(&mrsw->reading_mutex);
	if (ret) {
		ltfs_mutex_destroy(&mrsw->read_count_mutex);
		return -ret;
	}
	ret = ltfs_mutex_init(&mrsw->write_exclusive_mutex);
	if (ret) {
		ltfs_mutex_destroy(&mrsw->read_count_mutex);
		ltfs_mutex_destroy(&mrsw->reading_mutex);
		return -ret;
	}
	return 0;
}

NOTRACE_FN static inline void
destroy_mrsw(MultiReaderSingleWriter *mrsw)
{
	ltfs_mutex_destroy(&mrsw->read_count_mutex);
	ltfs_mutex_destroy(&mrsw->reading_mutex);
	ltfs_mutex_destroy(&mrsw->write_exclusive_mutex);
}

NOTRACE_FN static inline bool
try_acquirewrite_mrsw(MultiReaderSingleWriter *mrsw)
{
	int err;
	err = ltfs_mutex_trylock(&mrsw->write_exclusive_mutex);
	if (err)
		return false;
	err = ltfs_mutex_trylock(&mrsw->reading_mutex);
	if (err) {
		ltfs_mutex_unlock(&mrsw->write_exclusive_mutex);
		return false;
	}
	mrsw->writer=1;
	return true;
}

NOTRACE_FN static inline void
acquirewrite_mrsw(MultiReaderSingleWriter *mrsw)
{
	ltfs_mutex_lock(&mrsw->write_exclusive_mutex);
	ltfs_mutex_lock(&mrsw->reading_mutex);
	mrsw->writer=1;
}

NOTRACE_FN static inline void
releasewrite_mrsw(MultiReaderSingleWriter *mrsw)
{
	mrsw->writer=0;
	ltfs_mutex_unlock(&mrsw->reading_mutex);
	ltfs_mutex_unlock(&mrsw->write_exclusive_mutex);
}

NOTRACE_FN static inline void
acquireread_mrsw(MultiReaderSingleWriter *mrsw)
{
	ltfs_mutex_lock(&mrsw->write_exclusive_mutex);
	ltfs_mutex_unlock(&mrsw->write_exclusive_mutex);

	ltfs_mutex_lock(&mrsw->read_count_mutex);
	mrsw->read_count++;
	if(mrsw->read_count==1)
		ltfs_mutex_lock(&mrsw->reading_mutex);
	ltfs_mutex_unlock(&mrsw->read_count_mutex);
}

NOTRACE_FN static inline void
releaseread_mrsw(MultiReaderSingleWriter *mrsw)
{
	ltfs_mutex_lock(&mrsw->read_count_mutex);

	if ( mrsw->read_count<=0 ) {
		mrsw->read_count=0;
		ltfsmsg(LTFS_ERR, "17186E");
	} else {
		mrsw->read_count--;
		if(mrsw->read_count==0)
			ltfs_mutex_unlock(&mrsw->reading_mutex);
	}

	ltfs_mutex_unlock(&mrsw->read_count_mutex);
}

NOTRACE_FN static inline void
release_mrsw(MultiReaderSingleWriter *mrsw)
{
	if(mrsw->writer) {
		//If there is a writer, and we hold a lock, we must be the writer.
		releasewrite_mrsw(mrsw);
	} else {
		releaseread_mrsw(mrsw);
	}
}

//downgrades a write lock to a read lock
NOTRACE_FN static inline void
writetoread_mrsw(MultiReaderSingleWriter *mrsw)
{
	// This thread owns write protection meaning that
	// there are no threads that own read protection.

	// This means:
	// 1) read_count is currently 0 AND all threads requesting read and write protection
	//    are blocked on write_exclusive_mutex
	// 2) There may threads requesting read protection that got through the write_exclusive_mutex gate
	//    before this thread successfully got write protection.
	//    At most, one of this set of threads has gotten through the read_count_mutex gate, and may
	//    have already bumped read_count to 1, or is in the process of bumping read_count to 1. This
	//    thread may or may not be blocked on reading_mutex, but is following that code path.

	// Unset the writer flag before allowing any readers in.
	mrsw->writer = 0;

	// If there is a thread that got through the write_exclusive_mutex gate,
	// and the read_count_mutex gate, and is heading toward (or blocked on) the reading_mutex,
	// this will allow that thread to take the reading_mutex and then release the read_count_mutex.
	// The other threads that were blocked on the read_count_mutex gate can now go through,
	// and will update read_count correctly, and will not block on reading mutex.
	// read_count_mutex.
	ltfs_mutex_unlock(&mrsw->reading_mutex);

	// The rest of this function looks just like a readprotect function, except for the release
	// of the write_exclusive mutex at the end, which will resume normal many-reader single-writer
	// semantics for this structure.
	ltfs_mutex_lock(&mrsw->read_count_mutex);
	mrsw->read_count++;
	if(mrsw->read_count==1)
		ltfs_mutex_lock(&mrsw->reading_mutex);
	ltfs_mutex_unlock(&mrsw->read_count_mutex);

	//Release the write_exclusive_mutex, to allow others to write protect, and additional readers in.
	ltfs_mutex_unlock(&mrsw->write_exclusive_mutex);
}

#ifdef __cplusplus
}
#endif

#endif /* __MRSW_H__ */
