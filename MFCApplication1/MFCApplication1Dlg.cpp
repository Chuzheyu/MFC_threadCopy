
// MFCApplication1Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include "FolderOP.h"
#include "FolderList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD WINAPI ControlThreadFunc(LPVOID lpParam);
DWORD WINAPI StopThreadFunc(LPVOID lpParam);
DWORD WINAPI SaveThreadFunc(LPVOID lpParam);
DWORD WINAPI LoadThreadFunc(LPVOID lpParam);
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication1Dlg �Ի���



CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
	, m_srcFolder(_T(""))
	, m_destFolder(_T(""))
	, state(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SRC, m_srcFolder);
	DDV_MaxChars(pDX, m_srcFolder, 128);
	DDX_Text(pDX, IDC_DEST, m_destFolder);
	DDV_MaxChars(pDX, m_destFolder, 128);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BEGIN, &CMFCApplication1Dlg::OnBnClickedBegin)
	ON_BN_CLICKED(IDC_STOP, &CMFCApplication1Dlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_SAVE, &CMFCApplication1Dlg::OnBnClickedSave)
	ON_BN_CLICKED(IDC_LOAD, &CMFCApplication1Dlg::OnBnClickedLoad)
	ON_EN_CHANGE(IDC_SRC, &CMFCApplication1Dlg::OnEnChangeSrc)
	ON_EN_CHANGE(IDC_DEST, &CMFCApplication1Dlg::OnEnChangeDest)
END_MESSAGE_MAP()


// CMFCApplication1Dlg ��Ϣ�������

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD WINAPI ControlThreadFunc(LPVOID lpParam)
{
	AllPath* path = (AllPath*)lpParam;

	FolderList::BeginCopy(path->src, path->dest);

	delete path;
	return 1;
}

void CMFCApplication1Dlg::OnBnClickedBegin()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);

	m_srcFolder = _T("E:\\Tencent");
	m_destFolder = _T("E:\\desktop\\Tencent");
	UpdateData(FALSE);

	AllPath* path = new AllPath;
	path->src = m_srcFolder;
	path->dest = m_destFolder;

	DWORD dwThreadID;
	HANDLE contolThread = CreateThread(NULL, 0, ControlThreadFunc, LPVOID(path), 0, &dwThreadID);
	if (contolThread == NULL)
	{
		GetLastError();
		return;
	}

}

DWORD WINAPI StopThreadFunc(LPVOID lpParam)
{
	int state = int(lpParam);

	if (state == 0)
	{
		FolderList::StopCopy();

		return 2;
	}

	else
	{
		FolderList::RestartCopy();

		return 3;
	}

}

static int state = 0;
void CMFCApplication1Dlg::OnBnClickedStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DWORD dwThreadID;
	HANDLE stopThread = CreateThread(NULL, 0, StopThreadFunc, (LPVOID)state, 0, &dwThreadID);
	state = abs(1 - state);

}

DWORD WINAPI SaveThreadFunc(LPVOID lpParam)
{
	FolderList::SaveCopy();
	return 4;
}

void CMFCApplication1Dlg::OnBnClickedSave()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DWORD dwThreadID;
	HANDLE saveThread = CreateThread(NULL, 0, SaveThreadFunc, (LPVOID)state, 0, &dwThreadID);
}

DWORD WINAPI LoadThreadFunc(LPVOID lpParam)
{
	AllPath* path = (AllPath*)lpParam;

	FolderList::ContinueCopy(path->src, path->dest);

	delete path;
	return 5;
}

void CMFCApplication1Dlg::OnBnClickedLoad()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);

	m_srcFolder = _T("E:\\Tencent");
	m_destFolder = _T("E:\\desktop\\Tencent");
	UpdateData(FALSE);

	AllPath* path = new AllPath;
	path->src = m_srcFolder;
	path->dest = m_destFolder;

	DWORD dwThreadID;
	HANDLE loadThread = CreateThread(NULL, 0, LoadThreadFunc, LPVOID(path), 0, &dwThreadID);
	if (loadThread == NULL)
	{
		GetLastError();
		return;
	}
}


void CMFCApplication1Dlg::OnEnChangeSrc()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CMFCApplication1Dlg::OnEnChangeDest()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}

