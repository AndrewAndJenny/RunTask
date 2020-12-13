#include "RunTask.h"
#include "TaskRunable.hpp"
#include "chrono"
#include "boost/filesystem.hpp"
#include "ctime"
#ifdef  _WIN32
#include "io.h"
#else defined linux
#include "stdlib.h"
#include"unistd.h"
#endif

#include "LPFileOperator.hpp"

int PrjInfo::FindPosVector(std::vector <std::string> input, std::string content)
{
	std::vector<std::string>::iterator iter = std::find(input.begin(), input.end(), content);
	if (iter == input.end())
	{
		return -1;
	}
	else {
		return std::distance(input.begin(), iter);
	}
}

PrjInfo::PrjInfo(std::string taskIniPath, std::string appName)
{
#if 0
	std::string line;
	std::smatch result;
	std::regex r("=\\s*");

	//read RunTask.ini
	std::fstream fpRead(taskIniPath);
	if (!fpRead)
	{
		std::cerr << "Initialization is defeated!";
		exit(1);
	}

	std::getline(fpRead, line);
	//_id
	std::getline(fpRead, line);
	if (std::regex_search(line, result, r))
		_id = std::atoi(result.suffix().str().c_str());
	//_state
	std::getline(fpRead, line);
	if (std::regex_search(line, result, r))
		_state = std::atoi(result.suffix().str().c_str());

	std::string t_appName = "[" + appName + "]";
	//matching app
    std::getline(fpRead, line);
	while (!line.empty())
	{
		BasicTask tmp;

		if (strcmp(line.c_str(), t_appName.c_str()) == 0) {
			std::getline(fpRead, line);
			if (std::regex_search(line, result, r))
				_grpNum = std::atoi(result.suffix().str().c_str());
			for (int i = 0; i < _grpNum; i++) {
				std::getline(fpRead, line);
				if (std::regex_search(line, result, r))
					tmp.tskFile = result.suffix().str();
				else
					tmp.tskFile = "";

				std::getline(fpRead, line);
				if (std::regex_search(line, result, r))
					tmp.tskExe = result.suffix().str();
				else
					tmp.tskExe = "";

				std::getline(fpRead, line);
				if (std::regex_search(line, result, r))
					tmp.knlExe = result.suffix().str();
				else
					tmp.knlExe = "";
				_dealStage.insert(std::pair<int, BasicTask>(i, tmp));
			}
			std::getline(fpRead, line);
			if (std::regex_search(line, result, r))
				_chkExe = result.suffix().str();
			else
				_chkExe = "";
			break;
		}
        std::getline(fpRead, line);
	}
	fpRead.close();
#else
	int max_len = 1024;
	char strLoad[max_len];
	int grp_temp(0);
	GetPrivateProfileString(appName.c_str(), "GrpSum", "0", strLoad, max_len, taskIniPath.c_str());
	sscanf(strLoad, "%d", &grp_temp);

    char prefix[32];
    char keyName[max_len];

    int task_idx(0);
    for (int i = 0; i < grp_temp; ++i) {
        BasicTask task_item;

        {
            sprintf(prefix, "Grp%d_", i);

            sprintf(keyName, "%sTskFile", prefix);
            GetPrivateProfileString(appName.c_str(), keyName, "", strLoad, max_len, taskIniPath.c_str());
            task_item.tskFile = std::string(strLoad);

            sprintf(keyName, "%sTskExe", prefix);
            GetPrivateProfileString(appName.c_str(), keyName, "", strLoad, max_len, taskIniPath.c_str());
            task_item.tskExe = std::string(strLoad);

            sprintf(keyName, "%sKnlExe", prefix);
            GetPrivateProfileString(appName.c_str(), keyName, "", strLoad, max_len, taskIniPath.c_str());
            task_item.knlExe = std::string(strLoad);

            sprintf(keyName, "%sCore", prefix);
            GetPrivateProfileString(appName.c_str(), keyName, "-1", strLoad, max_len, taskIniPath.c_str());
            sscanf(strLoad, "%d", &task_item.coreNum);
        }

        if (task_item.tskExe.empty() && task_item.tskFile.empty())
            continue;

        _dealStage.insert(std::make_pair(task_idx, task_item));
        task_idx++;
    }
    _grpNum = task_idx;

    GetPrivateProfileString(appName.c_str(), "ChkExe", "", strLoad, max_len, taskIniPath.c_str());
    _chkExe = std::string(strLoad);
#endif
}

void PrjInfo::SplitPath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
	char *p_whole_name;

	char* ptr = const_cast<char*>(strchr(path, '/'));;
	snprintf(drive, ptr - path, "%s", path);
	if (NULL == path)
	{
		dir[0] = '\0';
		fname[0] = '\0';
		ext[0] = '\0';
		return;
	}

	if ('/' == path[strlen(path)])
	{
		strcpy(dir, path);
		fname[0] = '\0';
		ext[0] = '\0';
		return;
	}

	p_whole_name = const_cast<char*>(strrchr(path, '/'));
	if (NULL != p_whole_name)
	{
		p_whole_name++;
		SplitWholeName(p_whole_name, fname, ext);

		snprintf(dir, p_whole_name - path, "%s", path);
	}
	else
	{
		SplitWholeName(path, fname, ext);
		dir[0] = '\0';
	}
	char tmp[512];
	strcpy(tmp, dir);
	sprintf(dir, "%s/", tmp);

}

void PrjInfo::SplitWholeName(const char *whole_name, char *fname, char *ext)
{
	char *p_ext;

	p_ext = const_cast<char*>(strrchr(whole_name, '.'));
	if (NULL != p_ext)
	{
		strcpy(ext, p_ext);
		snprintf(fname, p_ext - whole_name + 1, "%s", whole_name);
	}
	else
	{
		ext[0] = '\0';
		strcpy(fname, whole_name);
	}
}

bool RunTask::run(std::string runTaskIniPath)
{
	char _drive[512], _dir[512], _fname[512], _ext[512];
	std::string grpTskFilePath = "", grpTskExePath = "";

	SplitPath(_xmlPath.c_str(), _drive, _dir, _fname, _ext);
	int grpNum = GetGrpNum();
	bTsk dealStage = GetDealStage();

	for (int i = 0; i < grpNum; i++)
	{
	    if (dealStage.find(i) == dealStage.end())
            continue;

		if (!dealStage[i].tskFile.empty())
			grpTskFilePath = std::string(_dir).append(dealStage[i].tskFile);
		if (!dealStage[i].tskExe.empty())
		{
		    char _dir_[512];
		    boost::filesystem::path iniRelPath(runTaskIniPath);
            boost::filesystem::path iniAbsPath = boost::filesystem::system_complete(iniRelPath);
            SplitPath(iniAbsPath.c_str(), _drive, _dir_, _fname, _ext);
		    grpTskExePath = std::string(_dir_).append(dealStage[i].tskExe);
		}

		std::string cmdTskExe;
		if (!grpTskExePath.empty())
		{
			cmdTskExe = grpTskExePath + " " + _xmlPath;
            system(cmdTskExe.c_str());
			if (grpTskFilePath.empty())//just run TskExe xml, current loop is over
				continue;
		}
		//if dont have TskExe,there must have a .gtsk,so we read the content of gtsk.
		MediumTask tmp;
		std::string line;
		//read .gtsk
		std::fstream fpRead(grpTskFilePath);
		if (!fpRead)
		{
			std::cerr << "Open "<<grpTskFilePath<<" is defeated!";
			exit(1);
		}

		std::getline(fpRead, line);
		tmp.tskNum = atoi(line.c_str());

		for (int j = 0; j < tmp.tskNum; j++)
		{
			std::getline(fpRead, line);
			tmp.callInfo.push_back(line);
		}
		_dealDetail.insert(std::pair<int, MediumTask>(i, tmp));//Store every loop information

		fpRead.close();

		if (_dealDetail[i].tskNum == 0)
			continue;

		//use threadpool to deal with detail tasks
		QThreadPool pool;
		std::string cmdLine;
		int maxThreadNum = pool.maxThreadCount();
		int totalTskNum = _dealDetail[i].tskNum;

		int thread_used = dealStage[i].coreNum > 0 ? dealStage[i].coreNum : _threadNum;
		if (thread_used < 1 || thread_used > maxThreadNum) {
            thread_used = QThread::idealThreadCount();
			//std::cerr << "ERROR! The number of thread must meet the requirements\n";
			//exit(1);
		}
		if(totalTskNum==1)
		    system(_dealDetail[i].callInfo[0].c_str());
		else
        {
            std::cout << thread_used << " cores used to execute " << totalTskNum << " sub tasks in group " << i << std::endl;

            pool.setMaxThreadCount(thread_used);
            pool.setExpiryTimeout(-1);

            std::cout << "The " << i+1 << " group task:" << std::endl;
            auto start_time = std::chrono::high_resolution_clock::now();
            for (int taskNum=0; taskNum < totalTskNum; taskNum++)
            {
                cmdLine = _dealDetail[i].callInfo[taskNum];
                TaskRunable* subTask = new TaskRunable(taskNum,cmdLine);
                subTask->setAutoDelete(true);
                pool.start(subTask);
            }

            pool.waitForDone();
            auto end_time = std::chrono::high_resolution_clock::now();
            auto cost_time = std::chrono::duration_cast<std::chrono::seconds>(end_time-start_time);

            std::cout << "The " << i+1 << " group task. the cost of time is " <<cost_time.count()<<"s"<<std::endl;
        }

	}

	std::string cmdChkExe = GetChkExe();

	if (!cmdChkExe.empty()){
        cmdChkExe = cmdChkExe+" "+_xmlPath;
        system(cmdChkExe.c_str());
    }

	return true;
}