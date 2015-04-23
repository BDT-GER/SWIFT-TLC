#ifndef SG_PT_WIN32_H
#define SG_PT_WIN32_H
/*
 * The information in this file was obtained from scsi-wnt.h by
 * Richard Stemmer, rs@epost.de . He in turn gives credit to
 * Jay A. Key (for scsipt.c).
 * The plscsi program (by Pat LaVarre <p.lavarre@ieee.org>) has
 * also been used as a reference.
 * Much of the information in this header can also be obtained
 * from msdn.microsoft.com .
 * Updated for cygwin version 1.7.17 changes 20121026
 */

#include <windows.h>

#define SCSI_MAX_SENSE_LEN 64
#define SCSI_MAX_CDB_LEN 16
#define SCSI_MAX_INDIRECT_DATA 16384

typedef struct {
        USHORT          Length;
        UCHAR           ScsiStatus;
        UCHAR           PathId;
        UCHAR           TargetId;
        UCHAR           Lun;
        UCHAR           CdbLength;
        UCHAR           SenseInfoLength;
        UCHAR           DataIn;
        ULONG           DataTransferLength;
        ULONG           TimeOutValue;
        ULONG_PTR       DataBufferOffset;  /* was ULONG; problem in 64 bit */
        ULONG           SenseInfoOffset;
        UCHAR           Cdb[SCSI_MAX_CDB_LEN];
} SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;


typedef struct {
        USHORT          Length;
        UCHAR           ScsiStatus;
        UCHAR           PathId;
        UCHAR           TargetId;
        UCHAR           Lun;
        UCHAR           CdbLength;
        UCHAR           SenseInfoLength;
        UCHAR           DataIn;
        ULONG           DataTransferLength;
        ULONG           TimeOutValue;
        PVOID           DataBuffer;
        ULONG           SenseInfoOffset;
        UCHAR           Cdb[SCSI_MAX_CDB_LEN];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;


typedef struct {
        SCSI_PASS_THROUGH spt;
        /* plscsi shows a follow on 16 bytes allowing 32 byte cdb */
        ULONG           Filler;
        UCHAR           ucSenseBuf[SCSI_MAX_SENSE_LEN];
        UCHAR           ucDataBuf[SCSI_MAX_INDIRECT_DATA];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;


typedef struct {
        SCSI_PASS_THROUGH_DIRECT spt;
        ULONG           Filler;
        UCHAR           ucSenseBuf[SCSI_MAX_SENSE_LEN];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;



typedef struct {
        UCHAR           NumberOfLogicalUnits;
        UCHAR           InitiatorBusId;
        ULONG           InquiryDataOffset;
} SCSI_BUS_DATA, *PSCSI_BUS_DATA;


typedef struct {
        UCHAR           NumberOfBusses;
        SCSI_BUS_DATA   BusData[1];
} SCSI_ADAPTER_BUS_INFO, *PSCSI_ADAPTER_BUS_INFO;


typedef struct {
        UCHAR           PathId;
        UCHAR           TargetId;
        UCHAR           Lun;
        BOOLEAN         DeviceClaimed;
        ULONG           InquiryDataLength;
        ULONG           NextInquiryDataOffset;
        UCHAR           InquiryData[1];
} SCSI_INQUIRY_DATA, *PSCSI_INQUIRY_DATA;


typedef struct {
        ULONG           Length;
        UCHAR           PortNumber;
        UCHAR           PathId;
        UCHAR           TargetId;
        UCHAR           Lun;
} SCSI_ADDRESS, *PSCSI_ADDRESS;


/*
 * method codes
 */
#define METHOD_BUFFERED         0
#define METHOD_IN_DIRECT        1
#define METHOD_OUT_DIRECT       2
#define METHOD_NEITHER          3

/*
 * file access values
 */
#define FILE_ANY_ACCESS         0
#ifndef FILE_READ_ACCESS
#define FILE_READ_ACCESS        0x0001
#endif
#ifndef FILE_WRITE_ACCESS
#define FILE_WRITE_ACCESS       0x0002
#endif


#define IOCTL_SCSI_BASE    0x00000004

/*
 * constants for DataIn member of SCSI_PASS_THROUGH* structures
 */
#define SCSI_IOCTL_DATA_OUT             0
#define SCSI_IOCTL_DATA_IN              1
#define SCSI_IOCTL_DATA_UNSPECIFIED     2

/*
 * Standard IOCTL define
 */
#ifndef CTL_CODE
#define CTL_CODE(DevType, Function, Method, Access)             \
        (((DevType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

#define IOCTL_SCSI_PASS_THROUGH         CTL_CODE(IOCTL_SCSI_BASE, 0x0401, \
	METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_MINIPORT             CTL_CODE(IOCTL_SCSI_BASE, 0x0402, \
	METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_GET_INQUIRY_DATA     CTL_CODE(IOCTL_SCSI_BASE, 0x0403, \
	METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_GET_CAPABILITIES     CTL_CODE(IOCTL_SCSI_BASE, 0x0404, \
	METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE(IOCTL_SCSI_BASE, 0x0405, \
	METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_GET_ADDRESS          CTL_CODE(IOCTL_SCSI_BASE, 0x0406, \
	METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif
