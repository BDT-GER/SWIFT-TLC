#pragma once

struct ErrorReturn
{
	int errorCode;
	std::string errorMsg;
};

struct BarcodeStatus
{
	std::string barcode;
	std::string status;
};

struct BarcodesList
{
	std::vector<std::string> barcodes;
};


struct ShareLit
{
	std::string	name;
	std::string shareUUID;
	std::string dualCopy;
};

struct CreateShareRequest
{
	struct ShareLit	shareToAdd;
	std::vector<std::string>	barcodeList;
};

struct TaskStatus
{
	std::string type;
	std::string status;
	std::string startTime;
	std::string endTime;
	std::string shareUUID;
	std::string shareName;
	std::string progress;

	std::vector<struct BarcodeStatus> tapeStatus;
};
struct QueryTaskRslt
{
	struct ErrorReturn errorReturn;
	std::vector<struct TaskStatus> taskStatus;
};
