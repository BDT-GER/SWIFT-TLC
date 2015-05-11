/* Copyright (c) 2012 BDT Media Automation GmbH
 *
 * Licensed under the terms of the GNU Lesser
 * General Public License as published by the Free Software Foundation,
 * version 2.1 of the License.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * or download the license from <http://www.gnu.org/licenses/>.
 *
 * LTFS.h
 *
 *  Created on: Oct 9, 2014
 *      Author: hausst
 */

#ifndef LTFS_H_
#define LTFS_H_

// forward declarations
struct ltfs_volume;
struct libltfs_plugin;
struct dentry;
struct tape_block;

struct filename {
	char *file;
	struct filename *next;
};

struct bad_block {
	tape_block_t pos;
    struct filename *file_list;
	struct bad_block *next;
};

int LTFS_init(struct ltfs_volume **vol, char *devname, struct libltfs_plugin *backend);
int LTFS_cleanup(struct ltfs_volume *vol, struct libltfs_plugin *backend);
int get_ltfs_blocksize(char* devname, unsigned long *block_size);
int get_ltfs_files(char* devname, struct bad_block *block_list);

void parse_dentry(struct dentry *d, struct bad_block *block);

#endif /* LTFS_H_ */
