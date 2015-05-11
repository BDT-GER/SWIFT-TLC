/*
 * ltfsSimulator.h
 *
 *  Created on: Aug 28, 2012
 *      Author: chento
 */

#ifndef __LTFSLIBCONVERTOR_H__
#define __LTFSLIBCONVERTOR_H__

#include "stdafx.h"
#include "../ltfs_management/TapeLibraryMgr.h"
#include "../bdt/Factory.h"

#ifndef __LTFS_SOAP_H__
#define __LTFS_SOAP_H__
#include "ltfs.h"
#endif

using namespace ltfs_management;

const string PERMISSION_READ = "1";
const string PERMISSION_WRITE = "2";


namespace ltfs_soapserver
{
	//errno for soap api
	enum SOAP_ERRNO
	{
		ERR_SUCCESS = 0,
		ERR_UNKNOWN_EXCEPTION,
		ERR_INVALID_PARAMETER,
		ERR_INVALID_USER = 100,
		ERR_INVALID_PASSWORD,
		ERR_INVALID_USER_DESCRIPTION,
		ERR_USER_LEN_TOO_LONG,
		ERR_PWD_LEN_TOO_LONG,
		ERR_USER_EXISTS,
		ERR_USER_NON_EXISTS,
		ERR_ADD_SYSTEM_USER,
		ERR_SET_USER_PWD,
		ERR_GET_USER_ID,
		ERR_SAVE_USER_INFO,
		ERR_UPDATE_USER_INFO,
		ERR_DELETE_USER,
		ERR_DELETE_USER_PERMISSION,
		ERR_INCORRECT_PASSWORD,
		ERR_NO_ADMIN_PRIVILEGE,
		ERR_SYSTEM_USER_EXISTS,
		ERR_GET_EVENT_LIST = 150,
		ERR_DISMISS_EVENT,
		ERR_DISMISS_ALL_EVENTS,
		ERR_DELETE_EVENT,
		ERR_DELETE_ALL_EVENT,
		ERR_SET_NOTI_CONF,
		ERR_GET_NOTI_CONF,
		ERR_GENERATE_LOG_PKG,
		ERR_INVALID_SHARE_NAME = 200,
		ERR_INVALID_PROTOCOL,
		ERR_INVALID_RETENTION_UNIT,
		ERR_INVALID_RETENTION_TIME,
		ERR_SHARE_EXISTS,
		ERR_SHARE_NON_EXISTS,
		ERR_SAVE_SHARE_INFO,
		ERR_MOUNT_SHARE,
		ERR_SET_SHARE_RETENTION,
		ERR_SET_SHARE_PERMISSION,
		ERR_CREATE_SHARE,
		ERR_UPDATE_SMB_SHARE_PERMISSION,
		ERR_DELETE_SHARE_PERMISSION,
		ERR_RELOAD_SHARE_CONF,
		ERR_UUID_EMPTY,
		ERR_DELETE_SHARE,
		ERR_STOP_SHARE,
		ERR_SHARE_REMOVE_TAPE,
		ERR_DELETE_SHARE_INFO,
		ERR_RENAME_SHARE,
		ERR_UPDATE_SHARE_CONF,
		ERR_GET_SYSTEM_STATUS = 250,
		ERR_GET_TASK_LIST,
		ERR_QUERY_TASK_STATUS,
		ERR_ADD_BACKEND_TASK,
		ERR_LOAD_TAPE = 300,
		ERR_IMPORT_TAPE,
		ERR_EXPORT_TAPE,
		ERR_ASSIGN_TAPE,
		ERR_EJECT_TAPE,
		ERR_STOP_DIRECT_ACCESS,
		ERR_OPEN_MAIL_SLOT,
		ERR_DELETE_OFFLINE_TAPE,
		ERR_DIAGNOSE_TAPE,
		ERR_CREATE_SHARE_TAPE_LOCKED,
		ERR_CREATE_SHARE_TAPE_USED,
		ERR_IMPORT_TAPE_LOCKED,
		ERR_IMPORT_TAPE_USED,
		ERR_ASSIGN_TAPE_LOCKED,
		ERR_ASSIGN_TAPE_USED,
		ERR_DEL_SHARE_TAPE_LOCKED,
		ERR_DEL_SHARE_TAPE_USED,
		ERR_REVOKE_TAPE_LOCKED,
		ERR_REVOKE_TAPE_USED,
		ERR_RECYCLE_TAPE_LOCKED,
		ERR_RECYCLE_TAPE_USED,
		ERR_EXPORT_TAPE_LOCKED,
		ERR_EXPORT_TAPE_USED,
		ERR_EJECT_TAPE_CHECK_CONDITIONS,
		ERR_EJECT_TAPE_LOCKED,
		ERR_EJECT_TAPE_USED,
		ERR_FORMAT_TAPE_LOCKED,
		ERR_FORMAT_TAPE_USED,
		ERR_DIRECT_ACCESS_NO_BARCODE,
		ERR_DIRECT_ACCESS_TAPE_LOCKED,
		ERR_DIRECT_ACCESS_TAPE_ALREADY_USED,
		ERR_DIRECT_ACCESS_TAPE_USED,
		ERR_DIRECT_ACCESS_TAPE_ACTIVITY,
		ERR_DIRECT_ACCESS_BACKEND_TASK,
		ERR_STOP_DIRECT_ACCESS_NO_BARCODE,
		ERR_STOP_DIRECT_ACCESS_TAPE_LOCKED,
		ERR_STOP_DIRECT_ACCESS_TAPE_ACTIVITY,
		ERR_DELETE_OFFLINE_TAPE_LOCKED,
		ERR_DELETE_OFFLINE_TAPE_NON_EXIST,
		ERR_DELETE_OFFLINE_TAPE_USED,
		ERR_DIAGNOSE_TAPE_LOCKED,
		ERR_DIAGNOSE_TAPE_DISABLED,
		ERR_EXPORT_CACHE_CHECK_FAILE,
		ERR_EXPORT_CACHE_TAPE,
		ERR_REVOKE_CACHE_CHECK_FAILE,
		ERR_REVOKE_CACHE_TAPE,
		ERR_CANCEL_IMPORT,
		ERR_MAIL_SLOT_BUSY,
		ERR_HW_NOT_READY,
		ERR_HW_NEED_DIAGNOSE,
		ERR_HW_NEED_DIAGNOSING,
		ERR_ADD_LICENSE,
		ERR_DEL_LICENSE,
		ERR_GET_LICENSE,
		ERR_INVALID_LICENSE,
		ERR_ASSIGN_COUPLE_TAPES,
		ERR_CREATE_SHARE_COUPLE_TAPES,
		ERR_FIND_COUPLED_TAPE,
		ERR_DIAGNOSE_UNMOUNT_SHARE_FAIL,
		ERR_GET_DRIVE_LIST,
		ERR_GET_TAPE_LIST,
		ERR_SET_TAPE_BARCODE,
		ERR_GET_DRIVE_TAPE_LIST,
		ERR_FORMAT_TAPE,
		ERR_DELETE_TAPE_CLIENT_ERR,
		ERR_DELETE_NO_TAPE,
		ERR_DELETE_TAPE_UNFORMAT_ERR,
		ERR_DELETE_TAPE_EJECT_ERR,
		ERR_DELETE_TAPE_RMLINK_ERR,
		ERR_DELETE_TAPE_ERR,
		ERR_NO_ENOUGH_META_CAPACITY,
		ERR_SAVE_USER_PERMISSION,
		ERR_SEARCH_FILE_FAIL,
		ERR_HW_NO_LICENSE,
		ERR_ADD_REGISTER_USER,
	};


#if 0
	static const string SHARE_ENABLE = "Yes";
	static const string SHARE_DISABLE = "No";
	struct SoftwareVersionInfo
	{
		string		version;
		string		packageType;
		int			mainVersion;
		int			subVersion;
		int			buildNumber;
		string		changeSet;
		string		hasCode;
		bool		simulator;
	public:
		SoftwareVersionInfo()
		{
			version = "";
			packageType = "";
			mainVersion = 0;
			subVersion = 0;
			buildNumber = 0;
			changeSet = "";
			hasCode = "";
			simulator = false;
		}
	};
#endif
  class LibConvertor
  {
  	private:
#if 0
		static bool
		GetTapeGroupMembers(const string& uid, vector<TapeInfo>& tapes);

		static void
		ConvertLibInfo(LtfsChangerInfo& changer, struct SoapLibrary &library);

		static void
		ConvertSlotInfo(vector<LtfsSlotInfo> &ltfsSlotList,
				vector<struct SoapSlot> &slotList);

		static void
		ConvertMailSlotInfo(vector<LtfsMailSlotInfo> &ltfsSlotList,
				vector<struct SoapMailSlot> &slotList);

		static void
		ConvertDriveInfo(vector<LtfsDriveInfo> &ltfsDriveList,
				vector<struct SoapDrive> &driveList);

		static void
		ConvertTapeInfo(vector<TapeInfo> &ltfsTapeList,
				vector<struct SoapTape> &tapeList, bool all=true);

		static void
		ConvertTapeliteInfo(const vector<TapeInfo> &ltfsTapeList,
				vector<struct SoapTapeLite> &tapeList);
#endif

    public:
#if 0
		static bool
		GetLibrarySystemStatus(SystemStatus &status);

    	static bool
    	GetLibraryList(InventoryRslt &result);

    	static bool
    	GetChangerMailSlots(string& changerSerial, GetChangerMailSlotsRslt& result);

		static bool
		ListTape(const string &type, ListTapeRslt &result);

		static bool
		ListTapeLite(const string &type, std::string& uid, ListTapeLiteRslt &result);

		static bool
		OpenMailSlot(const string& changerSerial, int slotID, ErrorReturn &result);

		static bool
		CheckTapeInGroup(const string& barcode, bool& inGroup);

		static bool
		CheckEject(const string& changerSerial, int& slotID, const string& barcode);

		static bool
		GetShareList(std::vector<struct SoapShare>& sharelist);

		static bool GetCacheInfo(const string cachePath, CacheInfo& cacheInfo);
		static bool GetSoftwareVersion(SoftwareVersionInfo& softVer);
#endif
		static int AddTape(std::string uid, struct BarcodesList barcodeList, bool import, struct ErrorReturn &result);
		static int CreateShare(struct CreateShareRequest req, struct ErrorReturn &result);
	};

}


#endif /* LTFSSIMULATOR_H_ */
