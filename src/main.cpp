#include "RunTask.h"
#include "boost/filesystem.hpp"
#include "LPFileOperator.hpp"

static void Usage(const char* pszErrorMsg = NULL)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "RunTask -i *.bprj -l APPNAME -svrid projectID\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "[-help,-h]						[produce help message]\n");
	fprintf(stderr, "[-i]									[input the absolute path of .xml]\n");
	fprintf(stderr, "[-l]									[input the name off APP]\n");
	fprintf(stderr, "[-svrid]							[the GCserver ID\n");

	if (pszErrorMsg != NULL)
		fprintf(stderr, "\nFAILURE: %s\n", pszErrorMsg);

	exit(1);
}

int main(int argc,  char* argv[])
{
	int svrId = 0;
	int threadNum = 4;
	std::string xmlPath = "";
	std::string appName = "";
	std::string runTaskIniPath = "RunTask.ini";

    boost::filesystem::path exeRelPath(argv[0]);
    boost::filesystem::path exeAbsPath = boost::filesystem::system_complete(exeRelPath);
#ifdef _WIN32
	size_t dotPos = exeAbsPath.string().find_last_of(".");
	runTaskIniPath = exeAbsPath.string().substr(0, dotPos) + ".ini";
#else
	runTaskIniPath = exeAbsPath.string() + ".ini";
#endif
    std::cout<<runTaskIniPath<<std::endl;

    char strLoad[1024];
	LPFile::GetPrivateProfileString("FlowInfo", "CORENUM", "-1", strLoad, 1024, runTaskIniPath.c_str());
    sscanf(strLoad, "%d", &threadNum);
	
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-h") == 0)
		{
			Usage();
		}
		else if (strcmp(argv[i], "-i") == 0)
		{
			i++; if (i >= argc) continue;
			xmlPath = argv[i];
		}
		else if (strcmp(argv[i], "-l") == 0) {
			i++; if (i >= argc) continue;
			appName = argv[i];
		}
		else if (strcmp(argv[i], "-svrid") == 0) {
			i++; if (i >= argc) continue;
			svrId = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "-t") == 0) {
			i++; if (i >= argc) continue;
			threadNum = atoi(argv[i]);
		}
		else
		{
			Usage("Too many command options.");
		}
	}

	RunTask runTask(runTaskIniPath, appName, svrId, threadNum, xmlPath);
	
	runTask.run(runTaskIniPath);

	return 0;
}
