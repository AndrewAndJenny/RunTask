/*
@File	: StFileOperator.hpp
@Brief	: Functions for file operation
@Author	: Xinyi Liu
@Date	: May 05th, 2016
*/
#ifndef __ST_FILE_OPERATOR_HPP__
#define __ST_FILE_OPERATOR_HPP__

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <tchar.h>
#include <direct.h>
#else
typedef 	unsigned int	HANDLE;
//typedef 	int				BOOL;
typedef 	const char*     LPCSTR;
typedef		char*			LPSTR;
typedef 	unsigned char	BYTE;
typedef 	unsigned int	RGBQUAD;
typedef 	unsigned short	WORD;
typedef 	unsigned int	DWORD;
typedef 	unsigned int	UINT;
typedef 	unsigned long	LONGLONG;
typedef		char			TCHAR;
typedef		const TCHAR	*	LPCTSTR;
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef strcmpi
#define strcmpi strcasecmp
#endif
#endif

#include <vector>
#include <string>
#include <time.h>
#include <chrono>
#include <random>
#include <algorithm>
#include <iostream>

#ifdef _UNICODE
#define IO_STR std::wstring
#else
#define IO_STR std::string
#endif //

namespace LPFile {

	inline void Dos2Unix(TCHAR *strCmd) { size_t len = strlen(strCmd); for (size_t i = 0; i < len; i++) { if (strCmd[i] == '\\') strCmd[i] = '/'; } }

	inline void Unix2Dos(TCHAR *strCmd) { size_t len = strlen(strCmd); for (size_t i = 0; i < len; i++) { if (strCmd[i] == '/') strCmd[i] = '\\'; } }

	inline void GetUnixDir(TCHAR *strCmd) { if (strlen(strCmd) < 1) return; Dos2Unix(strCmd); if (strCmd[strlen(strCmd) - 1] != '/') strcat(strCmd, "/"); }

	inline bool IsExist(LPCTSTR lpstrPathName) {
#ifdef _WIN32
		WIN32_FIND_DATA fd; HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind = ::FindFirstFile(lpstrPathName, &fd);
		if (hFind == INVALID_HANDLE_VALUE) return FALSE;
		::FindClose(hFind); return TRUE;
#else
		return ((access(lpstrPathName, F_OK)) != -1);
#endif
	}

	inline bool CreateDir(const std::string& dirName)
	{
#ifdef _WIN32
		LPCTSTR szPath = dirName.c_str();
		WIN32_FIND_DATA fd; HANDLE hFind = ::FindFirstFile(szPath, &fd);
		if (hFind != INVALID_HANDLE_VALUE) { ::FindClose(hFind); ::CreateDirectory(szPath, NULL); return TRUE; }
		TCHAR strPath[512]; _tcscpy_s(strPath, szPath);
		TCHAR *pSplit1 = _tcsrchr(strPath, '\\');
		TCHAR *pSplit2 = _tcsrchr(strPath, '/');
		TCHAR *pSplit = pSplit1 > pSplit2 ? pSplit1 : pSplit2;
		if (!pSplit) return TRUE; else *pSplit = 0;
		if (!CreateDir(strPath)) return FALSE;
		return ::CreateDirectory(szPath, NULL);
#else
		uint32_t beginCmpPath = 0;
		uint32_t endCmpPath = 0;

		std::string fullPath = "";

		//LOGD("path = %s\n", dirName.c_str());

		if ('/' != dirName[0])
		{ //Relative path
			//get current path
			fullPath = getcwd(nullptr, 0);

			beginCmpPath = fullPath.size();

			//LOGD("current Path: %s\n", fullPath.c_str());
			fullPath = fullPath + "/" + dirName;
		}
		else
		{
			//Absolute path
			fullPath = dirName;
			beginCmpPath = 1;
		}

		if (fullPath[fullPath.size() - 1] != '/')
		{
			fullPath += "/";
		}

		endCmpPath = fullPath.size();

		//create dirs;
		for (uint32_t i = beginCmpPath; i < endCmpPath; i++)
		{
			if ('/' == fullPath[i])
			{
				std::string curPath = fullPath.substr(0, i);
				if (access(curPath.c_str(), F_OK) != 0)
				{
					//if(mkdir(curPath.c_str(), S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH) == -1)
					if (mkdir(curPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH) == -1)
					{
						std::cerr << "mkdir " << curPath.c_str() << " failed: " << strerror(errno) << std::endl;
						//LOGD("mkdir(%s) failed(%s)\n", curPath.c_str(), strerror(errno));
						return false;
					}
				}
			}
		}

		return true;
#endif
	}

	inline TCHAR* GetExtName(LPCTSTR strPath)
	{
		TCHAR str[512]; strcpy(str, strPath);
		TCHAR *ptr; ptr = strrchr(str, '.');
		if (ptr) return ptr;
		return 0;
	}
	inline IO_STR ExcludeExt(IO_STR file_path)
	{
		TCHAR str[512]; strcpy(str, file_path.c_str());
		TCHAR *ptr = strrchr(str, '.');
		if (ptr) strcpy(ptr, "");
		return str;
	}

	inline char *GetFileDirectory(const char* filename) {
		char *ret = NULL;
		char dir[1024];
		char *cur;
		if (filename == NULL) return(NULL);

#if defined(WIN32) && !defined(__CYGWIN__)
#   define IS_SEP(ch) ((ch=='/')||(ch=='\\'))
#else
#   define IS_SEP(ch) (ch=='/')
#endif
		strncpy(dir, filename, 1023);
		dir[1023] = 0;
		cur = &dir[strlen(dir)];
		while (cur > dir)
		{
			if (IS_SEP(*cur)) break;

			cur--;
		}
		if (IS_SEP(*cur))
		{
			if (cur == dir)
			{
				//1.根目录  
				dir[1] = 0;
			}
			else
			{
				*cur = 0;
			}
			ret = strdup(dir);
		}
		else
		{
			//1.如果是相对路径,获取当前目录  
			//io.h
			if (getcwd(dir, 1024) != NULL)
			{
				dir[1023] = 0;
				ret = strdup(dir);
			}
		}
		strcat(ret, "\\");
		return ret;
#undef IS_SEP
	}

	inline std::vector<std::string> _splitString(char* str, const char* seps)
	{
		std::vector<std::string> sub_strs;
		char* token = strtok(str, seps);
		while (token) {
			std::string sub(token);
			sub = sub.substr(0, sub.find_last_of("\t\r\n"));
			sub_strs.push_back(sub);
			token = strtok(NULL, seps);
		}
		return sub_strs;
	}

	inline bool readlines(IO_STR path, std::vector<IO_STR> & lines)
	{
		FILE *fp = fopen(path.c_str(), "r");
		if (!fp) {
			return false;
		}
		char lbuf[2048];
		while (fgets(lbuf, 2048, fp)) {
			lines.push_back(lbuf);
		}
		fclose(fp);
		return true;
	}

#ifndef WIN32
	static void _split_whole_name(const char *whole_name, char *fname, char *ext)
	{
		char *p_ext;

		p_ext = const_cast<char*>(rindex(whole_name, '.'));
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

	inline void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
	{
		char *p_whole_name;

		drive[0] = '\0';
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

		p_whole_name = const_cast<char*>(rindex(path, '/'));
		if (NULL != p_whole_name)
		{
			p_whole_name++;
			_split_whole_name(p_whole_name, fname, ext);

			snprintf(dir, p_whole_name - path, "%s", path);
			GetUnixDir(dir);
		}
		else
		{
			_split_whole_name(path, fname, ext);
			dir[0] = '\0';
		}
	}

#endif

	//! @brief: 获取当前程序路径
	inline bool GetCurrentPath(char buf[], int len)
	{
#ifdef WIN32
		GetModuleFileName(NULL, buf, len);
		return true;
#else
		int n = readlink("/proc/self/exe", buf, len);
		if (n > 0 && n < sizeof(buf)) {
			return true;
		}
		else return false;
#endif
	}

	inline char *GetIniKeyString(char *title, char *key, char *filename)
	{
		FILE *fp;
		char szLine[1024];
		static char tmpstr[1024];
		int rtnval;
		int i = 0;
		int flag = 0;
		char *tmp;
		if ((fp = fopen(filename, "r")) == NULL) {
			return "";
		}

		while (!feof(fp)) {
			rtnval = fgetc(fp);
			if (rtnval == EOF) {
				break;
			}
			else {
				szLine[i++] = rtnval;
			}

			if (rtnval == '\n') {
#ifndef WIN32
				i--;
#endif
				szLine[--i] = '\0';
				i = 0;
				tmp = strchr(szLine, '=');
				if ((tmp != NULL) && (flag == 1)) {
					if (strstr(szLine, key) != NULL) {
						//注释行

						if ('#' == szLine[0]) {
						}
						else if ('\/' == szLine[0] && '\/' == szLine[1]) {
						}
						else {
							strcpy(tmpstr, tmp + 1);
							fclose(fp);
							return tmpstr;
						}
					}
				}
				else
				{
					strcpy(tmpstr, "[");
					strcat(tmpstr, title);
					strcat(tmpstr, "]");
					if (strncmp(tmpstr, szLine, strlen(tmpstr)) == 0) {
						//找到title
						flag = 1;
					}
				}
			}
		}
		fclose(fp);

		return "";
	}

	inline int GetIniKeyInt(char *title, char *key, char *filename)
	{
		return atoi(GetIniKeyString(title, key, filename));
	}

	inline bool copyFile(const char* src, const char* des, bool faile_if_exist = false)
	{
		if (IsExist(des) && faile_if_exist) {
			return false;
		}

		bool nRet = true;
		FILE* pSrc = NULL, *pDes = NULL;
		pSrc = fopen(src, "r");
		pDes = fopen(des, "w+");

		if (pSrc && pDes)
		{
			int nLen = 0;
			char szBuf[1024] = { 0 };
			while ((nLen = fread(szBuf, 1, sizeof szBuf, pSrc)) > 0)
			{
				fwrite(szBuf, 1, nLen, pDes);
			}
		}
		else
			nRet = false;

		if (pSrc)
			fclose(pSrc), pSrc = NULL;

		if (pDes)
			fclose(pDes), pDes = NULL;

		return nRet;
	}

#ifndef WIN32
#ifndef CopyFile
#define CopyFile copyFile
#endif
#endif

	inline bool RelativePath2AbsolutePath(const char* rlt, const char* current_dir, char* abs)
	{
		char path[512];	strcpy(path, current_dir);
		int len = strlen(path);	if (len < 1) { len = 1; }
		char* pD = path + len - 1;
		if (*pD == '/' || *pD == '\\') *pD = 0;

		const char* pS = rlt;
		if (*pS == '.' && (*(pS + 1) == '\\' || *(pS + 1) == '/')) { sprintf(abs, "%s%s", path, pS + 1); return true; }

		while (*pS == '.' && *(pS + 1) == '.' && (*(pS + 2) == '\\' || *(pS + 2) == '/'))
		{
			pD = strrchr(path, '/');		if (pD) pD = strrchr(path, '\\');	if (!pD) { *abs = 0; return false; }
			*pD = 0;
			pS += 3;
		}

		*pD = '/';
		sprintf(abs, "%s%s", path, pS);
		return true;
	}

	inline bool CheckChar(const char c) {
		if (c == '\n') return false;
		if (c == '\r') return false;
		if (c == '\0') return false;
		return true;
	}

	//return end position of long string
	inline int	MatchString(LPCSTR	lpLongString, LPCSTR lpShortString) {
		if (strlen(lpLongString) < strlen(lpShortString)) return 0;

		const char *pL, *pS;	int nMark, shortMark;
		pL = lpLongString;	pS = lpShortString;
		nMark = 0;	shortMark = 0;

		char headchar = *lpShortString;	int headcnt = 0;
		while (CheckChar(*pS) && (headchar == *pS++))
			headcnt++;

		pS = lpShortString;
		while (CheckChar(*pL)) {
			if (*pL == *pS) {
				pS++;	shortMark++;				//nMark++;
			}
			else {
				if (headcnt != shortMark || *pL != headchar) { pS = lpShortString; shortMark = 0; }
			}	//nMark = 0;
			pL++;		nMark++;
			if (!CheckChar(*pS))
				return nMark;
		}
		return 0;
	}

	inline int	PickString(LPCSTR	lpLongString, LPCSTR lpShortString) {
		if (strlen(lpLongString) < strlen(lpShortString)) return 0;
		int nMark;
		char strLong[1024], strShort[512];	bool bAddSpace_L = false;
		if (*lpLongString == ' ') strcpy(strLong, lpLongString);	else { sprintf(strLong, " %s", lpLongString); bAddSpace_L = true; }
		if (*lpShortString == ' ') strcpy(strShort, lpShortString);	else { sprintf(strShort, " %s", lpShortString); }

		nMark = MatchString(strLong, strShort);
		if (bAddSpace_L) nMark -= 1;
		if (nMark < 0) nMark = 0;

		return nMark;
	}

	inline bool ReplaceString(char* strLine, const char* strName, const char* strReplace)
	{
		int nTmp = 0;
		nTmp = MatchString(strLine, strName);

		if (nTmp == 0) return false;
		int nNamLen = strlen(strName);	nNamLen = nTmp - nNamLen;
		if (nNamLen < 0) return false;

		char strH[512];	memcpy(strH, strLine, nNamLen * sizeof(char));
		memcpy(strH + nNamLen, strReplace, strlen(strReplace) * sizeof(char));	nNamLen += strlen(strReplace);
		strcpy(strH + nNamLen, strLine + nTmp);
		strcpy(strLine, strH);

		return true;
	}

	inline bool GetPrivateProfilePath(char* filepath, LPCSTR lpstrFileName)
	{
		filepath[0] = 0;
		char execname[1024] = "";
		if (!GetCurrentPath(execname, 1024)) {
			return false;
		}

		Dos2Unix(execname);
		char	strFileName[128] = "";

		char* pS = NULL;	pS = strrchr(execname, '.');	if (!pS) pS = execname + strlen(execname);	strcpy(pS, ".ini");
		//	strcpy(filepath,execname);
		pS = strrchr(execname, '/');		if (!pS) return false;
		if (lpstrFileName) { strcpy(pS + 1, lpstrFileName);	strcpy(strFileName, lpstrFileName); }
		else strcpy(strFileName, pS + 1);

		while (1)
		{
			if (IsExist(execname)) { strcpy(filepath, execname); return true; }
			*pS = 0;
			pS = strrchr(execname, '/');		if (!pS) return false;
			strcpy(pS + 1, strFileName);

		}

		return false;
	}


	inline DWORD	GetPrivateProfileStringE(
		LPCSTR lpAppName,
		LPCSTR lpKeyName,
		LPCSTR lpDefault,
		LPSTR  lpReturnedString,
		DWORD  nSize,
		LPCSTR lpFileName
	)
	{
		DWORD nLen;	LPCSTR lpCopy;
		lpCopy = lpDefault;

		char strLine[1024];
		if (IsExist(lpFileName) && *lpAppName&&*lpKeyName) {
			FILE* fp = fopen(lpFileName, "rt");
			int nMark = 0, nt;
			bool bTmp = false;
			while (!feof(fp)) {
				memset(strLine, 0, sizeof(char) * 1024);	fgets(strLine, 1024, fp);
				if (bTmp)	if ((nMark = PickString(strLine, lpKeyName))) { if (strLine[nMark] == ' ' || strLine[nMark] == '\t' || strLine[nMark] == '=') { break; } }
				if ((nMark = PickString(strLine, "["))) {
					if (bTmp) { nMark = 0; break; }
					else if ((nt = PickString(strLine + nMark, lpAppName))) { if (strLine[nt + nMark] == ' ' || strLine[nt + nMark] == ']') bTmp = true; }
				}
			}
			fclose(fp);
			if (bTmp && (nMark)) {
				nLen = strlen(strLine);
				while (strLine[nLen - 1] == '\n' || strLine[nLen - 1] == '\r' || strLine[nLen - 1] == '\t' || strLine[nLen - 1] == ' ') { nLen--; strLine[nLen] = 0; }
				while (strLine[nMark] == ' ' || strLine[nMark] == '\t' || strLine[nMark] == '=') { nMark++; }
				lpCopy = strLine + nMark;
				if (strlen(lpCopy) == 0) lpCopy = lpDefault;
			}
		}

		nLen = strlen(lpCopy);
		if (nLen >= (nSize - 1)) { memcpy(lpReturnedString, lpCopy, nSize - 1); nLen = nSize - 1; }
		else  memcpy(lpReturnedString, lpCopy, nLen);
		*(lpReturnedString + nLen) = 0;

		return nLen + 1;

	}

#define GetPrivateProfileString	GetPrivateProfileStringE


#define MAXLINELENGTH 4096
	inline bool WritePrivateProfileStringE(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName)
	{
		char szsection[100] = { 0 };
		char szentry[100] = { 0 };
		char sztmp[MAXLINELENGTH] = { 0 };
		sprintf(szsection, "[%s]", lpAppName);
		sprintf(szentry, "%s=", lpKeyName);
		//read file conn
		if (!IsExist(lpFileName))
		{
			//file not exist
			FILE* pfile = fopen(lpFileName, "w");
			if (!pfile)
			{
				return false;
			}
			sprintf(sztmp, "%s\n%s%s\n", szsection, szentry, lpString);
			fwrite(sztmp, sizeof(char), strlen(sztmp), pfile);
			fclose(pfile);
			return true;
		}
		std::string strConn = "";
		std::string strRow = "";
		size_t nAppPos = std::string::npos;
		size_t nKeyPos = 0;
		bool bfindapp = true;
		bool bfindkey = false;
		FILE* pfile = NULL;
		pfile = fopen(lpFileName, "r");
		while (!feof(pfile))
		{
			memset(sztmp, 0, sizeof(sztmp));
			fscanf(pfile, "%s", sztmp);  //行读取 确保每行无空格以\n结尾，否则需自定义函数
			strRow = sztmp;
			if (nAppPos == std::string::npos && (nAppPos = strRow.find(szsection)) == 0)
			{
				strConn += szsection;
				strConn += "\n";
				nAppPos = strConn.length();
				nKeyPos = std::string::npos;
			}
			else if (nKeyPos == std::string::npos && (nKeyPos = strRow.find(szentry)) == 0)
			{
				strConn += szentry;
				strConn += lpString;
				strConn += "\n";
			}
			else
			{
				strConn += strRow;
				strConn += "\n";
			}
		}
		if (nAppPos == std::string::npos && nKeyPos == 0)
		{
			memset(sztmp, 0, sizeof(sztmp));
			sprintf(sztmp, "%s\n%s%s\n", szsection, szentry, lpString);
			strConn += sztmp;
		}
		else if (nAppPos != std::string::npos && nKeyPos == std::string::npos)
		{
			std::string strBack = strConn.substr(nAppPos);
			strConn = strConn.substr(0, nAppPos);
			strConn += szentry;
			strConn += lpString;
			strConn += "\n" + strBack;
		}
		fclose(pfile);
		pfile = fopen(lpFileName, "w");
		fwrite(strConn.c_str(), sizeof(char), strConn.length(), pfile);
		fclose(pfile);
		return true;
	}

#define WritePrivateProfileString	WritePrivateProfileStringE

}
#endif // !__ST_FILE_OPERATOR_HPP__