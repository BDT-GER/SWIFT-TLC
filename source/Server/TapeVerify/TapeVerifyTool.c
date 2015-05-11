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
 * Name        : TapeVerifyTool.cpp
 * Author      : hausst
 * Version     : 1.0
 * Description : Tool to analyze LTFS formatted medium for broken files
 */

#include <stdio.h>
#include <time.h>

// sg3 utils
#include "sg_lib.h"
#include "sg_pt.h"

// ltfs
#include "libltfs/ltfs.h"
#include "libltfs/ltfs_internal.h"
#include "libltfs/config_file.h"
#include "libltfs/plugin.h"
#include "libltfs/tape.h"

#include "LTFS.h"
#include "SCSI.h"

// SCSI defines
#define DEFAULT_TIMEOUT 60
#define MIN_SCSI_CDBSZ 6
#define MAX_SCSI_CDBSZ 256
#define MAX_SCSI_DXLEN (64 * 1024)

// Debug defines
//#define DEBUG

// disable the following define to speedup tool
// simulation of bad blocks will not work in this case
#define SIMULATE_BAD_BLOCKS

#include "debug.h"

// global variables
unsigned long g_block_size = 0;
unsigned long bad_blocks = 0;
int g_stop_verification = 0;
struct bad_block *bad_blocks_head = NULL;
struct bad_block *bad_blocks_tail = NULL;

/*
 * Signal handler to gracefully terminate the application
 */
void sig_handler(int signo) {
	g_stop_verification = 1;
	log_info_f("Stopping verification process...\n");
}

/*
 * Add a bad block to the linked list
 * Returns 0 on success, -1 otherwise
 */
int add_bad_block(tape_block_t pos) {
	struct bad_block *block = malloc(sizeof(struct bad_block));
	if (!block) {
		log_err("Out of memory!\n");
		return -1;
	}
	// add block to linked list
	block->next = NULL;
	block->pos = pos;
	block->file_list = NULL;
	bad_blocks_tail->next = block;
	bad_blocks_tail = block;

	bad_blocks++;

	return 0;
}

/*
 * Print all bad blocks
 */
void dump_bad_blocks() {
	struct bad_block *ptr = bad_blocks_head;
	while (ptr) {
		struct bad_block *current = ptr;
		ptr = ptr->next;
		if (current->pos != -1) {
			log_info("Bad block %lu\n", current->pos);
		}
	}
	fflush(stderr);
}

/*
 * Free linked list of bad blocks and file names
 */
void delete_bad_block_list() {
	struct bad_block *ptr = bad_blocks_head;
	while (ptr) {
		struct bad_block *current = ptr;
		struct filename *file = current->file_list;
		ptr = ptr->next;
		while (file) {
			struct filename *current_file = file;
			file = file->next;
			free(current_file->file);
			free(current_file);
		}
		free(current);
	}
}

/*
 * Dump all *possibly* corrupted files
 * Files are written to stderr and to a file if file_name is given
 */
void dump_corrupt_files(char *file_name) {
	struct bad_block *ptr = bad_blocks_head;

	// try to open file if file name is given
	FILE *f = NULL;
	if (file_name) {
		f = fopen(file_name, "w");
	}

	// loop through linked list
	while (ptr) {
		struct bad_block *current = ptr;
		struct filename *file_list = current->file_list;
		ptr = ptr->next;
		while (file_list) {
			struct filename *current_file = file_list;
			file_list = file_list->next;
			log_info("Block %ld, file %s\n", current->pos, current_file->file);
			// write file names to log file
			if (f) {
				fprintf(f, "%s\n", current_file->file);
			}
		}
	}
	fflush(stderr);

	// cleanup
	if (f) {
		fclose(f);
	}
}

/*
 * Use VERIFY6 SCSI CDBs to check from current tape position
 * Verify will ignore ILI, FM and will terminate on EOD or EOP
 *
 * Set bad_block_simluation a value > 0 to simulate a bad block
 * each bad_block_simluation number of blocks
 *
 * Set quick_check to a value > 0 to enable a quick check that
 * ends at quick_check blocks
 */
int verify_data(char* devname, int bad_block_simluation, int quick_check) {
	int ret = 0, i;
	int sg_fd = -1, sense_length = 0, cdb_length = 0, scsi_status = 0;
	unsigned long blocks_verified = 0, mb_verified = 0;
	struct sg_pt_base *ptvp = NULL;
	unsigned char cdb[MAX_SCSI_CDBSZ];
	unsigned char sense_buffer[32];
	time_t start, end;
	char *p;
	struct sg_scsi_sense_hdr ssh;

	// try to open SCSI device
	sg_fd = scsi_pt_open_device(devname, 1, 0);
	if (sg_fd < 0) {
		log_err("Failed to open device %s: %s\n", devname,
				safe_strerror(-sg_fd));
		ret = SG_LIB_FILE_ERROR;
		goto done;
	}

	// create SCSI object
	ptvp = construct_scsi_pt_obj();
	if (!ptvp) {
		log_err("construct_scsi_pt_obj failed\n");
		ret = SG_LIB_CAT_OTHER;
		goto done;
	}

	start = time(NULL);
	memset(sense_buffer, 0, sizeof(sense_buffer));

	// create CDB with correct block size
	p = (char*) &g_block_size;
	cdb[0] = 0x13;
	cdb[1] = 0x00;
	cdb[2] = p[2];	// block size MSB
	cdb[3] = p[1];
	cdb[4] = p[0];	// block size LSB
	cdb[5] = 0x00;
	cdb_length = 6;

	log_info("VERIFY6 CDB: ");
	for (i = 0; i < cdb_length; i++) {
		fprintf(stderr, "%02x ", cdb[i]);
	}
	fprintf(stderr, "\n");
	fflush(stderr);

	// loop until EOD/EOP or error detected
	while (!g_stop_verification) {
#ifdef SIMULATE_BAD_BLOCKS
		if(bad_block_simluation > 0) {
			// simulate a bad block
			if ((blocks_verified % bad_block_simluation) == 0) {
				struct tc_position pos;
				int s = readPositionLong(sg_fd, ptvp, &pos);
				if (s) {
					log_err_f("Error getting block position\n");
					break;
				}
				if (add_bad_block(pos.block)) {
					break;
				}
			}
		}
		// stop verification if quick check is enabled
		if (quick_check > 0) {
			if (blocks_verified >= quick_check) {
				ret = 0;
				break;
			}
		}
#endif
		// execute SCSI command
		ret = executeSCSICommand(sg_fd, ptvp, cdb, cdb_length, DEFAULT_TIMEOUT,
				&scsi_status, sense_buffer, sizeof(sense_buffer),
				&sense_length);

		// check for any other SG errors
		if (ret == SG_LIB_CAT_OTHER) {
			dump_scsi_info(scsi_status, sense_buffer, sense_length);
			break;
		}

		// parse SCSI result (some scenarios, e.g. ILI, FM, EOD are expected to happen
		if (scsi_status == SAM_STAT_GOOD) {
			; // nothing to do
		} else if (scsi_status == SAM_STAT_CHECK_CONDITION) {
			// no sense information present?
			if (ret == SG_LIB_CAT_NO_SENSE || ret == SG_LIB_CAT_RECOVERED) {
				// ILI is OK
				if (sense_buffer[2] & 0x20) {
					log_debug("Detected ILI at block %lu\n",
							blocks_verified);
				}
				// FM is also OK
				else if (sense_buffer[2] & 0x80) {
					log_info_f("Detected FM at block %lu\n", blocks_verified);
				} else {
					dump_scsi_info(scsi_status, sense_buffer, sense_length);
					break;
				}
			} else if (ret == SG_LIB_CAT_MEDIUM_HARD) {
				// check for EOD
				if (sg_scsi_normalize_sense(sense_buffer, sense_length, &ssh)) {
					dump_scsi_info(scsi_status, sense_buffer, sense_length);
					// EOD is 0x00/0x05, EOP is 0x00/0x02
					if (ssh.asc == 0x00
							&& (ssh.ascq == 0x02 || ssh.ascq == 0x05)) {
						log_info_f("EOP/EOD detected!\n");
						ret = 0;
						break;
					} else {
						// all other errors are treated as bad blocks up to now
						struct tc_position pos;
						int s = readPositionLong(sg_fd, ptvp, &pos);
						if (s) {
							log_err_f("Error getting block position\n");
							break;
						}
						if (add_bad_block(pos.block)) {
							break;
						}
					}
				} else {
					// this should not happen
					log_err_f("sg_scsi_normalize_sense failed\n");
					dump_scsi_info(scsi_status, sense_buffer, sense_length);
					break;
				}
			} else {
				// no medium or hardware error
				dump_scsi_info(scsi_status, sense_buffer, sense_length);
				break;
			}

			// reset sense buffer to not trigger additional warnings
			memset(sense_buffer, 0, sizeof(sense_buffer));
		} else {
			// all other status info are handled here (except GOOD, CHECK_CONDITION)
			dump_scsi_info(scsi_status, sense_buffer, sense_length);
			break;
		}
		blocks_verified++;
	}
	// interrupted by signal?
	if (g_stop_verification) {
		log_info("Not all blocks have been verified!\n");
		ret = 0;
	}
	end = time(NULL);

	log_info("Blocks verified: %lu\n", blocks_verified);
	log_info("Duration: %d seconds\n", (int) (end - start));
	// assume each block is filled up to 65% with data
	mb_verified = (blocks_verified * g_block_size / 1048576ul * 65ul / 100ul );
	if (((int)end - (int)start) > 0) {
		log_info("Blocks per second: %lu\n", blocks_verified / (end -start));
	}
	log_info("Approximate MB verified: %lu\n", mb_verified);
	if (((int)end - (int)start) > 0) {
		log_info("Approximate speed: %.2f MB/s\n",
			(double) ((double) mb_verified / (double) (end - start)));
	}
	log_info_f("\n");

done:
	if (ptvp)
		destruct_scsi_pt_obj(ptvp);
	if (sg_fd >= 0)
		scsi_pt_close_device(sg_fd);

	return ret;
}

/*
 * Print usage
 */
void printUsage(char* s) {
	fprintf(stderr, "\nUsage: %s -f <path to tape drive> [ -s <path to sg device -b <bad blocks> -q <quick check> -l <path to log file>]\n\n", s);
	fprintf(stderr, "Example: %s -f /dev/nst0\n", s);
	fprintf(stderr, "Example: %s -f /dev/IBMtape0 -g /dev/sg8\n", s);
	fprintf(stderr, "Set -b 10000 to simulate a bad block every 10000 blocks\n");
	fprintf(stderr, "Set -q 100000 to end verification after the first 100000 blocks\n");
	fprintf(stderr, "Set -l /tmp/verify.log to write possibly corrupted files to /tmp/verify.log\n");
	fprintf(stderr, "The drive must be loaded with a tape\n");
	fprintf(stderr, "before running this tool!\n\n");
}

/*
 * Entry point
 */
int main(int argc, char* argv[]) {
	int ret = 0, i;
	struct bad_block *bad_block_list = NULL;
	time_t start, end;
	int bad_block_simluation = 0;
	int quick_check = 0;
	char *device_name = NULL;
	char *sg_name = NULL;
	char *log_file = NULL;

	// check arguments
	while ((i = getopt(argc, argv, "b:q:f:g:l:")) != -1) {
		switch (i) {
		case 'b':
			bad_block_simluation = strtol(optarg, NULL, 10);
			break;
		case 'f':
			device_name = optarg;
			break;
		case 'g':
			sg_name = optarg;
			break;
		case 'l':
			log_file = optarg;
			break;
		case 'q':
			quick_check = strtol(optarg, NULL, 10);
			break;
		case '?':
			if (optopt == 'q' || optopt == 'b' || optopt == 'f' || optopt == 'g' || optopt == 'l')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			printUsage(argv[0]);
			return -1;
		default:
			printUsage(argv[0]);
			return -1;
		}
	}

	// device name must be given
	if(!device_name) {
		printUsage(argv[0]);
		return -1;
	}

#ifndef SIMULATE_BAD_BLOCKS
	log_warn("*********************************\n");
	log_warn("Warning: Simulation of bad blocks\n");
	log_warn("is disabled by compile option!\n");
	log_warn_f("*********************************\n");
#endif

	// for informational purpose
	start = time(NULL);

	/* **********************************************************
	 * Use LTFS library functionality to get block size of tape *
	 ************************************************************/
	// get LTFS block size
	log_info_f("Getting block size of loaded cartridge...\n");
	ret = get_ltfs_blocksize(device_name, &g_block_size);
	if (ret) {
		log_err("Failed to mount LTFS volume!\n");
		return 1;
	}

	log_info_f("LTFS block size: %lu (0x%lx)\n", g_block_size,
			g_block_size);

	/* ******************************************
	 * Use VERIFY6 CDBs to check data partition *
	 ********************************************/
	// initialize list of bad blocks
	bad_block_list = malloc(sizeof(struct bad_block));
	if (!bad_block_list) {
		log_err("Out of memory!\n");
		return -1;
	}
	bad_block_list->next = NULL;
	bad_block_list->pos = -1;
	bad_blocks_head = bad_block_list;
	bad_blocks_tail = bad_block_list;

	/*
	 * Install signal handler to gracefully stop verification
	 * Signal handler has to be installed at this point because
	 * the LTFS library will install its own signal handlers and
	 * removes them again
	 */
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		log_err("Unable to register signal handler!\n");
	}

	// verify data partition
	log_info_f("Verifying data on tape using VERIFY6 commands...\n");
	if (sg_name != NULL)
		ret = verify_data(sg_name, bad_block_simluation, quick_check);
	else
		ret = verify_data(device_name, bad_block_simluation, quick_check);

	// do not do any additional steps if no bad blocks found
	if (bad_blocks) {
		// print all bad blocks
		dump_bad_blocks();

		// get file names from bad blocks
		log_info_f("Retrieving file names for bad blocks on tape...\n");
		get_ltfs_files(device_name, bad_block_list);

		// print all corrupt files
		dump_corrupt_files(log_file);
	} else {
		log_info_f("No bad blocks found on tape!\n");
	}

	// cleanup
	delete_bad_block_list();

	// for informational purpose
	end = time(NULL);

	log_info("Complete verification took %d seconds\n", (int) (end - start));

	return ret;
}
