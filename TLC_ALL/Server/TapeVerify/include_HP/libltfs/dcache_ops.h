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
** FILE NAME:       dcache_ops.h
**
** DESCRIPTION:     Defines operations that must be supported by the dentry cache managers.
**
** AUTHOR:          Lucas C. Villa Real
**                  IBM Brazil Research Lab
**                  lucasvr@br.ibm.com
**
*************************************************************************************
*/
#ifndef __dcache_ops_h
#define __dcache_ops_h

#include "ltfs.h"

#include <sys/types.h>
#include <dirent.h>

enum dcache_flush_flags {
	FLUSH_XATTRS      = 1,
	FLUSH_EXTENT_LIST = 2,
	FLUSH_METADATA    = 4,
	FLUSH_ALL         = 7,
};

/**
 * Dentry cache options specified in the LTFS configuration file.
 */
struct dcache_options {
	bool enabled;  /**< Disk cache is enabled */
	int minsize;   /**< Minimum size (initial size of dcache image) in GB */
	int maxsize;   /**< Maximum size (final size of dcache image) in GB */
};

/**
 * dcache_ops structure.
 * Defines operations that must be supported by the dentry cache managers.
 */
struct dcache_ops {
	/* Initialization, deinitialization and management */
	void    *(*init)(const struct dcache_options *options, struct ltfs_volume *vol);
	int      (*destroy)(void *dcache_handle);
	int      (*mkcache)(const char *name, void *dcache_handle);
	int      (*rmcache)(const char *name, void *dcache_handle);
	int      (*cache_exists)(const char *name, bool *exists, bool *dirty, void *dcache_handle);
	int      (*set_workdir)(const char *workdir, void *dcache_handle);
	int      (*get_workdir)(char **workdir, void *dcache_handle);
	int      (*is_loaded)(bool *loaded, void *dcache_handle);
	int      (*load)(const char *name, void *dcache_handle);
	int      (*unload)(void *dcache_handle);
	int      (*set_dirty)(bool dirty, void *dcache_handle);

	/* Disk image management */
	int      (*diskimage_create)(void *dcache_handle);
	int      (*diskimage_remove)(void *dcache_handle);
	int      (*diskimage_mount)(void *dcache_handle);
	int      (*diskimage_unmount)(void *dcache_handle);
	bool     (*diskimage_is_full)(void);

	/* Advisory lock operations */
	int      (*get_advisory_lock)(const char *name, void *dcache_handle);
	int      (*put_advisory_lock)(const char *name, void *dcache_handle);

	/* File system operations */
	int      (*open)(const char *path, struct dentry **d, void *dcache_handle);
	int      (*openat)(const char *parent_path, struct dentry *parent, const char *name,
						struct dentry **result, void *dcache_handle);
	int      (*close)(struct dentry *d, bool lock_meta, bool descend, void *dcache_handle);
	int      (*create)(const char *path, struct dentry *d, void *dcache_handle);
	int      (*unlink)(const char *path, struct dentry *d, void *dcache_handle);
	int      (*rename)(const char *oldpath, const char *newpath, struct dentry **old_dentry,
						void *dcache_handle);
	int      (*flush)(struct dentry *d, enum dcache_flush_flags flags, void *dcache_handle);
	int      (*readdir)(struct dentry *d, bool dentries, void ***result, void *dcache_handle);
	int      (*setxattr)(const char *path, struct dentry *d, const char *xattr, const char *value,
						size_t size, int flags, void *dcache_handle);
	int      (*removexattr)(const char *path, struct dentry *d, const char *xattr,
						void *dcache_handle);
	int      (*listxattr)(const char *path, struct dentry *d, char *list, size_t size,
						void *dcache_handle);
	int      (*getxattr)(const char *path, struct dentry *d, const char *name,
						void *value, size_t size, void *dcache_handle);

	/* Helper operations */
	int      (*get_dentry)(struct dentry *d, void *dcache_handle);
	int      (*put_dentry)(struct dentry *d, void *dcache_handle);
};

struct dcache_ops *dcache_get_ops(void);
const char *dcache_get_message_bundle_name(void **message_data);

#endif /* __dcache_ops_h */
