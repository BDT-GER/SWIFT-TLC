#ifndef SG_LIB_H
#define SG_LIB_H

/*
 * Copyright (c) 2004-2014 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 */

/*
 *
 * On 5th October 2004 a FreeBSD license was added to this file.
 * The intention is to keep this file and the related sg_lib.c file
 * as open source and encourage their unencumbered use.
 *
 * Current version number is in the sg_lib.c file and can be accessed
 * with the sg_lib_version() function.
 */


/*
 * This header file contains defines and function declarations that may
 * be useful to applications that communicate with devices that use a
 * SCSI command set. These command sets have names like SPC-4, SBC-3,
 * SSC-3, SES-2 and draft standards defining them can be found at
 * http://www.t10.org . Virtually all devices in the Linux SCSI subsystem
 * utilize SCSI command sets. Many devices in other Linux device subsystems
 * utilize SCSI command sets either natively or via emulation (e.g. a
 * parallel ATA disk in a USB enclosure).
 */

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SCSI Peripheral Device Types (PDT) [5 bit field] */
#define PDT_DISK 0x0    /* direct access block device (disk) */
#define PDT_TAPE 0x1    /* sequential access device (magnetic tape) */
#define PDT_PRINTER 0x2 /* printer device (see SSC-1) */
#define PDT_PROCESSOR 0x3       /* processor device (e.g. SAFTE device) */
#define PDT_WO 0x4      /* write once device (some optical disks) */
#define PDT_MMC 0x5     /* CD/DVD/BD (multi-media) */
#define PDT_SCANNER 0x6 /* obsolete */
#define PDT_OPTICAL 0x7 /* optical memory device (some optical disks) */
#define PDT_MCHANGER 0x8        /* media changer device (e.g. tape robot) */
#define PDT_COMMS 0x9   /* communications device (obsolete) */
#define PDT_SAC 0xc     /* storage array controller device */
#define PDT_SES 0xd     /* SCSI Enclosure Services (SES) device */
#define PDT_RBC 0xe     /* Reduced Block Commands (simplified PDT_DISK) */
#define PDT_OCRW 0xf    /* optical card read/write device */
#define PDT_BCC 0x10    /* bridge controller commands */
#define PDT_OSD 0x11    /* Object Storage Device (OSD) */
#define PDT_ADC 0x12    /* Automation/drive commands (ADC) */
#define PDT_SMD 0x13    /* Security Manager Device (SMD) */
#define PDT_ZBC 0x14    /* Zoned Block Commands (ZBC) */
#define PDT_WLUN 0x1e   /* Well known logical unit (WLUN) */
#define PDT_UNKNOWN 0x1f        /* Unknown or no device type */

#ifndef SAM_STAT_GOOD
/* The SCSI status codes as found in SAM-4 at www.t10.org */
#define SAM_STAT_GOOD 0x0
#define SAM_STAT_CHECK_CONDITION 0x2
#define SAM_STAT_CONDITION_MET 0x4
#define SAM_STAT_BUSY 0x8
#define SAM_STAT_INTERMEDIATE 0x10              /* obsolete in SAM-4 */
#define SAM_STAT_INTERMEDIATE_CONDITION_MET 0x14  /* obsolete in SAM-4 */
#define SAM_STAT_RESERVATION_CONFLICT 0x18
#define SAM_STAT_COMMAND_TERMINATED 0x22        /* obsolete in SAM-3 */
#define SAM_STAT_TASK_SET_FULL 0x28
#define SAM_STAT_ACA_ACTIVE 0x30
#define SAM_STAT_TASK_ABORTED 0x40
#endif

/* The SCSI sense key codes as found in SPC-4 at www.t10.org */
#define SPC_SK_NO_SENSE 0x0
#define SPC_SK_RECOVERED_ERROR 0x1
#define SPC_SK_NOT_READY 0x2
#define SPC_SK_MEDIUM_ERROR 0x3
#define SPC_SK_HARDWARE_ERROR 0x4
#define SPC_SK_ILLEGAL_REQUEST 0x5
#define SPC_SK_UNIT_ATTENTION 0x6
#define SPC_SK_DATA_PROTECT 0x7
#define SPC_SK_BLANK_CHECK 0x8
#define SPC_SK_VENDOR_SPECIFIC 0x9
#define SPC_SK_COPY_ABORTED 0xa
#define SPC_SK_ABORTED_COMMAND 0xb
#define SPC_SK_RESERVED 0xc
#define SPC_SK_VOLUME_OVERFLOW 0xd
#define SPC_SK_MISCOMPARE 0xe
#define SPC_SK_COMPLETED 0xf

/* Transport protocol identifiers or just Protocol identifiers */
#define TPROTO_FCP 0
#define TPROTO_SPI 1
#define TPROTO_SSA 2
#define TPROTO_1394 3
#define TPROTO_SRP 4
#define TPROTO_ISCSI 5
#define TPROTO_SAS 6
#define TPROTO_ADT 7
#define TPROTO_ATA 8
#define TPROTO_UAS 9            /* USB attached SCSI */
#define TPROTO_SOP 0xa          /* SCSI over PCIe */
#define TPROTO_NONE 0xf


/* The format of the version string is like this: "1.87 20130731" */
const char * sg_lib_version();

/* Returns length of SCSI command given the opcode (first byte).
 * Yields the wrong answer for variable length commands (opcode=0x7f)
 * and potentially some vendor specific commands. */
int sg_get_command_size(unsigned char cdb_byte0);

/* Command name given pointer to the cdb. Certain command names
 * depend on peripheral type (give 0 if unknown). Places command
 * name into buff and will write no more than buff_len bytes. */
void sg_get_command_name(const unsigned char * cdbp, int peri_type,
                         int buff_len, char * buff);

/* Command name given only the first byte (byte 0) of a cdb and
 * peripheral type. */
void sg_get_opcode_name(unsigned char cdb_byte0, int peri_type, int buff_len,
                        char * buff);

/* Command name given opcode (byte 0), service action and peripheral type.
 * If no service action give 0, if unknown peripheral type give 0. */
void sg_get_opcode_sa_name(unsigned char cdb_byte0, int service_action,
                           int peri_type, int buff_len, char * buff);

/* Fetch scsi status string. */
void sg_get_scsi_status_str(int scsi_status, int buff_len, char * buff);

/* This is a slightly stretched SCSI sense "descriptor" format header.
 * The addition is to allow the 0x70 and 0x71 response codes. The idea
 * is to place the salient data of both "fixed" and "descriptor" sense
 * format into one structure to ease application processing.
 * The original sense buffer should be kept around for those cases
 * in which more information is required (e.g. the LBA of a MEDIUM ERROR). */
struct sg_scsi_sense_hdr {
    unsigned char response_code; /* permit: 0x0, 0x70, 0x71, 0x72, 0x73 */
    unsigned char sense_key;
    unsigned char asc;
    unsigned char ascq;
    unsigned char byte4;
    unsigned char byte5;
    unsigned char byte6;
    unsigned char additional_length;
};

/* Maps the salient data from a sense buffer which is in either fixed or
 * descriptor format into a structure mimicking a descriptor format
 * header (i.e. the first 8 bytes of sense descriptor format).
 * If zero response code returns 0. Otherwise returns 1 and if 'sshp' is
 * non-NULL then zero all fields and then set the appropriate fields in
 * that structure. sshp::additional_length is always 0 for response
 * codes 0x70 and 0x71 (fixed format). */
int sg_scsi_normalize_sense(const unsigned char * sensep, int sense_len,
                            struct sg_scsi_sense_hdr * sshp);

/* Attempt to find the first SCSI sense data descriptor that matches the
 * given 'desc_type'. If found return pointer to start of sense data
 * descriptor; otherwise (including fixed format sense data) returns NULL. */
const unsigned char * sg_scsi_sense_desc_find(const unsigned char * sensep,
                                              int sense_len, int desc_type);

/* Get sense key from sense buffer. If successful returns a sense key value
 * between 0 and 15. If sense buffer cannot be decode, returns -1 . */
int sg_get_sense_key(const unsigned char * sensep, int sense_len);

/* Yield string associated with sense_key value. Returns 'buff'. */
char * sg_get_sense_key_str(int sense_key, int buff_len, char * buff);

/* Yield string associated with ASC/ASCQ values. Returns 'buff'. */
char * sg_get_asc_ascq_str(int asc, int ascq, int buff_len, char * buff);

/* Returns 1 if valid bit set, 0 if valid bit clear. Irrespective the
 * information field is written out via 'info_outp' (except when it is
 * NULL). Handles both fixed and descriptor sense formats. */
int sg_get_sense_info_fld(const unsigned char * sensep, int sb_len,
                          uint64_t * info_outp);

/* Returns 1 if any of the 3 bits (i.e. FILEMARK, EOM or ILI) are set.
 * In descriptor format if the stream commands descriptor not found
 * then returns 0. Writes 1 or 0 corresponding to these bits to the
 * last three arguments if they are non-NULL. */
int sg_get_sense_filemark_eom_ili(const unsigned char * sensep, int sb_len,
                                  int * filemark_p, int * eom_p, int * ili_p);

/* Returns 1 if SKSV is set and sense key is NO_SENSE or NOT_READY. Also
 * returns 1 if progress indication sense data descriptor found. Places
 * progress field from sense data where progress_outp points. If progress
 * field is not available returns 0. Handles both fixed and descriptor
 * sense formats. N.B. App should multiply by 100 and divide by 65536
 * to get percentage completion from given value. */
int sg_get_sense_progress_fld(const unsigned char * sensep, int sb_len,
                              int * progress_outp);

/* Closely related to sg_print_sense(). Puts decoded sense data in 'buff'.
 * Usually multiline with multiple '\n' including one trailing. If
 * 'raw_sinfo' set appends sense buffer in hex. */
void sg_get_sense_str(const char * leadin, const unsigned char * sense_buffer,
                      int sb_len, int raw_sinfo, int buff_len, char * buff);

/* Yield string associated with peripheral device type (pdt). Returns
 * 'buff'. If 'pdt' out of range yields "bad pdt" string. */
char * sg_get_pdt_str(int pdt, int buff_len, char * buff);

/* Yield string associated with transport protocol identifier (tpi). Returns
 *    'buff'. If 'tpi' out of range yields "bad tpi" string. */
char * sg_get_trans_proto_str(int tpi, int buff_len, char * buff);

extern FILE * sg_warnings_strm;

void sg_set_warnings_strm(FILE * warnings_strm);

/* The following "print" functions send ACSII to 'sg_warnings_strm' file
 * descriptor (default value is stderr) */
void sg_print_command(const unsigned char * command);
void sg_print_sense(const char * leadin, const unsigned char * sense_buffer,
                    int sb_len, int raw_info);
void sg_print_scsi_status(int scsi_status);

/* Utilities can use these exit status values for syntax errors and
 * file (device node) problems (e.g. not found or permissions). */
#define SG_LIB_SYNTAX_ERROR 1   /* command line syntax problem */
#define SG_LIB_FILE_ERROR 15    /* device or other file problem */

/* The sg_err_category_sense() function returns one of the following.
 * These may be used as exit status values (from a process). Notice that
 * some of the lower values correspond to SCSI sense key values. */
#define SG_LIB_CAT_CLEAN 0      /* No errors or other information */
/* Value 1 left unused for utilities to use SG_LIB_SYNTAX_ERROR */
#define SG_LIB_CAT_NOT_READY 2  /* sense key, unit stopped? */
                                /*       [sk,asc,ascq: 0x2,*,*] */
#define SG_LIB_CAT_MEDIUM_HARD 3 /* medium or hardware error, blank check */
                                /*       [sk,asc,ascq: 0x3/0x4/0x8,*,*] */
#define SG_LIB_CAT_ILLEGAL_REQ 5 /* Illegal request (other than invalid */
                                /* opcode):   [sk,asc,ascq: 0x5,*,*] */
#define SG_LIB_CAT_UNIT_ATTENTION 6 /* sense key, device state changed */
                                /*       [sk,asc,ascq: 0x6,*,*] */
        /* was SG_LIB_CAT_MEDIA_CHANGED earlier [sk,asc,ascq: 0x6,0x28,*] */
#define SG_LIB_CAT_DATA_PROTECT 7 /* sense key, media write protected? */
                                /*       [sk,asc,ascq: 0x7,*,*] */
#define SG_LIB_CAT_INVALID_OP 9 /* (Illegal request,) Invalid opcode: */
                                /*       [sk,asc,ascq: 0x5,0x20,0x0] */
#define SG_LIB_CAT_COPY_ABORTED 10 /* sense key, some data transferred */
                                /*       [sk,asc,ascq: 0xa,*,*] */
#define SG_LIB_CAT_ABORTED_COMMAND 11 /* interpreted from sense buffer */
                                /*       [sk,asc,ascq: 0xb,! 0x10,*] */
#define SG_LIB_CAT_MISCOMPARE 14 /* sense key, probably verify */
                                /*       [sk,asc,ascq: 0xe,*,*] */
#define SG_LIB_CAT_NO_SENSE 20  /* sense data with key of "no sense" */
                                /*       [sk,asc,ascq: 0x0,*,*] */
#define SG_LIB_CAT_RECOVERED 21 /* Successful command after recovered err */
                                /*       [sk,asc,ascq: 0x1,*,*] */
#define SG_LIB_CAT_RES_CONFLICT SAM_STAT_RESERVATION_CONFLICT
                                /* 24: this is a SCSI status, not sense. */
                                /* It indicates reservation by another */
                                /* machine blocks this command */
#define SG_LIB_CAT_PROTECTION 40 /* subset of aborted command (for PI, DIF) */
                                /*       [sk,asc,ascq: 0xb,0x10,*] */
#define SG_LIB_CAT_MALFORMED 97 /* Response to SCSI command malformed */
#define SG_LIB_CAT_SENSE 98     /* Something else is in the sense buffer */
#define SG_LIB_CAT_OTHER 99     /* Some other error/warning has occurred */
                                /* (e.g. a transport or driver error) */

/* Returns a SG_LIB_CAT_* value. If cannot decode sense_buffer or a less
 * common sense key then return SG_LIB_CAT_SENSE .*/
int sg_err_category_sense(const unsigned char * sense_buffer, int sb_len);

/* Here are some additional sense data categories that are not returned
 * by sg_err_category_sense() but are returned by some related functions. */
#define SG_LIB_CAT_ILLEGAL_REQ_WITH_INFO 17 /* Illegal request (other than */
                                /* invalid opcode) plus 'info' field: */
                                /*  [sk,asc,ascq: 0x5,*,*] */
#define SG_LIB_CAT_MEDIUM_HARD_WITH_INFO 18 /* medium or hardware error */
                                /* sense key plus 'info' field: */
                                /*       [sk,asc,ascq: 0x3/0x4,*,*] */
#define SG_LIB_CAT_PROTECTION_WITH_INFO 41 /* aborted command sense key, */
                                /* protection plus 'info' field: */
                                /*  [sk,asc,ascq: 0xb,0x10,*] */
#define SG_LIB_CAT_TIMEOUT 33

/* Yield string associated with sense category. Returns 'buff' (or pointer
 * to "Bad sense category" if 'buff' is NULL). If sense_cat unknown then
 * yield "Sense category: <sense_cat>" string. */
const char * sg_get_category_sense_str(int sense_cat, int buff_len,
                                       char * buff, int verbose);


/* Iterates to next designation descriptor in the device identification
 * VPD page. The 'initial_desig_desc' should point to start of first
 * descriptor with 'page_len' being the number of valid bytes in that
 * and following descriptors. To start, 'off' should point to a negative
 * value, thereafter it should point to the value yielded by the previous
 * call. If 0 returned then 'initial_desig_desc + *off' should be a valid
 * descriptor; returns -1 if normal end condition and -2 for an abnormal
 * termination. Matches association, designator_type and/or code_set when
 * any of those values are greater than or equal to zero. */
int sg_vpd_dev_id_iter(const unsigned char * initial_desig_desc, int page_len,
                       int * off, int m_assoc, int m_desig_type,
                       int m_code_set);


/* <<< General purpose (i.e. not SCSI specific) utility functions >>> */

/* Always returns valid string even if errnum is wild (or library problem).
 * If errnum is negative, flip its sign. */
char * safe_strerror(int errnum);


/* Print (to stdout) 'str' of bytes in hex, 16 bytes per line optionally
 * followed at the right hand side of the line with an ASCII interpretation.
 * Each line is prefixed with an address, starting at 0 for str[0]..str[15].
 * All output numbers are in hex. 'no_ascii' allows for 3 output types:
 *     > 0     each line has address then up to 16 ASCII-hex bytes
 *     = 0     in addition, the bytes are listed in ASCII to the right
 *     < 0     only the ASCII-hex bytes are listed (i.e. without address)
*/
void dStrHex(const char* str, int len, int no_ascii);

/* Print (to sg_warnings_strm (stderr)) 'str' of bytes in hex, 16 bytes per
 * line optionally followed at right by its ASCII interpretation. Same
 * logic as dStrHex() with different output stream (i.e. stderr). */
void dStrHexErr(const char* str, int len, int no_ascii);

/* Read 'len' bytes from 'str' and output as ASCII-Hex bytes (space
 * separated) to 'b' not to exceed 'b_len' characters. Each line
 * starts with 'leadin' (NULL for no leadin) and there are 16 bytes
 * per line with an extra space between the 8th and 9th bytes. 'format'
 * is unused, set to 0 . */
void dStrHexStr(const char* str, int len, const char * leadin, int format,
                int b_len, char * b);

/* Returns 1 when executed on big endian machine; else returns 0.
 * Useful for displaying ATA identify words (which need swapping on a
 * big endian machine).
*/
int sg_is_big_endian();

/* Extract character sequence from ATA words as in the model string
 * in a IDENTIFY DEVICE response. Returns number of characters
 * written to 'ochars' before 0 character is found or 'num' words
 * are processed. */
int sg_ata_get_chars(const unsigned short * word_arr, int start_word,
                     int num_words, int is_big_endian, char * ochars);

/* Print (to stdout) 16 bit 'words' in hex, 8 words per line optionally
 * followed at the right hand side of the line with an ASCII interpretation
 * (pairs of ASCII characters in big endian order (upper first)).
 * Each line is prefixed with an address, starting at 0.
 * All output numbers are in hex. 'no_ascii' allows for 3 output types:
 *     > 0     each line has address then up to 8 ASCII-hex words
 *     = 0     in addition, the words are listed in ASCII pairs to the right
 *     = -1    only the ASCII-hex words are listed (i.e. without address)
 *     = -2    only the ASCII-hex words, formatted for "hdparm --Istdin"
 *     < -2    same as -1
 * If 'swapb' non-zero then bytes in each word swapped. Needs to be set
 * for ATA IDENTIFY DEVICE response on big-endian machines.
*/
void dWordHex(const unsigned short* words, int num, int no_ascii, int swapb);

/* If the number in 'buf' can not be decoded or the multiplier is unknown
 * then -1 is returned. Accepts a hex prefix (0x or 0X) or a 'h' (or 'H')
 * suffix. Otherwise a decimal multiplier suffix may be given. Recognised
 * multipliers: c C  *1;  w W  *2; b  B *512;  k K KiB  *1,024;
 * KB  *1,000;  m M MiB  *1,048,576; MB *1,000,000; g G GiB *1,073,741,824;
 * GB *1,000,000,000 and <n>x<m> which multiplies <n> by <m> . Ignore leading
 * spaces and tabs; accept comma, space, tab and hash as terminator. */
int sg_get_num(const char * buf);

/* If the number in 'buf' can not be decoded then -1 is returned. Accepts a
 * hex prefix (0x or 0X) or a 'h' (or 'H') suffix; otherwise decimal is
 * assumed. Does not accept multipliers. Accept a comma (","), a whitespace
 * or newline as terminator.  */
int sg_get_num_nomult(const char * buf);

/* If the number in 'buf' can not be decoded or the multiplier is unknown
 * then -1LL is returned. Accepts a hex prefix (0x or 0X) or a 'h' (or 'H')
 * suffix. Otherwise a decimal multiplier suffix may be given. In addition
 * to supporting the multipliers of sg_get_num(), this function supports:
 * t T TiB  *(2**40); TB *(10**12); p P PiB  *(2**50); PB  *(10**15) .
 * Ignore leading spaces and tabs; accept comma, space, tab and hash as
 * terminator. */
int64_t sg_get_llnum(const char * buf);


/* <<< Architectural support functions [is there a better place?] >>> */

/* Non Unix OSes distinguish between text and binary files.
 * Set text mode on fd. Does nothing in Unix. Returns negative number on
 * failure. */
int sg_set_text_mode(int fd);

/* Set binary mode on fd. Does nothing in Unix. Returns negative number on
 * failure. */
int sg_set_binary_mode(int fd);

#ifdef __cplusplus
}
#endif

#endif
