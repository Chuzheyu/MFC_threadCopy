#pragma once
#include <afx.h>
class CFolderOperation;

struct AllPath 
{
	CString src;
	CString dest;
};

class FolderList 
{
public:
	FolderList();
	~FolderList();

	CFolderOperation** fileList;

	int len;
	int turnNum;

	static void BeginCopy(CString &src, CString &dest);
	static void StopCopy();
	static void RestartCopy();
	static void SaveCopy();
	static void ContinueCopy(CString &src, CString &dest);
	static void CreateLogFile();
	static void OpenAndReadLogFile();

	void setLen(int len, int turnNum);
	void OneTimeCopy(int len);
};