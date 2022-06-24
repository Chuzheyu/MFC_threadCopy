#include "FolderList.h"
#include "FolderOP.h"

static FolderList list;
static CFile writeLogFile;
static CFile readLogFile;

FolderList::FolderList() 
{

}

FolderList::~FolderList()
{

}

BOOL IfAllFinshed(int n, CFolderOperation** file)
{
	BOOL flag = FALSE;

	for (int i = 0; i < n; i++) {
		flag = flag || file[i]->info.endSignals[1] == 0;
	}

	return flag;
}

void FolderList::CreateLogFile()
{

	CString logPath = _T("D:\\logfile\\log.ini");
	while (!writeLogFile.Open(logPath, CFile::modeCreate | CFile::modeWrite))
	{
		int error = GetLastError();
		Sleep(1000);
	}

}

void FolderList::OpenAndReadLogFile()
{

	CString logPath = _T("D:\\logfile\\log.ini");
	while (!readLogFile.Open(logPath, CFile::modeRead))
	{
		int error = GetLastError();
		Sleep(1000);
	}

	ULONGLONG length = readLogFile.GetLength();
	char* tempBuffer = new char[length];
	readLogFile.Read(tempBuffer, length);

	int vLength = length / sizeof(Infomation);
	CFolderOperation::infos.resize(vLength);

	for (int j = 0; j < vLength; j++)
	{

		char* temp = new char[sizeof(Infomation)];
		for (int i = 0; i < sizeof(Infomation); i++)
		{
			temp[i] = tempBuffer[j * sizeof(Infomation) + i];
		}

		Infomation* tempInfo = (Infomation*)(temp);
		list.setLen(vLength, tempInfo->turnNum);

		list.fileList[j] = new CFolderOperation;

		for (int n = 0; n < 2; n++)
		{
			for (int i = 0; i < MAXMEMERY; i++) {
				list.fileList[j]->info.buffer[n][i] = tempInfo->buffer[n][i];
			}

			list.fileList[j]->info.endSignals[n] = tempInfo->endSignals[n];
			list.fileList[j]->info.bufferStates[n] = tempInfo->bufferStates[n];
			list.fileList[j]->info.readStates[n] = tempInfo->readStates[n];
			list.fileList[j]->info.writeStates[n] = tempInfo->writeStates[n];
			list.fileList[j]->info.haveReadLength[n] = tempInfo->haveReadLength[n];
		}

		list.fileList[j]->info.fileLength = tempInfo->fileLength;

	}
}

void FolderList::BeginCopy(CString &src, CString &dest) 
{

	CFolderOperation::ReadAllFiles(src, dest);

	int nums = CFolderOperation::vFilePathList.size();
	for (int i = 0; i < nums; i += THREADNUM)
	{
		int len = THREADNUM;
		if (nums - i < len)
		{
			len = nums - i;
		}

		list.setLen(len, i);

		list.OneTimeCopy(len);
	}
}

void FolderList::setLen(int len, int turnNum)
{
	list.fileList = new CFolderOperation*[len];

	list.len = len;
	list.turnNum = turnNum;
}

void FolderList::OneTimeCopy(int len)
{
	for (int j = 0; j < len; j++)
	{
		fileList[j] = new CFolderOperation;

		fileList[j]->OpenReadFile(CFolderOperation::vFilePathList[list.turnNum + j]);
		fileList[j]->OpenWriteFile(CFolderOperation::vFileDestList[list.turnNum + j]);

		fileList[j]->CreateThreadsAndCriticalSections(fileList[j]);

		fileList[j]->info.len = len;
		fileList[j]->info.turnNum = turnNum;

	}

	while (IfAllFinshed(len, fileList)) {}

	for (int j = 0; j < len;j++)
	{
		delete fileList[j];
	}
	delete[] fileList;

}

void FolderList::StopCopy()
{
	for (int j = 0; j < list.len; j++)
	{
		list.fileList[j]->stopSignal = TRUE;
	}
}

void FolderList::RestartCopy()
{
	for (int j = 0; j < list.len; j++)
	{
		list.fileList[j]->stopSignal = FALSE;
	}
}

BOOL ifAllReady() {
	int i = 0;
	while (TRUE)
	{
		if (list.fileList[i]->info.endSignals[1] == 1)
		{
			i++;
		}

		else if (list.fileList[i]->saveSignal == FALSE)
		{
			i++;
		}

		if (i == list.len)
		{
			break;
		}
	}
	return TRUE;
}

void FolderList::SaveCopy()
{
	FolderList::CreateLogFile();

	for (int j = 0; j < list.len; j++)
	{
		list.fileList[j]->saveSignal = TRUE;
	}

	if (ifAllReady())
	{
		writeLogFile.Write(&CFolderOperation::infos[0], sizeof(Infomation)*CFolderOperation::infos.size());
	}

}

void FolderList::ContinueCopy(CString &src, CString &dest)
{
	CFolderOperation::ReadAllFiles(src, dest);

	OpenAndReadLogFile();
	
	list.OneTimeCopy(list.len);

	int nums = CFolderOperation::vFilePathList.size();
	for (int i = list.turnNum + 1; i < nums; i += THREADNUM)
	{
		int len = THREADNUM;
		if (nums - i < len)
		{
			len = nums - i;
		}

		list.setLen(len, i);

		list.OneTimeCopy(len);
	}
}