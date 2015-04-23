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
 * CmnFunc.cpp
 *
 *  Created on: Jul 30, 2012
 *      Author: Sam Chen
 */

#include <inttypes.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>
#include <stdarg.h>

#include "CmnFunc.h"

#define MOVE_TAPE_FAIL_RETRY_TIMES  10
#define MOVE_TAPE_FAIL_RETRY_WAIT	5
#define UNMOUNT_FAIL_RETRY_TIMES  30
#define UNMOUNT_FAIL_RETRY_WAIT	  5
#define MOUNT_FAIL_RETRY_TIMES  3
#define MOUNT_FAIL_RETRY_WAIT	  10
#define FORMAT_FAIL_RETRY_TIMES  5
#define FORMAT_FAIL_RETRY_WAIT	  10

#define UN_FORMAT_FAIL_RETRY_TIMES  5
#define UN_FORMAT_FAIL_RETRY_WAIT	  10

#define MAM_TAPE_GENERATION_INDEX  	0x080C
#define MAM_TAPE_LOAD_COUNTER		0x0003
#define MAM_TAPE_GROUP_UUID			0x1403
#define MAM_TAPE_FALUTY_FLAG		0x1406
#define MAM_TAPE_STATUS_FLAG		0x1407
#define MAM_TAPE_LTFS_FORMAT		0x080C
#define	MAM_TAPE_DUAL_COPY			0x1408
#define	MAM_TAPE_BARCODE			0x1409
#define	MAM_TAPE_UUID				0x1410
#define MAM_TAPE_MEIDUM_TYPE		0x0408

const int VSB_DRIVE_ID_START = 0;

#define UNLTFS_BIN_PATH "/usr/local/bin/unltfs"

namespace ltfs_management
{
	bool TestUnitReady(const string& devName, int fd = -1, int timeOut = 0);

	fs::path GetLtfsFolder()
	{
		return fs::path(COMM_MOUNT_PATH);
	}

	string GetDiagnoseTapeLogPath(const string& barcode)
	{
		return DIAGNOSIS_TAPE_LOG_PATH + string("/tape_check_log_") + barcode + string(".log");
	}

	string stringFormat(const std::string &fmt, ...)
	{
		int size = 100;
		string str;
		va_list ap;
		while (1) {
			str.resize(size);
			va_start(ap, fmt);
			int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
			va_end(ap);
			if (n > -1 && n < size) {
				str.resize(n);
				return str;
			}
			if (n > -1){
				size = n+1;
			}else{
				size *= 2;
			}
		}
	}

	bool IsIBMLtfs()
	{
		try{
			fs::path unltfsPath = UNLTFS_BIN_PATH;//"/usr/local/bin/unltfs"
			if(fs::exists(unltfsPath)){
				return false;
			}
		}catch(std::exception& e){
			LtfsLogWarn("IsIBMLtfs error: " << e.what());
		}

		return true;
	}
	string GenerateTapeUUID()
	{
		boost::uuids::uuid uuid = boost::uuids::random_generator()();
		string strUUID = boost::lexical_cast<std::string>(uuid);

		return strUUID;
	}

	int SetTapeAttributeBuf(unsigned char* buf, int bufSize, const string& attrValue, int attrId)
	{
		int           len;
		int			  srclen;
		int 		  attrLen = 128;

		memset (buf, (int)0x20, bufSize);
		buf[0] = (unsigned char)(attrId >> 8);//
		buf[1] = (unsigned char)(attrId & 0xFF);
		buf[2] = 0x01;   // ASCII format
		buf[3] = 0;
		buf[4] = attrLen;

		srclen = attrValue.length();
		len = (srclen > attrLen) ? attrLen : srclen;
		memcpy (buf+5, attrValue.c_str(), len);
		len = attrLen + 5;

		return len;
	}

	string GetMountedDriveMountPoint(const string& driveStDevName, const string& barcode)
	{
		string mountPoint = "";

		string cmdCheckMount = "mount -l -t fuse | grep ltfs | grep 'ltfs:" + driveStDevName + " on'";
	    vector<string> mounts = GetCommandOutputLines(cmdCheckMount);
	    for(unsigned int i = 0; i < mounts.size(); i++){
			//ltfs:/dev/IBMtape0 on /opt/VS/vsMounts/858ABUL5 type fuse (rw,nosuid,nodev,default_permissions,allow_other)
			regex matchMount("^.*on\\s+(\\S+)\\s+type.*");
			cmatch match;
			if(regex_match(mounts[i].c_str(), match, matchMount)){
				mountPoint = match[1];
			}

			if(mountPoint != ""){
				string pathMount =  (GetLtfsFolder() / barcode).string();
				if(mountPoint != pathMount){
					LtfsError lfsErr;
					// the tape is not mounted in ltfstor rule, unmount it
					if(UnMount(barcode, driveStDevName, lfsErr, mountPoint)){
						mountPoint = "";
					}else{
						LtfsLogError("Failed to unmount tape " << barcode << " which is mounted on " << mountPoint);
					}
				}
			}
			break;
	    }

		return mountPoint;
	}

	bool IsMountPointMounted(const string& mountPoint)
	{
        // if it is already mounted, return true
		string cmdCheckMount = "mount -l -t fuse | grep " + mountPoint;
	    vector<string> mounts = GetCommandOutputLines(cmdCheckMount);
	    if(mounts.size() > 0){
	    	LtfsLogDebug(mounts[0]);
			LtfsLogDebug("DDEBUG: " << mountPoint << " is mounted.");
	    	return true;
	    }
		LtfsLogDebug("DDEBUG: " << mountPoint << " is not mounted.");
	    return false;
	}

	bool IsDriveMounted(const string& driveStDevName, const string& barcode)
	{
		if("" != driveStDevName){
			string chkCmd = "ps --cols 1024 ax | grep ltfs | grep \" devname=" + driveStDevName + "\"";
			vector<string> retChecks = GetCommandOutputLines(chkCmd);
			if(retChecks.size() > 0){
				for(vector<string>::iterator it = retChecks.begin(); it != retChecks.end(); it++){
					//0:00 ltfs -o devname=/dev/IBMtape0 /opt/VS/vsMounts/863ABUL5 -o trace
					regex matchMount("^.*\\d+\\:\\d+\\s+ltfs.*");
					cmatch match;
					if(regex_match((*it).c_str(), match, matchMount)){
						LtfsLogDebug("DDEBUG: " << driveStDevName << ":" << barcode << " is mounted:" << (*it));
						return true;
					}else{
						LtfsLogDebug("DDEBUG: " << driveStDevName << ":" << barcode << " is not mounted:" << (*it));
					}
				}
			}
		}

		/*if("" != driveStDevName){
			string chkCmd = "lsof " + driveStDevName + "";
			vector<string> retChecks = GetCommandOutputLines(chkCmd);
			if(retChecks.size() > 0){
				for(vector<string>::iterator it = retChecks.begin(); it != retChecks.end(); it++){
					//COMMAND     PID USER   FD   TYPE   DEVICE SIZE/OFF    NODE NAME
					//ltfs    19230 root    3u   CHR  251,1      0t0 3776502 /dev/IBMtape1
					regex matchMount("^.*\\s*ltfs\\s+\\d+.*");
					cmatch match;
					LtfsLogDebug("DDEBUG: *it = " << *it << ".");
					if(regex_match((*it).c_str(), match, matchMount)){
						LtfsLogDebug("DDEBUG: " << driveStDevName << ":" << barcode << " is mounted:" << (*it));
						return true;
					}else{
						LtfsLogDebug("DDEBUG: " << driveStDevName << ":" << barcode << " is not mounted:" << (*it));
					}
				}
			}
		}*/

		if("" != barcode){
			fs::path pathLTFS = GetLtfsFolder() / barcode;
			if(true == IsMountPointMounted(pathLTFS.string())){
				LtfsLogDebug("DDEBUG: " << driveStDevName << ":" << barcode << " is mounted:" << pathLTFS.string());
				return true;
			}
			LtfsLogDebug("DDEBUG: " << driveStDevName << ":" << barcode << " is not mounted:" << pathLTFS.string());
			return false;
		}

		LtfsLogDebug("DDEBUG: " << driveStDevName  << ":" << barcode << " is not mounted.");
		return false;
	}


	string GetScsiCmdStr(scsi_cmd_io *ioCmd)
	{
		if(ioCmd == NULL){
			return "";
		}

		string strCdb = "";
		for(int i = 0; i < ioCmd->cdb_length; i++){
			strCdb += stringFormat("%.2x", ioCmd->cdb[i]);
			if(i < ioCmd->cdb_length - 1){
				strCdb += string(" ");
			}
		}
		return strCdb;
	}

	void GetSenseCode(scsi_cmd_io *ioCmd)
	{
		LtfsLogDebug("GetSenseCode: ioCmd->sense_length = " << ioCmd->sense_length << ", ioCmd->sensedata[0] = " << hex << ioCmd->sensedata[0]);
		if(ioCmd->sense_length > 0
			&& ioCmd->sense_length <= 128
			&& (ioCmd->sensedata[0] == 0x70 || ioCmd->sensedata[0] == 0x71)
		)
		if(1)
		{
			int senseKey = (int)(ioCmd->sensedata[2]) & 0x0F;
			int asc = (int)(ioCmd->sensedata[12]);
			int ascq = (int)(ioCmd->sensedata[13]);
			ioCmd->senseCode.senseCode = senseKey;
			ioCmd->senseCode.ascCode = asc;
			ioCmd->senseCode.ascqCode = ascq;
			LtfsLogDebug("GetSenseCode: senseKey = " << hex << senseKey << ", asc = " << hex << asc << ", ascq = " << hex << ascq << endl);
		}
	}


	int ExecScsiCommand(scsi_cmd_io *ioCmd, int retry, int retryWait)
	{
		sg_io_hdr_t    sgIo;
		int status;
		int            scsi_status;
		time_t			tStart = time(NULL);

RETRY:
		memset ((void*)&sgIo, 0, sizeof (sgIo));
		memset (ioCmd->sensedata, 0, sizeof(ioCmd->sensedata));
		if(ioCmd->data_direction == HOST_READ){
			memset ((void*)ioCmd->data, 0, ioCmd->data_length);
		}
		ioCmd->senseCode.senseCode = 0;
		ioCmd->senseCode.ascCode = 0;
		ioCmd->senseCode.ascqCode = 0;

		//setup required fields
		sgIo.interface_id = (int) 'S';
		sgIo.timeout      = (unsigned int) ioCmd->timeout_ms;
		sgIo.flags        = SG_FLAG_LUN_INHIBIT;
		sgIo.cmd_len      = (unsigned char) ioCmd->cdb_length;
		sgIo.cmdp         = ioCmd->cdb;
		sgIo.mx_sb_len    = sizeof(ioCmd->sensedata);
		sgIo.sbp          = ioCmd->sensedata;
		sgIo.dxfer_len    = ioCmd->data_length;
		sgIo.dxferp       = (void*) ioCmd->data;
		sgIo.dxfer_direction = (ioCmd->data_direction == HOST_READ) ? SG_DXFER_FROM_DEV :
			(ioCmd->data_direction == HOST_WRITE) ? SG_DXFER_TO_DEV  :  SG_DXFER_NONE;

		DebugPrintScsiCommand(ioCmd);
		//
		time_t tTmpStart = time(NULL);
		status = ioctl (ioCmd->fd, SG_IO, &sgIo);
		LtfsLogDebug("ExecScsiCommand: status = " << status << ", sgIo.host_status = " << sgIo.host_status \
				<< ", sgIo.driver_status = " << sgIo.driver_status << ", time used for this ioctl = " << time(NULL) - tTmpStart << endl);

		scsi_status = sgIo.status;

		ioCmd->actual_data_length = sgIo.dxfer_len - sgIo.resid;
		ioCmd->sense_length       = sgIo.sb_len_wr;

		DebugPrintScsiCommandResult(ioCmd);
		GetSenseCode(ioCmd);

		if (scsi_status != S_GOOD) {
			if (scsi_status == S_CHECK_CONDITION) {
				if ((ioCmd->cdb[0] == CMDread) && (SENSE_HAS_ILI_SET(ioCmd->sensedata))) {
					int resId = ((int)ioCmd->sensedata[3] << 24)
						+ ((int)ioCmd->sensedata[4] << 16)
						+ ((int)ioCmd->sensedata[5] <<  8)
						+ ((int)ioCmd->sensedata[6]);
					ioCmd->actual_data_length = ioCmd->data_length - resId;
					status = ioCmd->actual_data_length;
					LtfsLogDebug("DDBG: ExecScsiCommand return: " << status);
				} else if (((ioCmd->cdb[0] == CMDwrite)
						|| (ioCmd->cdb[0] == CMDwrite_filemarks) )
						&& (SENSE_EARLY_WARNING_EOM(ioCmd->sensedata))){
					ioCmd->actual_data_length = ioCmd->data_length;
					status = ioCmd->actual_data_length;
				} else if (((ioCmd->cdb[0] == CMDwrite)
						|| (ioCmd->cdb[0] == CMDwrite_filemarks))
						&&(SENSE_END_OF_MEDIA(ioCmd->sensedata))) {
					ioCmd->actual_data_length = 0;
					status = -1;
					LtfsLogDebug("DDBG: ExecScsiCommand return: " << status);
				}else {
					status = -1;
					LtfsLogDebug("DDBG: ExecScsiCommand return: " << status);
				}
			} else {
				status = -1;
				LtfsLogDebug("DDBG: ExecScsiCommand return: " << status);
			}
		} else if ((ioCmd->cdb[0] == CMDread) || (ioCmd->cdb[0] == CMDwrite)) {
			status = ioCmd->actual_data_length;
			LtfsLogDebug("DDBG: ExecScsiCommand return: " << status);
		} else {
			status = 0;
		}

		if(status != 0 && retry > 0){
			retry--;
			sleep(retryWait);
			goto RETRY;
		}

		if(status != 0){
			string strSense = stringFormat("%.2xH %0.2xH %0.2xH", ioCmd->senseCode.senseCode, \
					ioCmd->senseCode.ascCode, ioCmd->senseCode.ascqCode);
			LtfsLogError("Failed to execute SCSI Command (" << GetScsiCmdStr(ioCmd) << "), with sense code (" << strSense << ") received.");
		}

		LtfsLogDebug("ExecScsiCommand return: " << status << ", total time used = " << time(NULL) - tStart << endl);
		return status;
	}

	void DebugPrintScsiCommand(scsi_cmd_io *ioCmd)
	{
		if(ioCmd == NULL){
			return;
		}

		string strCdb = "ExecScsiCommand: scsi command starts: cdb = ";
		for(int i = 0; i < ioCmd->cdb_length; i++){
			strCdb += stringFormat("%.2x ", ioCmd->cdb[i]);
		}
		strCdb += "\n";
		LtfsLogDebug(strCdb);
	}

	void DebugPrintScsiCommandResult(scsi_cmd_io *ioCmd)
	{
		if(ioCmd == NULL){
			return;
		}

		string strCdb = "Result of scsi command: cdb = ";
		for(int i = 0; i < ioCmd->cdb_length; i++){
			strCdb += stringFormat("%.2x ", ioCmd->cdb[i]);
		}
		strCdb += "\n";
		LtfsLogDebug(strCdb);

		string strSense = "sense data = ";
		int sense_length = (ioCmd->sense_length >= 128)? 128 : ioCmd->sense_length;
		for(int i = 0; i < sense_length; i++){
			strSense += stringFormat("%.2x ", ioCmd->sensedata[i]);
		}
		strSense += "\n";
		LtfsLogDebug(strSense);

		string strData = "data = ";
		for(int i = 0; i < ioCmd->data_length; i++){
			strData += stringFormat("%.2x ", ioCmd->data[i]);
		}
		strData += "\n";
		LtfsLogDebug(strData);
	}

	vector<string> PopenCmdOutputLines(const string& cmd)
	{
		LtfsLogDebug("PopenCmdOutputLines: cmd = " + cmd << "..." << endl);

		std::vector<string> ret;
		FILE *file = popen(cmd.c_str(), "r");
		if(file == NULL)
		{
			LtfsLogError("PopenCmdOutputLines failed: cmd = " + cmd << "..." << endl);
			return ret;
		}
		char buffer[4096];
		while (fgets(buffer,sizeof(buffer),file))
		{
			string line = buffer;
			size_t newline = line.find('\n');
			if ( newline != string::npos ) {
				line.resize(newline);
			}
			ret.push_back(line);
			LtfsLogDebug(line);
		}
		pclose(file);
		LtfsLogDebug("PopenCmdOutputLines finished: cmd = " + cmd << "..." << endl);

		return ret;
	}

	bool KillPid(pid_t ppid, pid_t pid)
	{
		LtfsLogDebug("Starting to kill process " << pid << ".");
		if(0 == kill(pid, SIGTERM)){
			LtfsLogDebug("Succeed to kill process " << pid << ".");
			return true;
		}

		LtfsLogError("Failed to kill process " << pid << ".");
		return false;
	}

	bool KillAll(pid_t pid)
	{
		bool bRet = true;
		vector<string> psTreeLines = PopenCmdOutputLines("pstree " + boost::lexical_cast<string>(pid) + " -pGA");
		//python(722)-+-python(723)---python(725)---python(727)---python(728)---python(733)---tail(734)
		//            `-python(724)---python(726)---python(729)---python(730)---python(731)---tail(732)
		string retLine = "";
		for(unsigned int i = 0; i < psTreeLines.size(); i++){
			retLine += psTreeLines[i];
		}
		cout << "retLine:  " << retLine << endl;
		regex matchPid("^.*\\((\\d+)\\).*");
		cmatch match;
		while(regex_match(retLine.c_str(), match, matchPid)){
			pid_t tmpId = (pid_t)boost::lexical_cast<int>(match[1]);
			if(false == KillPid(pid, tmpId)){
				LtfsLogError("Failed to kill sub process " << tmpId << " of " << pid << ".");
				bRet = false;
			}
			size_t pos = retLine.find("(" + match[1] + ")", 0);
			if(pos == string::npos){
				break;
			}
			retLine = retLine.substr(0, pos);
		}

		return bRet;
	}
	vector<string> GetCommandOutputLines(const string& cmd, int& status, int timeOut, bool bTotalTimeOut)
	{
		int pid = 0;
		return GetCommandOutputLines(cmd, status, pid, timeOut, bTotalTimeOut);
	}

	vector<string> GetCommandOutputLines(const string& cmd, int& status, int& runPid, int timeOut, bool bTotalTimeOut)
	{
		LtfsLogDebug("GetCommandOutputLines: cmd = " + cmd << "..." << endl);

		std::vector<string> ret;
		int pids[2];
		int pid;

		status = -1;

		int pipeRet = pipe(pids);
		if(pipeRet < 0){
			LtfsLogError("create pipe error for command: " << cmd << ". errno: " << errno << ".");
			return ret;
		}

		pid = fork();
		if(pid < 0){
			close(pids[0]);
			close(pids[1]);
			LtfsLogError("fork error for command: " << cmd << ".");
			return ret;
		}
		else if (pid == 0){//child
			// close all other fds
			for(int i = 3; i < 256; i++){
				if(i != pids[0] && i != pids[1]){
					close(i);
				}
			}
			close(1); // close current stdout
			dup(pids[1]); // Make stdout go to write end of pipe
			close(0); // close current stdin
			close(2); // close current stderr
			close(pids[0]);
			execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL);
			LtfsLogError("Failed to exec command: " << cmd << ".");
			exit(1);
		}else{// parent
			char buffer[16384];
			close(pids[1]);
			int fd = pids[0];
			fcntl (fd, F_SETFL, O_NONBLOCK);
			int retry = 0;
			string strRet = "";
			bool bFinished = false;
			runPid = pid;

			while(retry < timeOut){
				struct	timeval	tv;
				tv.tv_sec = 1;  // seconds
				tv.tv_usec = 0;
				fd_set	set_fd;
				FD_ZERO (&set_fd);
				FD_SET (fd, &set_fd);
				int retval  = select (fd+1, &set_fd, NULL, NULL, &tv);
				if ( ((retval < 0) && (errno != EINTR)) || (!(FD_ISSET (fd, &set_fd))) ){
					status = -1;
					int wret = waitpid (pid, &status, WNOHANG);
					if( wret == -1 || WIFEXITED(status) || WIFSIGNALED(status)){
						LtfsLogDebug("GetCommandOutputLines finished:  1 cmd = " + cmd << "..." << endl);
						bFinished = true;
						goto READ_DATA;
					}
					retry++;
					continue;
				}
				if(false == bTotalTimeOut){
					retry = 0;
				}
READ_DATA:
				memset(buffer, 0, sizeof(buffer));
				int readSize = read(pids[0], buffer, sizeof(buffer));
				if(readSize <=0 ){
					LtfsLogDebug("GetCommandOutputLines finished:  2 cmd = " + cmd << "..." << endl);
					goto GET_RETURN;
				}else{
					string line = buffer;
					line.resize(readSize);
					LtfsLogDebug(line);
					strRet += line;
				}
				if(bFinished){
					goto GET_RETURN;
				}
			}

GET_RETURN:
			size_t oldPos = 0;
			size_t pos = strRet.find('\n', 0);
			while(pos != string::npos){
				string line = strRet.substr(oldPos, pos - oldPos);
				ret.push_back(line);
				oldPos = pos + 1;
				pos = strRet.find('\n', oldPos);
			}
			if(oldPos != strRet.length()){
				string line = strRet.substr(oldPos, strRet.length() - oldPos);
				ret.push_back(line);
			}
			status = -1;
			int wret = waitpid (pid, &status, WNOHANG);
			int waitRetry = 5;
			while(wret == 0 && waitRetry > 0){
				LtfsLogDebug( "command: " << cmd << " not finished, retry waitpid, waitRetry: " << waitRetry << ".");
				sleep(1);
				wret = waitpid (pid, &status, WNOHANG);
				waitRetry--;
			}
			/*
			if(wret == 0){
				int kRet = KillAll(pid);
				if(kRet != 0){
					LtfsLogError("Failed to kill command: " << cmd << ", errno: " << errno << ".");
				}
			}*/
			close(pids[0]);
			LtfsLogDebug("GetCommandOutputLines finished: 3, retry = " << retry << ", status = " << status << ", wret = " << wret << ", cmd = " + cmd << endl);
			return ret;
		}//if(pid < 0){

		LtfsLogDebug("GetCommandOutputLines finished: cmd = " + cmd << endl);

		return ret;
	}

	vector<string> GetCommandOutputLines(const string& cmd, int timeOut, bool bTotalTimeOut)
	{
		LtfsLogDebug("GetCommandOutputLines: cmd = " + cmd << "..." << endl);

		int status;

		return GetCommandOutputLines(cmd, status, timeOut, bTotalTimeOut);
	}

	string GetCommandOutput(const string& cmd)
	{
		return "";
	}

	int LockDevice(const string& devName)
	{
		LtfsLogDebug("LockDevice: sgDev = " << devName << endl);
		int fd = -1;

		fd = open ((devName + string(".lock")).c_str(), O_CREAT | O_RDWR);
		if(fd > 0){
			LtfsLogDebug("LockDevice START: devName = " << devName << ", fd = " << fd << endl);
			if (flock(fd, LOCK_EX) != 0) {
				LtfsLogError("LockDevice: Failed to lock device..." << devName << endl);
				close(fd);
				fd = -1;
			}
		}else{
			LtfsLogError("LockDevice: Failed to lock device: " << devName << endl);
		}
		if(fd > 0){
			int flags = fcntl(fd, F_GETFD);
			flags |= FD_CLOEXEC;
			if(fcntl(fd, F_SETFD, flags) < 0){
				LtfsLogError("fcntl failed on " << fd  << "...........");
			}
		}
		LtfsLogDebug("LockDevice finished: devName = " << devName << ", fd = " << fd << endl);

		return fd;
	}

	int OpenDevice(const string& devName)
	{
		LtfsLogDebug("OpenDevice: sgDev = " << devName << endl);
		int fd = -1;

#ifdef SIMULATOR
		fd = 1;
#else
		fd = open (devName.c_str(), O_RDWR | O_NDELAY);
		if(fd <= 0){
			LtfsLogError("OpenDevice: Failed to open device: " << devName << endl);
		}else{
			LtfsLogDebug("OpenDevice finished: devName = " << devName << ", fd = " << fd << endl);
		}
#endif

		return fd;
	}

	int UnlockDevice(int& lockFd)
	{
		LtfsLogDebug("UnlockDevice: lockFd = " << lockFd << endl);
		if(lockFd > 0){
			close(lockFd);
			lockFd = -1;
		}

		return lockFd;
	}

	int CloseDevice(int& deviceFd)
	{
		LtfsLogDebug("CloseDevice: deviceFd = " << deviceFd << endl);
#ifdef SIMULATOR
		deviceFd = -1;
#else
		if(deviceFd > 0){
			close(deviceFd);
			deviceFd = -1;
		}
#endif

		return deviceFd;
	}



	string GetStDeviceFromSCSIAddr(const string& scsiAddr)
	{
		vector<string> lines = GetCommandOutputLines("cat /proc/scsi/IBMtape  | grep " + scsiAddr);
		vector<string>::iterator it;
		try{
			for(it = lines.begin(); it != lines.end(); it++){
				//Number  model       SN                HBA             SCSI            FO Path
				//0       ULT3580-HH5 1013000216        MPT SAS Host    0:0:8:0         NA
				regex matchDev("^\\s*(\\d+)\\s+.*");
				cmatch match;
				if(regex_match((*it).c_str(), match, matchDev)){
					string stDev = "/dev/IBMtape" +  match[1];
					return stDev;
				}
			}
		}catch(...){
			LtfsLogError("Exception in GetStDeviceFromSCSIAddr()..........." << endl);
		}

		return "";
	}

	bool GetLsscsiDrive(LSSCSI_INFO& info, const string& serial)
	{
		// get scsi device list from 'lsscsi -g'
		vector<string> lines = GetCommandOutputLines("lsscsi -g");
		vector<string>::iterator it;
		try{
			for(it = lines.begin(); it != lines.end(); it++){
				//[0:0:0:0]    tape    HP       Ultrium 5-SCSI   Z58B  /dev/st0   /dev/sg0
				//[0:0:8:0]    tape    IBM      ULT3580-HH5      C7R3  -         /dev/sg0
				//[0:0:1:0]    tape    IBM      ULT3580-HH6      D2DB  -          /dev/sg1
				regex matchChanger("^.*\\[(\\S+)\\]\\s+tape\\s+(\\w+)\\s+(.*)\\s+(\\S+)\\s+(\\S+)\\s+(/dev/sg\\w+).*$");
				cmatch match;
				if(regex_match((*it).c_str(), match, matchChanger)){
					string tmpSerial = GetDriveSerial(match[6]);
					if(tmpSerial != serial){
						continue;
					}
					info.scsiAddr = match[1];
					info.vendor = match[2];
					info.product = match[3];
					info.version = match[4];
					info.stDev = match[5];
					if(info.stDev == "-"){
						info.stDev = GetStDeviceFromSCSIAddr(match[1]);
					}
					info.sgDev = match[6];
					return true;
				}
			}
		}catch(...){
			LtfsLogError("Exception in GetLsscsiDrive()..........." << endl);
		}

		return false;
	}

	string GetStringFromBuf(unsigned char buf[], int len, bool withEmpty)
	{
		string ret = "";
		for(int j = 0; j < len; j++)
		{
			unsigned char ch =  buf[j];
			if(ch == '\0'){
				break;
			}
			if(!withEmpty && ch == ' '){
				break;
			}

			ret.push_back(ch);
		}
		int retLen = ret.length();
		while(ret[retLen - 1] == ' '){
			ret.erase(retLen - 1, 1);
			retLen = ret.length();
		}
		return ret;
	}


	string GetDriveSerial(const string& sgDev)
	{
		LtfsLogDebug("GetDriveSerial:  sgDev = " << sgDev << endl);
		string serial = "";

		int lockFd = LockDevice(sgDev);
		int fd = OpenDevice(sgDev);
		if(fd <= 0)
		{
			UnlockDevice(lockFd);
			return serial;
		}

		scsi_cmd_io sio;
		unsigned char        inqbuffer [240];
		int                  status;


		sio.fd = fd;
		//Set up the cdb for Inquiry:
		sio.cdb[0] = 0x12;
		sio.cdb[1] = 1;
		sio.cdb[2] = 0x80;
		sio.cdb[3] = 0;
		sio.cdb[4] = 0xff;
		sio.cdb[5] = 0;

		sio.cdb_length = 6;		/* six-byte cdb */

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		/*
		* Set the timeout then execute:
		*/
		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;

		int retry = 2;
		while(retry > 0){
			memset(inqbuffer, 0, sizeof(inqbuffer));
			status = ExecScsiCommand (&sio);
			if (status == 0) {
				serial = GetStringFromBuf(inqbuffer + 4, 10);
				break;
			}else{  // Error, retry
				retry--;
				LtfsLogDebug("GetDriveSerial: error, retrying " << retry);
				sleep(1);
			}
		}
		close(fd);
		UnlockDevice(lockFd);
		LtfsLogDebug("GetDriveSerial: drive serial = " << serial << endl);

		return serial;
	}

	bool RemoveFile(const fs::path removePath)
	{
		try{
			boost::filesystem::remove(removePath);
		}catch(std::exception& e){
			LtfsLogError("Failed to remove file/path: " << removePath.string() << ". Error: " << e.what() << ".");
			return false;
		}
		return true;
	}

	bool ChangerPreventMediaRemoval(const string& changerDevName, bool bPrevent)
	{
		scsi_cmd_io sio;
		sio.fd = OpenDevice(changerDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open changer device " << changerDevName << " to prevent/allow media removal.");
			return false;
		}

		unsigned char        inqbuffer [240];
		int                  status;
		//Set up the cdb for PREVENT / ALLOW MEDIA REMOVAL (1Eh)
		sio.cdb[0] = CMDPreventAllowMediaRemoval/*0x1E*/;
		sio.cdb[1] = 0;
		sio.cdb[2] = 0;
		sio.cdb[3] = 0;
		sio.cdb[4] = (bPrevent)? 1:0;
		sio.cdb[5] = 0;

		sio.cdb_length = 6;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		sio.timeout_ms = LTO_PREVENTALLOWMEDIA_TIMEOUT;
		status = ExecScsiCommand (&sio, 2);
		CloseDevice(sio.fd);

		if(status != 0){
			LtfsLogError("Failed to set prevent/allow media removal for changer " << changerDevName << ". bPrevent = " << bPrevent);
			return false;
		}

		LtfsLogInfo("Succeed to set prevent/allow media removal for changer " << changerDevName << ". bPrevent = " << bPrevent);
		return true;
	}

	bool GetChangerStatus(const string& changerDevName, ChangerStatus& changerStatus)
	{
		scsi_cmd_io sio;
		sio.fd = OpenDevice(changerDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open changer device " << changerDevName << " to get status.");
			return false;
		}
		unsigned char        inqbuffer [240];
		int                  status;
		//Set up the cdb for request sense
		sio.cdb[0] = CMDRequestSense/*0x03*/;
		sio.cdb[1] = 0;
		sio.cdb[2] = 0;
		sio.cdb[3] = 0;
		sio.cdb[4] = 0xFF;
		sio.cdb[5] = 0;

		sio.cdb_length = 6;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;

		int retry = 2;
		while(retry > 0){
			memset(inqbuffer, 0, sizeof(inqbuffer));
			status = ExecScsiCommand (&sio);
			if (status == 0) {
				int senseKey = (int)(inqbuffer[2]) << 4;
				int asc = (int)(inqbuffer[12]);
				int ascq = (int)(inqbuffer[13]);
				LtfsLogDebug("CheckSenseCode():: senseKey = " << senseKey << ", asc = " << asc << ", ascq = " << ascq << endl);

				// changer status default: connected
				changerStatus = CHANGER_STATUS_CONNECTED;

				//There is no real problem. The sense information is most likely indicating	some condition
				if(senseKey == 0){
					changerStatus = CHANGER_STATUS_CONNECTED;
				}
				//Indicates drive is not in a state to be able to execute the request just made to it.
				if(senseKey == 2){
					changerStatus = CHANGER_STATUS_BUSY;
				}

				// 04	12	offline
				if(senseKey == 2 && asc == 0x04 && ascq == 0x12){
					changerStatus = CHANGER_STATUS_DISCONNECTED;
				}
				//82	F1	drive communication error
				if(senseKey == 4 && asc == 0x82 && ascq == 0xF1){
					//changerStatus = CHANGER_STATUS_ERROR;
				}

				break;
			}else{// Error, retry
				retry--;
				LtfsLogDebug("CheckSenseCode: error, retrying " << retry);
				sleep(1);
			}
		}
		CloseDevice(sio.fd);

		if(status != 0){
			LtfsLogError("Failed to get status for changer " << changerDevName << ".");
			return false;
		}

		return true;
	}


	bool GetChangerAutoCleanMode(const string& changerDevName, bool& bAutoClean, LtfsError& lfsErr)
	{
		bool ret = false;
		scsi_cmd_io sio;
		unsigned char        inqbuffer [1024];
		int                  status;
		memset(inqbuffer, 0, sizeof(inqbuffer));

		sio.fd = OpenDevice(changerDevName);
		if(sio.fd < 0){
			LtfsLogError("GetChangerAutoCleanMode: Failed to open device " << changerDevName << " to get auto clean mode.");
			return false;
		}
		//Set up the cdb for mode sense
		sio.cdb[0] = CMDModeSenseChanger;//0x5A
		sio.cdb[1] = 0;
		sio.cdb[2] = 0x1F;  // Device Capabilities page
		sio.cdb[3] = 0;
		sio.cdb[4] = 0;
		sio.cdb[5] = 0;
		sio.cdb[6] = 0;
		sio.cdb[7] = (unsigned char)(sizeof(inqbuffer) >> 8 );
		sio.cdb[8] = (unsigned char)(sizeof(inqbuffer) && 0xFF );
		sio.cdb[9] = 0;

		sio.cdb_length = 10;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;

		status = ExecScsiCommand (&sio, 1);
		CloseDevice(sio.fd);
		if(sio.senseCode.senseCode != 0){
			lfsErr.SetSenseCode(sio.senseCode);
		}

		///LTFSLE: #define SIOC_MODE_SENSE               _IOR ('C', 0x0D, struct mode_sense)
		if(status == 0){
			bool bPS = (((int)(inqbuffer[8]) & 0x80) > 0) ? true : false;
			bAutoClean = (((int)(inqbuffer[11]) & 0x04) > 0) ? true : false;
			LtfsLogDebug("GetChangerAutoCleanMode for changer " << changerDevName << " finished. bAutoClean: " << bAutoClean << ", bPS = " << bPS << ".");
			ret = true;
		}// if(status == 0){
		else{
			lfsErr.SetErrCode(ERR_GET_AUTOCLEAN_MODE_FAIL);
			LtfsLogError("GetChangerAutoCleanMode failed for changer " << changerDevName);
			ret = false;
		}

		return ret;
	}

	bool GetChangerSerial(const string& changerDevName, string& serial)
	{
		scsi_cmd_io sio;
		sio.fd = OpenDevice(changerDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open changer device " << changerDevName << " to get serial.");
			return false;
		}
		unsigned char        inqbuffer [240];
		int                  status;
		//Set up the cdb for Inquiry:
		sio.cdb[0] = CMDInquiry/*0x12*/;
		sio.cdb[1] = 1;
		sio.cdb[2] = 0x83; // device identification page
		sio.cdb[3] = 0;
		sio.cdb[4] = (unsigned char) sizeof(inqbuffer);
		sio.cdb[5] = 0;

		sio.cdb_length = 6;		/* six-byte cdb */

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		/*
		* Set the timeout then execute:
		*/
		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;
		int retry = 5;
		// LTFSLE: #define SIOC_INQUIRY                  _IOR ('C',0x01,struct inquiry_data)
		while(retry){
			memset(inqbuffer, 0, sizeof(inqbuffer));
			status = ExecScsiCommand (&sio);

			if (status == 0) {
				serial.assign((char *) inqbuffer+34, 10);
				CloseDevice(sio.fd);
				return true;
			}else{
				LtfsLogInfo("GetChangerSerial error, retrying...");
				retry--;
				sleep(1);
			}
		}
		CloseDevice(sio.fd);
		return true;
	}


	bool IsDriveCleaningInProgress(const string& driveDevName)
	{
		bool bCleaning = false;
		scsi_cmd_io sio;
		unsigned char        inqbuffer [240];
		int                  status;
		memset(inqbuffer, 0, sizeof(inqbuffer));

		sio.fd = OpenDevice(driveDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open device " << driveDevName << " to check cleaning status.");
			return false;
		}
		//Set up the cdb for Log sense
		sio.cdb[0] = CMDLogSense/*0x4D*/;
		sio.cdb[1] = 0;
		sio.cdb[2] = (unsigned char) (0x40 | (0x11 & 0x3F)); //Tape Alert Flag. set PC=01b for current values
		sio.cdb[3] = 0;
		sio.cdb[4] = 0;
		sio.cdb[5] = 0;
		sio.cdb[6] = 0;
		sio.cdb[7] = sizeof(inqbuffer) >> 8;
		sio.cdb[8] = sizeof(inqbuffer) & 0xFF;
		sio.cdb[9] = 0;

		sio.cdb_length = 10;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;

		//LTFSLE: #define SIOC_LOG_SENSE10_PAGE _IOR('z', 0x51, struct log_sense10_page)
		status = ExecScsiCommand (&sio, 1);
		CloseDevice(sio.fd);

		if (status == 0) {
			int devAct = (int)(inqbuffer[10]);  //01h    Cleaning operation in progress
			if(devAct == 1){
				bCleaning = true;
			}
		}

		LtfsLogDebug("IsDriveCleaningInProgress: bCleaning = " << bCleaning << ".");
		return bCleaning;
	}

	bool GetDriveStatus(const string& driveDevName, DriveStatus& driveStatus, DriveCleaningStatus& cleaningStatus)
	{
		scsi_cmd_io sio;
		unsigned char        inqbuffer [240];
		int                  status;

		sio.fd = OpenDevice(driveDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open drive device " << driveDevName << " to get ingerface type.");
			return false;
		}
		//LTFSLE: #define SIOC_REQSENSE                 _IOR ('C',0x02,struct request_sense)
		//Set up the cdb for request sense
		sio.cdb[0] = CMDRequestSense/*0x03*/;
		sio.cdb[1] = 0;
		sio.cdb[2] = 0;
		sio.cdb[3] = 0;
		sio.cdb[4] = 0xFF;
		sio.cdb[5] = 0;

		sio.cdb_length = 6;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;
		int retry = 2;
		while(retry > 0){
			memset(inqbuffer, 0, sizeof(inqbuffer));
			status = ExecScsiCommand (&sio);
			if (status == 0) {
				break;
			}
			retry--;
			sleep(1);
		}
		CloseDevice(sio.fd);
		if (status == 0) {
			int senseKey = (int)(inqbuffer[2]) << 4;
			int asc = (int)(inqbuffer[12]);
			int ascq = (int)(inqbuffer[13]);
			int clnBit = int(inqbuffer[21]) & 0x08; //Reserved (0):4     CLN:1          Reserved (0):3
			LtfsLogDebug("senseKey = " << senseKey << ", asc = " << asc << ", ascq = " << ascq << ", clnBit = " << clnBit << endl);

			// drive status default: OK, cleaning status default: NONE
			driveStatus = DRIVE_STATUS_OK;
			cleaningStatus = CLEANING_NONE;

			//There is no real problem. The sense information is most likely indicating	some condition
			if(senseKey == 0){
				driveStatus = DRIVE_STATUS_OK;
			}
			//Indicates drive is not in a state to be able to execute the request just made to it.
			if(senseKey == 2){
				//driveStatus = DRIVE_STATUS_BUSY;
			}
			//Hardware Error. Believe to be due to bad drive hardware
			if(senseKey == 4){
				driveStatus = DRIVE_STATUS_ERROR;
			}

			// 8282	Drive Requires Cleaning	| The drive has detected that a cleaning operation is advisable to maintain good operation.
			// if the CLN bit in the sense data is set, the drive needs cleaning.
			if( (asc == 0x82 && ascq == 0x82) || clnBit == 0x08)
			{
				cleaningStatus = CLEANING_REQUIRED;
			}
			//3007	Cleaning Failure	A cleaning operation was attempted, but could not be completed for some reason
			if(asc == 0x30 && ascq == 0x07)
			{
				//cleaningStatus = CLEANING_FAILED;
			}

		}else{
			LtfsLogError("Failed to get status for drive " << driveDevName << ".");
			return false;
		}

		// check if the drive is being cleaned
		if(true == IsDriveCleaningInProgress(driveDevName)){
			cleaningStatus = CLEANING_IN_PROGRESS;
		}

		LtfsLogDebug("GetDriveStatus: driveDevName = " << driveDevName << ",cleaningStatus = " << cleaningStatus << ", driveStatus = "<< driveStatus <<".");
		return true;
	}

	bool GetDriveInterfaceType(const string& driveDevName, DriveInterfaceType& interfaceType)
	{
		scsi_cmd_io sio;
		unsigned char        inqbuffer [240];
		int                  status;

		sio.fd = OpenDevice(driveDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open drive device " << driveDevName << " to get ingerface type.");
			return false;
		}
		//LTFSLE: #define SIOC_INQUIRY_PAGE             _IOWR('C',0x1E,struct inquiry_page)
		//Set up the cdb for inquiry
		sio.cdb[0] = CMDInquiry/*0x12*/;
		sio.cdb[1] = 01;
		sio.cdb[2] = 0x88; //SCSI Ports Page
		sio.cdb[3] = 0;
		sio.cdb[4] = 0xFF;
		sio.cdb[5] = 0;

		sio.cdb_length = 6;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;

		int retry = 2;
		while(retry > 0){
			memset(inqbuffer, 0, sizeof(inqbuffer));
			status = ExecScsiCommand (&sio);
			if (status == 0) {
				break;
			}
			retry--;
			sleep(1);
		}
		CloseDevice(sio.fd);

		if (status == 0) {
			// 7  6  5  4          | 3  2  1  0
			// Protocol Identifier |Code Set (1)
			// The Protocol Identifier value is 0h on Fibre Channel devices and 6h on SAS devices.
			int iType = (UInt64_t)((inqbuffer[16] >> 4));
			if(iType == 0){
				interfaceType = INTERFACE_FC;
			}else{
				interfaceType = INTERFACE_SAS;
			}
		}else{
			LtfsLogError("Failed to GetDriveInterfaceType for drive " << driveDevName << ".");
			return false;
		}

		return true;
	}
	bool UpdateDriveInfo(driveElement& drive)
	{
		return true;
	}


	bool HandleElementBuffCheck(int start, unsigned char buf[], int bufLen, map<int, bool> &slotMap)
	{
		int elementType = (int)buf[start];
		int volTag = (int)buf[start + 1] >> 7;
		int elSize = (int)((buf[start + 2]) << 8) + (buf[start + 3]);
		unsigned long elTotalSize = (unsigned long)(buf[start + 5] << 16) + (unsigned long)(buf[start + 6] << 8) + (buf[start + 7]);
		int elCount = (elSize > 0) ? (elTotalSize / elSize) : 0;
		LtfsLogDebug("elementType = " << elementType << ", volTag = " << volTag << ", elSize = " << elSize << ", elTotalSize = " << elTotalSize\
				<< "elCount = " << elCount << endl);

		// check if the data returned is correct
		if(start + elCount * elSize > bufLen){
			LtfsLogError("Read element status returned data not correct, will not use this data.");
			return false;
		}

		for(int i = 0; i < elCount; i++){
			int startPos = start + i * elSize;
			int slotId = (int)((buf[startPos + 8]) << 8) + (buf[startPos + 9]);
			int full = (int)(buf[startPos + 10]) & 0x1;
			slotMap[slotId] = (full == 1)? true : false;
		}

		int newStart = start + 8 + elTotalSize;
		if(newStart < bufLen){
			return HandleElementBuffCheck(newStart, buf, bufLen, slotMap);
		}

		return true;
	}

	void CalculateLogicSlotId(ChangerInfo_& changerInfo)
	{
		// mail slot
		for(unsigned int i = 0; i < changerInfo.mailSlots.size(); i++){
			changerInfo.mailSlots[i].mLogicSlotID = changerInfo.mailSlots[i].mSlotID - changerInfo.mMailSlotStart + 1;
		}

		// storage slot
		for(unsigned int i = 0; i < changerInfo.slots.size(); i++){
			changerInfo.slots[i].mLogicSlotID = changerInfo.slots[i].mSlotID - changerInfo.mSlotStart + 1;
		}

		// drive
		for(unsigned int i = 0; i < changerInfo.drives.size(); i++){
			changerInfo.drives[i].mLogicSlotID = changerInfo.drives[i].mSlotID - changerInfo.mDriveStart + 1;
		}

		// tape
		for(unsigned int j = 0; j < changerInfo.tapes.size(); j++){
			int tapeSlotId = changerInfo.tapes[j].mSlotID;
			bool  bFound = false;
			// mail slot
			for(unsigned int i = 0; i < changerInfo.mailSlots.size(); i++){
				if(changerInfo.mailSlots[i].mSlotID == tapeSlotId){
					changerInfo.tapes[j].mLogicSlotID = changerInfo.mailSlots[i].mLogicSlotID;
					bFound = true;
					break;
				}
			}
			if(bFound){
				continue;
			}

			// storage slot
			for(unsigned int i = 0; i < changerInfo.slots.size(); i++){
				if(changerInfo.slots[i].mSlotID == tapeSlotId){
					changerInfo.tapes[j].mLogicSlotID = changerInfo.slots[i].mLogicSlotID;
					bFound = true;
					break;
				}
			}
			if(bFound){
				continue;
			}

			// drive
			for(unsigned int i = 0; i < changerInfo.drives.size(); i++){
				if(changerInfo.drives[i].mSlotID == tapeSlotId){
					changerInfo.tapes[j].mLogicSlotID = changerInfo.drives[i].mLogicSlotID;
					bFound = true;
					break;
				}
			}
			if(bFound){
				continue;
			}
		}
	}

	bool HandleElementBuff(int start, unsigned char buf[], int bufLen, ChangerInfo_& changeInfo, bool bDriveScsi, bool bFirst = false)
	{
		int elementType = (int)buf[start];
		int volTag = (int)buf[start + 1] >> 7;
		int elSize = (int)((buf[start + 2]) << 8) + (buf[start + 3]);
		unsigned long elTotalSize = (unsigned long)(buf[start + 5] << 16) + (unsigned long)(buf[start + 6] << 8) + (buf[start + 7]);
		int elCount = (elSize > 0) ? (elTotalSize / elSize) : 0;
		LtfsLogDebug("elementType = " << elementType << ", volTag = " << volTag << ", elSize = " << elSize << ", elTotalSize = " << elTotalSize\
				<< "elCount = " << elCount << endl);
		int startAddr = 0;

		// check if the data returned is correct
		if(start + elCount * elSize > bufLen){
			LtfsLogError("Read element status returned data not correct, will not use this data.");
			return false;
		}

		for(int i = 0; i < elCount; i++){
			string barcode = "";
			TapeMediumType mType = MEDIUM_UNKNOWN;

			int startPos = start + i * elSize;
			int slotId = (int)((buf[startPos + 8]) << 8) + (buf[startPos + 9]);
			int full = (int)(buf[startPos + 10]) & 0x1;
			int accessible = (int)(buf[startPos + 10]) & 0x8;  //Access bit on byte 10
			int abnormal =  (int)(buf[startPos + 10]) & 0x4;  //abnormal bit on byte 10

			if(i == 0){
				startAddr = slotId;
			}

			LtfsLogDebug("slotId = " << slotId << ", full = " << full << ", startPos = " << startPos << endl);
			// element full with tape
			if(1 == full){
				int mediumType = (int)(buf[startPos + 17]) & 0x7;
				switch(mediumType){
				case 0:
					mType = MEDIUM_UNKNOWN;
					break;
				case 1:
					mType = MEDIUM_DATA;
					break;
				case 2:
					mType = MEDIUM_CLEANING;
					break;
				case 3:
					mType = MEDIUM_DIAGNOSTICS;
					break;
				case 4:
					mType = MEDIUM_WORM;
					break;
				default:
					mType = MEDIUM_UNKNOWN;
					break;
				}
				int srcSlot = (int)(buf[startPos + 18] << 8) + (buf[startPos + 19]);
				for(int j = 0; j < 35; j++){
					unsigned char ch =  buf[startPos + 20 + j];
					if(ch == '\0' || ch == ' '){
						break;
					}else{
						barcode.push_back(ch);
					}
				}
				barcode = GetStringFromBuf(buf + startPos + 20, 35);
				LtfsLogDebug("slotId = " << slotId << ", barcode = " << barcode << ", mediumType = " << mediumType << ", srcSlot = " << srcSlot<< endl);

				// added tape to changer tape list
				tapeElement tape;
				tape.mBarcode = barcode;
				tape.mMediumType = mType;
				tape.mSlotID = slotId;
				changeInfo.tapes.push_back(tape);
			}//if

			bool bFull = (full == 1)? true : false;
			switch(elementType){
			case 1:// Medium Transport
				break;
			case 2:// Storage Element Slots
				{
					changeInfo.mSlotStart = startAddr;
					slotElement slot;
					slot.mSlotID = slotId;
					slot.mBarcode = barcode;
					slot.mIsEmpty =  !bFull;
					slot.mAbnormal = (abnormal > 0)? true : false;
					slot.mAccessible = (accessible > 0)? true : false;
					changeInfo.slots.push_back(slot);
				}
				break;
			case 3:// Import Export Element
				{
					changeInfo.mMailSlotStart = startAddr;
					mailSlotElement slot;
					slot.mSlotID = slotId;
					slot.mBarcode = barcode;
					slot.mIsEmpty =  !bFull;
					slot.mAbnormal = (abnormal > 0)? true : false;
					slot.mAccessible = (accessible > 0)? true : false;
					slot.mIsOpen = !slot.mAccessible;
					changeInfo.mailSlots.push_back(slot);
				}
				break;
			case 4:// Data Transfer Element
				{
					changeInfo.mDriveStart = startAddr;
					string serial = GetStringFromBuf(buf + startPos + 60 + 24, 10);
					driveElement drive;
					drive.mSlotID = slotId;
					drive.mBarcode = barcode;
					drive.mIsEmpty =  !bFull;
					drive.mSerial = serial;
					drive.mAbnormal = (abnormal > 0)? true : false;
					drive.mAccessible = (accessible > 0)? true : false;
					string productIdentify = GetStringFromBuf(buf + startPos + 60 + 8, 16, true);
					drive.mScsiInfo.product = productIdentify;
					drive.mScsiInfo.vendor = GetStringFromBuf(buf + startPos + 60 + 0, 8);
					//ULTRIUM-TD5
					//ULT3580-HH5
					regex matchDriveIBM("^.*\\-(\\w\\w)(\\d).*");
					//Ultrium 5-SCSI  	(HP)
					regex matchDriveHP("^.*(\\w+)\\s+(\\d+).*");
					cmatch match;
					drive.mIsFullHight = true;
					drive.mGeneration = 0;
					if(regex_match(productIdentify.c_str(), match, matchDriveIBM)
						|| regex_match(productIdentify.c_str(), match, matchDriveHP)
					){
						if(match[1] == "HH"){
							drive.mIsFullHight = false;
						}
						drive.mGeneration = boost::lexical_cast<int>(match[2]);
					}
					if(true == bDriveScsi){
						GetDriveInfo(serial, drive.mScsiInfo, drive.mStatus, drive.mCleaningStatus, drive.mInterfaceType);
					}
					changeInfo.drives.push_back(drive);
				}
				break;
			default:
				break;
			}
		}//for

		bool bRet = true;
		int newStart = start + 8 + elTotalSize;
		if(newStart < bufLen){
			bRet = HandleElementBuff(newStart, buf, bufLen, changeInfo, bDriveScsi);
		}

		if(bFirst){
			CalculateLogicSlotId(changeInfo);
		}

		return bRet;
	}

	bool VsbGetDriveList(vector<driveElement>& drives)
	{
		// get scsi device list from 'lsscsi -g'
		vector<string> lines = GetCommandOutputLines("lsscsi -g");
		vector<string>::iterator it;
		int idIndex = VSB_DRIVE_ID_START;
		try{
			for(it = lines.begin(); it != lines.end(); it++){
				//[0:0:0:0]    tape    HP       Ultrium 5-SCSI   Z58B  /dev/st0   /dev/sg0
				//[0:0:8:0]    tape    IBM      ULT3580-HH5      C7R3  -         /dev/sg0
				regex matchChanger("\\[(\\S+)\\]\\s+tape\\s+(\\w+)\\s+(.*)\\s+(\\S+)\\s+(\\S+)\\s+(/dev/sg\\w+)");
				cmatch match;
				if(regex_match((*it).c_str(), match, matchChanger)){
					string tmpSerial = GetDriveSerial(match[6]);
					if(tmpSerial == ""){
						continue;
					}
					LSSCSI_INFO info;
					info.scsiAddr = match[1];
					info.vendor = match[2];
					info.product = match[3];
					info.version = match[4];
					info.stDev = match[5];
					if(info.stDev == "-"){
						info.stDev = GetStDeviceFromSCSIAddr(match[1]);
					}
					info.sgDev = match[6];

					driveElement drive;
					drive.mScsiInfo = info;
					drive.mSerial = tmpSerial;
					drive.mLogicSlotID = idIndex;
					drive.mSlotID = idIndex;
					//ULTRIUM-TD5
					//ULT3580-HH5
					regex matchDriveIBM("^.*\\-(\\w\\w)(\\d).*");
					//Ultrium 5-SCSI  	(HP)
					regex matchDriveHP("^.*(\\w+)\\s+(\\d+).*");
					cmatch match;
					drive.mIsFullHight = true;
					if(regex_match(info.product.c_str(), match, matchDriveIBM)
						|| regex_match(info.product.c_str(), match, matchDriveHP)
					){
						if(match[1] == "HH"){
							drive.mIsFullHight = false;
						}
						drive.mGeneration = boost::lexical_cast<int>(match[2]);
					}
					drives.push_back(drive);
					idIndex++;
				}
			}
		}catch(std::exception& e){
			LtfsLogError("Exception in VsbGetDriveList()..........." << e.what());
			return false;
		}

		LtfsLogDebug("VsbGetDriveList ... " << drives.size());
		return true;
	}

	bool ReadChangerElementStatus(const string& changerDevName, unsigned char inqbuffer[], int dataLen)
	{
		scsi_cmd_io sio;
		sio.fd = OpenDevice(changerDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open changer device " << changerDevName << " to read element status.");
			return false;
		}
		int                  status;
		//LTFSLE: #define SMCIOC_INVENTORY              _IOW ('C',0x07,struct inventory)
		//Set up the cdb for Inquiry:
		sio.cdb[0] = CMDReadElementStatus/*0xB8*/;
		sio.cdb[1] = 0x10; // Report all element types, return VolTag information
		sio.cdb[2] = 0;
		sio.cdb[3] = 0;
		sio.cdb[4] = 0xff;
		sio.cdb[5] = 0xff;
		sio.cdb[6] = 0x03;
		sio.cdb[7] = (unsigned char)(dataLen >> 16 );
		sio.cdb[8] = (unsigned char)(dataLen >> 8 );
		sio.cdb[9] = (unsigned char)(dataLen & 0xFF);
		sio.cdb[10] = 0;
		sio.cdb[11] = 0;

		sio.cdb_length = 12;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = dataLen;
		sio.data_direction = HOST_READ;

		/*
		* Set the timeout then execute:
		*/
		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;
		int retry = 5;
		while(retry){
			memset(inqbuffer, 0, sizeof(inqbuffer));
			status = ExecScsiCommand (&sio);
			if (status == 0) {
				CloseDevice(sio.fd);
				return true;
			}else{
				LtfsLogError("ReadElementStatus error , retrying...");
				retry--;
				sleep(1);
			}
		}
		CloseDevice(sio.fd);
		return false;
	}

	bool GetChangerElements(const string& changerDevName, ChangerInfo_& changer, LtfsError& error, bool bDriveScsi)
	{
		int retry = 5;
		while(retry > 0){
			retry--;
			unsigned char        inqbuffer [10800];
			if(true == ReadChangerElementStatus(changerDevName, inqbuffer, sizeof(inqbuffer))){
				int totalSize = (int)((inqbuffer[6]) << 8) + (inqbuffer[7]) + 8;
				if(false == HandleElementBuff(8, inqbuffer, totalSize, changer, bDriveScsi, true)){
					LtfsLogError("Failed to read changer element status for changer " << changerDevName << ". Retry: " << retry << ".");//TODO
					sleep(5);
					continue;
				}
			}else{
				LtfsLogError("Failed to read changer element status for changer " << changerDevName << ". Retry: " << retry << ".");//TODO
				sleep(5);
				continue;
			}
			return true;
		}
		LtfsLogError("Failed to read changer element status for changer " << changerDevName << ".");//TODO
		return false;
	}

	bool GetChanger(const LSSCSI_INFO& info, ChangerInfo_& changer, LtfsError& error, bool bDriveScsi)
	{
		changer.drives.clear();
		changer.tapes.clear();
		changer.slots.clear();
		changer.mailSlots.clear();

		changer.mScsiInfo = info;

		if(false == GetChangerSerial(info.sgDev, changer.mSerial)){
			LtfsLogError("Failed to get serial number for changer " << info.sgDev << ".");
			return false;
		}

		if(false == GetChangerStatus(info.sgDev, changer.mStatus)){
			LtfsLogError("Failed to get status for changer " << info.sgDev << ".");
			return false;
		}

		LtfsError lfsErr;
		if(false == GetChangerAutoCleanMode(info.sgDev, changer.mAutoCleanMode, lfsErr)){
			LtfsLogError("Failed to get auto clean mode for changer " << info.sgDev << ".");
			return false;
		}

		return GetChangerElements(info.sgDev, changer, error, bDriveScsi);
	}

	bool GetChangerList(vector<LSSCSI_INFO>& changers, LtfsError& error)
	{
    	try
    	{
        	// get scsi device list from 'lsscsi -g'
        	vector<string> lines = GetCommandOutputLines("lsscsi -g");
        	vector<string>::iterator it;
			for(it = lines.begin(); it != lines.end(); it++){
				//[0:0:0:1]    mediumx BDT      FlexStor II      4.80  /dev/sch0  /dev/sg1
				//[0:0:0:1]    mediumx IBM      3573-TL          Bd60  -         /dev/sg3
				regex matchChanger("^\\s*\\[(\\S+)\\]\\s+mediumx\\s+(\\w+)\\s+(.*)\\s+(\\S+)\\s+(\\S+)\\s+(/dev/sg\\w+).*");
				cmatch match;
				if(regex_match((*it).c_str(), match, matchChanger))
				{
					LtfsLogDebug("match[1] = " << match[1] << ", match[6] = " << match[6] << ", match[5] = " \
							<< match[5] << ",match[2] = " << match[2] << ", match[3] = " << match[3] << ", match[4] = " <<  match[4] << endl);
					LSSCSI_INFO lsscsiInfo;
					lsscsiInfo.scsiAddr = match[1];
					lsscsiInfo.vendor = match[2];
					lsscsiInfo.product = match[3];
					lsscsiInfo.version = match[4];
					lsscsiInfo.stDev = match[5];
						if(lsscsiInfo.stDev == "-"){
							lsscsiInfo.stDev = GetStDeviceFromSCSIAddr(match[1]);
						}
					lsscsiInfo.sgDev = match[6];
					changers.push_back(lsscsiInfo);
				}
			}
    	}catch(...){
    		LtfsLogError("Exception on getting changer list.");
    		error.SetErrCode(ERR_GET_CHANGER_LIST);
			return false;
    	}
		return true;
	}

	bool GetDriveInfo(const string& driveSerial, LSSCSI_INFO& scsiInfo, DriveStatus& driveStatus,
			DriveCleaningStatus& cleaningStatus, DriveInterfaceType& interfaceType)
	{
		if(false == GetLsscsiDrive(scsiInfo, driveSerial)){
			LtfsLogError("Drive not connected. Serial: " << driveSerial << endl);
			driveStatus = DRIVE_STATUS_DISCONNECTED;
			return true;
		}
		string driveDevName = scsiInfo.sgDev;
		if(false == GetDriveStatus(driveDevName, driveStatus, cleaningStatus)){
			LtfsLogError("Failed to get status info for drive " << driveDevName << ".");
			return false;
		}
		if(false == GetDriveInterfaceType(driveDevName, interfaceType)){
			LtfsLogError("Failed to get interface type  for drive " << driveDevName << ".");
			return false;
		}
		return true;
	}

	bool TestUnitReady(const string& devName, int fd, int timeOut)
	{
		bool bRet = false;
		if("" != devName){
			fd = OpenDevice(devName);
		}
		if(fd < 0){
			LtfsLogError("Failed to open device " << devName << " to test unit ready.");
			return false;
		}
		scsi_cmd_io sio;
		unsigned char        cmdBuffer [1024];
		int                  status;
		int expire = timeOut;
		do{
			memset(cmdBuffer, 0, sizeof(cmdBuffer));
			sio.fd = fd;
			//LTFSLE: #define SIOC_TEST_UNIT_READY          _IO  ('C',0x0A)
			//Set up the cdb test unit ready
			sio.cdb[0] = CMDTestUnitReady/*0x00*/;
			sio.cdb[1] = 0;
			sio.cdb[2] = 0;
			sio.cdb[3] = 0;
			sio.cdb[4] = 0;
			sio.cdb[5] = 0;

			sio.cdb_length = 6;

			//Set up the data part:
			sio.data = cmdBuffer;
			sio.data_length = sizeof(cmdBuffer);
			sio.data_direction = NO_TRANSFER;

			/*
			* Set the timeout then execute:
			*/
			sio.timeout_ms = DAT_TESTUNITREADY_TIMEOUT;

			status = ExecScsiCommand (&sio);
			if(status == 0){
				bRet = true;
				break;
			}
			//sleep 1 seconds
			sleep(1);
			expire -= 1;
		}while(expire > 0 && bRet == false);

		if("" != devName){
			CloseDevice(sio.fd);
		}
		return bRet;
	}
	bool MoveCartridge(const string& changerDevName, int srcSlotId, int dstSlotId, LtfsError& error)
	{
    	bool bRet = false;
		scsi_cmd_io sio;
		unsigned char        cmdBuffer [1024];
		int                  status;
		int retry = MOVE_TAPE_FAIL_RETRY_TIMES;

		LtfsLogInfo("MoveCartridge: srcSlotId = " << srcSlotId << ", dstSlotId = " << dstSlotId << endl);

		if(srcSlotId == dstSlotId){
			error.SetErrCode(ERR_MOVE_SRC_DST_ID_SAME);
			LtfsLogError("LtfsChanger::MoveCartridge: source and destination is the same, no need to move.");
			return false;
		}

		// check if the changer is ready to run the command
		if(!TestUnitReady(changerDevName, -1, 60)){
			error.SetErrCode(ERR_CHANGER_NOT_READY);
			error.AddStringParam(changerDevName);
			LtfsLogError("LtfsChanger::MoveCartridge: Changer is not ready to move medium.");
			return false;
		}

		//LTFSLE: #define SMCIOC_MOVE_MEDIUM            _IOW ('C',0x04,struct move_medium)
		sio.fd = OpenDevice(changerDevName);
		//Set up the cdb move medium
		sio.cdb[0] = 0xA5;
		sio.cdb[1] = 0;
		sio.cdb[2] = 0;
		sio.cdb[3] = 0;
		sio.cdb[4] = (unsigned char)(srcSlotId >> 8);
		sio.cdb[5] = (unsigned char)(srcSlotId & 0xFF);
		sio.cdb[6] = (unsigned char)(dstSlotId >> 8);
		sio.cdb[7] = (unsigned char)(dstSlotId & 0xFF);
		sio.cdb[8] = 0;
		sio.cdb[9] = 0;
		sio.cdb[10] = 0;
		sio.cdb[11] = 0;
		sio.cdb_length = 12;
		sio.data_length = sizeof(cmdBuffer);
		sio.data_direction = HOST_READ;
		sio.timeout_ms = LTFS_MOVEMEDIA_TIMEOUT;

		bool bPrevent = false;
		while(retry > 0){
			retry--;
			memset(cmdBuffer, 0, sizeof(cmdBuffer));
			error.ClearParams();
			error.SetErrCode(ERR_NO_ERR);
			//Set up the data part:
			sio.data = cmdBuffer;
			status = ExecScsiCommand (&sio);
			if(sio.senseCode.senseCode != 0){
				error.SetSenseCode(sio.senseCode);
			}
			// check sense code
			// Illegal Request (05h)	3Bh 0Dh Medium destination element full
			if(sio.senseCode.senseCode == 0x05 && sio.senseCode.ascCode == 0x3B && sio.senseCode.ascqCode == 0x0D){
				error.SetErrCode(ERR_MOVE_DST_FULL);
				error.AddIntParam(dstSlotId);
				LtfsLogError("LtfsChanger::MoveCartridge: Medium destination element full.");
				goto CHECK_RETURN;
			}
			// Illegal Request (05h)  3Bh 0Eh Medium source element empty
			else if(sio.senseCode.senseCode == 0x05 && sio.senseCode.ascCode == 0x3B && sio.senseCode.ascqCode == 0x0E){
				// previous error status is drive media removal prevented, this time it should have succeed
				if(bPrevent){
					LtfsLogDebug("LtfsChanger::MoveCartridge: Medium source element empty. Last error is drive media removal prevented.");
					// wait for the changer to finished moving tape
					bRet = TestUnitReady("", sio.fd, 180);
					break;
				}
				error.SetErrCode(ERR_MOVE_SRC_EMPTY);
				error.AddIntParam(srcSlotId);
				LtfsLogError("LtfsChanger::MoveCartridge: Medium source element empty.");
				goto CHECK_RETURN;
			}
			// Illegal Request (05h)  53h 03h Drive media removal prevented state set
			else if(sio.senseCode.senseCode == 0x05 && sio.senseCode.ascCode == 0x53 && sio.senseCode.ascqCode == 0x03){
				bPrevent = true;
				if(retry > 0){
					sleep(MOVE_TAPE_FAIL_RETRY_WAIT);
					LtfsLogError("LtfsChanger::MoveCartridge: Drive media removal prevented state set. Retry times left " << retry << ".");
					continue;
				}
				error.SetErrCode(ERR_DRIVE_PREVENT_REMOVE_MEDIUM);
				error.AddIntParam(srcSlotId);
				LtfsLogError("LtfsChanger::MoveCartridge: Drive media removal prevented state set.");
				goto CHECK_RETURN;
			}
			// Illegal Request (05h)  04h 83h Door open
			else if(sio.senseCode.senseCode == 0x05 && sio.senseCode.ascCode == 0x04 && sio.senseCode.ascqCode == 0x83){
				/*if(retry > 0){
					sleep(MOVE_TAPE_FAIL_RETRY_WAIT);
					LtfsLogError("LtfsChanger::MoveCartridge: mail slot open, please close the mail slot first. Retry times left " << retry << ".");
					continue;
				}*/
				error.SetErrCode(ERR_MAIL_SLOT_OPEN);
				error.AddIntParam(dstSlotId);
				LtfsLogError("LtfsChanger::MoveCartridge: mail slot open, please close the mail slot first.");
				goto CHECK_RETURN;
			}
			// Illegal Request (05h)  3Bh A0h Medium transfer element full
			else if(sio.senseCode.senseCode == 0x05 && sio.senseCode.ascCode == 0x3B && sio.senseCode.ascqCode == 0xA0){
				if(retry > 0){
					sleep(MOVE_TAPE_FAIL_RETRY_WAIT);
					LtfsLogError("LtfsChanger::MoveCartridge: Medium transfer element full. Retry times left " << retry << ".");
					continue;
				}
				error.SetErrCode(ERR_CHANGER_TRANSFER_FULL);
				LtfsLogError("LtfsChanger::MoveCartridge: Medium transfer element full.");
				goto CHECK_RETURN;
			}
			// Not Ready (02)  04h 8Eh Not ready, the media changer is in sequential mode
			else if(sio.senseCode.senseCode == 0x02 && sio.senseCode.ascCode == 0x04 && sio.senseCode.ascqCode == 0x8E){
				error.SetErrCode(ERR_CHANGER_SEQ_MODE);
				LtfsLogError("LtfsChanger::MoveCartridge: Not ready, the media changer is in sequential mode.");
				goto CHECK_RETURN;
			}
			// Not Ready (02)  04h 01H Not ready, in progress becoming ready, scanning magazines, etc.
			/*else if(sio.senseCode.senseCode == 0x02 && sio.senseCode.ascCode == 0x04 && sio.senseCode.ascqCode == 0x01){
			error.SetErrCode(ERR_CHANGER_NOT_READY);
			error.AddStringParam(changerDevName);
				LtfsLogError("LtfsChanger::MoveCartridge: Not ready, in progress becoming ready, scanning magazines, etc.");
				goto CHECK_RETURN;
			}*/
			// When there is a error with the cleaning cartridge the MOVE MEDIUM or EXCHANGE MEDIUM command shall return CHECK
			//CONDITON status and move the cleaning cartridge back to its source element address.
			//Possible sense data on moving cleaning cartridges are:
			//3h 30h 07h MEDIUM ERROR, the cleaning cartridge is expired
			/*else if(sio.senseCode.senseCode == 0x03 && sio.senseCode.ascCode == 0x30 && sio.senseCode.ascqCode == 0x07){
			error.SetErrCode(ERR_CLEANING_TAPE_EXPIRED);
				LtfsLogError("LtfsChanger::MoveCartridge: the cleaning cartridge is expired.");
				goto CHECK_RETURN;
			}*/
			// Unit Attention (06h)  2Ah 01h Mode parameters changed
			else if(sio.senseCode.senseCode == 0x06 && sio.senseCode.ascCode == 0x2A && sio.senseCode.ascqCode == 0x01){
				if(retry > 0){
					sleep(MOVE_TAPE_FAIL_RETRY_WAIT);
					LtfsLogError("LtfsChanger::MoveCartridge: Changer mode parameters changed. Retry times left " << retry << ".");
					continue;
				}
				error.SetErrCode(ERR_CHANGER_MODE_PARAM_CHANGED);
				LtfsLogError("LtfsChanger::MoveCartridge: Changer mode parameters changed.");
				goto CHECK_RETURN;
			}
			if (status != 0){
				error.SetErrCode(ERR_CHANGER_MOVE_FAILED);
				error.AddIntParam(srcSlotId);
				error.AddIntParam(dstSlotId);
				LtfsLogError("LtfsChanger::MoveCartridge: Failed to move medium from slot " + TO_STRING(srcSlotId) + string(" to slot ") + TO_STRING(dstSlotId) + ".");
				goto CHECK_RETURN;
			}
			// wait for the changer to finished moving tape
			bRet = TestUnitReady("", sio.fd, 180);
			if(!bRet){
				error.SetErrCode(ERR_CHANGER_MOVE_FAILED);
				error.AddIntParam(srcSlotId);
				error.AddIntParam(dstSlotId);
				LtfsLogError("LtfsChanger::MoveCartridge: Failed to move medium from slot " + TO_STRING(srcSlotId) + string(" to slot ") + TO_STRING(dstSlotId) + ".");
				goto CHECK_RETURN;
			}
			retry = 0;
		}//while(retry-- > 0){

CHECK_RETURN:
		CloseDevice(sio.fd);
		bRet = false;
		unsigned char        inqbuffer [10800];
		map<int, bool> slotMap;
		int chkRetry = 5;
		bool bCheck = false;
		while(!bCheck && chkRetry-- >= 0){
			slotMap.clear();
			if(true == ReadChangerElementStatus(changerDevName, inqbuffer, sizeof(inqbuffer))){
				int totalSize = (int)((inqbuffer[6]) << 8) + (inqbuffer[7]) + 8;
				bCheck = HandleElementBuffCheck(8, inqbuffer, totalSize, slotMap);
			}
			if(true == bCheck){
				break;
			}
			sleep(5);
		}
		map<int, bool>::iterator itSrc = slotMap.find(srcSlotId);
		map<int, bool>::iterator itDst = slotMap.find(dstSlotId);
		if(itSrc != slotMap.end() && itDst != slotMap.end()){
			if(itSrc->second == false && itDst->second == true){
				bRet = true;
			}
		}

		if(bRet){
			LtfsLogInfo("LtfsChanger::MoveCartridge() finished. Result = " << bRet << endl);
		}else{
			if(error.GetErrCode() == ERR_NO_ERR){
				error.SetErrCode(ERR_CHANGER_MOVE_FAILED);
				error.AddIntParam(srcSlotId);
				error.AddIntParam(dstSlotId);
				LtfsLogError("LtfsChanger::MoveCartridge: Failed to move medium from slot " + TO_STRING(srcSlotId) + string(" to slot ") + TO_STRING(dstSlotId) + ".");
			}
			LtfsLogError("LtfsChanger::MoveCartridge() failed. Result = " << bRet << endl);
		}
		return bRet;
	}

	string GetTapeDiagnoseLog(const string& barcode)
	{
		string strLogFile = DIAGNOSIS_TAPE_LOG_PATH + string("/tape_check_log_") + barcode + string(".log");
		return strLogFile;
	}

	bool CheckTape(const string& driveStDevName, const string& barcode, CHECK_TAPE_FLAG flag, LtfsError& error)
	{
		if(barcode == ""){
			error.SetErrCode(ERR_CHECK_EMPTY);
			LtfsLogError("CheckTape: barcode empty, will not check." << endl);
			return false;
		}

        // if it is already unmounted, return true
	    if(true == IsDriveMounted(driveStDevName, barcode)){
			LtfsLogError("Failed to run ltfsck on drive " + driveStDevName + ". Medium is already mounted or in use.");
	    	error.SetErrCode(ERR_CHECK_MEDIUM_INUSE);
			error.AddStringParam(driveStDevName);
	    	return false;
	    }

        try{
        	string logFile = GetDiagnoseTapeLogPath(barcode);
			string cmd = "ltfsck ";
			if(flag == FLAG_FULL_RECOVERY){
				cmd += " --full-recovery "; // Recover extra data blocks into directory _ltfs_lostandfound            TODO: to check if we need to enalbe it
			}
			if(flag == FLAG_DEEP_RECOVERY){
				cmd += " --deep-recovery "; // Recover EOD missing cartridge. Some blocks may be erased but
										// recover to final unmount point which has an index version  2.0.0  or earlier at least    TODO: to check if we need to enalbe it
			}
			cmd += " " + driveStDevName + " 1>" + logFile + " 2>&1";

			setenv("LC_ALL", "en_US.UTF-8", 1);

			vector<string> checkLogs = GetCommandOutputLines(cmd, 900); // 15 minutes
			for(vector<string>::iterator it = checkLogs.begin(); it != checkLogs.end(); it++){
				cmatch match;

				//	LTFS20035E Unable to lock device (Resource temporarily unavailable)   (HP ltfs 2.0.0)
				//  LTFS12113E /dev/IBMtape0: medium is already mounted or in use   (IBM ltfs 1.3.0.0)
				regex matchMount("^\\s*(LTFS20035E|LTFS12113E).*");
				if(regex_match((*it).c_str(), match, matchMount)){
					error.SetErrCode(ERR_CHECK_MEDIUM_INUSE);
					error.AddStringParam(driveStDevName);
					LtfsLogError("Failed to run ltfsck on drive " + driveStDevName + ". Medium is already mounted or in use.");
					return false;
				}


				// LTFS12017E Cannot load the medium (3)


				//HP
				//LTFS12017E Cannot load the medium after 3 tries (-1)
				//LTFS16012E Cannot load medium
				regex matchLoad("^\\s*(LTFS12017E|LTFS16012E).*");
				// IBM
				// LTFS12016E No medium present
				// LTFS12173E Error on test unit ready: Medium Not Present     (ltfs 1.3.0.0)
				regex matchMedium("^\\s*LTFS12016E.*");
				regex matchMediumTest("^\\s*LTFS12173E.*Medium\\s+Not\\s+Present.*");
				if(regex_match((*it).c_str(), match, matchMedium)
					|| regex_match((*it).c_str(), match, matchMediumTest)
					|| regex_match((*it).c_str(), match, matchLoad)
				){
					error.SetErrCode(ERR_CHECK_NO_MEDIUM);
					error.AddStringParam(driveStDevName);
					LtfsLogError("Failed to run ltfsck on drive " + driveStDevName + ". No medium present.");
					return false;
				}

				/*
				//LTFS12157E Unsupported cartridge type (0x48)
				//LTFS11298E Cannot mount volume: unsupported medium         (old ltfs version)
				//LTFS11298E Cannot read volume: unsupported medium				(ltfs 1.3.0.0)
				regex matchUnsupportedMedium("^\\s*(LTFS12157E|LTFS11298E).*");
				if(regex_match((*it).c_str(), match, matchUnsupportedMedium)){
					error.SetErrCode(ERR_XXXXXX);
					error.AddStringParam(driveStDevName);
					LtfsLogError("Failed to run ltfsck on drive " + driveStDevName + ". Unsupported cartridge type.");
					return false;
				}*/
			}
        }catch(...){
			LtfsLogError("Failed to run ltfsck on drive " + driveStDevName + ".");
	    	error.SetErrCode(ERR_CHECK_FAILED);
			error.AddStringParam(driveStDevName);
	    	return false;
        }

        return true;
	}

	bool Mount(const string& barcode, const string& driveStDevName, LtfsError& error)
	{
		LtfsLogDebug("MountDebugStart: Mount: barcode = " << barcode << ", driveStDevName = " << driveStDevName);
		if(barcode == ""){
			error.SetErrCode(ERR_MOUNT_EMPTY);
			LtfsLogError("Mount: barcode empty, will not mount." << endl);
			return false;
		}
        fs::path pathLTFS = GetLtfsFolder() / barcode;
        // if it is already unmounted, return true
	    if(true == IsDriveMounted(driveStDevName, barcode)){
			LtfsLogDebug("MountDebug: Mount: barcode = " << barcode << ": already mounted.");
	    	return true;
	    }
        try{
			if ( !fs::exists(pathLTFS) ) {
				if(!fs::create_directory(pathLTFS)){
					error.SetErrCode(ERR_MOUNT_CREATE_MOUNT_POINT);
					error.AddStringParam(barcode);
					LtfsLogError("Mount: Fail to create mount point:." << pathLTFS.string() << endl);
					return false;
				}
			}

			int retry = MOUNT_FAIL_RETRY_TIMES;
			while(retry >= 0){
				string cmd = "ltfs -o devname=" + driveStDevName + " " + pathLTFS.string() + " -o trace 2>&1";
				setenv("LC_ALL", "en_US.UTF-8", 1);

				bool bNeedRetry = false;
				LtfsLogDebug("MountDebug: Mount: barcode = " << barcode << ": cmd start: " <<cmd );
				vector<string> rets = GetCommandOutputLines(cmd, 900, false); // 15 minutes
				LtfsLogDebug("MountDebug: Mount: barcode = " << barcode << ": cmd end: " <<cmd );
				for(vector<string>::iterator it = rets.begin(); it != rets.end(); it++){
					cmatch match;
					//LTFS10001E Memory allocation failed (_xml_input_read_callback)
					//LTFS10001E Memory allocation failed (fs_allocate_dentry)
					//LTFS10001E Memory allocation failed (_xml_parse_dirtree)
					//LTFS17037E XML parser: failed to read from XML stream
					//LTFS17016E Cannot parse index direct from medium
					//LTFS11194W Cannot read index: failed to read and parse XML data (-1014)
					//Entity: line 1: parser error : Extra content at the end of the document

					//  fuse: device not found, try 'modprobe fuse' first
					regex matchFuse("^\\s*fuse.*modprobe\\s+fuse.*");
					if(regex_match((*it).c_str(), match, matchFuse)){
						if(retry <= 0){
							error.SetErrCode(ERR_MOUNT_FUSE);
							error.AddStringParam(barcode);
							LtfsLogError("Failed to mount the medium " + barcode + ". Fuse not started.");
							RemoveFile(pathLTFS);
							return false;
						}
						bNeedRetry = true;
						break;
					}

					//fuse: mountpoint is not empty
					//fuse: if you are sure this is safe, use the 'nonempty' mount option
					regex matchNotEmpty("^\\s*fuse:\\s+mountpoint\\s+is\\s+not\\s+empty.*");
					if(regex_match((*it).c_str(), match, matchNotEmpty)){
						if(retry <= 0){
							error.SetErrCode(ERR_MOUNT_MOUNTPOINT_NOT_EMPTY);
							error.AddStringParam(barcode);
							LtfsLogError("Failed to mount the medium " + barcode + ": mountpoint is not empty.");
							//RemoveFile(pathLTFS);
							return false;
						}
						bNeedRetry = true;
						break;
					}

					//LTFS10004E Cannot open device '/dev/st00'
					//LTFS10004E Cannot open device '/dev/IBMtape0'
					regex matchDev("^\\s*LTFS10004E.*");
					if(regex_match((*it).c_str(), match, matchDev)){
						if(retry <= 0){
							error.SetErrCode(ERR_MOUNT_OPEN_DRIVE);
							error.AddStringParam(barcode);
							error.AddStringParam(driveStDevName);
							RemoveFile(pathLTFS);
							LtfsLogError("Failed to mount the medium " + barcode + ". Failed to open drive device " +driveStDevName + ".");
							return false;
						}
						bNeedRetry = true;
						break;
					}

					// LTFS11006E Cannot mount volume: failed to load the tape  (HP ltfs 2.0.0)
					// LTFS11006E Cannot read volume: failed to load the tape   (IBM ltfs 1.3.0.0)
					regex matchLoad("^\\s*LTFS11006E.*");
					if(regex_match((*it).c_str(), match, matchLoad)){
						if(retry <= 0){
							error.SetErrCode(ERR_MOUNT_LOAD_TAPE);
							error.AddStringParam(barcode);
							RemoveFile(pathLTFS);
							LtfsLogError("Failed to mount the medium " + barcode + ". Failed to load tape.");
							return false;
						}
						bNeedRetry = true;
						break;
					}

					/*
					//12173E:string { "Error on %s: %s (%d) %s"}
					//LTFS12173E Error on test unit ready: Medium Not Present (-20209) 1068011102     (ltfs 1.3.0.0)
					regex matchNoMedium("^\\s*LTFS11006E.*Medium\\s+Not\\s+Present.*");
					if(regex_match((*it).c_str(), match, matchLoad)){
						if(retry <= 0){
							error.SetErrCode(ERR_MOUNT_LOAD_TAPE);
							error.AddStringParam(barcode);
							RemoveFile(pathLTFS);
							LtfsLogError("Failed to mount the medium " + barcode + ". Failed to load tape.");
							return false;
						}
						bNeedRetry = true;
						break;
					}
					*/

					//LTFS14013E Cannot mount the volume. Please format the medium with mkltfs or check it with ltfsck.  (not in ltfs 1.3.0.0)
					//LTFS14013E Cannot mount the volume  (1.3.0.0)
					regex matchFormatCheck("^\\s*LTFS14013E.*");
					if(regex_match((*it).c_str(), match, matchFormatCheck)){
						if(retry <= 0){
							error.SetErrCode(ERR_MOUNT_FORMAT_CHECK);
							error.AddStringParam(barcode);
							RemoveFile(pathLTFS);
							LtfsLogError("Failed to mount the medium " + barcode + ". Please format the medium with mkltfs or check it with ltfsck.");
							return false;
						}
						bNeedRetry = true;
						break;
					}

					//LTFS17142E Both EODs are missing
					//LTFS17146E EOD of IP(0) is missing. The deep recovery is required
					//LTFS17148E Please try to ltfsck with --deep-recovery option
					regex matchEodMissing("^\\s*(LTFS17142E|LTFS17146E|LTFS17148E).*");
					if(regex_match((*it).c_str(), match, matchEodMissing)){
						if(retry <= 0){
							error.SetErrCode(ERR_MOUNT_EOD_MISSING);
							error.AddStringParam(barcode);
							RemoveFile(pathLTFS);
							LtfsLogError("Failed to mount the medium " + barcode + ". EOD missing. The deep recovery is required.");
							return false;
						}
						bNeedRetry = true;
						break;
					}

					//LTFS12157E Unsupported cartridge type (0x48)
					//LTFS11298E Cannot mount volume: unsupported medium         (old ltfs version)
					//LTFS11298E Cannot read volume: unsupported medium				(ltfs 1.3.0.0)
					regex matchUnsupportedMedium("^\\s*(LTFS12157E|LTFS11298E).*");
					if(regex_match((*it).c_str(), match, matchUnsupportedMedium)){
						if(retry <= 0){
							error.SetErrCode(ERR_MOUNT_UNSUPPORTED_MEDIUM);
							error.AddStringParam(barcode);
							RemoveFile(pathLTFS);
							LtfsLogError("Failed to mount the medium " + barcode + ". Unsupported cartridge type.");
							return false;
						}
						bNeedRetry = true;
						break;
					}

					//17126E:string { "Unexpected EOD status (%d, %d)" }  //TODO: need to handle this?

					//LTFS11009E Cannot mount volume: failed to read partition labels.
					regex matchReadPartFail("^\\s*LTFS11009E.*");
					if(regex_match((*it).c_str(), match, matchReadPartFail)){
						if(retry <= 0){
							error.SetErrCode(ERR_MOUNT_READ_PARTITION_FAIL);
							error.AddStringParam(barcode);
							RemoveFile(pathLTFS);
							LtfsLogError("Failed to mount the medium " + barcode + ". Failed to read partition labels.");
							return false;
						}
						bNeedRetry = true;
						break;
					}

					//LTFS11031I Volume mounted successfully                      (HP 2.0.0)
					//LTFS14111I Initial setup completed successfully             (IBM ltfs 1.3.0.0)
					regex matchSuccess("^\\s*(LTFS11031I|LTFS14111I).*");
					if(regex_match((*it).c_str(), match, matchSuccess)){
						// make sure the mount point exists
						int tmpRetry = 8;
						while(tmpRetry-- > 0){
							LtfsLogDebug("MountDebug: Mount: barcode = " << barcode << ": check retrying: " << tmpRetry);
							if(true == IsMountPointMounted(pathLTFS.string())){
								LtfsLogDebug("MountDebug: Mount: barcode = " << barcode << ": mounted.");
								return true;
							}
							sleep(2);
						}
						bNeedRetry = true;
						break;
						/*LtfsLogError("MountDebug: Mount: barcode = " << barcode << ": 14.1");
						error.SetErrCode(ERR_MOUNT_FAIL);
						error.AddStringParam(barcode);
						RemoveFile(pathLTFS);
						return false;*/
					}
				}// for

				/*if(ret == false){
					if(retry <= 0){
						error.SetErrCode(ERR_MOUNT_FAIL);
						error.AddStringParam(barcode);
						RemoveFile(pathLTFS);
						LtfsLogError("Failed to mount the medium " + barcode + ". Unknown error.");
						LtfsLogError("MountDebug: Mount: barcode = " << barcode << ": 15");
						return IsMountPointMounted(pathLTFS.string());
					}
					bNeedRetry = true;
				}*/
				if(bNeedRetry){
					LtfsLogWarn("Mount tape " << barcode << " in drive " << driveStDevName << " failed. Retry left: " << retry << ".");
					retry--;
					sleep(MOUNT_FAIL_RETRY_WAIT);
					continue; // while
				}

				LtfsLogDebug("MountDebug: Mount: barcode = " << barcode << ": debug 17");
				break; //while
        	}//while
		}catch(...){
			LtfsLogError("Mount: Exception on mounting tape ." << barcode << endl);
		}

		bool bRet = IsMountPointMounted(pathLTFS.string());
		if(false == bRet){
			error.SetErrCode(ERR_MOUNT_FAIL);
			error.AddStringParam(barcode);
			RemoveFile(pathLTFS);
		}
		LtfsLogDebug("MountDebug: Mount: barcode = " << barcode << ". bRet = " << bRet);
		return bRet;
	}

	bool UnMount(const string& barcode, const string& driveStDevName, LtfsError& error, const string& mountPath)
	{
		LtfsLogDebug("UnMountDebugStart: Mount: barcode = " << barcode << ", driveStDevName = " << driveStDevName << ", mountPath = " << mountPath);

		string mountPoint = mountPath;
		if(mountPoint == ""){
			mountPoint = (GetLtfsFolder() / barcode).string();
		}

		if(barcode == ""){
			error.SetErrCode(ERR_UNMOUNT_EMPTY);
			LtfsLogError("UnMount: barcode empty, will not unmount." << endl);
			return false;
		}

        // if it is already unmounted, return true
	    if(false == IsMountPointMounted(mountPoint)){
	    	LtfsLogDebug("UnMount mount point " << mountPoint << " is not mounted yet.");
	    	return true;
	    }

		LtfsLogDebug("UnMountDebug:  1");
        try{
        	int retry = UNMOUNT_FAIL_RETRY_TIMES;
        	int tmpRetry = 1;
			string cmd = "fusermount -u " + mountPoint + " 2>&1";
			while(tmpRetry > 0){
				retry--;
				tmpRetry = 0;
				vector<string> rets = GetCommandOutputLines(cmd);
				LtfsLogDebug("UnMountDebug:  2");
				for(vector<string>::iterator it = rets.begin(); it != rets.end(); it++){
					LtfsLogDebug((*it) << endl);
					//fusermount: failed to unmount /opt/VS/vsMounts/LT1017L5: Device or resource busy
					regex matchFail("^.*" + barcode + ":\\s+Device\\s+or\\s+resource\\s+busy.*");
					cmatch match;
					if(regex_match((*it).c_str(), match, matchFail)){
						if(retry > 0){
							tmpRetry = retry;
							sleep(UNMOUNT_FAIL_RETRY_WAIT);
							LtfsLogError("unmount tape " << barcode << " failed: device busy. Retry times left " << retry << endl);
							break;
						}
						error.SetErrCode(ERR_UNMOUNT_BUSY);
						error.AddStringParam(barcode);
						LtfsLogError("unmount tape " << barcode << " failed: device busy." << endl);
						return false;
					}
					//fusermount: /opt/VS/vsMounts/LT1007L5 not mounted
					regex matchNo("^.*" + barcode + "\\s+not\\s+mounted.*");
					if(regex_match((*it).c_str(), match, matchNo)){
						error.SetErrCode(ERR_UNMOUNT_NOT_MOUNTED);
						error.AddStringParam(barcode);
						LtfsLogError("unmount tape " << barcode << " failed: device not mounted." << endl);
						return !IsMountPointMounted(mountPoint);
					}
				}
			}//while(tmpRetry-- > 0){
			LtfsLogDebug("UnMountDebug:  3");
		    if(true == IsMountPointMounted(mountPoint)){
				error.SetErrCode(ERR_UNMOUNT_FAIL);
				error.AddStringParam(barcode);
				LtfsLogError("unmount tape fail:" << barcode << endl);
				return false;
		    }
			// need to check if it is really unmounted
			int chkRetry = 300;
			LtfsLogDebug("UnMountDebug:  4");
			while(chkRetry-- > 0 && true == IsDriveMounted(driveStDevName, "") ){
				sleep(2);
			}
			LtfsLogDebug("UnMountDebug:  5");
			if(chkRetry <= 0){
				error.SetErrCode(ERR_UNMOUNT_FAIL);
				error.AddStringParam(barcode);
				LtfsLogError("unmount tape fail:" << barcode << endl);
				return false;
			}

		    // remove the mount point
			RemoveFile(mountPoint);
			LtfsLogDebug("UnMountDebug:  6");
		}catch(...){
			error.SetErrCode(ERR_UNMOUNT_FAIL);
			error.AddStringParam(barcode);
			LtfsLogError("UnMountDebug: Exception on unmounting tape ." << barcode << endl);
			bool bRet =  !IsMountPointMounted(mountPoint);
			LtfsLogDebug("UnMountDebug:  1, bRet = " << bRet);
			return bRet;
		}

		LtfsLogDebug("UnMountDebug:  7");
		bool bRet = !IsMountPointMounted(mountPoint);
		LtfsLogDebug("UnMountDebug:  8, bRet = " << bRet);
		return bRet;
	}
	bool Format(const string& driveStDevName, const string& barcode, LtfsError& error)
	{
		bool ret = false;

		if(barcode == ""){
			error.SetErrCode(ERR_FORMAT_EMPTY);
			LtfsLogError("Format: barcode empty, will not format the tape." << endl);
			return false;
		}

		// check if drive is mounted, if yes, not continue
		if(true == IsDriveMounted(driveStDevName)){
			LtfsLogInfo("Drive " << driveStDevName << " has mounted tape, format is not permitted.");
			error.SetErrCode(ERR_DRIVE_MOUNTED);
			error.AddStringParam(driveStDevName);
			return false;
		}

        try{
			string cmd = "mkltfs  --trace --force --device=" + driveStDevName + " 2>&1";
			int retry = FORMAT_FAIL_RETRY_TIMES;
			while(retry > 0){
				vector<string> rets = GetCommandOutputLines(cmd, 900); // 15 minutes
				ret = false;
				bool bNeedRetry = false;
				for(vector<string>::iterator it = rets.begin(); it != rets.end(); it++){
					cmatch match;
					// the tape is already mounted
					//LTFS12525E Unable to lock device (Resource temporarily unavailable)  (HP ltfs 2.0.0)
					regex matchFuse("^\\s*.*LTFS12525E.*");
					if(regex_match((*it).c_str(), match, matchFuse)){
						error.SetErrCode(ERR_FORMAT_NOT_READY);
						error.AddStringParam(barcode);
						LtfsLogError("format tape " << barcode << " failed: Resource temporarily unavailable." << endl);
						ret = false;
						break;
					}

					//LTFS12012E Cannot open device: backend open call failed
					//LTFS15009E Cannot open device '/dev/IBMtape0' (-5)
					//LTFS15009E Cannot open device '/dev/st0' (-5)
					regex matchDev("^\\s*.*(LTFS12012E|LTFS15009E).*");
					if(regex_match((*it).c_str(), match, matchDev)){
						if(retry > 0){
							LtfsLogDebug("Fromat tape " << barcode << " in drive " << driveStDevName << " retrying " << retry << ".");
							retry--;
							sleep(FORMAT_FAIL_RETRY_WAIT);
							bNeedRetry = true;
							break;
						}
						error.SetErrCode(ERR_FORMAT_FAIL);
						error.AddStringParam(barcode);
						//error.AddStringParam(driveStDevName);
						LtfsLogError("format tape " << barcode << " failed: cannot open device " << driveStDevName << "." << endl);
						ret = false;
						break;
					}
					// LTFS11095E Cannot format: medium is write protected
					regex matchProtect("^\\s*.*LTFS11095E.*");
					if(regex_match((*it).c_str(), match, matchProtect)){
						error.SetErrCode(ERR_FORMAT_WRITE_PROTECT);
						error.AddStringParam(barcode);
						LtfsLogError("format tape " << barcode << " failed: medium is write protected." << endl);
						ret = false;
						break;
					}

					// no tape in drive
					//LTFS11093E Cannot format: failed to load the medium

					//LTFS15023I Formatting failed

					/*
					//LTFS12113E /dev/IBMtape0: medium is already mounted or in use
					regex matchBusy("^\\s*.*LTFS12113E.*");
					if(regex_match((*it).c_str(), match, matchBusy)){
						error.SetErrCode(ERR_FORMAT_TAPE_MOUNTED_BUSY);
						error.AddStringParam(barcode);
						LtfsLogError("format tape " << barcode << " failed: medium is already mounted or in use." << endl);
						ret = false;
						break;
					}
					//LTFS11299E:string { "Cannot format: unsupported medium" }
					regex matchUnsupport("^\\s*.*LTFS11299E.*");
					if(regex_match((*it).c_str(), match, matchUnsupport)){
						error.SetErrCode(ERR_FORMAT_TAPE_NOT_SUPPORTED);
						error.AddStringParam(barcode);
						LtfsLogError("format tape " << barcode << " failed: unsupported medium." << endl);
						ret = false;
						break;
					}
					//LTFS12016E:string { "No medium present" }
					//LTFS12173E Error on test unit ready: Medium Not Present (-20209) 1068011019      (LTFS 1.3.0.0)
					regex matchNoTape("^\\s*.*LTFS12173E.*Medium\\s+Not\\s+Present.*");
					if(regex_match((*it).c_str(), match, matchNoTape)){
						error.SetErrCode(ERR_FORMAT_NO_TAPE);
						error.AddStringParam(barcode);
						LtfsLogError("format tape " << barcode << " failed: No medium present." << endl);
						ret = false;
						break;
					}
					*/

					//11004E:string { "Cannot take the device lock (%s)" }
					//11093E:string { "Cannot format: failed to load the medium" }
					//11096E:string { "Cannot format: requested block size is %lu bytes, but the device only supports %u" }
					//11098E:string { "Cannot format: failed to partition the medium (%d)" }
					//11099E:string { "Cannot format: failed to set medium compression (%d)" }
					//11279E:string { "Cannot write Index to partition %c (%d)" }
					//11030E:string { "Cannot release the device lock (%s)" }
					//12010E:string { "Failed to grab the device lock (%s)" }
					//12017E:string { "Cannot load the medium after %d tries (%d)" }
					//12018E:string { "Cannot load the medium: failed to lock the medium in the drive (%d)" }
					//12019E:string { "Cannot load the medium: failed to determine medium position (%d)" }
					//12020E:string { "Cannot load the medium: failed to set device defaults (%d)" }
					//12021E:string { "Cannot load the medium: failed to get device parameters (%d)" }
					//12010E:string { "Failed to grab the device lock (%s)" }
					//12027E:string { "Cannot lock medium in the drive: backend call failed (%d)" }
					//LTFS15023I Formatting failed

					// LTFS15024I Medium formatted successfully
					regex matchSuccess("^\\s*.*LTFS15024I.*");
					if(regex_match((*it).c_str(), match, matchSuccess)){
						ret = true;
						break;
					}
				}// for
				if(bNeedRetry){
					continue;
				}
				break;
			}//while
		}catch(...){
			ret = false;
		}
		if(false == ret && error.GetErrCode() == ERR_NO_ERR){
			error.SetErrCode(ERR_FORMAT_FAIL);
			error.AddStringParam(barcode);
			LtfsLogError("Failed on formatting tape " << barcode << endl);
		}

		return ret;
	}

	bool UnFormat(const string& driveDevName, const string& driveStDevName, const string& barcode, LtfsError& lfsErr)
	{
		bool bRet = true;
		if(barcode == ""){
			lfsErr.SetErrCode(ERR_UN_FORMAT_EMPTY);
			LtfsLogError("UnFormat: barcode empty, will not unformat the tape." << endl);
			return false;
		}

		// check if drive is mounted, if yes, not continue
		if(true == IsDriveMounted(driveStDevName)){
			LtfsLogInfo("Drive " << driveStDevName << " has mounted tape, unformat is not permitted.");
			lfsErr.SetErrCode(ERR_DRIVE_MOUNTED);
			lfsErr.AddStringParam(driveStDevName);
			return false;
		}

		bool bIsIBMLtfs = IsIBMLtfs();

        try{
			string cmd = "unltfs --device=" + driveStDevName + " -t -y 2>&1";
			if(bIsIBMLtfs){
				cmd = "mkltfs -w --device=" + driveStDevName + " -t -f 2>&1";
			}

			int retry = UN_FORMAT_FAIL_RETRY_TIMES;
			while(retry > 0){
				vector<string> rets = GetCommandOutputLines(cmd, 900); // 15 minutes
				bRet = false;
				bool bNeedRetry = false;
				for(vector<string>::iterator it = rets.begin(); it != rets.end(); it++){
					cmatch match;
					// the tape is already mounted
					//LTFS12525E Unable to lock device (Resource temporarily unavailable)  (HP ltfs 2.0.0)
					regex matchFuse("^\\s*.*LTFS12525E.*");
					if(regex_match((*it).c_str(), match, matchFuse)){
						lfsErr.SetErrCode(ERR_FORMAT_NOT_READY);
						lfsErr.AddStringParam(barcode);
						LtfsLogError("format tape " << barcode << " failed: Resource temporarily unavailable." << endl);
						bRet = false;
						break;
					}

					//LTFS12012E Cannot open device: backend open call failed
					//LTFS15009E Cannot open device '/dev/IBMtape0' (-5)
					//LTFS15009E Cannot open device '/dev/st0' (-5)
					regex matchDev("^\\s*.*(LTFS12012E|LTFS15009E).*");
					if(regex_match((*it).c_str(), match, matchDev)){
						if(retry > 0){
							LtfsLogDebug("Fromat tape " << barcode << " in drive " << driveStDevName << " retrying " << retry << ".");
							retry--;
							sleep(UN_FORMAT_FAIL_RETRY_WAIT);
							bNeedRetry = true;
							break;
						}
						lfsErr.SetErrCode(ERR_UN_FORMAT_FAIL);
						lfsErr.AddStringParam(barcode);
						//error.AddStringParam(driveStDevName);
						LtfsLogError("format tape " << barcode << " failed: cannot open device " << driveStDevName << "." << endl);
						bRet = false;
						break;
					}

					//LTFS12173E Error on test unit ready: Medium Not Present (-20209) 1068011102
					regex matchNoTape("^\\s*.*LTFS12173E.*Medium\\s+Not\\s+Present.*");
					if(regex_match((*it).c_str(), match, matchNoTape)){
						lfsErr.SetErrCode(ERR_UN_FORMAT_NO_TAPE);
						lfsErr.AddStringParam(barcode);
						LtfsLogError("unformat tape " << barcode << " failed: No medium present." << endl);
						bRet = false;
						break;
					}

					// LTFS11095E Cannot format: the medium is write protected
					regex matchProtect("^\\s*.*LTFS11095E.*");
					if(regex_match((*it).c_str(), match, matchProtect)){
						lfsErr.SetErrCode(ERR_UN_FORMAT_WRITE_PROTECT);
						lfsErr.AddStringParam(barcode);
						LtfsLogError("unformat tape " << barcode << " failed: medium is write protected." << endl);
						bRet = false;
						break;
					}

					// LTFS15040I Medium unformatted successfully  (IBM)
					// Operation succeeded. Cartridge no longer contains a valid LTFS volume.   //HP
					regex matchSuccess("^\\s*.*(LTFS15040I|Operation succeeded).*");
					if(regex_match((*it).c_str(), match, matchSuccess)){
						bRet = true;
						break;
					}
				}// for
				if(bNeedRetry){
					continue;
				}
				break;
			}//while
		}catch(...){
			bRet = false;
		}
		if(false == bRet && lfsErr.GetErrCode() == ERR_NO_ERR){
			lfsErr.SetErrCode(ERR_UN_FORMAT_FAIL);
			lfsErr.AddStringParam(barcode);
			LtfsLogError("Failed on unformatting tape " << barcode << endl);
		}

		if(bIsIBMLtfs){
			return bRet;
		}


		// set non ltfs first
		if(false == SetLoadedTapeLtfsFormat(driveDevName, driveStDevName, false, lfsErr)){
			LtfsLogError("UnFormat: Failed to set tape " << barcode << " to non LTFS format.");
			bRet = false;
		}

		// update status in MAM
		if(false == SetLoadedTapeStatus(driveDevName, driveStDevName, TAPE_UNKNOWN, lfsErr)){
			LtfsLogError("UnFormat: Failed to set tape " << barcode << " to unknown status.");
			bRet = false;
		}

		// update faulty flag in MAM
		if(false == SetLoadedTapeFaulty(driveDevName, driveStDevName, false, lfsErr)){
			LtfsLogError("UnFormat: Failed to set tape " << barcode << " to non LTFS format.");
			bRet = false;
		}

		// update tape group uuid
		if(false == SetLoadedTapeGroup(driveDevName, driveStDevName, "", lfsErr)){
			LtfsLogError("UnFormat: Failed to set tape " << barcode << " with empty tape group uuid.");
			bRet = false;
		}

		// update tape dual copy info
		if(false == SetLoadedTapeDualCopy(driveDevName, driveStDevName, "", lfsErr)){
			LtfsLogError("UnFormat: Failed to set tape " << barcode << " with empty dual copy info.");
			bRet = false;
		}

		// update tape dual copy info
		if(false == SetLoadedTapeBarcode(driveDevName, driveStDevName, "", lfsErr)){
			LtfsLogError("UnFormat: Failed to set tape " << barcode << " with empty barcode in MAM.");
			bRet = false;
		}

		/*
		// use dd to make sure the tape cannot be mounted by ltfs
		string ddCmd = string("dd if=/dev/zero of=") + driveStDevName + string(" bs=1M count=100 2>&1");
		vector<string> rets = GetCommandOutputLines(ddCmd, 60); // 1 minutes
		bool bDdOk = false;
		for(vector<string>::iterator it = rets.begin(); it != rets.end(); it++){
			//104857600 bytes (105 MB) copied, 3.02214 s, 34.7 MB/s
			regex matchDone("^.*\\s*MB\\)\\s*copied.*");
			cmatch match;
			if(regex_match((*it).c_str(), match, matchDone)){
				bDdOk = true;
			}
		}
		if(bDdOk == false){
			LtfsLogError("UnFormat: Failed to use dd to unformat tape " << barcode << ".");
			bRet = false;
		}*/

		return bRet;
	}


	bool TAF(int n, unsigned char* inqbuffer, int bufLen)
	{
		int posStart = 3;
		int nStep = 5;
		for(int i = 1; ; i++){
			int parmIndex = posStart + (i -1)* nStep + 1;
			int flagIndex = posStart + i * nStep;
			if(flagIndex >= bufLen){
				return false;
			}
			// find out the correct parameter
			if( (((int)inqbuffer[parmIndex]) << 8) + (int)(inqbuffer[parmIndex + 1]) == n){
				if( (inqbuffer[flagIndex] & 0xFF) == 0 ){
					return false;
				}else{
					return true;
				}
			}
		}
		return false;
	}

#define TAF1(n) 	true == TAF(n, inqbuffer, sizeof(inqbuffer))
#define TAF0(n) 	false == TAF(n, inqbuffer, sizeof(inqbuffer))
	bool GetLoadedTapeAlertFlag(const string& driveDevName, const string& driveStDevName, TapeStatus &tapeStatus, bool& bFaulty, LtfsError& error, bool bForce)
	{
		scsi_cmd_io sio;
		unsigned char        inqbuffer [240];
		int                  status;
		memset(inqbuffer, 0, sizeof(inqbuffer));

		// check if drive is mounted, if yes, not continue
		if(IsDriveMounted(driveStDevName) && (false == bForce)){
			LtfsLogInfo("GetLoadedTapeAlertFlag: Drive " << driveStDevName << " has mounted, operation is not permitted.");
			error.SetErrCode(ERR_DRIVE_MOUNTED);
			error.AddStringParam(driveStDevName);
			return false;
		}

		sio.fd = OpenDevice(driveDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open device " << driveDevName << " to get TAF.");
			return false;
		}
		//LTFSLE: #define SIOC_LOG_SENSE_PAGE           _IOR ('C',0x23,struct log_sense_page)
		//Set up the cdb for Log sense
		sio.cdb[0] = CMDLogSense/*0x4D*/;
		sio.cdb[1] = 0;
		sio.cdb[2] = (unsigned char) (0x40 | (0x2E & 0x3F)); //Tape Alert Flag. set PC=01b for current values
		sio.cdb[3] = 0;
		sio.cdb[4] = 0;
		sio.cdb[5] = 0;
		sio.cdb[6] = 0;
		sio.cdb[7] = sizeof(inqbuffer) >> 8;
		sio.cdb[8] = sizeof(inqbuffer) & 0xFF;
		sio.cdb[9] = 0;

		sio.cdb_length = 10;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;

		status = ExecScsiCommand (&sio, 1);
		CloseDevice(sio.fd);
		if(sio.senseCode.senseCode != 0){
			error.SetSenseCode(sio.senseCode);
		}

		if (status == 0) {
			//int posStart = 3;
			//int nStep = 5;
			/*
			Hex Decimal 	Description 	Set 	Clear 	Type
			01h - 02h 1 - 2 N/A
			03h 3 The operation has stopped because an error has occurred while reading or
				  writing data which the drive cannot correct. R Warning
			04h 4 Media can no longer be written or read, or performance is severely degraded.	R Critical
			05h 5 The tape is damaged or the drive is faulty. R Critical
			06h 6 The drive can no longer write data to the tape. R Critical
			07h 7 Media Life E L Warning
			08h 8 The cartridge is not data-grade. R Warning
			09h 9 The WRITE command was attempted to a write-protected tape. R Critical
			0Ah 10 A manual or software unload was attempted when Prevent Media Removal was on. R Informational
			0Bh 11 The tape in the drive is a cleaning cartridge. R Informational
			0Ch 12 You attempted to load a cartridge with an unsupported tape format (for example,
				   Ultrium 2 cartridge in Ultrium 1 drive). R Informational
			0Dh - 0Eh 13 - 14 N/A
			0Fh 15 The memory chip failed in the cartridge. R Critical
			10h 16 The operation has failed because the tape cartridge was manually ejected while the tape drive
				   was actively writing or reading. L Warning
			11h 17 Media loaded is Read-Only format. E R Informational
			12h 18 The tape directory on the tape cartridge has been corrupted. File search performance will be
				   degraded. The tape directory can be rebuilt by reading all the data on the cartridge. R Warning
			13h 19 Nearing media life R Informational
			14h 20 Clean now C Critical
			15h 21 Clean periodic C Warning
			16h 22 Expired cleaning media C L Critical
			17h 23 Invalid cleaning tape C R Critical
			18h - 1Dh 24 - 29 N/A
			1Eh 30 The drive has a hardware fault that requires a reset to recover. Critical
			1Fh 31 The drive has a hardware fault that is not related to the read/write operation, or the
				   drive requires a power cycle to recover. Critical
			20h 32 The drive has identified an interface fault. Warning
			21h 33 Eject media U, R Critical
			22h 34 Firmware download failed Warning
			23h 35 N/A
			24h 36 The drive's temperature limits are exceeded. S Warning
			25h 37 The drive's voltage limits are exceeded. S Warning
			26h 38 Predictive failure of drive hardware Critical
			27h 39 Diagnostics required Warning
			28h - 32h 40 - 50 N/A
			33h 51 Tape directory invalid at unload E L Warning
			34h 52 Tape system area write failure E R Critical
			35h 53 Tape system area read failure E R Critical
			36h 54 N/A
			37h 55 Loading failure E R Critical
			38h 56 Unrecoverable unload failure E U Critical
			39h-3Ah 57-58 N/A
			3Bh 59 (WORM Medium - integrity check failed) set when the drive determines that the
				   data on tape is suspect from a WORM point of view. L Critical
			3Ch 60 (WORM Medium - Overwrite Attempted) set when the drive rejects a Write operation because
				   the rules for allowing WORM writes have not been met.) E Critical
			3Dh-40h 61 - 64 N/A
			*/
			tapeStatus = TAPE_UNKNOWN;
			bFaulty = false;
			if(TAF1(4)	//4 Media can no longer be written or read, or performance is severely degraded
			|| TAF1(5)	//5 The tape is damaged or the drive is faulty.
			|| TAF1(6)	//6 The drive can no longer write data to the tape
			|| TAF1(18)	//The tape directory on the tape cartridge has been corrupted. File search performance will be
			   			//degraded. The tape directory can be rebuilt by reading all the data on the cartridge
			){
				bFaulty = true;
			}

			if(TAF1(22)){		//22 Expired Cleaning Media
				tapeStatus = TAPE_CLEAN_EXPIRED;
			}

			/*
			if(TAF1(9)){		//9 Write Protect Critical
				wpFlag = WRITE_PROTECT_TRUE;
			}else{
				wpFlag = WRITE_PROTECT_FALSE;
			}
			*/

			LtfsLogDebug("GetLoadedTapeAlertFlag: tapeStatus = " << tapeStatus << endl);
		}else
		{
			LtfsLogError("GetLoadedTapeAlertFlag for tape in drive " << driveDevName << " failed.");
			return false;
		}

		return true;
	}

	bool WriteLoadeTapeAttribute(const string& driveDevName, const string& driveStDevName, const unsigned char *buf, unsigned long long part, const size_t size, LtfsError& error, bool bForce = false)
	{
		scsi_cmd_io sio;
		int length = size + 4;
		unsigned char* pRawData = (unsigned char*) calloc(1, length);
		if (pRawData == NULL) {
			return false;
		}

		*pRawData     = (unsigned char)(size >> 24);
		*(pRawData+1) = (unsigned char)(size >> 16);
		*(pRawData+2) = (unsigned char)(size >>  8);
		*(pRawData+3) = (unsigned char)(size & 0xFF);
		memcpy (pRawData+4, buf, size);

		// check if drive is mounted, if yes, not continue
		if(IsDriveMounted(driveStDevName) && (false == bForce)){
			LtfsLogInfo("WriteLoadeTapeAttribute: Drive " << driveStDevName << " has mounted, operation is not permitted. part = " << part << ".");
			error.SetErrCode(ERR_DRIVE_MOUNTED);
			error.AddStringParam(driveStDevName);
			free(pRawData);
			return false;
		}

		sio.fd = OpenDevice(driveDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open device " << driveDevName << " to writte attribute.");
			free(pRawData);
			return false;
		}
		//LTFSLE: #define SCSI_PASS_THROUGH _IOWR('P',0x01,SCSIPassThrough) /* Pass Through  */
		//LTFSLE: #define SIOC_PASS_THROUGH     _IOWR('C', 0x34, struct sioc_pass_through)
		//Set up the cdb write attribute
		sio.cdb[0]  = CMDWriteAttribute/*0x8D*/;
		sio.cdb[1]  = 0; //Service Action 0x00
		sio.cdb[2]  = 0;
		sio.cdb[3]  = 0;
		sio.cdb[4]  = 0;
		sio.cdb[5]  = 0;
		sio.cdb[6]  = 0;
		sio.cdb[7]  = (unsigned char) part;
		sio.cdb[8]  = 0;
		sio.cdb[9]  = 0;
		sio.cdb[10] = (unsigned char) ((length & 0xFF000000)  >> 24);
		sio.cdb[11] = (unsigned char) ((length & 0xFF0000)    >> 16);
		sio.cdb[12] = (unsigned char) ((length & 0xFF00)      >> 8 );
		sio.cdb[13] = (unsigned char) ((length & 0xFF)             );
		sio.cdb[14] = 0;
		sio.cdb[15] = 0;

		sio.cdb_length = 16;

		//Set up the data part:
		sio.data = pRawData;
		sio.data_length = length;
		sio.data_direction = HOST_WRITE;

		sio.timeout_ms = LTO_WRITEATTRIB_TIMEOUT;

		int status = ExecScsiCommand (&sio, 1);
		CloseDevice(sio.fd);
		if(sio.senseCode.senseCode != 0){
			error.SetSenseCode(sio.senseCode);
		}

		if(status == 0){
		}else{
			LtfsLogError("WriteLoadeTapeAttribute failed.");
			free(pRawData);
			return false;
		}
		free(pRawData);

		return true;
	}


	bool ReadLoadeTapeAttribute(const string& driveDevName, const string& driveStDevName, const UInt16_t id,
			unsigned char *buf, unsigned long long part, const size_t size, LtfsError& error, bool bForce = false)
	{
		scsi_cmd_io sio;
		int length = size + 4;
		unsigned char* pRawData = (unsigned char*) calloc(1, length);
		if (pRawData == NULL) {
			LtfsLogError("Failed to alloc data to execute scsi command.");
			return false;
		}

		// check if drive is mounted, if yes, not continue
		if(IsDriveMounted(driveStDevName) && false == bForce){
			LtfsLogInfo("ReadLoadeTapeAttribute: Drive " << driveStDevName << " has mounted, operation is not permitted. id = " << id << ".");
			error.SetErrCode(ERR_DRIVE_MOUNTED);
			error.AddStringParam(driveStDevName);
			free(pRawData);
			return false;
		}

		sio.fd = OpenDevice(driveDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open device " << driveDevName << " to read attribute.");
			free(pRawData);
			return false;
		}
		//LTFSLE: #define SCSI_PASS_THROUGH _IOWR('P',0x01,SCSIPassThrough) /* Pass Through  */
		//LTFSLE: #define SIOC_PASS_THROUGH     _IOWR('C', 0x34, struct sioc_pass_through)
		//Set up the cdb read attribute
		sio.cdb[0]  = CMDReadAttribute/*0x8C*/;
		sio.cdb[1]  = 0; //Service Action 0x00 = Return Value
		sio.cdb[2]  = 0;
		sio.cdb[3]  = 0;
		sio.cdb[4]  = 0;
		sio.cdb[5]  = 0;
		sio.cdb[6]  = 0;
		sio.cdb[7]  = (unsigned char) 0;
		sio.cdb[8]  = (unsigned char) ((id >> 8) & 0xFF);
		sio.cdb[9]  = (unsigned char) (id & 0xFF);
		sio.cdb[10] = (unsigned char) ((length & 0xFF000000)  >> 24);
		sio.cdb[11] = (unsigned char) ((length & 0xFF0000)    >> 16);
		sio.cdb[12] = (unsigned char) ((length & 0xFF00)      >> 8 );
		sio.cdb[13] = (unsigned char) ((length & 0xFF)             );
		sio.cdb[14] = 0;
		sio.cdb[15] = 0;

		sio.cdb_length = 16;

		//Set up the data part:
		sio.data = pRawData;
		sio.data_length = length;
		sio.data_direction = HOST_READ;

		sio.timeout_ms = LTO_READATTRIB_TIMEOUT;

		int status = ExecScsiCommand (&sio, 1);
		CloseDevice(sio.fd);
		if(sio.senseCode.senseCode != 0){
			error.SetSenseCode(sio.senseCode);
		}

		if(status == 0){
			memcpy (buf, (pRawData + 4), size);
		}else{
			LtfsLogError("ReadLoadeTapeAttribute failed.");
			free(pRawData);
			return false;
		}
		free(pRawData);

		return true;
	}


	bool GetLoadedTapeLtfsFormat(const string& driveDevName, const string& driveStDevName, LTFS_FORMAT& ltfsFormat, LtfsError& error)
	{
		bool ret;
		unsigned char        inqbuffer [240];

		string strLtfs = "";
		ltfsFormat = LTFS_UNKNOWN;
		ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_LTFS_FORMAT/*0x080C*/, inqbuffer, 0, sizeof(inqbuffer), error);
		if(ret == true){
			int vcrLen = (int)(inqbuffer[5]);
			strLtfs = GetStringFromBuf(inqbuffer + vcrLen + 24, 4);
		}else{
			LtfsLogError("GetLoadedTapeLtfsFormat for tape in drive " << driveDevName << " failed.");
			if(error.GetErrCode() == ERR_DRIVE_MOUNTED){
				return false;
			}
		}
		if(strLtfs == "LTFS"){
			ltfsFormat = LTFS_VALID;
		}else{
			ltfsFormat = LTFS_UNKNOWN;
		}
		LtfsLogDebug("GetLoadedTapeLtfsFormat for tape in drive " << driveDevName << " finished. tapeLtfs: " << ltfsFormat << ".");

		return true;
	}

	bool SetLoadedTapeLtfsFormat(const string& driveDevName, const string& driveStDevName, bool bLtfs, LtfsError& error)
	{
		unsigned char buf[1024];
	    int           len;
	    int 		  attrLen = 128;
		unsigned char        inqbuffer [240];


		string strLtfs = "";
		bool ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_LTFS_FORMAT/*0x080C*/, inqbuffer, 0, sizeof(inqbuffer), error);
		if(ret == false){
			return false;
		}

	    string strVal = "XXXX";
	    if(bLtfs){
	    	strVal = "LTFS";
	    }
		int vcrLen = (int)(inqbuffer[5]);
		memcpy(inqbuffer + vcrLen + 24, strVal.c_str(), strVal.length());  //TODO: check if this is correct
		attrLen = (int)(inqbuffer[3] << 8) + (int)(inqbuffer[4]);


	    memset (buf, (int)0x20, sizeof(buf));
	    buf[0] = (unsigned char)(MAM_TAPE_LTFS_FORMAT/*0x080C*/ >> 8);
	    buf[1] = (unsigned char)(MAM_TAPE_LTFS_FORMAT/*0x080C*/ & 0xFF);
	    buf[2] = 0x00;   // binary format
	    buf[3] = 0;
	    buf[4] = attrLen;


	    memcpy (buf+5, inqbuffer + 5, sizeof(inqbuffer) - 5);
	    len = attrLen + 5;

		ret = WriteLoadeTapeAttribute(driveDevName, driveStDevName, buf, 0, len, error);
		if(false == ret){
			LtfsLogError("SetLoadedTapeLtfsFormat for tape in drive " << driveDevName << " failed.");
			if(error.GetErrCode() == ERR_NO_ERR){
				//error.AddStringParam(driveDevName);
				//error.SetErrCode(ERR_DRIVE_SET_LTFS_FORMAT_FAIL);
			}
		}
		LtfsLogDebug("SetLoadedTapeLtfsFormat for tape in drive " << driveDevName << " finished. bLtfs = " << bLtfs << ".");
		return ret;
	}

	bool GetLoadedTapeLoadCounter(const string& driveDevName, const string& driveStDevName, UInt16_t& loadCounter, LtfsError& error)
	{
		bool ret = false;
		unsigned char        inqbuffer [240];

		ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_LOAD_COUNTER/*0x0003*/, inqbuffer, 0, sizeof(inqbuffer), error);
		if(ret == false){
			if(error.GetErrCode() == ERR_NO_ERR){
				error.SetErrCode(ERR_DRIVE_GET_LOAD_COUNTER_FAIL);
			}
			LtfsLogError("GetLoadedTapeLoadCounter for tape in drive " << driveDevName << " failed.");
			return ret;
		}

		int attrLen = (int)(inqbuffer[3] << 8) + (int)(inqbuffer[4]);
		loadCounter = 0;
		for(int i = 0; i < attrLen; i++){
			loadCounter += (UInt16_t)(inqbuffer[4 + attrLen - i] << i*8);
		}
		LtfsLogDebug("GetLoadedTapeLoadCounter for tape in drive " << driveDevName << " finished. LoadCounter: " << loadCounter << ".");
		return true;
	}

	bool GetLoadedTapeMediaType(const string& driveDevName, const string& driveStDevName, TapeMediaType& mediaType, LtfsError& error)
	{
		bool ret = false;
		scsi_cmd_io sio;
		unsigned char        inqbuffer [240];
		int                  status;
		memset(inqbuffer, 0, sizeof(inqbuffer));

		// check if drive is mounted, if yes, not continue
		if(true == IsDriveMounted(driveStDevName)){
			LtfsLogInfo("GetLoadedTapeMediaType: Drive " << driveStDevName << " has mounted a tape, operation is not permitted.");
			if(error.GetErrCode() == ERR_NO_ERR){
				error.SetErrCode(ERR_DRIVE_MOUNTED);
				error.AddStringParam(driveStDevName);
			}
			return false;
		}

		sio.fd = OpenDevice(driveDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open device " << driveDevName << " to get media type.");
			return false;
		}
		//Set up the cdb report density support
		sio.cdb[0] = CMDReportSensity/*0x44*/;
		sio.cdb[1] = 1;
		sio.cdb[2] = 0;
		sio.cdb[3] = 0;
		sio.cdb[4] = 0;
		sio.cdb[5] = 0;
		sio.cdb[6] = 0;
		sio.cdb[7] = sizeof(inqbuffer) >> 8;
		sio.cdb[8] = sizeof(inqbuffer) & 0xFF;
		sio.cdb[9] = 0;

		sio.cdb_length = 10;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;

		status = ExecScsiCommand (&sio, 1);
		CloseDevice(sio.fd);
		if(sio.senseCode.senseCode != 0){
			error.SetSenseCode(sio.senseCode);
		}

		if(status == 0){
			mediaType = MEDIA_UNKNOWN;
			string org = GetStringFromBuf(inqbuffer + 20, 8);
			string description = GetStringFromBuf(inqbuffer + 36, 20, true);

			int ltoGen = 0;
			// LTO-CVE
			if(org.substr(0, 3) == "LTO"){
				// Ultrium 5/16T
				regex matchDes("^.*(\\d+)\\/\\d+.*");
				cmatch match;
				if(regex_match(description.c_str(), match, matchDes)){
					ltoGen = boost::lexical_cast<int>(match[1]);
				}
			}
			switch(ltoGen){
			case 1:
				mediaType = MEDIA_LTO1;
				break;
			case 2:
				mediaType = MEDIA_LTO2;
				break;
			case 3:
				mediaType = MEDIA_LTO3;
				break;
			case 4:
				mediaType = MEDIA_LTO4;
				break;
			case 5:
				mediaType = MEDIA_LTO5;
				break;
			case 6:
				mediaType = MEDIA_LTO6;
				break;
			default:
				mediaType = MEDIA_OTHERS;
				break;
			}
			ret = true;
		}// if(status == 0){
		else{
			error.SetErrCode(ERR_DRIVE_GET_MEDIA_TYPE_FAIL);
			LtfsLogError("GetLoadedTapeMediaType for tape in drive " << driveDevName << " failed.");
			ret = false;
		}

		LtfsLogDebug("GetLoadedTapeMediaType for tape in drive " << driveDevName << " finished. mediaType: " << mediaType << ".");
		return ret;
	}


	bool GetLoadedTapeMediumType(const string& driveDevName, const string& driveStDevName, TapeMediumType& mediumType, LtfsError& error)
	{
		bool ret = false;
		unsigned char        inqbuffer [240];

		mediumType = MEDIUM_UNKNOWN;
		ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_MEIDUM_TYPE/*0x0408*/, inqbuffer, 0, sizeof(inqbuffer), error);
		if(ret == false){
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_GET_MEDIUM_TYPE_FAIL);
			}
			LtfsLogError("GetLoadedTapeMediumType for tape in drive " << driveDevName << " failed.");
			return ret;
		}

		int iType = (int)(inqbuffer[5]);
		switch(iType){
			case 0:
				mediumType = MEDIUM_DATA;
				break;
			case 1:
				mediumType = MEDIUM_CLEANING;
				break;
			case 128://0x80H
				mediumType = MEDIUM_WORM;
				break;
			default:
				break;
		}

		LtfsLogDebug("GetLoadedTapeMediumType for tape in drive " << driveDevName << " finished. mediumType: " << mediumType << ".");
		return true;
	}

	bool GetLoadedTapeWPFlag(const string& driveDevName, const string& driveStDevName, bool& bIsWP, LtfsError& error)
	{
		bool ret = false;
		scsi_cmd_io sio;
		unsigned char        inqbuffer [240];
		int                  status;
		memset(inqbuffer, 0, sizeof(inqbuffer));

		// check if drive is mounted, if yes, not continue
		if(true == IsDriveMounted(driveStDevName)){
			LtfsLogInfo("GetLoadedTapeWPFlag: Drive " << driveStDevName << " has mounted a tape, operation is not permitted.");
			if(error.GetErrCode() == ERR_NO_ERR){
				error.SetErrCode(ERR_DRIVE_MOUNTED);
				error.AddStringParam(driveStDevName);
			}
			return false;
		}

		sio.fd = OpenDevice(driveDevName);
		if(sio.fd < 0){
			LtfsLogError("GetLoadedTapeWPFlag: Failed to open device " << driveDevName << " to get WP flag of tape.");
			return false;
		}
		//LTFSLE: #define SIOC_MODE_SENSE               _IOR ('C', 0x0D, struct mode_sense)
		//LTFSLE: #define SIOC_MODE_SENSE_PAGE          _IOR ('C',0x0E,struct mode_sense_page)
		//Set up the cdb for mode sense
		sio.cdb[0] = CMDModeSense;//0x1A
		sio.cdb[1] = 0;
		sio.cdb[2] = 0;
		sio.cdb[3] = 0;
		sio.cdb[4] = sizeof(inqbuffer);
		sio.cdb[5] = 0;

		sio.cdb_length = 6;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;

		status = ExecScsiCommand (&sio, 1);
		CloseDevice(sio.fd);
		if(sio.senseCode.senseCode != 0){
			error.SetSenseCode(sio.senseCode);
		}

		if(status == 0){
			bIsWP = false;
			int wpFlag = (int)(inqbuffer[2]) & 0x80;
			if(wpFlag >= 1){
				bIsWP = true;
			}
			ret = true;
		}// if(status == 0){
		else{
			error.SetErrCode(ERR_DRIVE_GET_WP_FLAG_FAIL);
			LtfsLogError("GetLoadedTapeWPFlag for tape in drive " << driveDevName << " failed.");
			ret = false;
		}

		LtfsLogDebug("GetLoadedTapeWPFlag for tape in drive " << driveDevName << " finished. bIsWP: " << bIsWP << ".");
		return ret;
	}


	bool GetLoadedTapeGenerationIndex(const string& driveDevName, const string& driveStDevName, UInt64_t& genIndex, LtfsError& error)
	{
		/*
		if(IsEmpty()){
			error.SetErrCode(ERR_DRIVE_EMPTY);
			return false;
		}*/

		bool ret = false;
		unsigned char        inqbuffer [1024];

		ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_GENERATION_INDEX/*0x080C*/, inqbuffer, 0, sizeof(inqbuffer), error);
		if(ret == false){
			if(error.GetErrCode() == ERR_NO_ERR){
				error.SetErrCode(ERR_DRIVE_GET_TAPE_GENERATION_INDEX_FAIL);
			}
			LtfsLogError("Failed to get generation index of loaded tape in drive " << driveDevName);
			return ret;
		}

		int vcrLen = (int)(inqbuffer[5]);
		genIndex = 0;
		for(int i = 0; i < 8; i++){
			genIndex += (UInt16_t)(inqbuffer[5 + vcrLen + 8 - i] << i*8);
		}

		LtfsLogDebug("Generation index for loaded tape in drive " << driveDevName << " is " << genIndex << ".");
		return true;
	}

	bool GetLoadedTapeCapacity(const string& driveDevName, const string& driveStDevName, const string& barcode, Int64_t& freeCapacity, LtfsError& error, bool bIsLtfs)
	{
		Int64_t totalCapacity = -1;
		return GetLoadedTapeCapacity(driveDevName, driveStDevName, barcode, freeCapacity, totalCapacity, error, bIsLtfs);
	}

	bool GetLoadedTapeCapacity(const string& driveDevName, const string& driveStDevName, const string& barcode, Int64_t& freeCapacity, Int64_t& totalCapacity, LtfsError& error, bool bIsLtfs)
	{
		scsi_cmd_io sio;
		unsigned char        inqbuffer [240];
		int                  status;
		memset(inqbuffer, 0, sizeof(inqbuffer));
		bool bRet = false;

		// if the drive is mounted, use 'df ' command to get capacity/size
		if(true == IsDriveMounted(driveStDevName, barcode)){
			try{
				string cmd = "";
				fs::path pathLTFS = GetLtfsFolder() / barcode;
				if(false == IsMountPointMounted(pathLTFS.string())){
					LtfsLogError("Failed to get capacity/freeSize of the mounted tape: " << barcode);
					error.SetErrCode(ERR_DRIVE_GET_CAPACITY_FAIL);
					return false;
				}
				cmd = "df " + pathLTFS.string();
				vector<string> rets = GetCommandOutputLines(cmd, CMD_DF_TIMEOUT);
				for(unsigned int i = 0; i < rets.size(); i++){
					//Filesystem      1K-blocks     Used  Available Use% Mounted on
					//ltfs:/dev/IBMtape1 1391601152 1391601152         0 100% /opt/VS/vsMounts/401000L5
					regex matchDf("^.*ltfs.*\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\%\\s+.*\\/(\\S+).*");
					cmatch match;
					if(regex_match(rets[i].c_str(), match, matchDf)){
						LtfsLogDebug("match[1] = " << match[1] << ", match[2] = " << match[2] << ", match[3] = " << match[3] << ",match[4] = " << match[4] << ", match[5] = " << match[5]);
						if(match[5] != barcode){
							continue;
						}
						totalCapacity = (boost::lexical_cast<Int64_t>(match[1])) * 1024;
						Int64_t usedCapacity = (boost::lexical_cast<Int64_t>(match[2])) * 1024;
						freeCapacity = (boost::lexical_cast<Int64_t>(match[3])) * 1024;
						if(totalCapacity < 107374182400 || freeCapacity < 0
							|| freeCapacity > 3298534883328 || totalCapacity > 3298534883328  // 3T
							|| usedCapacity + freeCapacity < 107374182400  // 100G
						){ // 100G
							LtfsLogError("GetLoadedTapeCapacity by df, size not correct: barcode = " << barcode <<", freeCapacity = " << freeCapacity << ", usedCapacity = " << usedCapacity << ", totalCapacity = " << totalCapacity << ".");
							error.SetErrCode(ERR_DRIVE_GET_CAPACITY_FAIL);
							return false;
						}
						LtfsLogInfo("freeCapacity = " << freeCapacity << ", totalCapacity = " << totalCapacity << ", usedCapacity = " << usedCapacity << ".");
						return true;
					}
				}
			}catch(...){
				LtfsLogError("Failed to get capacity/freeSize of the mounted tape: " << barcode);
				error.SetErrCode(ERR_DRIVE_GET_CAPACITY_FAIL);
				return false;
			}
			return false;
		}

		// tape/drive not mounted, use scsi command to get tape capacity
		sio.fd = OpenDevice(driveDevName);
		if(sio.fd < 0){
			LtfsLogError("Failed to open device " << driveDevName << " to get tape free size.");
			error.SetErrCode(ERR_DRIVE_GET_CAPACITY_FAIL);
			return false;
		}
		//LTFSLE: #define SIOC_LOG_SENSE_PAGE           _IOR ('C',0x23,struct log_sense_page)
		//Set up the cdb for Log sense
		sio.cdb[0] = CMDLogSense/*0x4D*/;
		sio.cdb[1] = 0;
		sio.cdb[2] = (unsigned char) (0x40 | (0x31 & 0x3F)); //Tape Capacity. set PC=01b for current values
		sio.cdb[3] = 0;
		sio.cdb[4] = 0;
		sio.cdb[5] = 0;
		sio.cdb[6] = 0;
		sio.cdb[7] = sizeof(inqbuffer) >> 8;
		sio.cdb[8] = sizeof(inqbuffer) & 0xFF;
		sio.cdb[9] = 0;

		sio.cdb_length = 10;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_READ;

		sio.timeout_ms = DAT_INQUIRY_TIMEOUT;

		status = ExecScsiCommand (&sio, 1);
		CloseDevice(sio.fd);
		if(sio.senseCode.senseCode != 0){
			error.SetSenseCode(sio.senseCode);
		}

		if (status == 0 && sio.senseCode.senseCode == 0) {
			UInt32_t freeSize = ((UInt32_t)inqbuffer[8] << 24)
				+ ((UInt32_t)inqbuffer[9] << 16) + ((UInt32_t)inqbuffer[10] << 8) + (UInt32_t)(inqbuffer[11]);
			UInt32_t totalSize = ((UInt32_t)inqbuffer[24] << 24)
				+ ((UInt32_t)inqbuffer[25] << 16) + ((UInt32_t)inqbuffer[26] << 8) + (UInt32_t)(inqbuffer[27]);
			UInt32_t freeSize2 = ((UInt32_t)inqbuffer[16] << 24)
				+ ((UInt32_t)inqbuffer[17] << 16) + ((UInt32_t)inqbuffer[18] << 8) + (UInt32_t)(inqbuffer[19]);
			UInt32_t totalSize2 = ((UInt32_t)inqbuffer[32] << 24)
				+ ((UInt32_t)inqbuffer[33] << 16) + ((UInt32_t)inqbuffer[34] << 8) + (UInt32_t)(inqbuffer[35]);
			LtfsLogDebug(" freeSize = " << freeSize << ",  totalSize = " << totalSize << endl);
			LtfsLogDebug("freeSize2 = " << freeSize << ", totalSize2 = " << totalSize << endl);

			// if the total size is not correct, get it from alternative partition
			if(totalSize < 100 * 1024){ // 100G
				if(totalSize2 > totalSize){
					totalSize = totalSize2;
					freeSize = freeSize2;
				}
			}

			LtfsLogDebug("GetLoadedTapeCapacity: barcode = " << barcode << ", freeSize = " << freeSize << ", totalSize = " << totalSize << ".");
			if(freeSize < 0 || totalSize < 100 * 1024 // 100G
				|| freeSize > 3145728 || totalSize > 3145728  // 3T
			){
				LtfsLogError("GetLoadedTapeCapacity by SCSI, size not correct: barcode = " << barcode <<", freeSize = " << freeSize << "M, totalSize = " << totalSize << "M.");
				bRet = false;
			}else{
				bRet = true;
				freeCapacity = (Int64_t)freeSize * 1024*1024;
				totalCapacity = (Int64_t)totalSize * 1024*1024;
				LtfsLogInfo("freeSize = " << freeSize << "M, freeCapacity = " << freeCapacity << ", totalSize = " << totalSize << "M, totalCapacity = " << totalCapacity << endl);
			}

		}
		if(false == bRet){
			error.SetErrCode(ERR_DRIVE_GET_CAPACITY_FAIL);
			LtfsLogError("Failed to get capacity info for tape " << barcode << ".");
			return false;
		}
		return true;
	}

	bool GetLoadedTapeStatus(const string& driveDevName, const string& driveStDevName, TapeStatus& tapeStatus, LtfsError& error)
	{
		bool bRet = false;
		unsigned char        inqbuffer [240];
		tapeStatus = TAPE_UNKNOWN;

		LtfsLogDebug("GetLoadedTapeStatus: starting...");
		bool ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_STATUS_FLAG/*0x1407*/, inqbuffer, 0, sizeof(inqbuffer), error);
		bRet = ret;
		if(ret == true){
			tapeStatus = (TapeStatus)(int)(inqbuffer[5]);
			LtfsLogDebug("GetLoadedTapeStatus: (int)(inqbuffer[5]) = " << (int)(inqbuffer[5]) << ", tapeStatus = " << tapeStatus << endl);
			if(tapeStatus == 0){
				//tapeStatus = TAPE_OPEN;
			}
		}
		if(error.GetErrCode() == ERR_DRIVE_MOUNTED){
			LtfsLogError("GetLoadedTapeStatus for tape in drive " << driveDevName << " failed.");
			return false;
		}

		TapeStatus tapeTAFStatus = TAPE_UNKNOWN;
		bool bFaulty = false;
		LtfsError err;
		bool bTAF = GetLoadedTapeAlertFlag(driveDevName, driveStDevName, tapeTAFStatus, bFaulty, err);
		if(err.GetErrCode() == ERR_DRIVE_MOUNTED){
			LtfsLogError("GetLoadedTapeStatus for tape in drive " << driveDevName << " failed.");
			return false;
		}
		if(true == bTAF){
			if(TAPE_CLEAN_EXPIRED == tapeTAFStatus){
				tapeStatus = tapeTAFStatus;
				bRet = true;
			}
		}
		if(false == bRet){
			error.ClearParams();
			error.AddStringParam(driveDevName);
			error.SetErrCode(ERR_DRIVE_GET_TAPE_STATUS_FAIL);
			LtfsLogError("GetLoadedTapeStatus for tape in drive " << driveDevName << " failed.");
			return ret;
		}
		LtfsLogDebug("GetLoadedTapeStatus for tape in drive " << driveDevName << " finished. tapeStatus: " << tapeStatus << ".");
		return bRet;
	}

	bool SetLoadedTapeStatus(const string& driveDevName, const string& driveStDevName, TapeStatus tapeStatus, LtfsError& error)
	{
		unsigned char buf[1024];
	    int 		  attrLen = 1;

	    memset (buf, (int)0x20, sizeof(buf));
	    buf[0] = (unsigned char)(MAM_TAPE_STATUS_FLAG/*0x1407*/ >> 8);
	    buf[1] = (unsigned char)(MAM_TAPE_STATUS_FLAG/*0x1407*/ & 0xFF);
	    buf[2] = 0x00;
	    buf[3] = 0;
	    buf[4] = attrLen;

	    buf[5] = tapeStatus;
		bool ret = WriteLoadeTapeAttribute(driveDevName, driveStDevName, buf, 0, attrLen + 5, error);
		if(false == ret){
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_SET_TAPE_STATUS_FAIL);
			}
			LtfsLogError("SetLoadedTapeStatus for tape in drive " << driveDevName << " failed.");
		}
		LtfsLogDebug("SetLoadedTapeStatus for tape in drive " << driveDevName << " finished. tapeStatus: " << tapeStatus << ".");
		return ret;
	}

	bool GetLoadedTapeFaulty(const string& driveDevName, const string& driveStDevName, bool& bFaulty, LtfsError& error)
	{
		bool bRet = false;
		bool ret = false;
		unsigned char        inqbuffer [1024];

		bFaulty = false;

		ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_FALUTY_FLAG/*0x1406*/, inqbuffer, 0, sizeof(inqbuffer), error);
		bRet = ret;
		if(ret == true){
			bFaulty = ((UInt16_t)(inqbuffer[5]) == 1)? true : false;
		}

		TapeStatus tapeTAFStatus = TAPE_UNKNOWN;
		bool bTAFFaulty = false;
		LtfsError err;
		bool bTAF = GetLoadedTapeAlertFlag(driveDevName, driveStDevName, tapeTAFStatus, bTAFFaulty, err);
		bRet |= bTAF;
		if(true == bTAF){
			if(true == bTAFFaulty){
				bFaulty = true;
			}
		}
		if(false == bRet){
			error.ClearParams();
			error.AddStringParam(driveDevName);
			error.SetErrCode(ERR_DRIVE_GET_FAULTY_FLAG_FAIL);
			LtfsLogError("GetLoadedTapeFaulty for tape in drive " << driveDevName << " failed.");
		}

		LtfsLogDebug("GetLoadedTapeFaulty for tape in drive " << driveDevName << " finished. bFaulty: " << bFaulty << ".");
		return bRet;
	}
	bool SetLoadedTapeFaulty(const string& driveDevName, const string& driveStDevName, bool bFaulty, LtfsError& error)
	{
		unsigned char buf[1024];
	    int 		  attrLen = 1;

	    memset (buf, (int)0x20, sizeof(buf));
	    buf[0] = (unsigned char)(MAM_TAPE_FALUTY_FLAG/*0x1406*/ >> 8);
	    buf[1] = (unsigned char)(MAM_TAPE_FALUTY_FLAG/*0x1406*/ & 0xFF);
	    buf[2] = 0x00;   // binary format
	    buf[3] = 0;
	    buf[4] = attrLen;

	    buf[5] = (bFaulty == true)? 1:0;
		bool ret = WriteLoadeTapeAttribute(driveDevName, driveStDevName, buf, 0, attrLen + 5, error);
		if(false == ret){
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_SET_FAULTY_FLAG_FAIL);
			}
			LtfsLogError("SetLoadedTapeFaulty for tape in drive " << driveDevName << " failed.");
		}
		LtfsLogDebug("SetLoadedTapeFaulty for tape in drive " << driveDevName << " finished. bFaulty: " << bFaulty << ".");
		return ret;
	}

	bool SetLoadedTapeGroup(const string& driveDevName, const string& driveStDevName, const string& uuid, LtfsError& error)
	{
		unsigned char buf[1024];
	    int           len;
	    int			  srclen;
	    int 		  attrLen = 128;

	    memset (buf, (int)0x20, sizeof(buf));
	    buf[0] = (unsigned char)(MAM_TAPE_GROUP_UUID/*0x1403*/ >> 8);
	    buf[1] = (unsigned char)(MAM_TAPE_GROUP_UUID/*0x1403*/ & 0xFF);
	    buf[2] = 0x01;   // ASCII format
	    buf[3] = 0;
	    buf[4] = attrLen;

	    srclen = uuid.length();
	    len = (srclen > attrLen) ? attrLen : srclen;
	    memcpy (buf+5, uuid.c_str(), len);
	    len = attrLen + 5;

		bool ret = WriteLoadeTapeAttribute(driveDevName, driveStDevName, buf, 0, len, error);
		if(false == ret){
			LtfsLogError("SetLoadedTapeGroup for tape in drive " << driveDevName << " failed.");
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_SET_TAPE_GROUP_FAIL);
			}
		}
		LtfsLogDebug("SetLoadedTapeGroup for tape in drive " << driveDevName << " finished. uuid: " << uuid << ".");
		return ret;
	}

	bool GetLoadedTapeGroup(const string& driveDevName, const string& driveStDevName, string& uuid, LtfsError& error)
	{
		bool ret = false;

		unsigned char        inqbuffer [1024];
		uuid = "";
		ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_GROUP_UUID/*0x1403*/, inqbuffer, 0, sizeof(inqbuffer), error);
		if(ret == false){
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_GET_TAPE_GROUP_FAIL);
			}
			LtfsLogError("GetLoadedTapeGroup for tape in drive " << driveDevName << " failed.");
			return ret;
		}

		int attrLen = (int)(inqbuffer[3] << 8) + (int)(inqbuffer[4]);
		uuid = GetStringFromBuf(inqbuffer + 5, attrLen);
		LtfsLogDebug("GetLoadedTapeGroup for tape in drive " << driveDevName << " finished. uuid: " << uuid << ".");
		return ret;
	}


	bool SetLoadedTapeUUID(const string& driveDevName, const string& driveStDevName, const string& uuid, LtfsError& error)
	{
		unsigned char buf[1024];
	    int len = SetTapeAttributeBuf(buf, sizeof(buf), uuid, MAM_TAPE_UUID/*0x1410*/);

		bool ret = WriteLoadeTapeAttribute(driveDevName, driveStDevName, buf, 0, len, error);
		if(false == ret){
			LtfsLogError("SetLoadedTapeUUID for tape in drive " << driveDevName << " failed.");
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_SET_TAPE_UUID_FAIL);
			}
		}
		LtfsLogDebug("SetLoadedTapeUUID for tape in drive " << driveDevName << " finished. uuid: " << uuid << ".");
		return ret;
	}
	bool GetLoadedTapeUUID(const string& driveDevName, const string& driveStDevName, string& uuid, LtfsError& error)
	{
		bool ret = false;
		unsigned char        inqbuffer [1024];
		uuid = "";
		ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_UUID/*0x1410*/, inqbuffer, 0, sizeof(inqbuffer), error);
		if(ret == false){
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_GET_TAPE_UUID_FAIL);
			}
			LtfsLogError("GetLoadedTapeUUID for tape in drive " << driveDevName << " failed.");
			return ret;
		}

		int attrLen = (int)(inqbuffer[3] << 8) + (int)(inqbuffer[4]);
		uuid = GetStringFromBuf(inqbuffer + 5, attrLen);
		LtfsLogDebug("GetLoadedTapeUUID for tape in drive " << driveDevName << " finished. uuid: " << uuid << ".");
		return ret;
	}

	bool SetLoadedTapeBarcode(const string& driveDevName, const string& driveStDevName, const string& barcode, LtfsError& error)
	{
		unsigned char buf[1024];
	    int len = SetTapeAttributeBuf(buf, sizeof(buf), barcode, MAM_TAPE_BARCODE/*0x1409*/);

		bool ret = WriteLoadeTapeAttribute(driveDevName, driveStDevName, buf, 0, len, error);
		if(false == ret){
			LtfsLogError("SetLoadedTapeBarcode for tape in drive " << driveDevName << " failed.");
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_SET_TAPE_BARCODE_FAIL);
			}
		}
		LtfsLogDebug("SetLoadedTapeBarcode for tape in drive " << driveDevName << " finished. barcode: " << barcode << ".");
		return ret;
	}
	bool GetLoadedTapeBarcode(const string& driveDevName, const string& driveStDevName, string& barcode, LtfsError& error, bool bForce)
	{
		bool ret = false;
		unsigned char        inqbuffer [1024];
		barcode = "";
		ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_BARCODE/*0x1409*/, inqbuffer, 0, sizeof(inqbuffer), error, bForce);
		if(ret == false){
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_GET_TAPE_BARCODE_FAIL);
			}
			LtfsLogError("GetLoadedTapeBarcode for tape in drive " << driveDevName << " failed.");
			return ret;
		}

		int attrLen = (int)(inqbuffer[3] << 8) + (int)(inqbuffer[4]);
		barcode = GetStringFromBuf(inqbuffer + 5, attrLen);
		LtfsLogDebug("GetLoadedTapeBarcode for tape in drive " << driveDevName << " finished. barcode: " << barcode << ".");
		return ret;
	}

	bool SetLoadedTapeDualCopy(const string& driveDevName, const string& driveStDevName, const string& dualCopy, LtfsError& error)
	{
		unsigned char buf[1024];
	    int           len;
	    int			  srclen;
	    int 		  attrLen = 128;

	    memset (buf, (int)0x20, sizeof(buf));
	    buf[0] = (unsigned char)(MAM_TAPE_DUAL_COPY/*0x1408*/ >> 8);//
	    buf[1] = (unsigned char)(MAM_TAPE_DUAL_COPY/*0x1408*/ & 0xFF);
	    buf[2] = 0x01;   // ASCII format
	    buf[3] = 0;
	    buf[4] = attrLen;

	    srclen = dualCopy.length();
	    len = (srclen > attrLen) ? attrLen : srclen;
	    memcpy (buf+5, dualCopy.c_str(), len);
	    len = attrLen + 5;

		bool ret = WriteLoadeTapeAttribute(driveDevName, driveStDevName, buf, 0, len, error);
		if(false == ret){
			LtfsLogError("SetLoadedTapeDualCopy for tape in drive " << driveDevName << " failed.");
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_SET_TAPE_DUALCOPY_FAIL);
			}
		}
		LtfsLogDebug("SetLoadedTapeDualCopy for tape in drive " << driveDevName << " finished. dualCopy: " << dualCopy << ".");
		return ret;
	}

	bool GetLoadedTapeDualCopy(const string& driveDevName, const string& driveStDevName, string& dualCopy, LtfsError& error)
	{
		bool ret = false;
		unsigned char        inqbuffer [1024];
		dualCopy = "";
		ret =  ReadLoadeTapeAttribute(driveDevName, driveStDevName, MAM_TAPE_DUAL_COPY/*0x1408*/, inqbuffer, 0, sizeof(inqbuffer), error);
		if(ret == false){
			if(error.GetErrCode() == ERR_NO_ERR){
				error.AddStringParam(driveDevName);
				error.SetErrCode(ERR_DRIVE_GET_TAPE_DUALCOPY_FAIL);
			}
			LtfsLogError("GetLoadedTapeDualCopy for tape in drive " << driveDevName << " failed.");
			return ret;
		}

		int attrLen = (int)(inqbuffer[3] << 8) + (int)(inqbuffer[4]);
		dualCopy = GetStringFromBuf(inqbuffer + 5, attrLen);
		LtfsLogDebug("GetLoadedTapeDualCopy for tape in drive " << driveDevName << " finished. dualCopy: " << dualCopy << ".");
		return ret;
	}

	bool OpenMailSlot(const string& changerDevName, int mailSlotId, LtfsError& error)
	{
		// if prevent media removal is set on this changer, we need to allow media removal first
		if(false == ChangerPreventMediaRemoval(changerDevName, false)){
			error.SetErrCode(ERR_OPEN_MAILSLOT_ALLOW_REMOVAL_FAIL);
			LtfsLogError("Failed to open mail slot for changer " << changerDevName << ". Failed to allow media removal for this changer.");
			return false;
		}

		int fd = OpenDevice(changerDevName);
		if(fd <= 0){
			error.SetErrCode(ERR_OPEN_MAILSLOT_FAIL);
			LtfsLogError("Failed to open mail slot. Changer " << changerDevName << " not found.");
			return false;
		}
		scsi_cmd_io sio;
		unsigned char        inqbuffer [240];
		int                  status = -1;
		int					 retry = 2;
		memset(inqbuffer, 0, sizeof(inqbuffer));

		//LTFSLE: #define SCSI_PASS_THROUGH _IOWR('P',0x01,SCSIPassThrough) /* Pass Through  */
		//LTFSLE: #define SIOC_PASS_THROUGH     _IOWR('C', 0x34, struct sioc_pass_through)
		sio.fd = fd;
		//Set up the cdb for open mail slot:
		sio.cdb[0] = CMDOpenCloseMailSlot/*0x1B*/;
		sio.cdb[1] = 0;
		sio.cdb[2] = (unsigned char)(mailSlotId >> 8 );
		sio.cdb[3] = (unsigned char)(mailSlotId);
		sio.cdb[4] = 0;
		sio.cdb[5] = 0;

		sio.cdb_length = 6;

		//Set up the data part:
		sio.data = inqbuffer;
		sio.data_length = sizeof(inqbuffer);
		sio.data_direction = HOST_WRITE;

		//Set the timeout then execute:
		sio.timeout_ms = LTO_OPEN_MAIL_SLOT_TIMEOUT;

		while(retry > 0 && status != 0){
			status = ExecScsiCommand (&sio);
			retry--;
		}
		CloseDevice(fd);

		if(sio.senseCode.senseCode != 0){
			error.SetSenseCode(sio.senseCode);
		}

		// check sense code
		// Illegal Request (05h)	53h 02h Library media removal prevented state set
		if(sio.senseCode.senseCode == 0x05 && sio.senseCode.ascCode == 0x53 && sio.senseCode.ascqCode == 0x02){
			error.SetErrCode(ERR_CHANGER_PREVENT_REMOVE);
			LtfsLogError("Library media removal prevented state set.");
			return false;
		}

		if (status != 0){
			error.SetErrCode(ERR_OPEN_MAILSLOT_FAIL);
			LtfsLogError("Failed to open mail slot.");  //TODO:EVENT
			// after the mail slot is open, set prevent media removal for the changer
			if(false == ChangerPreventMediaRemoval(changerDevName, true)){
				LtfsLogError("Failed to set prevent media removal for changer " << changerDevName << ".");
			}
			return false;
		}

		// after the mail slot is open, set prevent media removal for the changer
		if(false == ChangerPreventMediaRemoval(changerDevName, true)){
			LtfsLogError("Failed to set prevent media removal for changer " << changerDevName << ".");
		}

		return true;
	}

	bool GetChangerMailSlots(const string& changerDevName, vector<mailSlotElement>& mailSlots, LtfsError lfsErr)
	{
		ChangerInfo_  changerInfo;
		if(false == GetChangerElements(changerDevName, changerInfo, lfsErr, false)){
			LtfsLogError("Failed to get changer info for " << changerDevName << " to get mail slot info.");
			return false;
		}
		mailSlots.clear();
		mailSlots = changerInfo.mailSlots;
		for(unsigned int i = 0; i < mailSlots.size(); i++){
			DebugPrintMailSlot(mailSlots[i], "   ");
		}
		return true;
	}

	void DebugPrintDrive(const driveElement& drive, const string& indentStr)
	{
    	LtfsLogDebug(indentStr << "mSerial:      	  	 " << drive.mSerial);
    	LtfsLogDebug(indentStr << "scsiAddr:          	 " << drive.mScsiInfo.scsiAddr);
    	LtfsLogDebug(indentStr << "vendor:         	  	 " << drive.mScsiInfo.vendor);
    	LtfsLogDebug(indentStr << "product:      	  	 " << drive.mScsiInfo.product);
    	LtfsLogDebug(indentStr << "version:            	 " << drive.mScsiInfo.version);
    	LtfsLogDebug(indentStr << "stDev:        	  	 " << drive.mScsiInfo.sgDev);
    	LtfsLogDebug(indentStr << "sgDev:      		  	 " << drive.mScsiInfo.sgDev);
    	LtfsLogDebug(indentStr << "mAccessible:      	 " << drive.mAccessible);
    	LtfsLogDebug(indentStr << "mAbnormal:      	  	 " << drive.mAbnormal);
    	LtfsLogDebug(indentStr << "mIsFullHight:      	 " << drive.mIsFullHight);
    	LtfsLogDebug(indentStr << "mGeneration:      	 " << drive.mGeneration);
    	LtfsLogDebug(indentStr << "mStatus:              " << drive.mStatus \
    	<< " (DRIVE_STATUS_UNKNOWN:" << DRIVE_STATUS_UNKNOWN << ", DRIVE_STATUS_OK:" << DRIVE_STATUS_OK \
    	<< ", DRIVE_STATUS_ERROR:" << DRIVE_STATUS_ERROR \
    	<< ", DRIVE_STATUS_DISCONNECTED:" << DRIVE_STATUS_DISCONNECTED <<  ")");

    	LtfsLogDebug(indentStr << "mCleaningStatus:     " << drive.mCleaningStatus \
    	<<" (CLEANING_NONE:" << CLEANING_NONE << ", CLEANING_REQUIRED:" \
    	<< CLEANING_REQUIRED << ", CLEANING_IN_PROGRESS:" << CLEANING_IN_PROGRESS \
    	<< ", CLEANING_UNKNOWN:" << CLEANING_UNKNOWN << ")" );

    	LtfsLogDebug(indentStr << "mInterfaceType:      " << drive.mInterfaceType \
    	<< " (INTERFACE_UNKNOWN:" << INTERFACE_UNKNOWN << ", INTERFACE_FC:" << INTERFACE_FC << ", INTERFACE_SAS:" << INTERFACE_SAS << ")");

    	LtfsLogDebug("" << indentStr << "mSlotID: " << drive.mSlotID << ", mLogicSlotID = " << drive.mLogicSlotID);
    	LtfsLogDebug("" << indentStr << "mIsEmpty: " << drive.mIsEmpty << ", mBarcode: " << drive.mBarcode);
	}

	void DebugPrintTape(const tapeElement& tape, const string& indentStr)
	{
    	LtfsLogDebug(indentStr << "mMediumType:         " << tape.mMediumType \
    	<< " (MEDIUM_UNKNOWN:" << MEDIUM_UNKNOWN << ", MEDIUM_DATA:" << MEDIUM_DATA << ", MEDIUM_CLEANING:" << MEDIUM_CLEANING \
    	<< ", MEDIUM_DIAGNOSTICS:" << MEDIUM_DIAGNOSTICS << ", MEDIUM_WORM:" << MEDIUM_WORM <<  ")");

    	LtfsLogDebug("" << indentStr << "mSlotID:" << tape.mSlotID << ", mLogicSlotID = " << tape.mLogicSlotID);
    	LtfsLogDebug("" << indentStr << "mBarcode:" << tape.mBarcode);
	}

	void DebugPrintSlot(const slotElement& slot, const string& indentStr)
	{
    	LtfsLogDebug("" << indentStr << "mSlotID: " << slot.mSlotID << ", mLogicSlotID = " << slot.mLogicSlotID
    			<< ", mIsEmpty: " << slot.mIsEmpty
    			<< ", mAccessible: " << slot.mAccessible
    			<< ", mAbnormal: " << slot.mAbnormal
    			<< ", mBarcode: " << slot.mBarcode);
	}
	void DebugPrintMailSlot(const mailSlotElement& mailSlot, const string& indentStr)
	{
    	LtfsLogDebug("" << indentStr << "mSlotID: " << mailSlot.mSlotID << ", mLogicSlotID = " << mailSlot.mLogicSlotID
    			<< ", mIsEmpty: " << mailSlot.mIsEmpty
    			<< ", mIsOpen: " << mailSlot.mIsOpen
    			<< ", mAccessible: " << mailSlot.mAccessible
    			<< ", mAbnormal: " << mailSlot.mAbnormal
    			<< ", mBarcode: " << mailSlot.mBarcode);
	}

	void DebugPrintChanger(const ChangerInfo_& changer, const string& indentStr)
	{
    	LtfsLogDebug(indentStr << "Information for changer:");
    	LtfsLogDebug(indentStr << "mSerial:              " << changer.mSerial);
    	LtfsLogDebug(indentStr << "scsiAddr:             " << changer.mScsiInfo.scsiAddr);
    	LtfsLogDebug(indentStr << "vendor:         	  	 " << changer.mScsiInfo.vendor);
    	LtfsLogDebug(indentStr << "product:      	  	 " << changer.mScsiInfo.product);
    	LtfsLogDebug(indentStr << "version:            	 " << changer.mScsiInfo.version);
    	LtfsLogDebug(indentStr << "stDev:        	  	 " << changer.mScsiInfo.sgDev);
    	LtfsLogDebug(indentStr << "sgDev:                " << changer.mScsiInfo.sgDev);
    	LtfsLogDebug(indentStr << "mAutoCleanMode:       " << changer.mAutoCleanMode);
    	LtfsLogDebug(indentStr << "mDriveStart:          " << changer.mDriveStart);
    	LtfsLogDebug(indentStr << "mSlotStart:           " << changer.mSlotStart);
    	LtfsLogDebug(indentStr << "mMailSlotStart:       " << changer.mMailSlotStart);
    	LtfsLogDebug(indentStr << "mStatus:              " << changer.mStatus << "(CHANGER_STATUS_UNKNOWN:" << CHANGER_STATUS_UNKNOWN \
    	<< ", CHANGER_STATUS_CONNECTED:" << CHANGER_STATUS_CONNECTED << ", CHANGER_STATUS_BUSY:" << CHANGER_STATUS_BUSY \
    	<< ", CHANGER_STATUS_DISCONNECTED:" << CHANGER_STATUS_DISCONNECTED << ")");

    	LtfsLogDebug(indentStr << "Information for drives:");
    	for(unsigned int i = 0; i < changer.drives.size(); i++){
    		DebugPrintDrive(changer.drives[i], indentStr + "    ");
    	}
    	LtfsLogDebug(indentStr << "Information for tapes:");
    	for(unsigned int i = 0; i < changer.tapes.size(); i++){
    		DebugPrintTape(changer.tapes[i], indentStr + "    ");
    		LtfsLogDebug("");
    	}
    	LtfsLogDebug(indentStr << "Information for mail slots:");
    	for(unsigned int i = 0; i < changer.mailSlots.size(); i++){
    		DebugPrintMailSlot(changer.mailSlots[i], indentStr + "    ");
    		LtfsLogDebug("");
    	}
    	LtfsLogDebug(indentStr << "Information for storage slots:");
    	for(unsigned int i = 0; i < changer.slots.size(); i++){
    		DebugPrintSlot(changer.slots[i], indentStr + "    ");
    	}
	}
}
