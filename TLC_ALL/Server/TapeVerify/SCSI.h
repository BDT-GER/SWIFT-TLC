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
 * SCSI.h
 *
 *  Created on: Oct 9, 2014
 *      Author: hausst
 */

#ifndef SCSI_H_
#define SCSI_H_

struct sg_pt_base;
struct tc_position;

void dump_scsi_info(int status, unsigned char* sense_buffer, int sense_length);

int executeSCSICommand(int sg_fd, struct sg_pt_base *ptvp, unsigned char *cdb,
		int cdb_length, int timeout, int *status, unsigned char *sense_buffer,
		int sense_buffer_length, int *sense_length);

int executeSCSICommandDataIn(int sg_fd, struct sg_pt_base *ptvp, unsigned char *cdb,
		int cdb_length, int timeout, int *status, unsigned char *sense_buffer,
		int sense_buffer_length, int *sense_length, unsigned char *in_buf, int in_buf_length, int *data_len);

int readPositionLong(int sg_fd, struct sg_pt_base *ptvp, struct tc_position *pos);

#endif /* SCSI_H_ */
