/* Copyright (c) 2012 BDT Media Automation GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * File:         TapeLibraryMgr.h
 * Description:
 * Author:       Sam Chen
 * Created:      Jul 30, 2012
 *
 */


#pragma once


#include "../lib/ltfs_library/stdafx.h"
#include "../lib/ltfs_library/LtfsError.h"
#include "../lib/ltfs_library/LtfsDetails.h"
#include "../lib/ltfs_library/CmnFunc.h"
#include "../tape/ObserverSubject.h"
#include "../ltfs_format/ltfsFormatDetails.h"
#include "CatalogDbManager.h"

#include "stdafx.h"


using namespace tape;
using namespace ltfs_management;

namespace ltfs_management
{
	const string LTFS_STORAGE_ROOT = COMM_STORAGE_VFS_PATH;
	const string LTFS_MERAT_ROOT = COMM_META_CACHE_PATH;
	const string LTFS_CACHE_ROOT = COMM_DATA_CACHE_PATH;
	const int FILE_EXPORTED_FLAG = 5;

	#define STM_LOCK_DEF_TIMEOUT	3*1000  // 3 seconds
	#define CHANGER_MOVE_LOCK_TIMEOUT	10*60*1000  // 10 minutes


	class LtfsLibraries;
	class TapeDbManager;
	class TapeStateMachineMgr;
	class TapeSchedulerMgr;
	class FormatManager;
	class FormatThread;

    enum TapeState
    {
    	TAPE_STATE_IDLE = 0,
    	TAPE_STATE_READWRITE,
    	TAPE_STATE_ACCESSING,
    	TAPE_STATE_DIAGNOSING
    };

    struct DriveInfo
    {
    	string		mDriveSerial;
    	string		mDriveName;
    public:
    	DriveInfo()
    	{
    		mDriveSerial = "";
    		mDriveName = "";
    	}
    };

	struct CartridgeDetail
    {
        string      mBarcode;
        string      mTapeGroupUUID;
        string		mDualCopy;
        string		mTapeUUID;
        int        	mMediaType;
        int			mMediumType;
        int        	mStatus;
        long long   mLoadCount;
        int         mGenerationNumber;
        long long   mUsedCapacity;
        long long   mFreeCapacity;
        long        mFileNumber;
        long long   mFileCapacity;
        int         mFormat;
        bool        mFaulty;
        int			mState;
        int			mActivity;
        bool		mWriteProtect;
        bool		mOffline;
        time_t	    mLastMountTime;
        time_t		mLastAuditTime;
	public:
		CartridgeDetail()
		{
	        mBarcode = "";
	        mTapeGroupUUID = "";
	        mDualCopy = "";
	        mTapeUUID = "";
	        mMediaType = 0;
	        mStatus = 0;
	        mLoadCount = 0;
	        mGenerationNumber = 0;
	        mUsedCapacity = 0;
	        mFreeCapacity = 0;
	        mFileNumber = 0;
	        mFileCapacity = 0;
	        mFormat = 0;
	        mFaulty = false;
	        mState = 0;
	        mActivity = 0;
	        mWriteProtect = false;
	        mOffline = false;
	        mLastMountTime = 0;
		}
    };

	enum SchResult
	{
		SCH_SUCCESS = 0,
		SCH_WAIT,
		SCH_BUSY,
		SCH_NO_RESOURCE
	};


    enum STM_STATE
    {
    	STM_ST_INVALID = 0,
    	STM_ST_ASSIGNED_OPEN,
    	STM_ST_ASSIGNED_ACTIVE,
    	STM_ST_ASSIGNED_CLOSED,
    	STM_ST_ASSIGNED_EXPORTED,
    	STM_ST_UN_ASSIGNED,
    	STM_ST_RAW,
    	STM_ST_OFFLINE,
    	STM_ST_ACCESSING,
    	STM_ST_DIAGNOSING,
    	STM_ST_UN_SUPPORTED,
    	STM_ST_CONFLICT,
    	STM_ST_WRITE_PROTECT,
    	STM_ST_READWRITE
    };

    enum STM_OPERATION
    {
    	STM_OP_WRITE = 0,
    	STM_OP_STOP_READWRITE,
    	STM_OP_READ,
    	STM_OP_CLOSE,
    	STM_OP_DIAGNOSE,
    	STM_OP_FINISH_DIAGNOSE,
    	STM_OP_CREATE_CACHE,
    	STM_OP_ASSIGN,
    	STM_OP_UNASSIGN,
    	STM_OP_EXPORT,
    	STM_OP_IMPORT,
    	STM_OP_RECYCLE,
    	STM_OP_REVOKE,
    	STM_OP_FORMAT,
    	STM_OP_ACCESS,
    	STM_OP_UNACCESS,
    	STM_OP_EJECT,
    	STM_OP_INVENTORY,
    	STM_OP_OTHER
    };

    struct TapeInfo
	{
		string  mBarcode;
		string  mGroupID;
        string	mDualCopy;
        string	mTapeUUID;
		int     mSlotID;
		int     mStatus;
		int     mMediumType;
		int     mMediaType;
		int     mLtfsFormat;
		int     mGenerationIndex;
		int     mLoadCount;
		Int64_t mTotalCapacity;
		Int64_t mFreeCapacity;
		int     mFileNumber;
		Int64_t mFileCapacity;
		bool    mFaulty;
		int     mState;
		int     mActivity;
		bool	mWriteProtect;
		bool	mOffline;
        time_t	mLastMountTime;
        time_t	mLastAuditTime;
    public:
    	TapeInfo(){
    		mBarcode = "";
    		mGroupID = "";
    		mDualCopy = "";
    		mTapeUUID = "";
    		mSlotID = 0;
    		mStatus = 0;
    		mMediumType = 0;
    		mMediaType = 0;
    		mLtfsFormat = 0;
    		mGenerationIndex = 0;
    		mLoadCount = 0;
    		mTotalCapacity = 0;
    		mFreeCapacity = 0;
    		mFileNumber = 0;
    		mFileCapacity = 0;
    		mFaulty = false;
    		mState = 0;
    		mActivity = 0;
    		mWriteProtect = false;
    		mOffline = false;
    		mLastMountTime = 0;
    		mLastAuditTime = 0;
    	}
	};

    struct INV_TAPE_INFO{
    	TapeInfo info;
    	string		changerSerial;
    };
    struct INV_DRIVE_INFO{
    	LtfsDriveInfo info;
    	string		changerSerial;
    };

    enum SYS_DIAGNOSE_STATUS
    {
    	DIAG_NOT_NEED = 0,
    	DIAG_NOT_RUN,
    	DIAG_RUNNING,
    	DIAG_CANCELED,
    	DIAG_FINISHED,
    	DIAG_FAILED
    };
    const int MAM_INFO_MASK_UUID = 1;
    const int MAM_INFO_MASK_STATUS = 2;
    const int MAM_INFO_MASK_FAULTY = 4;
    const int MAM_INFO_MASK_FORMAT = 8;
    const int MAM_INFO_MASK_DUAL_COPY = 16;
    struct TapeMamInfo
    {
    	string 			mTapeGroup;
    	string			mDualCopy;
    	TapeStatus		mStatus;
    	bool			mFaulty;
    	int				mPriority;
    	int				mMask;
    public:
    	TapeMamInfo()
    	{
    		mTapeGroup = "";
    		mDualCopy = "";
    		mStatus = TAPE_UNKNOWN;
    		mFaulty = false;
			mPriority = -1;
			mMask = 0;
    	}
    };

    typedef map<time_t,vector<fs::path> > TimeMap;
    typedef map<long long,fs::path> OffsetMap;

    class TapeLibraryMgr : public ObserverSubject
    {
    public:
        static TapeLibraryMgr * Instance();
        static void Destroy();

        //////////////////////////////////////////
		/// changer/drive/mail slot operations
        bool IsMailSlotAvailable(const string& barcode);
        bool OpenMailSlot(const string & changer, int slotID, LtfsError & error);
        bool OpenMailSlot();
        bool IsMailSlotBusy(const string& changerSerial);
        void PreventChangerMediaRemoval(bool bPrevent);
        bool MoveCartridge(const string& changerSerial, int srcSlot, int dstSlot, LtfsError& error, const string& barcode = "");
        bool RequestMoveLock(const string& changerSerial, int millTimeOut = CHANGER_MOVE_LOCK_TIMEOUT);
        void ReleaseMoveLock(const string& changerSerial);

        //////////////////////////////////////////
        ///get changer, drive, tape list info, inventory
		SystemHwMode GetSystemHwMode(bool bRefresh = false);
		bool CheckSystemHwMode(bool bRefresh = true);
        bool Refresh(bool bInventory, LtfsError & error);
        bool GetChangerList(vector<LtfsChangerInfo> & changers, LtfsError & error);
        bool GetAllTapeList(vector<TapeInfo> & tapes, LtfsError & error);
        bool GetTapeListForChanger(const string & changer, vector<TapeInfo> & tapes, LtfsError & error);
        bool GetSlotListForChanger(const string & changer, vector<LtfsSlotInfo> & slots, LtfsError & error);
        bool RefreshChangerMailSlots(const string& changerSerial, vector<LtfsMailSlotInfo>& mailSlots, LtfsError& lfsErr);
        bool GetMailSlotListForChanger(const string & changer, vector<LtfsMailSlotInfo> & slots, LtfsError & error);
        bool GetDriveListForChanger(const string & changer, vector<LtfsDriveInfo> & drives, LtfsError & error);
        bool GetDrive(const string& driveSerial, LtfsDriveInfo& drive);
        bool GetDriveCleaningStatus(const string& changer, const string& drive, int& status, LtfsError error, bool bForceRefresh = false);

        //////////////////////////////////////////
        /// tape group, share client
		bool GetTapeGroupList(vector<string>& list);
        bool GetTapeGroupMembers(vector<TapeInfo> & tapes, const string & tapeGroup, LtfsError & error, bool bIncludeOffline = false);
		bool GetTapeGroupMerbers(const string& group, vector<string>& list);
        bool AssignToTapeGroup(const string & barcode, const string & tapeGroup, const string& groupName, int status);
		bool AddTapeGroup(const string& group, const string& name);
        bool DeleteTapeGroup(const string & tapeGroup);
        bool SetGroupName(const string& group, const string& name);
        bool GetGroupName(const string& group, string& name);
		bool GetShareUUID(const string& name, string& uuid);
        bool SetClientName(const string & tapeGroup, const string & name);
        static bool GetTapeGroupCapacity(const string & tapeGroup,size_t & fileNumber,off_t & usedSize,off_t & freeSize);
        bool GetTapeGroupCacheSize(const string & tapeGroup,off_t & size);
        bool GetActiveTape(string & barcode, const string & tapeGroup, off_t size);
        bool GetActiveTape(vector<string> & barcodes, const string & tapeGroup, off_t size);
        bool GetTape(const string & barcode, TapeInfo & tape);
        bool SetTapeFaulty(const string & barcode, bool faulty, int priority, bool SetMam = true);
        bool SetTapeGroup(const string& barcode, const string& tapeGroup, int priority, bool SetMam = true);
        bool SetTapeDualCopy(const string& barcode, const string& dualCopy, int priority, bool SetMam = true);
        bool SetTapeStatus(const string & barcode, TapeStatus status, int priority, bool SetMam = true);
		bool SetTapeLtfsFormat(const string& barcode, int format);
        bool SetTapeMamInfo(const string& barcode, TapeMamInfo mamInfo);
        bool SetTapeOffline(const string & barcode, bool offline);
        bool SetTapeCapacity(const string & barcode, int fileNumber, off_t fileSize, off_t tapeSize);
        bool GetTapeFreeSize(const string & barcode, off_t &freesize);
        bool SetTapeState(const string & barcode, enum TapeState state);
		bool SetFileNumberForTape(const string& barcode, long fileNumber);
		bool SetFileCapacityForTape(const string& barcode, long long fileCapacity);
        long long GetMinTapeFreeSize();
        bool IsNativeTapeGroup(const string& tapeGroup);
        bool DeleteFile(const string & tapeGroup, const fs::path & path);
        bool SetDeletingActivity(const string& barcode, bool bSet);
        bool GetMetaFreeCapacity(off_t & usedCapacity, off_t & freeCapacity);
        int  MountAllShare();
        int  UnmountAllShare();
        int  MountShare(const string& uuid, const string& name);
        int  UnmountShare(const string& uuid);
        string GenerUUIDByName(const char* name);
        bool CheckTapeGroupExistByName(const string name);
        void StartTaskShare();
        bool GetTapeGroupDualCopy(const string& uuid);
        bool GetShareAvailableTapes(const string& uuid, vector<map<string, off_t> >& tapesList);

        //////////////////////////////////////////
		/// tape operations related
        bool MountTape(const string & barcode);
        bool UnmountTape(const string & barcode, bool bUpdateGeneration = true);
        bool DirectAccessTape(const string& barcode);
        bool StopDirectAccessTape(const string& barcode);
        bool CleanTape(const string & tapeGroup, const string & tape, bool bExport = false);
        bool CheckTape(const string & barcode, CHECK_TAPE_FLAG flag, LtfsError& lfsErr);
		bool DeleteTape(const string& barcode);
		bool UnFormatTape(const string& barcode);
        bool StartReadTape(const string& barcode);
        bool StartWriteTape(const string& barcode);
        bool StopReadWriteTape(const string& barcode);
        bool StopTapeGroupReadWrite(const string& uid);
        bool StopTapeReadWrite(const string& barcode, const string& uid);
        bool StopReadWriteTape(const string& barcode, const string& uuid);
        bool CanWriteCache();
		bool GetLoadedTapeStatus(const string& barcode, int& status, LtfsError& ltfsErr);
        bool Format(const string& barcode, LtfsError& ltfsErr);
		bool GetLoadedTapeLtfsFormat(const string& barcode, int& ltfsFormat, LtfsError& ltfsErr);
		bool GetLoadedTapeLoadCounter(const string& barcode, int& count, LtfsError& ltfsErr);
		bool GetLoadedTapeMediaType(const string& barcode, int& mediaType, LtfsError& ltfsErr);
		bool GetLoadedTapeWPFlag(const string& barcode, bool& bIsWP, LtfsError& ltfsErr);
		bool GetLoadedTapeGenerationIndex(const string& barcode, long long& index, LtfsError& ltfsErr);
		bool GetLoadedTapeCapacity(const string& barcode, long long& freeCapacity, long long& totalCapacity, LtfsError& ltfsErr);
		bool SetLoadedTapeStatus(const string& barcode, int status, LtfsError& ltfsErr, bool bSetMam = false, bool SetDb = true);
		bool GetLoadedTapeFaulty(const string& barcode, bool& faulty, LtfsError& ltfsErr);
		bool SetLoadedTapeFaulty(const string& barcode, bool faulty, LtfsError& ltfsErr, bool bSetMam = false, bool SetDb = true);
		bool SetLoadedTapeGroup(const string& barcode, const string& uuid, LtfsError& ltfsErr, bool bSetMam = false, bool SetDb = true);
		bool GetLoadedTapeGroup(const string& barcode, string& uuid, LtfsError& ltfsErr);
		bool SetLoadedTapeDualCopy(const string& barcode, const string& dualCopy, LtfsError& ltfsErr, bool bSetMam = false, bool SetDb = true);
		bool GetLoadedTapeDualCopy(const string& barcode, string& dualCopy, LtfsError& ltfsErr);
		bool SetLoadedTapeUUID(const string& barcode, const string& uuid, LtfsError& ltfsErr, bool bSetMam = false, bool SetDb = true);
		bool GetLoadedTapeUUID(const string& barcode, string& uuid, LtfsError& ltfsErr);
		bool SetLoadedTapeBarcode(const string& driveSerial, const string& barcode, LtfsError& ltfsErr);
		bool GetLoadedTapeBarcode(const string& driveSerial, string& barcode, LtfsError& ltfsErr, bool bRefresh = true);
        bool IsTapeInDrive(const string& barcode);
        bool GetDriveInfoForTape(const string& barcode, LtfsDriveInfo& driveInfo);


		//////////////////////////////////////////
        //// functions for state machine manager ///////////////////////////////////
        bool StmRequestTape(const string& barcode, int millTimeOut = STM_LOCK_DEF_TIMEOUT);
        bool StmRequestTape(const string& barcode, STM_OPERATION stmOperation, const string& shareUuid = "", int millTimeOut = STM_LOCK_DEF_TIMEOUT);
        void StmReleaseTape(const string& barcode);
        bool StmTapeCanBeChangedTo(const string& barcode, STM_STATE stmState, STM_OPERATION stmOperation);
        bool StmTapeCanBeChangedTo(const string& barcode, STM_STATE stmState, const vector<STM_OPERATION> stmOperations);
        string GetStmStateStr(STM_STATE stmState);
        string GetStmOperationStr(STM_OPERATION stmOp);
        STM_STATE GetTapeStmState(const string& barcode);
        //////////////////////////////////////////
        /// functions for request/release tape
        bool RequestTape(const string & barcode,bool mount,int priority,int timeout,bool & busy);
        bool RequestTape(const string & barcode,bool mount,int priority,int timeout);
        bool RequestTapes(const vector<string>& barcodes, bool mount, int priority, int timeout);
        bool ReleaseTape(const string & barcode);
        bool ReleaseTapes(const vector<string>& barcodes);
        bool BindTape(const string& barcode, const string& changerSerial, const string& driveSerial);
        bool UnbindTape(const string& barcode);
        //// functions for drive scheduler manager ////////////////////
        SchResult SchRequestTape(const string& barcode, vector<string>& tapesInUse, int priority);
    	SchResult SchRequestTapes(const vector<string>& barcodes, map<string, vector<string> >& tapesInUse, int priority);
		bool SchReleaseTape(const string & barcode);
		bool SchReleaseTapes(const vector<string>& barcodes);

        /////////////////////////////////////////
        ////functions for format queue
		FormatThread* StartFormat(const string& barcode, FormatType type, int priority, Labels& labels, time_t startTimeInMicroSecs = 0, bool bRequested = false);
		bool CancelFormat(const string& barcode);
		bool GetFormatStatus(const string& barcode, FormatThreadStatus& status);
		bool GetFormatStatus(FormatThread* formatThread, FormatThreadStatus& status);
		bool GetDetailForBackup(vector<FormatDetail>& details);

        //////////////////////////////////////////////////
        /// misc, help functions
        void LogDebugTapeInfo(const TapeInfo& tapeInfo);
        //void Stop();
		bool IsDriveMounted(const string& changerSerial, const string& driveSerial, bool& mounted, LtfsError& ltfsErr);

        bool StartDriveCleaningTask(const string& changerSerial, const string& driveSerial);// should be private
        bool UpdateTape(const string & changer, const string & drive, LtfsError & error);// should be private
        bool UpdateTapeGeneraionIndex(const string& barcode, UInt64_t genIndex);  // should be private

        bool GetDiagnoseSystemStatus(SYS_DIAGNOSE_STATUS& diagStatus, string& logFile);
        bool DiagnoseSystem(bool bDoDiagnose);
        bool ServiceDiagnoseTape(bool bDirtyShutdown, const string& barcode, const string& tapeGroupUUID, vector<fs::path> &failedFileList);
        bool DiagnoseCheckDualCopyTape(const string& barcode, const string& dualCopy, const string& uuid);

		int GetChangerTapeNum(const string& changerSerial, bool bIncludeDrive, bool bIncludeMailSlot);
        int GetDriveNum();
        bool GetTapeActivity(const string& barcode, int& act, int& percentage);
		int GetChangerSlotNum(const string& changerSerial);

        bool GetWriteCacheAction(int & action);
        bool SetWriteCacheAction(int action);

        bool CheckCacheUsage();

        bool GetCartridge(const string& barcode, CartridgeDetail& detail);

        bool AddUpdateTapeGroupForSwift();

        bool NeedReformatTape(const TapeInfo& tapeInfo);
        string GetNextTapeToAudit(const vector<string>& auditingTapes, bool& bFaulty);
        bool SetLastAuditTime(const string& barcode, time_t auditTime);
        bool SetTapeActivity(const string& barcode, TapeActivity activity);
        bool StartDeleteTapeFileTask(const string& barcode);
        bool DeleteFilesOnTape(const string& barcode);
        long long GetPendingDeleteFileNum(const string& barcode);
        static long long GetSizeMinTapeFree();

    private:
        TapeLibraryMgr(LtfsLibraries* hal, TapeDbManager* db, TapeSchedulerMgr* sch, TapeStateMachineMgr* stm, FormatManager* fmgr, CatalogDbManager* catalogDb);
        ~TapeLibraryMgr();
        bool FindTape(const string & barcode,string & serialChanger,string & serialDrive,int & slotID);
        bool GetTape(TapeInfo & tape,const LtfsTapeInfo & info);
        TapeInfo ConvertTape(const CartridgeDetail& detail);
        bool UpdateTapeUnlock(const string & changer, const string & drive, LtfsError & error, bool bCheckWP = false, bool bForce = false);
        bool UnMountDrive(const string& changerSerial, const string& driveSerial,
                LtfsError& ltfsErr, const string& barcode, bool bUpdateGeneration = true);
        bool MoveCartridge(const string& barcode, int dstSlot, LtfsError& error);

        bool CheckTapeGroupBuffer(const string & tapeGroup,const string & barcode,long long size);
        bool CheckTapeGroupTape(const string & tapeGroup,const string & barcode,long long size);
        int GetClientHandle(const string & tapeGroup,int retry = 3);
        bool IsTapeWritable(const CartridgeDetail & detail);

        string GetShareNameByBarcode(const string& barcode);

		bool HalSetLoadedTapeStatus(const string& barcode, int status, LtfsError& ltfsErr);
		bool HalSetLoadedTapeFaulty(const string& barcode, bool faulty, LtfsError& ltfsErr);
		bool HalSetLoadedTapeGroup(const string& barcode, const string& uuid, LtfsError& ltfsErr);
		bool HalSetLoadedTapeUUID(const string& barcode, const string& uuid, LtfsError& ltfsErr);
		bool HalSetLoadedTapeDualCopy(const string& barcode, const string& dualCopy, LtfsError& ltfsErr);
        bool HalMountTape(const string & barcode, LtfsError& ltfsErr);
        bool HalUnmountTape(const string & barcode, UInt64_t& genIndex, LtfsError& ltfsErr);
        bool UpdateTapeWPFlag(const string & changer, const string & drive, LtfsError & error);
        void InventoryTape(const string& changerSerial, const LtfsTapeInfo& tapeInfo, map<string, bool>& handledTapes,
        		bool& bOpenMailSlot, bool bNew, bool& bRetry);
        bool MoveTapeToStorageSlot(const string& changerSerial, const LtfsMailSlotInfo& mailSlot);
        bool InventoryEjectCheck(const string& changerSerial, bool& bRetry,
        		bool& bOpenMailSlot, const LtfsMailSlotInfo& mailSlotInfo );
        bool RefreshChanger(const string& changerSerial, map<string, bool>& lockedTapes, map<string, bool>& handledTapes,
            		bool& bRetry, bool& bOpenMailSlot, LtfsError & lfsErr);
        bool InventoryTapeList(const vector<LtfsTapeInfo>& tapes, const string& changerSerial,
        		map<string, bool>& lockedTapes, map<string, bool>& handledTapes, bool& bOpenMailSlot, bool bNew, bool& bRetry);

        ChangerStatus GetChangerStatus(const string& changerSerial);
        bool DeleteFile(const string& pathName);
        bool SaveDiagnoseLog(const string& logMsg);
        void DiagnoseSystemThread(bool bDoDiagnose);
        void RefreshSysHwModeThread();

        bool DiagnoseCheckFolderFile(const string& barcode, const string& dualCopy, const string subPath, const string& uuid, map<string, off_t>& copyFileMap);
        LtfsDriveInfo ConvertDriveInfo(const DriveInfo& driveInfo);
        void MarkTapeInDriveFlag(const string barcode, bool bCreate);
        void MarkTapeInDriveFlag(const string barcode, const string& srcDrive, const string& dstDrive);

    private:
        TapeDbManager *     	database_;
        CatalogDbManager* 		catalogDb_;
        LtfsLibraries* 			hal_;
        TapeSchedulerMgr*		schduler_;
        TapeStateMachineMgr*	stateMachine_;
        FormatManager* 			formatMgr_;

        long long           sizeMinTapeFree_;
        long long           autoRecycleFree_;
        typedef map<string,long long> 		TapeGroupBufferMap;
        TapeGroupBufferMap 					buffer_;
        boost::mutex        mutexBuffer_;
        boost::mutex        mutexTape_;
        bool				bInited;
        bool				bStop_;
        map<string, int>	drives_;
        SystemHwMode		lastHwMode_;
        bool 					bNeedDiagnose_;
        SYS_DIAGNOSE_STATUS		diagStatus_;
        string 					sysDiagLogFile_;
        boost::mutex			mutexDiagnose_;
        boost::mutex			mutexRefreshHwMode_;
        bool					refreshHwModeRunning_;
        bool					bBusy_;
        map<string, string>		invEjectTapes_;
        string					m_ipHostName;  // ipaddr or host name of the server
        time_t					lastCacheUsageEventTime_; //time for last event for cache free space low event
        map<string, time_t>		mLastDeleteFileTaskMap_;

        static boost::mutex   mutexRefresh_;
        static boost::mutex   mutexInstance_;
        static TapeLibraryMgr *         instance_;

        boost::mutex                mutexImport_;
        struct ImportTask
        {
            bool run;
            int total;
            int current;
            int percentEvent;

            ImportTask(int total)
            : run(true), total(total), current(0), percentEvent(0)
            {
            }
        };
        typedef map<string,ImportTask>    ImportMap;
        ImportMap                   import_;

        friend class FormatThread;
        friend class TapeSchedulerMgr;
    };
}

