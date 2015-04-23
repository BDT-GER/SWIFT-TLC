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
 * debug.h
 *
 *  Created on: Oct 9, 2014
 *      Author: hausst
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG_FILE_LINES

#ifndef DEBUG
#define log_debug(M, ...)
#else
#define log_ debug(M, ...) do { fprintf(stderr, "DEBUG %s:%d: " M, __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
#define log_ debug_f(M, ...) do { fprintf(stderr, "DEBUG %s:%d: " M, __FILE__, __LINE__, ##__VA_ARGS__); fflush(stderr); } while (0)
#endif

#define log_err(M, ...) do { fprintf(stderr, "[ERROR] (%s:%d) " M, __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
#define log_err_f(M, ...) do { fprintf(stderr, "[ERROR] (%s:%d) " M, __FILE__, __LINE__, ##__VA_ARGS__); fflush(stderr); } while (0)

#define log_warn(M, ...) do { fprintf(stderr, "[WARN] (%s:%d) " M, __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
#define log_warn_f(M, ...) do { fprintf(stderr, "[WARN] (%s:%d) " M, __FILE__, __LINE__, ##__VA_ARGS__); fflush(stderr); } while (0)

#define log_info(M, ...) do { fprintf(stderr, "[INFO] (%s:%d) " M, __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
#define log_info_f(M, ...) do { fprintf(stderr, "[INFO] (%s:%d) " M, __FILE__, __LINE__, ##__VA_ARGS__); fflush(stderr); } while (0)

#else

#ifndef DEBUG
#define log_debug(M, ...)
#else
#define log_ debug(M, ...) do { fprintf(stderr, "DEBUG: " M, ##__VA_ARGS__); } while (0)
#define log_ debug_f(M, ...) do { fprintf(stderr, "DEBUG: " M, ##__VA_ARGS__); fflush(stderr); } while (0)
#endif

#define log_err(M, ...) do { fprintf(stderr, "[ERROR] " M, ##__VA_ARGS__); } while (0)
#define log_err_f(M, ...) do { fprintf(stderr, "[ERROR] " M, ##__VA_ARGS__); fflush(stderr); } while (0)

#define log_warn(M, ...) do { fprintf(stderr, "[WARN] " M, ##__VA_ARGS__); } while (0)
#define log_warn_f(M, ...) do { fprintf(stderr, "[WARN] " M, ##__VA_ARGS__); fflush(stderr); } while (0)

#define log_info(M, ...) do { fprintf(stderr, "[INFO] " M, ##__VA_ARGS__); } while (0)
#define log_info_f(M, ...) do { fprintf(stderr, "[INFO] " M, ##__VA_ARGS__); fflush(stderr); } while (0)

#endif

#endif /* DEBUG_H_ */
