#pragma once
#include<afx.h>
#include<vector>
#include "FolderList.h"
class FolderList;

#define MAXMEMERY 855
#define THREADNUM 4

struct Infomation
{
	int endSignals[2];
	int bufferStates[2];
	int readStates[2];
	int writeStates[2];

	ULONGLONG haveReadLength[2];
	ULONGLONG fileLength;

	char buffer[2][MAXMEMERY + 1];
	int len;
	int turnNum;

};

class CFolderOperation {
public:
	CFolderOperation();
	~CFolderOperation();

	static std::vector<CString> vFilePathList;
	static std::vector<CString> vFileDestList;
	static std::vector<Infomation> infos;

	CRITICAL_SECTION gSection[2];
	HANDLE readThread;
	HANDLE writeThread;
	HANDLE stopThread;

	BOOL stopSignal;
	BOOL saveSignal;

	Infomation info;

	CFile readFile;
	CFile writeFile;

	void OpenReadFile(CString& src);
	void OpenWriteFile(CString& dest);
	void CreateThreadsAndCriticalSections(CFolderOperation* file);

	static void ReadAllFiles(CString &src, CString &dest);
};