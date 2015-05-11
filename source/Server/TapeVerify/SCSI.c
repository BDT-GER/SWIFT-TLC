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
 * SCSI.c
 *
 *  Created on: Oct 9, 2014
 *      Author: hausst
 */

// sg3 utils
#include "sg_lib.h"
#include "sg_pt.h"

#include "SCSI.h"
#include "debug.h"

#include "libltfs/tape_ops.h"

/*
 * Print information about SCSI sense buffer
 */
void dump_scsi_info(int status, unsigned char* sense_buffer, int sense_length) {
	fprintf(stderr, "SCSI Status: ");
	sg_print_scsi_status(status);
	fprintf(stderr, "\n\n");

	if (status == SAM_STAT_CHECK_CONDITION) {
		if (sense_length > 0) {
			fprintf(stderr, "Sense Information:\n");
			sg_print_sense(NULL, sense_buffer, sense_length, 1);
			fprintf(stderr, "\n");
		}
	}
	fflush(stderr);
}

/*
 * Execute SCSI command (no data transfer)
 * Caller must ensure that CDB and sense buffer is properly initialized / allocated
 * Function will return SCSI result status, sense buffer and sense length
 */
int executeSCSICommand(int sg_fd, struct sg_pt_base *ptvp, unsigned char *cdb,
		int cdb_length, int timeout, int *status, unsigned char *sense_buffer,
		int sense_buffer_length, int *sense_length) {
	int ret = 0;
	char b[128];
	int slen = 0;
	int res_cat = 0;

	// sanity checks
	if (sg_fd < 0) {
		ret = SG_LIB_CAT_OTHER;
		goto done;
	}
	if (ptvp == NULL || cdb == NULL || status == NULL || sense_buffer == NULL
			|| sense_length == NULL) {
		ret = SG_LIB_CAT_OTHER;
		goto done;
	}

	// prepare SCSI object
	set_scsi_pt_cdb(ptvp, cdb, cdb_length);
	set_scsi_pt_sense(ptvp, sense_buffer, sense_buffer_length);

	//set_scsi_pt_data_out(ptvp, dxfer_buffer_out, op->dataout_len);
	//set_scsi_pt_data_in(ptvp, dxfer_buffer_in, op->datain_len);

	// execute SCSI command
	ret = do_scsi_pt(ptvp, sg_fd, timeout, 0);
	if (ret > 0) {
		if (SCSI_PT_DO_BAD_PARAMS == ret) {
			log_err("do_scsi_pt: bad pass through setup\n");
			ret = SG_LIB_CAT_OTHER;
		} else if (SCSI_PT_DO_TIMEOUT == ret) {
			log_err("do_scsi_pt: timeout");
			ret = SG_LIB_CAT_TIMEOUT;
		} else
			ret = SG_LIB_CAT_OTHER;
		goto done;
	} else if (ret < 0) {
		log_err("do_scsi_pt: %s\n", safe_strerror(-ret));
		ret = SG_LIB_CAT_OTHER;
		goto done;
	}

	slen = 0;
	res_cat = get_scsi_pt_result_category(ptvp);
	switch (res_cat) {
	case SCSI_PT_RESULT_GOOD:
		ret = 0;
		break;
	case SCSI_PT_RESULT_SENSE:
		slen = get_scsi_pt_sense_len(ptvp);
		ret = sg_err_category_sense(sense_buffer, slen);
		break;
	case SCSI_PT_RESULT_TRANSPORT_ERR:
		get_scsi_pt_transport_err_str(ptvp, sizeof(b), b);
		log_warn(">>> transport error: %s\n", b);
		ret = SG_LIB_CAT_OTHER;
		break;
	case SCSI_PT_RESULT_OS_ERR:
		get_scsi_pt_os_err_str(ptvp, sizeof(b), b);
		log_warn(">>> os error: %s\n", b);
		ret = SG_LIB_CAT_OTHER;
		break;
	default:
		log_warn(">>> unknown pass through result " "category (%d)\n", res_cat);
		ret = SG_LIB_CAT_OTHER;
		break;
	}

	*sense_length = slen;

	*status = get_scsi_pt_status_response(ptvp);

	if ((SAM_STAT_CHECK_CONDITION == *status)) {
		if (SCSI_PT_RESULT_SENSE != res_cat)
			slen = get_scsi_pt_sense_len(ptvp);
	}
	if (SAM_STAT_RESERVATION_CONFLICT == *status)
		ret = SG_LIB_CAT_RES_CONFLICT;

done:
	// clear SCSI object to be used again
	clear_scsi_pt_obj(ptvp);

	return ret;
}


/*
 * Execute SCSI command (data transfer in)
 * Caller must ensure that CDB and sense buffer is properly initialized / allocated
 * Function will return SCSI result status, sense buffer and sense length
 */
int executeSCSICommandDataIn(int sg_fd, struct sg_pt_base *ptvp, unsigned char *cdb,
		int cdb_length, int timeout, int *status, unsigned char *sense_buffer,
		int sense_buffer_length, int *sense_length, unsigned char *in_buf, int in_buf_length, int *data_len) {
	int ret = 0;
	char b[128];
	int slen = 0;
	int res_cat = 0;

	// sanity checks
	if (sg_fd < 0) {
		ret = SG_LIB_CAT_OTHER;
		goto done;
	}
	if (ptvp == NULL || cdb == NULL || status == NULL || sense_buffer == NULL
			|| sense_length == NULL || in_buf == NULL ) {
		ret = SG_LIB_CAT_OTHER;
		goto done;
	}

	// prepare SCSI object
	set_scsi_pt_cdb(ptvp, cdb, cdb_length);
	set_scsi_pt_sense(ptvp, sense_buffer, sense_buffer_length);
	set_scsi_pt_data_in(ptvp, in_buf, in_buf_length);

	// execute SCSI command
	ret = do_scsi_pt(ptvp, sg_fd, timeout, 0);
	if (ret > 0) {
		if (SCSI_PT_DO_BAD_PARAMS == ret) {
			log_err("do_scsi_pt: bad pass through setup\n");
			ret = SG_LIB_CAT_OTHER;
		} else if (SCSI_PT_DO_TIMEOUT == ret) {
			log_err("do_scsi_pt: timeout");
			ret = SG_LIB_CAT_TIMEOUT;
		} else
			ret = SG_LIB_CAT_OTHER;
		goto done;
	} else if (ret < 0) {
		log_err("do_scsi_pt: %s\n", safe_strerror(-ret));
		ret = SG_LIB_CAT_OTHER;
		goto done;
	}

	slen = 0;
	res_cat = get_scsi_pt_result_category(ptvp);
	switch (res_cat) {
	case SCSI_PT_RESULT_GOOD:
		ret = 0;
		break;
	case SCSI_PT_RESULT_SENSE:
		slen = get_scsi_pt_sense_len(ptvp);
		ret = sg_err_category_sense(sense_buffer, slen);
		break;
	case SCSI_PT_RESULT_TRANSPORT_ERR:
		get_scsi_pt_transport_err_str(ptvp, sizeof(b), b);
		log_warn(">>> transport error: %s\n", b);
		ret = SG_LIB_CAT_OTHER;
		break;
	case SCSI_PT_RESULT_OS_ERR:
		get_scsi_pt_os_err_str(ptvp, sizeof(b), b);
		log_warn(">>> os error: %s\n", b);
		ret = SG_LIB_CAT_OTHER;
		break;
	default:
		log_warn(">>> unknown pass through result " "category (%d)\n", res_cat);
		ret = SG_LIB_CAT_OTHER;
		break;
	}

	*sense_length = slen;

	*status = get_scsi_pt_status_response(ptvp);

	if ((SAM_STAT_CHECK_CONDITION == *status)) {
		if (SCSI_PT_RESULT_SENSE != res_cat)
			slen = get_scsi_pt_sense_len(ptvp);
	}
	if (SAM_STAT_RESERVATION_CONFLICT == *status)
		ret = SG_LIB_CAT_RES_CONFLICT;

	*data_len = in_buf_length - get_scsi_pt_resid(ptvp);

done:
	// clear SCSI object to be used again
	clear_scsi_pt_obj(ptvp);

	return ret;
}

int readPositionLong(int sg_fd, struct sg_pt_base *ptvp, struct tc_position *pos) {
	int ret = 0;
	unsigned char cdb[10];
	int status = 0;
	unsigned char sense_buffer[32];
	unsigned char in_buf[32];
	int sense_length = 0;
	int data_len = 0;

	cdb[0] = 0x34;
	cdb[1] = 0x06;
	cdb[2] = 0x00;
	cdb[3] = 0x00;
	cdb[4] = 0x00;
	cdb[5] = 0x00;
	cdb[6] = 0x00;
	cdb[7] = 0x00;
	cdb[8] = 0x00;
	cdb[9] = 0x00;

	ret = executeSCSICommandDataIn(sg_fd, ptvp, cdb, sizeof(cdb), 60, &status, sense_buffer,
			sizeof(sense_buffer), &sense_length, in_buf, sizeof(in_buf), &data_len);
	if (ret)
		return ret;

	 pos->partition = ((tape_partition_t) in_buf[4] << 24) + ((tape_partition_t) in_buf[5] << 16) +
			  ((tape_partition_t) in_buf[6] <<  8) +  (tape_partition_t) in_buf[7];

	 pos->block =     ((tape_block_t) in_buf[8]  << 56)    + ((tape_block_t) in_buf[9]  << 48) +
					  ((tape_block_t) in_buf[10] << 40)    + ((tape_block_t) in_buf[11] << 32) +
					  ((tape_block_t) in_buf[12] << 24)    + ((tape_block_t) in_buf[13] << 16) +
					  ((tape_block_t) in_buf[14] <<  8)    +  (tape_block_t) in_buf[15];

	 pos->filemarks = ((tape_block_t) in_buf[16] << 56)    + ((tape_block_t) in_buf[17] << 48) +
					  ((tape_block_t) in_buf[18] << 40)    + ((tape_block_t) in_buf[19] << 32) +
					  ((tape_block_t) in_buf[20] << 24)    + ((tape_block_t) in_buf[21] << 16) +
					  ((tape_block_t) in_buf[22] <<  8)    +  (tape_block_t) in_buf[23];

	return 0;
}
