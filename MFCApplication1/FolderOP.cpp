#include<afx.h>
#include<vector>
#include <iostream>
#include"FolderOP.h"

DWORD WINAPI ReadThreadFunc(LPVOID lpParam);
DWORD WINAPI WriteThreadFunc(LPVOID lpParam);

std::vector<CString> CFolderOperation::vFilePathList;
std::vector<CString> CFolderOperation::vFileDestList;
std::vector<Infomation> CFolderOperation::infos;

CFolderOperation::CFolderOperation() 
{
	for (int j = 0; j < 2; j++) 
	{
		info.endSignals[j] = 0;
		info.bufferStates[j] = 0;
		info.readStates[j] = 0;
		info.writeStates[j] = 0;
		info.haveReadLength[j] = 0;
	}
	info.fileLength = 0;
	stopSignal = FALSE;
	saveSignal = FALSE;
}
CFolderOperation::~CFolderOperation() 
{

}

void CFolderOperation::ReadAllFiles(CString &src, CString &dest)
{
	CFileFind fileFinder;

	if (src.Right(1) != "\\") 
	{
		src += "\\";
	}
	CString filePath = src + _T("\\*.*");
	BOOL isNotEmpty = fileFinder.FindFile(filePath);

	CString strPath, strDest;

	while (isNotEmpty) 
	{
		isNotEmpty = fileFinder.FindNextFile();
		strPath = fileFinder.GetFilePath();
		strDest = dest + _T("\\") + fileFinder.GetFileName();

		if (fileFinder.IsDirectory() && !fileFinder.IsDots()) 
		{
			CreateDirectory(strDest, NULL);
			ReadAllFiles(strPath, strDest);
		}

		else if (!fileFinder.IsDirectory() && !fileFinder.IsDots()) 
		{
			CFolderOperation::vFilePathList.push_back(strPath);
			CFolderOperation::vFileDestList.push_back(strDest);
		}

	}
}

void CFolderOperation::CreateThreadsAndCriticalSections(CFolderOperation* file) 
{

	for (int j = 0; j < 2; j++) 
	{
		InitializeCriticalSection(&file->gSection[j]);
	}

	DWORD dwThreadID;
	file->readThread = CreateThread(NULL, 0, ReadThreadFunc, (LPVOID)file, 0, &dwThreadID);
	if (file->readThread == NULL) 
	{
		std::cout << GetLastError();
		return;
	}
	file->writeThread = CreateThread(NULL, 0, WriteThreadFunc, (LPVOID)file, 0, &dwThreadID);
	if (file->writeThread == NULL) 
	{
		std::cout << GetLastError();
		return;
	}

}

void CFolderOperation::OpenReadFile(CString& src) 
{
	if (!readFile.Open(src, CFile::modeRead)) 
	{
		int error = GetLastError();
		std::cout << error << std::endl;

		return;
	}

	CFileStatus fileStatus;
	if (CFile::GetStatus(src, fileStatus)) 
	{
		info.fileLength = fileStatus.m_size;
	}

}

void CFolderOperation::OpenWriteFile(CString& dest) 
{
	if (!writeFile.Open(dest, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate)) 
	{
		int error = GetLastError();
		std::cout << error << std::endl;
	}
}


void FileCopyToReadBuffer(CFolderOperation* file, int n, char readBuffer[2][MAXMEMERY + 1]) 
{
	if (file->info.readStates[n] == 0) 
	{

		ULONGLONG haveLength = file->info.haveReadLength[0];
		ULONGLONG wholeLength = file->info.fileLength;

		if (haveLength + MAXMEMERY < wholeLength) 
		{
			file->readFile.Read(readBuffer[n], MAXMEMERY);
			file->info.haveReadLength[0] += MAXMEMERY;
		}

		else 
		{
			int temp = wholeLength - haveLength;
			file->readFile.Read(readBuffer[n], temp);
			file->info.haveReadLength[0] += temp;
			file->info.endSignals[0] = 1;
		}

		file->info.readStates[n] = 1;

	}
}

int ReadBufferCopyToBuffer(CFolderOperation* file, int n, int flag, char readBuffer[2][MAXMEMERY + 1]) 
{
	if (file->info.bufferStates[n] == 0)
	{
		for (int i = 0; i < MAXMEMERY; i++) 
		{
			file->info.buffer[n][i] = readBuffer[n][i];
		}
		file->info.bufferStates[n] = 1;
		file->info.readStates[n] = 0;
	}
	else 
	{
		flag = 1;
	}
	return flag;
}

void BufferCopyToWriteBuffer(CFolderOperation* file, int n, char writeBuffer[2][MAXMEMERY + 1]) 
{
	if (file->info.bufferStates[n] == 1)
	{
		for (int i = 0; i < MAXMEMERY; i++) 
		{
			writeBuffer[n][i] = file->info.buffer[n][i];
		}
		file->info.bufferStates[n] = 0;
		file->info.writeStates[n] = 1;
	}
}

int WriteBufferCopyToFile(CFolderOperation* file, int n, int flag, char writeBuffer[2][MAXMEMERY + 1]) 
{
	if (file->info.writeStates[n] == 1)
	{

		ULONGLONG haveLength = file->info.haveReadLength[1];
		ULONGLONG wholeLength = file->info.fileLength;

		if (haveLength + MAXMEMERY < wholeLength) 
		{
			file->writeFile.Write(writeBuffer[n], MAXMEMERY);
			file->info.haveReadLength[1] += MAXMEMERY;
		}

		else 
		{
			int temp = wholeLength - haveLength;
			file->writeFile.Write(writeBuffer[n], temp);
			file->info.haveReadLength[1] += temp;
			file->info.endSignals[1] = 1;
		}

		file->info.writeStates[n] = 0;
	}

	else 
	{
		flag = 1;
	}
	return flag;
}

void StopAndPushInfo(CFolderOperation* file)
{
	while (file->stopSignal == TRUE)
	{
		if (file->saveSignal == TRUE)
		{			
			CFolderOperation::infos.push_back(file->info);
			file->saveSignal = FALSE;
		}
	}
}

DWORD WINAPI ReadThreadFunc(LPVOID lpParam) 
{
	CFolderOperation* file = (CFolderOperation*)lpParam;
	int n = 0;
	char readBuffer[2][MAXMEMERY + 1];

	while (TRUE) 
	{
		int flag = 0;

		FileCopyToReadBuffer(file, n, readBuffer);

		EnterCriticalSection(&file->gSection[n]);
		flag = ReadBufferCopyToBuffer(file, n, flag, readBuffer);
		LeaveCriticalSection(&file->gSection[n]);

		if (flag != 1) 
		{
			n = abs(n - 1);
		}

		StopAndPushInfo(file);

		if (file->info.endSignals[0] == 1)
		{
			break;
		}

	}
	return 0;
}

DWORD WINAPI WriteThreadFunc(LPVOID lpParam) 
{
	CFolderOperation* file = (CFolderOperation*)lpParam;
	int n = 0;
	char writeBuffer[2][MAXMEMERY + 1];

	while (TRUE) 
	{

		int flag = 0;

		EnterCriticalSection(&file->gSection[n]);
		BufferCopyToWriteBuffer(file, n, writeBuffer);
		LeaveCriticalSection(&file->gSection[n]);

		flag = WriteBufferCopyToFile(file, n, flag, writeBuffer);

		if (flag != 1) 
		{
			n = abs(n - 1);
		}

		StopAndPushInfo(file);

		if (file->info.endSignals[1] == 1)
		{
			break;
		}
	}
	return 0;
}
