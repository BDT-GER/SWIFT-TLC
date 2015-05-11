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
 * LTFS.c
 *
 *  Created on: Oct 9, 2014
 *      Author: hausst
 */

// ltfs library
#include "libltfs/ltfs.h"
#include "libltfs/ltfs_internal.h"
#include "libltfs/config_file.h"
#include "libltfs/plugin.h"
#include "libltfs/tape.h"

#include "LTFS.h"

#include "debug.h"

extern unsigned long g_block_size;

int LTFS_init(struct ltfs_volume **vol, char *devname, struct libltfs_plugin *backend) {
	int ret = PROG_NO_ERRORS;
	char *backend_path = NULL;
	char *config_file = NULL;
	struct config_file *config;

	// sanity checks
	if (!vol) {
		return -1;
	}

	// Start up libltfs with the default logging level
#ifdef HP_BUILD
    ret = ltfs_init(LTFS_INFO, true);
#else
	ret = ltfs_init(LTFS_INFO, true, true);
#endif
	if (ret < 0) {
		log_err("ltfs_init failed\n");
		return PROG_OPERATIONAL_ERROR;
	}

	// Setup signal handler to terminate cleanly
	ret = ltfs_set_signal_handlers();
	if (ret < 0) {
		log_err("ltfs_set_signal_handlers failed\n");
		goto out_ltfs_finish;
	}

	// Load configuration file
	ret = config_file_load(config_file, &config);
	if (ret < 0) {
		log_err("config_file_load failed\n");
		goto out_unset_signal_handlers;
	}

	// Allocate new LTFS volume
	ret = ltfs_volume_alloc("TapeVerifyTool", vol);
	if (ret < 0) {
		log_err("ltfs_volume_alloc failed\n");
		goto out_config_free;
	}

	// get the driver for the default backend
	const char *default_backend = config_file_get_default_plugin("driver",
			config);
	if (!default_backend) {
		log_err("config_file_get_default_plugin failed\n");
		goto out_volume_free;
	}
	backend_path = strdup(default_backend);

	// load the backend, open the tape device, and load a tape
	ret = plugin_load(backend, "driver", backend_path, config);
	if (ret < 0) {
		log_err("plugin_load failed\n");
		goto out_volume_free;
	}

	ret = PROG_OPERATIONAL_ERROR;
	if (ltfs_device_open(devname, backend->ops, *vol) < 0) {
		log_err("ltfs_device_open failed\n");
		goto out_unload_backend;
	}

	(*vol)->append_only_mode = false;
	(*vol)->set_pew = false;
	if (ltfs_setup_device(*vol)) {
		goto out_close;
	}

	// success
	ret = 0;

	// config file is not used anymore
	free(backend_path);
	config_file_free(config);

	goto out;

out_close:
	ltfs_device_close(*vol);

out_unload_backend:
	plugin_unload(backend);

out_volume_free:
	if (backend_path)
		free(backend_path);
	ltfs_volume_free(vol);

out_config_free:
	config_file_free(config);

out_unset_signal_handlers:
	ltfs_unset_signal_handlers();

out_ltfs_finish:
	ltfs_finish();

out:
	return ret;
}

/*
 * cleanup LTFS resources
 */
int LTFS_cleanup(struct ltfs_volume *vol, struct libltfs_plugin *backend) {
	int ret = 0;
	// first we need to close the device
	if (vol) {
		ltfs_device_close(vol);
	}
	// then we can unload the plugin
	if (backend) {
		plugin_unload(backend);
	}
	// now finally free the volume
	if (vol) {
		ltfs_volume_free(&vol);
	}

	// remaining static cleanup
	ltfs_unset_signal_handlers();
	ltfs_finish();

	return ret;
}

/*
 * Get LTFS block size
 * This function will initialize the LTFS library, get the
 * block size and will position the tape to the start of the
 * data partition
 */
int get_ltfs_blocksize(char* devname, unsigned long *block_size) {
	int ret = 0;
	struct ltfs_volume *vol = NULL;
	struct tc_position seekpos;
	struct libltfs_plugin backend;

	ret = LTFS_init(&vol, devname, &backend);
	if (ret) {
		goto done;
	}

	ret = ltfs_start_mount(false, vol);
	if (ret < 0) {
		// ltfs_start_mount() already generated an appropriate error message
		goto done;
	}

	// get blocksize
	*block_size = ltfs_get_blocksize(vol);

	// position to start of data partition
	seekpos.partition = ltfs_part_id2num(vol->label->partid_dp, vol);
	seekpos.block = 0;
	ret = tape_seek(vol->device, &seekpos);
	if (ret != 0) {
		log_err("tape_seek failed\n");
	}

done:
	LTFS_cleanup(vol, &backend);
	vol = NULL;

	return ret;
}

/*
 * Get files on LTFS volume by block number
 * This function will initialize the LTFS library, mount the
 * LTFS volume and retrieve the filename for each block
 */
int get_ltfs_files(char* devname, struct bad_block *block_list) {
	int ret = 0;
	struct ltfs_volume *vol = NULL;
	struct libltfs_plugin backend;

	ret = LTFS_init(&vol, devname, &backend);
	if (ret) {
		goto done;
	}

	ret = ltfs_mount(false, false, false, false, 0, vol);
	if (ret < 0) {
		// ltfs_start_mount() already generated an appropriate error message
		goto done;
	}

	if(vol->index->root) {
		while (block_list != NULL) {
			if (block_list->pos != -1) {
				parse_dentry(vol->index->root, block_list);
			}
			block_list = block_list->next;
		}
	}

	// unmount (not needed yet)
#ifdef HP_BUILD
	ltfs_unmount(vol);
#else
	ltfs_unmount(SYNC_UNMOUNT, vol);
#endif

done:
	LTFS_cleanup(vol, &backend);
	vol = NULL;

	return ret;
}

/*
 * Adds a filename to the bad block structure
 */
void add_filename_to_bad_block(struct bad_block *block, char *file_name) {
	struct filename *file_list_ptr = block->file_list;
	struct filename *file = malloc(sizeof(struct filename));
	if (!file) {
		log_err("Out of memory!\n");
		return;
	}
	// add filename to linked list
	file->next = NULL;
	// file->file must be free'd by the caller!
	file->file = strdup(file_name);

	// check for empty file list
	if (!file_list_ptr) {
		block->file_list = file;
		return;
	}

	// reaching here means that at least one file
	// is already in the file list, so traverse the list
	while (file_list_ptr) {
		if (!file_list_ptr->next) {
			file_list_ptr->next = file;
			break;
		}
		file_list_ptr = file_list_ptr->next;
	}
}

/*
 * Parse an LTFS DENTRY to check if the bad block b
 * is contained in the DENTRY
 * If so, add the file name to the bad block list
 */
void parse_dentry(struct dentry *d, struct bad_block *block) {
	struct name_list *list_ptr = d->child_list;

	while(list_ptr) {
		if (list_ptr->d->isdir) {
			parse_dentry(list_ptr->d, block);
		} else {
			struct extent_info *ext;

			if (! TAILQ_EMPTY(&list_ptr->d->extentlist)) {
				TAILQ_FOREACH(ext, &list_ptr->d->extentlist, list) {
					tape_block_t ext_lastblock;
					ext_lastblock = ext->start.block + ext->bytecount / g_block_size;
					ext_lastblock += (ext->bytecount % g_block_size > 0) ? 1 : 0;

					if (block->pos >= ext->start.block && block->pos <= ext_lastblock) {
						// generate full file name for found file
						char file_name[1024], temp[1024];
						struct dentry *p = list_ptr->d->parent;
						memset(file_name, 0, sizeof(file_name));
						strncpy(file_name, list_ptr->d->name, sizeof(file_name) - 1);
						while(p) {
							snprintf(temp, sizeof(temp), "%s/%s", p->name, file_name);
							strncpy(file_name, temp, sizeof(file_name) - 1);
							p = p->parent;
						}

						log_debug("Found file for block %lu: %s\n", block->pos, file_name);

						// add filename to bad block list
						add_filename_to_bad_block(block, file_name);
					}
				}
			}
		}
		list_ptr = list_ptr->hh.next;
	}
}
