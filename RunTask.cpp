#include "RunTask.h"
//#include"time.h"
#ifdef  _WIN32
#include "windows.h"
#include "io.h"
#else defined linux
#include "sys/io.h"
#include "stdlib.h"
#endif


const std::vector<std::string> PrjInfo::s_appList = { "FILTER","REGIST", "CLASSIFY","BDSEG","MESHRECON","POISSONRECON","TEXTURING","StockerLoDs","PlaneSeg","LODTEXTURE","VisCheck" };

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
	std::string line;
	std::smatch result;
	std::regex r("=\\s*");

	int pos = FindPosVector(s_appList, appName);
	if (pos == -1)
	{
		std::cerr << "No such processing stage!";
		exit(1);
	}

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

	while (!fpRead.eof())
	{
		BasicTask tmp;

		std::getline(fpRead, line);

		if (strcmp(line.c_str(), t_appName.c_str()) == 0)
		{
			std::getline(fpRead, line);
			if (std::regex_search(line, result, r))
				_grpNum = std::atoi(result.suffix().str().c_str());
			for (int i = 0; i < _grpNum; i++)
			{
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
		}

	}
	fpRead.close();
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
	char tmp[256];
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

bool RunTask::run()
{
	char _drive[512], _dir[512], _fname[512], _ext[512];
	std::string grpTskFilePath="", grpTskExePath="";

	SplitPath(_xmlPath.c_str(), _drive, _dir, _fname, _ext);
	int grpNum = GetGrpNum();
	bTsk dealStage = GetDealStage();

	for (int i=0;i<grpNum;i++)
	{
		if(!dealStage[i].tskFile.empty())
			grpTskFilePath = std::string(_dir).append(dealStage[i].tskFile);
		if (!dealStage[i].tskExe.empty())
			grpTskExePath = std::string(_dir).append(dealStage[i].tskExe);

		std::string cmdTskExe;
		if (!grpTskExePath.empty())
		{
			SplitPath(grpTskExePath.c_str(), _drive, _dir, _fname, _ext);
			if (strcmp(".bat", _ext) == 0)
				cmdTskExe = grpTskExePath;
			else
			{
				if (_access(_xmlPath.c_str(), 0) != 0)
				{
					std::cerr << "couldn't find xml!";
					exit(1);
				}
				cmdTskExe = grpTskExePath + " " + _xmlPath;
			}
				
			system(cmdTskExe.c_str());//build the content of tskfile

			if(grpTskFilePath.empty())//just run TskExe xml, current loop is over
				continue;
		}
		//if dont have TskExe,there must have a .gtsk,so we read the content of gtsk.
		MediumTask tmp;
		std::string line;

		std::fstream fpRead(grpTskFilePath);
		if (!fpRead)
		{
			std::cerr << "Open .gtsk  is defeated!";
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

		//use thread to deal with detail tasks
		int maxThreadNum = boost::thread::hardware_concurrency();
		int totalTskNum = _dealDetail[i].tskNum;

		if (_threadNum<1 || _threadNum>maxThreadNum) {
			std::cerr << "ERROR! The number of thread must meet the requirements\n";
			exit(1);
		}

		//clock_t start_time = clock();

		for (int j = 0; j < totalTskNum; j+= _threadNum)
		{
			int remainThreadNum = totalTskNum - j >= _threadNum ? _threadNum : totalTskNum - j;
#ifdef  _WIN32
			for (int k = 0; k < remainThreadNum; k++)
			{
				LPSTR cmdLine = (LPSTR)_dealDetail[i].callInfo[j + k].c_str();
				STARTUPINFOA startUpInfo = { 0 };
				PROCESS_INFORMATION processInformation = { 0 };
				startUpInfo.cb = sizeof(STARTUPINFOA);
				//CREATE_NO_WINDOW or CREATE_NEW_CONSOLE
				 bool flag = CreateProcess(NULL,cmdLine,NULL,NULL,FALSE, CREATE_NO_WINDOW,NULL,NULL,&startUpInfo, &processInformation);
				 
				 if (flag)
					 std::cout << "Finish the number of " << j + k << " task\n";
				 else
					 std::cout << "Unfinish the number of " << j + k << " task\n";
			}
#else defined linux

#endif
		}
		//clock_t end_time = clock();
		//std::cout << "The run time is: " << (double)(end_time - start_time)<< "ms" << std::endl;
	}

	std::string cmdChkExe = GetChkExe();

	if (!cmdChkExe.empty())
		system(cmdChkExe.c_str());

	return true;
}