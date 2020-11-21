#pragma once
#ifndef BASICINFO_HPP
#define BASICINFO_HPP
#define _CRT_SECURE_NO_WARNINGS
#include "iostream"
#include"map"
#include "fstream"
#include "string"
#include "sstream"
#include "regex"
#include "vector"
#include "stdio.h"
#include "stdlib.h"
struct BasicTask
{
	std::string tskFile;
	std::string tskExe;
	std::string knlExe;
};

struct MediumTask
{
	int tskNum;
	std::vector<std::string> callInfo;
};
typedef std::map<int, BasicTask> bTsk;
typedef std::map<int, MediumTask> mTsk;

class PrjInfo
{
public:
	PrjInfo() {};
	~PrjInfo() { _dealStage.clear(); };
	explicit PrjInfo(std::string taskIniPath, std::string appName);

	int GetGrpNum() {return _grpNum;}
	bTsk GetDealStage() {return _dealStage;}
	std::string GetChkExe() { return _chkExe; }

	static int FindPosVector(std::vector <std::string> input, std::string content);
	void SplitPath(const char *path, char *drive, char *dir, char *fname, char *ext);

protected:
	void SplitWholeName(const char *whole_name, char *fname, char *ext);

private:
	int _id;
	int _state;
	int _grpNum;
	bTsk _dealStage;
	std::string _chkExe;
	static const std::vector<std::string> s_appList;
};

class RunTask :public PrjInfo
{
public:
	RunTask() {};
	explicit RunTask(std::string taskIniPath, std::string appName, int svrId, int threadNum, std::string xmlPath):
		PrjInfo(taskIniPath, appName), _svrId(svrId), _threadNum(threadNum), _xmlPath(xmlPath) {}
	~RunTask() {};
	bool run();
	
protected:

private:
	int _svrId;
	int _threadNum;
	std::string _xmlPath;
	mTsk _dealDetail;
};

#endif

